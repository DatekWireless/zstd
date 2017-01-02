# Zstd

**Zstd**, short for Zstandard, is a new lossless compression algorithm, which
provides both good compression ratio _and_ speed for your standard compression
needs.

**Zstd** is developed by Yann Collet and the source is available at:
https://github.com/facebook/zstd

This gem wraps the **Zstd** library and provides bindings for ruby.

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'zstd'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install zstd

## Usage

```ruby
require 'zstd'

# Compress example
zstd = Zstd.new
compressed_buffer = zstd.compress 'foo'

# Decompress example
decompressed_buffer = zstd.decompress compressed_buffer

# Dictionary example
zstd = Zstd.new '/tmp/dictionary'
zstd.compress 'foo'
zstd.decompress "(\xB5/\xFD#\x0E%5\x1A\x03\x19\x00\x00foo"
```

Dictionaries must be created using **Zstd** binary.

    zstd --train FullPathToTrainingSet/* -o dictionaryName
    
**Zstd** can be obtained from: http://facebook.github.io/zstd/

Streaming is not supported, but may be added in the future.

Version numbers follow **Zstd** version numbers, with gem release version appended if needed (ie. 1.2.1.9 is **Zstd** version 1.2.1, gem release 9 within 1.2.1).

## Contributing
Feel free to contribute, standard fork and pull request is preferable.

To install this gem onto your local machine, run `bundle exec rake install`. To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and tags, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## License

The gem is available as open source under the terms of the [MIT License](http://opensource.org/licenses/MIT).
The native Zstd library is licensed under 3-clause BSD license with a patent grant. See the LICENSE and ext/zstd/libzstd/LICENSE for full copythight and conditions.
