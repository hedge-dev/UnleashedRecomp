#ifdef FOUND_GIO
#include <gio/gio.h>
#endif
#include <os/media.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#ifdef FOUND_GIO
static std::vector<std::string> get_mpris_nams(GDBusConnection *connection,
                                               GError *error) {
  std::vector<std::string> names;

  GVariant *result = g_dbus_connection_call_sync(
      connection,
      "org.freedesktop.DBus",  // destination
      "/org/freedesktop/DBus", // object path
      "org.freedesktop.DBus",  // interface
      "ListNames",             // method
      NULL,                    // parameters
      G_VARIANT_TYPE("(as)"),  // expected return type
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

  if (!result) {
    g_printerr("Failed to list names: %s\n", error->message);
    g_error_free(error);
    return {};
  }

  // Extract an array of names
  GVariantIter *iter;
  const gchar *name;
  g_variant_get(result, "(as)", &iter);
  while (g_variant_iter_loop(iter, "s", &name)) {
    if (g_str_has_prefix(name, "org.mpris.MediaPlayer2.")) {
      names.push_back(name);
    }
  }

  g_variant_iter_free(iter);
  g_variant_unref(result);
  return names;
}
#endif

bool os::media::IsExternalMediaPlaying() {
    #ifdef FOUND_GIO
    GDBusConnection *connection;
    GError *error = NULL;

    // Connect to the session bus
    connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    if (!connection) {
      g_printerr("Failed to connect to session bus: %s\n", error->message);
      g_error_free(error);
      return false;
    }

    auto names = get_mpris_nams(connection, error);
    bool found = false;
    for (auto name : names) {
      GVariant *prop_result = g_dbus_connection_call_sync(
          connection, name.c_str(), "/org/mpris/MediaPlayer2",
          "org.freedesktop.DBus.Properties", "Get",
          g_variant_new("(ss)", "org.mpris.MediaPlayer2.Player",
                        "PlaybackStatus"),
          G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

      if (!prop_result) {
        g_clear_error(&error);
        continue;
      }

      GVariant *variant;
      g_variant_get(prop_result, "(v)", &variant);
      const gchar *playback_status = g_variant_get_string(variant, NULL);
      std::string status(playback_status);
      found = found or (status.find("Playing") != std::string::npos);

      g_variant_unref(variant);
      g_variant_unref(prop_result);
    }
    g_object_unref(connection);
    return found;
    #else
    return false;
    #endif
}
