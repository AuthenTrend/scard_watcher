import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'scard_watcher_method_channel.dart';

abstract class ScardWatcherPlatform extends PlatformInterface {
  /// Constructs a ScardWatcherPlatform.
  ScardWatcherPlatform() : super(token: _token);

  static final Object _token = Object();

  static ScardWatcherPlatform _instance = MethodChannelScardWatcher();

  /// The default instance of [ScardWatcherPlatform] to use.
  ///
  /// Defaults to [MethodChannelScardWatcher].
  static ScardWatcherPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [ScardWatcherPlatform] when
  /// they register themselves.
  static set instance(ScardWatcherPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  void setMethodCallHandler(Future<dynamic> Function(MethodCall)? handler) async {
    throw UnimplementedError('setMethodCallHandler() has not been implemented.');
  }

  Future<List<ScardInfo>> getSmartCards() async {
    throw UnimplementedError('getSmartCards() has not been implemented.');
  }

  Future<ScardInfo?> getSmartCardDetails(String readerName) async {
    throw UnimplementedError('getSmartCardDetails(String readerName) has not been implemented.');
  }
}

base class ScardInfo {
  String readerName;
  ScardInfo({required this.readerName});
  static ScardInfo parse(MapEntry<String, dynamic> source) => switch (defaultTargetPlatform) {
    TargetPlatform.macOS => ScardInfoMacos(
        readerName: source.key,
        tokens: (source.value! as List).map<ScardTokenInfo>((e) => ScardTokenInfo(
          tokenId: e['tokenID'],
          slotName: e['slotName'],
          driverName: e['driverName'],
        ),
    ).toList()),
    _ => throw UnsupportedError('Platform ${defaultTargetPlatform.name} is not supported.'),
  };

  @override
  String toString() {
    return '{readerName: $readerName}';
  }
}

final class ScardInfoMacos extends ScardInfo {
  List<ScardTokenInfo> tokens;
  ScardInfoMacos({required super.readerName, required this.tokens});

  @override
  String toString() {
    var str = '{readerName: $readerName';
    for (final tokenInfo in tokens) {
      str += ', {';
      str += 'tokenId: ${tokenInfo.tokenId}';
      (tokenInfo.slotName != null) ? str += ', slotName: ${tokenInfo.slotName}' : null;
      (tokenInfo.driverName != null) ? str += ', driverName: ${tokenInfo.driverName}' : null;
      str += '}';
    }
    str += '}';
    return str;
  }
}

final class ScardTokenInfo {
  String tokenId;
  String? slotName;
  String? driverName;
  ScardTokenInfo({required this.tokenId, this.slotName, this.driverName});
}
