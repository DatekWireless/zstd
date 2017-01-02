# encoding: ASCII-8BIT

require 'test_helper'

class ZstdTest < Minitest::Test
  FOO                       = 'foo'
  COMPRESSED_FOO            = "(\xB5/\xFD \x03\x19\x00\x00foo"
  DICTIONARY_COMPRESSED_FOO = "(\xB5/\xFD#\x0E%5\x1A\x03\x19\x00\x00foo"
  
  def test_that_it_has_a_version_number
    refute_nil ::Zstd::VERSION
  end

  def test_initialize
    assert Zstd.new
  end

  def test_initialize_with_dictionary
    assert Zstd.new File.expand_path('fixtures/files/dictionary', __dir__)
  end

  def test_compress
    zstd = Zstd.new
    assert_equal COMPRESSED_FOO, zstd.compress(FOO)
  end

  def test_compress_with_dictionary
    zstd = Zstd.new File.expand_path('fixtures/files/dictionary', __dir__)
    assert_equal DICTIONARY_COMPRESSED_FOO, zstd.compress(FOO)
  end

  def test_decompress
    zstd = Zstd.new
    assert_equal FOO, zstd.decompress(COMPRESSED_FOO)
  end

  def test_decompress_with_dictionary
    zstd = Zstd.new File.expand_path('fixtures/files/dictionary', __dir__)
    assert_equal FOO, zstd.decompress(DICTIONARY_COMPRESSED_FOO)
  end

  def test_decompress_with_wrong_dictionary
    zstd = Zstd.new File.expand_path('fixtures/files/dictionary', __dir__)
    assert_equal DICTIONARY_COMPRESSED_FOO, zstd.compress(FOO)
    assert_equal FOO, zstd.decompress(DICTIONARY_COMPRESSED_FOO)
    
    zstd2 = Zstd.new
    exception = assert_raises { zstd2.decompress(DICTIONARY_COMPRESSED_FOO) }
    assert_equal "error decompressing: Dictionary mismatch", exception.message
  end

  def test_initialize_with_non_existing_dictionary
    exception = assert_raises { Zstd.new('non_existing_file') }
    assert_equal "file not found: non_existing_file", exception.message
  end

  def test_memory_leakage
    GC.start
    startv = GC.stat[:heap_live_slots]

    zstd = nil
    1000.times do
      zstd = Zstd.new File.expand_path('fixtures/files/dictionary', __dir__)
      zstd.compress(FOO)
      zstd.decompress(DICTIONARY_COMPRESSED_FOO)
    end

    zstd = nil
    GC.start
    endv = GC.stat[:heap_live_slots]
    assert endv <= (startv + 4) # not always exact match, allow 4 byte difference
  end
end
