#include <os/media.h>
#include <os/logger.h>
#include <dbus/dbus.h>

#include "app.h"

#define MEDIA_CHECK_RATE 1

static float g_mediaCheckTimer;
static bool g_mediaLastResult;

static DBusConnection* CreateDBusConnection()
{
    DBusError dbusError;
    dbus_error_init(&dbusError);

    DBusConnection* result = dbus_bus_get(DBUS_BUS_SESSION, &dbusError);
    if (dbus_error_is_set(&dbusError))
    {
        LOGF_ERROR("Failed to create DBus connection: {0}", dbusError.name);
        return nullptr;
    }

    return result;
}

static void DestroyDBusConnection(DBusConnection* connection)
{
    assert(connection != nullptr);
	dbus_connection_unref(connection);
}

static std::vector<std::string> GetMediaPlayerBusNames(DBusConnection* connection)
{
    assert(connection != nullptr);

    auto result = std::vector<std::string>();

    DBusMessageIter rootIterator;
    DBusMessageIter arrayIterator;
	DBusPendingCall* pendingReturn;

    DBusMessage *message = dbus_message_new_method_call("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames");
    if (!message)
    {
        LOG_ERROR("Failed to create D-Bus Message!");
        return result;
    }

    dbus_message_iter_init_append(message, &rootIterator);
    if (!dbus_connection_send_with_reply(connection, message, &pendingReturn, 40))
    {
        LOG_ERROR("Failed to create D-Bus Message!");
        return result;
    }

    if (!pendingReturn)
    {
        LOG_ERROR("D-Bus Pending call is null!");
        return result;
    }

    dbus_connection_flush(connection);
    dbus_message_unref(message);
    dbus_pending_call_block(pendingReturn);
    message = dbus_pending_call_steal_reply(pendingReturn);
    if (!message)
    {
        LOG_ERROR("Failed to get D-Bus reply!");
        return result;
    }

	dbus_pending_call_unref(pendingReturn);

    if (!dbus_message_iter_init(message, &rootIterator))
    {
        LOG_ERROR("D-Bus message has no arguments!");
        return result;
    }

    if (dbus_message_iter_get_arg_type(&rootIterator) != DBUS_TYPE_ARRAY)
    {
        LOG_ERROR("D-Bus message returned invalid type!");
        return result;
    }

    dbus_message_iter_recurse(&rootIterator, &arrayIterator);
    do
    {
        const char *rawName;
        dbus_message_iter_get_basic(&arrayIterator, &rawName);
        const std::string name = rawName;
        if (name.starts_with("org.mpris.MediaPlayer2."))
            result.emplace_back(name);
    } while (dbus_message_iter_next(&arrayIterator));

    return result;
}

static bool IsMediaPlayerPlaying(DBusConnection* connection, const std::string& busName)
{
    assert(connection != nullptr);

    DBusMessageIter rootIterator;
    DBusMessageIter arrayIterator;
    DBusPendingCall* pendingReturn;

    DBusMessage* message = dbus_message_new_method_call(busName.c_str(), "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    if (!message)
    {
        LOG_ERROR("Failed to create D-Bus Message!");
        return false;
    }

    auto interfaceName = "org.mpris.MediaPlayer2.Player";
    auto propertyName = "PlaybackStatus";

    dbus_message_iter_init_append(message, &rootIterator);
    if (!dbus_message_iter_append_basic(&rootIterator, DBUS_TYPE_STRING, &interfaceName))
    {
        LOG_ERROR("Failed to append interface name to D-Bus Message!");
        return false;
    }

    if (!dbus_message_iter_append_basic(&rootIterator, DBUS_TYPE_STRING, &propertyName))
    {
        LOG_ERROR("Failed to append property name to D-Bus Message!");
        return false;
    }

    if (!dbus_connection_send_with_reply(connection, message, &pendingReturn, 40))
    {
        LOG_ERROR("Failed to create D-Bus Message!");
        return false;
    }

    if (!pendingReturn)
    {
        LOG_ERROR("D-Bus Pending call is null!");
        return false;
    }

    dbus_connection_flush(connection);
    dbus_message_unref(message);
    dbus_pending_call_block(pendingReturn);
    message = dbus_pending_call_steal_reply(pendingReturn);
    if (!message)
    {
        LOG_ERROR("Failed to get D-Bus reply!");
        return false;
    }

    dbus_pending_call_unref(pendingReturn);

    if (!dbus_message_iter_init(message, &rootIterator))
    {
        LOG_ERROR("D-Bus message has no arguments!");
        return false;
    }

    if (dbus_message_iter_get_arg_type(&rootIterator) == DBUS_TYPE_VARIANT)
        dbus_message_iter_recurse(&rootIterator, &arrayIterator);
    else
        arrayIterator = rootIterator;

    if (dbus_message_iter_get_arg_type(&arrayIterator) != DBUS_TYPE_STRING)
    {
        LOG_ERROR("D-Bus message returned invalid type!");
        return false;
    }

    const char *rawStatus;
    dbus_message_iter_get_basic(&arrayIterator, &rawStatus);
    const std::string status = rawStatus;
    return status == "Playing";
}

bool os::media::IsExternalMediaPlaying()
{
    //Calling D-Bus functions too much seems cause D-Bus to stop working, so perhaps it should be run less often.
    g_mediaCheckTimer -= App::s_deltaTime;
    if (g_mediaCheckTimer > 0)
        return g_mediaLastResult;

    g_mediaCheckTimer = MEDIA_CHECK_RATE;

    const auto dbusConnection = CreateDBusConnection();
    if (!dbusConnection)
        return false;

    bool result = false;

    std::vector<std::string> busNames = GetMediaPlayerBusNames(dbusConnection);
    for (const auto& bus : busNames)
    {
        if (IsMediaPlayerPlaying(dbusConnection, bus))
        {
            result = true;
            break;
        }
    }

    DestroyDBusConnection(dbusConnection);
    g_mediaLastResult = result;
    return result;
}