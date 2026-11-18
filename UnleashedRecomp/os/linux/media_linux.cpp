#include <algorithm>
#include <atomic>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <ranges>
#include <gio/gio.h>
#include <os/media.h>
#include <os/logger.h>

enum class PlaybackStatus
{
    Stopped,
    Playing,
    Paused
};

static const char* DBusInterface = "org.freedesktop.DBus";
static const char* DBusPropertiesInterface = "org.freedesktop.DBus.Properties";
static const char* DBusPath = "/org/freedesktop/DBus";
static const char* MPRIS2Interface = "org.mpris.MediaPlayer2";
static const char* MPRIS2PlayerInterface = "org.mpris.MediaPlayer2.Player";
static const char* MPRIS2Path = "/org/mpris/MediaPlayer2";

static std::optional<std::thread> g_dbusThread;
static std::unordered_map<std::string, PlaybackStatus> g_playerStatus;
static std::atomic<bool> g_isPlaying = false;

static PlaybackStatus PlaybackStatusFromString(const char* str)
{
    if (g_str_equal(str, "Playing"))
        return PlaybackStatus::Playing;
    else if (g_str_equal(str, "Paused"))
        return PlaybackStatus::Paused;
    else
        return PlaybackStatus::Stopped;
}

static void UpdateActiveStatus()
{
    g_isPlaying = std::ranges::any_of(
            g_playerStatus | std::views::values,
            [](PlaybackStatus status) { return status == PlaybackStatus::Playing; }
    );
}

static void UpdateActivePlayers(const char* name, PlaybackStatus status)
{
    g_playerStatus.insert_or_assign(name, status);
    UpdateActiveStatus();
}

static PlaybackStatus MPRISGetPlaybackStatus(GDBusConnection* connection, const gchar* name)
{
    GError* error;
    GVariant* response;
    GVariant* tupleChild;
    GVariant* value;
    PlaybackStatus status;

    error = NULL;

    response = g_dbus_connection_call_sync(
        connection,
        name,
        MPRIS2Path,
        DBusPropertiesInterface,
        "Get",
        g_variant_new("(ss)", MPRIS2PlayerInterface, "PlaybackStatus"),
        G_VARIANT_TYPE("(v)"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error
    );

    if (!response)
    {
        LOGF_ERROR("Failed to process D-Bus Get: {}", error->message);
        g_clear_error(&error);
        return PlaybackStatus::Stopped;
    }

    tupleChild = g_variant_get_child_value(response, 0);
    value = g_variant_get_variant(tupleChild);

    if (!g_variant_is_of_type(value, G_VARIANT_TYPE_STRING))
    {
        LOG_ERROR("Failed to process D-Bus Get");
        g_variant_unref(tupleChild);
        return PlaybackStatus::Stopped;
    }

    status = PlaybackStatusFromString(g_variant_get_string(value, NULL));

    g_variant_unref(value);
    g_variant_unref(tupleChild);
    g_variant_unref(response);

    return status;
}

// Something is very wrong with the system if this happens
static void DBusConnectionClosed(GDBusConnection* connection,
                                 gboolean remotePeerVanished,
                                 GError* error,
                                 gpointer userData)
{
    LOG_ERROR("D-Bus connection closed");
    g_isPlaying = false;
    g_main_loop_quit((GMainLoop*)userData);
}

static void DBusNameOwnerChanged(GDBusConnection* connection,
                                 const gchar* senderName,
                                 const gchar* objectPath,
                                 const gchar* interfaceName,
                                 const gchar* signalName,
                                 GVariant* parameters,
                                 gpointer userData)
{
    const char* name;
    const char* oldOwner;
    const char* newOwner;

    g_variant_get(parameters, "(&s&s&s)", &name, &oldOwner, &newOwner);

    if (g_str_has_prefix(name, MPRIS2Interface))
    {
        if (oldOwner[0])
        {
            g_playerStatus.erase(oldOwner);
        }

        UpdateActiveStatus();
    }
}

static void MPRISPropertiesChanged(GDBusConnection* connection,
                                   const gchar* senderName,
                                   const gchar* objectPath,
                                   const gchar* interfaceName,
                                   const gchar* signalName,
                                   GVariant* parameters,
                                   gpointer userData)
{
    const char* interface;
    GVariant* changed;
    GVariantIter iter;
    const char* key;
    GVariant* value;
    PlaybackStatus playbackStatus;

    g_variant_get_child(parameters, 0, "&s", &interface);
    g_variant_get_child(parameters, 1, "@a{sv}", &changed);

    g_variant_iter_init(&iter, changed);
    while (g_variant_iter_next(&iter, "{&sv}", &key, &value))
    {
        if (g_str_equal(key, "PlaybackStatus"))
        {
            playbackStatus = PlaybackStatusFromString(g_variant_get_string(value, NULL));
            UpdateActivePlayers(senderName, playbackStatus);
            g_variant_unref(value);
            break;
        }
        g_variant_unref(value);
    }

    g_variant_unref(changed);
}

/* Called upon CONNECT to discover already active MPRIS2 players by looking for
   well-known bus names that begin with the MPRIS2 path.
   g_playerStatus stores unique connection names,
   not their well-known ones, as the PropertiesChanged signal only provides the
   former. */
static void DBusListNamesReceived(GObject* object, GAsyncResult* res, gpointer userData)
{
    GDBusConnection* connection;
    GError* error;
    GVariant* response;
    GVariant* tupleChild;
    GVariantIter iter;
    const gchar* name;

    connection = G_DBUS_CONNECTION(object);
    error = NULL;
    response = g_dbus_connection_call_finish(connection, res, &error);

    if (!response)
    {
        LOGF_ERROR("Failed to process D-Bus ListNames: {}", error->message);
        g_clear_error(&error);
        return;
    }

    tupleChild = g_variant_get_child_value(response, 0);

    g_variant_iter_init(&iter, tupleChild);
    while (g_variant_iter_next(&iter, "&s", &name))
    {
        GVariant* ownerResponse;
        const gchar* ownerName;
        PlaybackStatus status;

        if (!g_str_has_prefix(name, MPRIS2Interface))
            continue;

        ownerResponse = g_dbus_connection_call_sync(
            connection,
            DBusInterface,
            DBusPath,
            DBusInterface,
            "GetNameOwner",
            g_variant_new("(s)", name),
            G_VARIANT_TYPE("(s)"),
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            &error
        );

        if (!ownerResponse)
        {
            LOGF_ERROR("Failed to process D-Bus GetNameOwner: {}", error->message);
            g_clear_error(&error);
            g_variant_unref(tupleChild);
            g_variant_unref(response);
            return;
        }

        g_variant_get(ownerResponse, "(&s)", &ownerName);
        status = MPRISGetPlaybackStatus(connection, ownerName);

        g_playerStatus.insert_or_assign(ownerName, status);
        g_variant_unref(ownerResponse);
    }

    UpdateActiveStatus();

    g_variant_unref(tupleChild);
    g_variant_unref(response);
}

static void DBusThreadProc()
{
    GMainContext* mainContext;
    GMainLoop* mainLoop;
    GError* error;
    GDBusConnection* connection;

    mainContext = g_main_context_new();
    g_main_context_push_thread_default(mainContext);
    mainLoop = g_main_loop_new(mainContext, FALSE);
    error = NULL;

    connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    if (!connection)
    {
        LOGF_ERROR("Failed to connect to D-Bus: {}", error->message);
        g_clear_error(&error);
        g_main_context_unref(mainContext);
        g_main_loop_unref(mainLoop);
        return;
    }

    g_dbus_connection_set_exit_on_close(connection, FALSE);
    g_signal_connect(connection, "closed", G_CALLBACK(DBusConnectionClosed), mainLoop);

    // Listen for player connection changes
    g_dbus_connection_signal_subscribe(
        connection,
        DBusInterface,
        DBusInterface,
        "NameOwnerChanged",
        DBusPath,
        NULL,
        G_DBUS_SIGNAL_FLAGS_NONE,
        DBusNameOwnerChanged,
        NULL,
        NULL
    );

    // Listen for player status changes
    g_dbus_connection_signal_subscribe(
        connection,
        NULL,
        DBusPropertiesInterface,
        "PropertiesChanged",
        MPRIS2Path,
        NULL,
        G_DBUS_SIGNAL_FLAGS_NONE,
        MPRISPropertiesChanged,
        NULL,
        NULL
    );

    // Request list of current players
    g_dbus_connection_call(
        connection,
        DBusInterface,
        DBusPath,
        DBusInterface,
        "ListNames",
        NULL,
        G_VARIANT_TYPE("(as)"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        DBusListNamesReceived,
        NULL
    );

    g_main_loop_run(mainLoop);
}

bool os::media::IsExternalMediaPlaying()
{
    if (!g_dbusThread)
    {
        g_dbusThread.emplace(DBusThreadProc);
        g_dbusThread->detach();
    }

    return g_isPlaying;
}
