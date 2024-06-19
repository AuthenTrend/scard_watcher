#ifndef FLUTTER_PLUGIN_SCARD_WATCHER_PLUGIN_H_
#define FLUTTER_PLUGIN_SCARD_WATCHER_PLUGIN_H_

#include "scard_watcher.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

namespace scard_watcher {

class ScardWatcherPlugin : public flutter::Plugin {
private:
    std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> mChannel;
  std::unique_ptr<ScardWatcher> mScardWatcher;
  flutter::PluginRegistrarWindows *mRegistrar;
  int mProcId;
  std::optional<LRESULT> MessageHandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  ScardWatcherPlugin();

  virtual ~ScardWatcherPlugin();

  // Disallow copy and assign.
  ScardWatcherPlugin(const ScardWatcherPlugin&) = delete;
  ScardWatcherPlugin& operator=(const ScardWatcherPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  void Init(std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel, flutter::PluginRegistrarWindows *registrar);
};

}  // namespace scard_watcher

#endif  // FLUTTER_PLUGIN_SCARD_WATCHER_PLUGIN_H_
