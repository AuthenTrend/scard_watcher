import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'scard_watcher_platform_interface.dart';

/// An implementation of [ScardWatcherPlatform] that uses method channels.
class MethodChannelScardWatcher extends ScardWatcherPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('scard_watcher');

  @override
  void setMethodCallHandler(Future<dynamic> Function(MethodCall)? handler) {
    methodChannel.setMethodCallHandler(handler);
  }

  @override
  Future<List<ScardInfo>> getSmartCards() async {
    final scards = await methodChannel.invokeMethod<Map>('getSmartCards');
    return scards?.entries.map((e) => ScardInfo.parse(MapEntry<String, dynamic>(e.key, e.value))).toList() ?? [];
  }

  @override
  Future<ScardInfo?> getSmartCardDetails(String readerName) async {
    final details = await methodChannel.invokeMethod<Map?>('getSmartCardDetails', readerName);
    return (details == null) ? null : details.entries.map((e) => ScardInfo.parse(MapEntry<String, dynamic>(e.key, e.value))).firstOrNull;
  }
}
