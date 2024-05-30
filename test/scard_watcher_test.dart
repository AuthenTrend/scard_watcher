import 'package:flutter/src/services/message_codec.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:scard_watcher/scard_watcher.dart';
import 'package:scard_watcher/scard_watcher_platform_interface.dart';
import 'package:scard_watcher/scard_watcher_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockScardWatcherPlatform
    with MockPlatformInterfaceMixin
    implements ScardWatcherPlatform {

  @override
  Future<List<ScardInfo>> getSmartCards() async {
    return [ScardInfo(readerName: 'test reader name')];
  }

  @override
  void setMethodCallHandler(Future Function(MethodCall)? handler) async {
    Future.delayed(const Duration(seconds: 1), () {
      handler?.call(const MethodCall('scardDidInsert', 'test reader name'));
      handler?.call(const MethodCall('scardDidRemove', 'test reader name'));
    });
    return;
  }

  @override
  Future<ScardInfo?> getSmartCardDetails(String readerName) async {
    return ScardInfo(readerName: readerName);
  }
}

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  final ScardWatcherPlatform initialPlatform = ScardWatcherPlatform.instance;

  test('$MethodChannelScardWatcher is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelScardWatcher>());
  });

  test('getSmartCards', () async {
    ScardWatcher scardWatcherPlugin = ScardWatcher();
    MockScardWatcherPlatform fakePlatform = MockScardWatcherPlatform();
    ScardWatcherPlatform.instance = fakePlatform;
    final scards = await scardWatcherPlugin.getSmartCards();
    expect(scards.length, 1);
    expect(scards.first.readerName, 'test reader name');
  });

  test('setMethodCallHandler', () async {
    ScardWatcher scardWatcherPlugin = ScardWatcher();
    MockScardWatcherPlatform fakePlatform = MockScardWatcherPlatform();
    fakePlatform.setMethodCallHandler(scardWatcherPlugin.methodCallHandler);
    ScardWatcherPlatform.instance = fakePlatform;
    final listener = TestListener();
    scardWatcherPlugin.addListener(listener);
    await Future.delayed(const Duration(seconds: 2));
    expect(listener.insertedScard, 'test reader name');
    expect(listener.removedScard, 'test reader name');
    scardWatcherPlugin.removeListener(listener);
  });

  test('getSmartCardDetails', () async {
    ScardWatcher scardWatcherPlugin = ScardWatcher();
    MockScardWatcherPlatform fakePlatform = MockScardWatcherPlatform();
    ScardWatcherPlatform.instance = fakePlatform;
    const readerName = 'test reader name';
    final details = await scardWatcherPlugin.getSmartCardDetails(readerName);
    expect(details?.readerName, readerName);
  });
}

class TestListener implements ScardWatcherListener {
  String? insertedScard;
  String? removedScard;

  @override
  Future<void> scardDidInsert(String readerName) async {
    insertedScard = readerName;
  }

  @override
  Future<void> scardDidRemove(String readerName) async {
    removedScard = readerName;
  }

}
