# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'zstd/version'

Gem::Specification.new do |spec|
  spec.name          = 'zstd'
  spec.version       = Zstd::VERSION
  spec.authors       = ['Uwe Kubosch']
  spec.email         = ['uwe@kubosch.no']

  spec.summary       = %q{Ruby wrapper for Facebook Zstandard}
  spec.description   = %q{A thin Ruby wrapper around the Facebook Zstandard library}
  spec.homepage      = 'https://github.com/jarredholman/ruby-zstd/tree/master/ext/zstd/libzstd'
  spec.license       = 'MIT'

  # Prevent pushing this gem to RubyGems.org. To allow pushes either set the 'allowed_push_host'
  # to allow pushing to a single host or delete this section to allow pushing to any host.
  if spec.respond_to?(:metadata)
    spec.metadata['allowed_push_host'] = "TODO: Set to 'http://mygemserver.com'"
  else
    raise "RubyGems 2.0 or newer is required to protect against " \
      "public gem pushes."
  end

  spec.files         = `git ls-files -z`.split("\x0").reject do |f|
    f.match(%r{^(test|spec|features)/})
  end
  spec.bindir        = "exe"
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ["lib"]
  spec.extensions    = ["ext/zstd/extconf.rb"]

  spec.add_development_dependency "bundler", "~> 1.12"
  spec.add_development_dependency "rake", "~> 10.0"
  spec.add_development_dependency "rake-compiler"
  spec.add_development_dependency "minitest", "~> 5.0"
end
