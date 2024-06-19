#include "scard_watcher_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>
#include <dbt.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <flutter/flutter_view_controller.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <optional>

namespace scard_watcher {

// static
void ScardWatcherPlugin::RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar) {
  auto channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(registrar->messenger(), "scard_watcher", &flutter::StandardMethodCodec::GetInstance());
  auto plugin = std::make_unique<ScardWatcherPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  plugin->Init(std::move(channel), registrar);

  registrar->AddPlugin(std::move(plugin));
}

ScardWatcherPlugin::ScardWatcherPlugin() {}

ScardWatcherPlugin::~ScardWatcherPlugin() {
  mRegistrar->UnregisterTopLevelWindowProcDelegate(mProcId);
}

void ScardWatcherPlugin::HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue> &method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("getSmartCards") == 0) {
    flutter::EncodableMap scardMap;
    auto smartCards = mScardWatcher->GetSmartCards();
    for (auto &scard : smartCards) {
      flutter::EncodableMap detailsMap;
      for (auto &details : scard.second) {
        detailsMap[flutter::EncodableValue(details.first)] = flutter::EncodableValue(details.second);
      }
      scardMap[flutter::EncodableValue(scard.first)] = flutter::EncodableValue(detailsMap);
    }
    result->Success(flutter::EncodableValue(scardMap));
  } else if (method_call.method_name().compare("getSmartCardDetails") == 0) {
    auto retValue = flutter::EncodableValue();
    const auto* readerName = std::get_if<std::string>(method_call.arguments());
    if (readerName != nullptr) {
      auto details = mScardWatcher->GetSmartCardDetails(*readerName);
      if (!details.empty()) {
        flutter::EncodableMap detailsMap;
        for (auto &detail : details) {
          detailsMap[flutter::EncodableValue(detail.first)] = flutter::EncodableValue(detail.second);
        }
        flutter::EncodableMap map{{flutter::EncodableValue(*readerName), flutter::EncodableValue(detailsMap)}};
        retValue = flutter::EncodableValue(map);
      }
    }
    result->Success(retValue);
  } else {
    result->NotImplemented();
  }
}

void ScardWatcherPlugin::Init(std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel, flutter::PluginRegistrarWindows *registrar) {
  mRegistrar = registrar;
  mProcId = mRegistrar->RegisterTopLevelWindowProcDelegate([&, this](HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> std::optional<LRESULT> {
    return this->MessageHandler(hwnd, message, wparam, lparam);
  });

  mChannel = std::move(channel);
  mScardWatcher = std::make_unique<ScardWatcher>();
}

std::optional<LRESULT> ScardWatcherPlugin::MessageHandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  static BOOL inited = false;
  if (!inited) {
    inited = true;
    mScardWatcher->DoRegisterDeviceInterfaceToHwnd(hwnd);
    mScardWatcher->UpdateReadersState();
  }
  if (message != WM_DEVICECHANGE) {
    return std::nullopt;
  }

  PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lparam;
  return mScardWatcher->DeviceChangeEventHandler(wparam, lpdb, [channel_ptr = mChannel.get()](std::string readerName) {
    channel_ptr->InvokeMethod("scardDidInsert", std::make_unique<flutter::EncodableValue>(readerName));
  }, [channel_ptr = mChannel.get()](std::string readerName) {
    channel_ptr->InvokeMethod("scardDidRemove", std::make_unique<flutter::EncodableValue>(readerName));
  });
}

}  // namespace scard_watcher
