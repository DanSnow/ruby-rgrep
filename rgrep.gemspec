# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
ext = File.expand_path('../ext', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
$LOAD_PATH.unshift(ext) unless $LOAD_PATH.include?(ext)
require 'rgrep/version'

Gem::Specification.new do |spec|
  spec.name          = 'rgrep'
  spec.version       = Rgrep::VERSION
  spec.authors       = ['DanSnow']
  spec.email         = ['dododavid006@gmail.com']
  spec.extensions    = ['ext/rgrep/extconf.rb']

  spec.summary       = 'rgrep'
  spec.description   = 'rgrep'
  spec.homepage      = 'http://example.com'

  spec.files         = `git ls-files -z`.split("\x0").reject { |f| f.match(%r{^(test|spec|features)/}) }
  spec.bindir        = 'exe'
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ['lib']

  spec.add_dependency 'rice'
  spec.add_development_dependency 'bundler', '~> 1.11'
  spec.add_development_dependency 'rake', '~> 10.0'
  spec.add_development_dependency 'rspec', '~> 3.0'
end
