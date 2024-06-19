#include "include/scard_watcher/scard_watcher_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "scard_watcher_plugin.h"

void ScardWatcherPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  scard_watcher::ScardWatcherPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
