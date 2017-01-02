require "mkmf"

$CFLAGS = '-I. -O3 -std=c99 -Wall -pedantic'

CDIRS = ['common', 'compress', 'decompress', 'dictBuilder']

Dir.chdir File.expand_path('..', __FILE__) do
  $srcs = Dir['*.c']
  
  CDIRS.each do |d|
    $VPATH << "$(srcdir)/libzstd/#{d}"
    $INCFLAGS << " -I$(srcdir)/libzstd/#{d}"
    $srcs += Dir["libzstd/#{d}/*.c"]
  end
end

$INCFLAGS << " -I$(srcdir) -I$(srcdir)/libzstd"
$VPATH << "$(srcdir)"

create_makefile("zstd/zstd")
