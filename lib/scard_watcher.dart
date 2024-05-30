
import 'package:flutter/services.dart';

import 'scard_watcher_platform_interface.dart';

abstract interface class ScardWatcherListener {
  Future<void> scardDidInsert(String readerName);
  Future<void> scardDidRemove(String readerName);
}

class ScardWatcher {

  static ScardWatcher? _instance;
  static ScardWatcher get instance {
    _instance ??= ScardWatcher._internal();
    return _instance!;
  }

  final Set<ScardWatcherListener> _listeners = {};

  ScardWatcher._internal() {
    ScardWatcherPlatform.instance.setMethodCallHandler(methodCallHandler);
  }

  factory ScardWatcher() {
    _instance ??= ScardWatcher._internal();
    return _instance!;
  }

  Future<List<ScardInfo>> getSmartCards() async {
    return ScardWatcherPlatform.instance.getSmartCards();
  }

  Future<ScardInfo?> getSmartCardDetails(String readerName) async {
    return ScardWatcherPlatform.instance.getSmartCardDetails(readerName);
  }

  void addListener(ScardWatcherListener listener) => _listeners.add(listener);

  void removeListener(ScardWatcherListener listener) => _listeners.remove(listener);

  Future<dynamic> methodCallHandler(MethodCall methodCall) async {
    switch (methodCall.method) {
      case 'scardDidInsert':
        final readerName = methodCall.arguments as String;
        for (final listener in _listeners.toList()) {
          listener.scardDidInsert(readerName);
        }
      case 'scardDidRemove':
        final readerName = methodCall.arguments as String;
        for (final listener in _listeners.toList()) {
          listener.scardDidRemove(readerName);
        }
      default:
        throw UnsupportedError('Method ${methodCall.method} is not supported.');
    }
    return null;
  }

}
