import Cocoa
import FlutterMacOS

public class ScardWatcherPlugin: NSObject, FlutterPlugin, ScardWatcherDelegate {
  let channel: FlutterMethodChannel

  init(channel: FlutterMethodChannel) {
    self.channel = channel
    super.init()
  }

  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: "scard_watcher", binaryMessenger: registrar.messenger)
    let instance = ScardWatcherPlugin(channel: channel)
    registrar.addMethodCallDelegate(instance, channel: channel)
    ScardWatcher.shared.delegate = instance
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case "getSmartCards":
      result(ScardWatcher.shared.smartCards)
    case "getSmartCardDetails":
      guard let readerName = call.arguments as? String else {
        result(nil)
        break
      }
      result([readerName : ScardWatcher.shared.getTokenInfo(readerName: readerName)])
    default:
      result(FlutterMethodNotImplemented)
    }
  }

  func scardDidInsert(readerName: String) {
    self.channel.invokeMethod("scardDidInsert", arguments: readerName)
  }

  func scardDidRemove(readerName: String) {
    self.channel.invokeMethod("scardDidRemove", arguments: readerName)
  }
}
