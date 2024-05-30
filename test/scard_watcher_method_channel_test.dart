import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:scard_watcher/scard_watcher_method_channel.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  MethodChannelScardWatcher platform = MethodChannelScardWatcher();
  const MethodChannel channel = MethodChannel('scard_watcher');

  setUp(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger.setMockMethodCallHandler(
      channel,
      (MethodCall methodCall) async {
        switch (methodCall.method) {
          case 'getSmartCards':
            Map scards = {'test reader name': [{'tokenID': 'test token id'}]};
            return scards;
          case 'getSmartCardDetails':
            Map details = {'test reader name': [{'tokenID': 'test token id'}]};
            return details;
          default:
            throw UnsupportedError('Method ${methodCall.method} is not supported');
        }
      },
    );
  });

  tearDown(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger.setMockMethodCallHandler(channel, null);
  });

  test('getSmartCards', () async {
    final scards = await platform.getSmartCards();
    expect(scards.first.readerName , 'test reader name');
  });

  test('getSmartCardDetails', () async {
    const readerName = 'test reader name';
    final details = await platform.getSmartCardDetails(readerName);
    expect(details?.readerName , readerName);
  });
}
