#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint scard_watcher.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'scard_watcher'
  s.version          = '0.0.1'
  s.summary          = 'Smart card watcher.'
  s.description      = <<-DESC
Smart card watcher.
                       DESC
  s.homepage         = 'http://example.com'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Your Company' => 'email@example.com' }

  s.source           = { :path => '.' }
  s.source_files     = 'Classes/**/*'
  s.dependency 'FlutterMacOS'

  s.platform = :osx, '12.0'
  s.pod_target_xcconfig = { 'DEFINES_MODULE' => 'YES' }
  s.swift_version = '5.0'
end
