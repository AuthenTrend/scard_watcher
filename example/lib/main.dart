import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'dart:async';

import 'package:scard_watcher/scard_watcher.dart';
import 'package:scard_watcher/scard_watcher_platform_interface.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> implements ScardWatcherListener {
  final _scardWatcherPlugin = ScardWatcher();
  List<ScardInfo> _scards = [];

  @override
  void initState() {
    super.initState();
    _scardWatcherPlugin.addListener(this);
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: ListView.builder(
          itemCount: _scards.length,
          itemBuilder: (context, index) {
            final scard = _scards[index];
            return ListTile(
              title: Text(scard.toString()),
            );
          },
        ),
        floatingActionButton: FloatingActionButton(
          onPressed: refresh,
          child: const Icon(Icons.refresh),
        ),
      ),
    );
  }

  @override
  Future<void> scardDidInsert(String readerName) async {
    print('scardDidInsert: $readerName');
    final details = await _scardWatcherPlugin.getSmartCardDetails(readerName);
    print('Details: ${details.toString()}');
    refresh();
  }

  @override
  Future<void> scardDidRemove(String readerName) async {
    print('scardDidRemove: $readerName');
    refresh();
  }

  void refresh() {
    _scardWatcherPlugin.getSmartCards().then((value) {
      if (!mounted) return;
      setState(() {
        _scards = value;
      });
    });
  }
}
