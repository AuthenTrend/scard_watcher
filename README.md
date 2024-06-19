# scard_watcher

A Flutter plugin for watching smart cards insertion and removal.

| Android | iOS | MacOS | Web | Linux | Windows |
| :-----: | :-: | :---: | :-: | :---: | :-----: |
|    X    |  X  |   V   |  X  |   X   |    V    |

## Usage

To use this plugin, add `scard_watcher` as a [dependency in your pubspec.yaml file](https://flutter.dev/platform-plugins/).

### Example

``` dart
class _MyAppState extends State<MyApp> implements ScardWatcherListener {
  final _scardWatcherPlugin = ScardWatcher();

  @override
  void initState() {
    super.initState();
    _scardWatcherPlugin.addListener(this);
  }

  @override
  Future<void> scardDidInsert(String readerName) async {
    print('scardDidInsert: $readerName');
    final details = await _scardWatcherPlugin.getSmartCardDetails(readerName);
    print('Details: ${details.toString()}');
  }

  @override
  Future<void> scardDidRemove(String readerName) async {
    print('scardDidRemove: $readerName');
  }
}
```

See the example app for more complex examples.
