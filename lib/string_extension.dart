
import 'package:flutter/foundation.dart';

extension StringExt on String {
  void log() => debugPrint(this);
  void debugLog() => kDebugMode ? debugPrint(this) : null;
}