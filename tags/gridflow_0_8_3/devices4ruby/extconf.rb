#!/usr/bin/env ruby
# $Id$
# installer for RubyX11 / MetaRuby / etc
# by Mathieu Bouchard

require "rbconfig"
require "ftools"
include Config

$DESTDIR = "#{CONFIG["sitedir"]}/#{CONFIG["MAJOR"]}.#{CONFIG["MINOR"]}"
#$DESTDIR = "/home/matju/lib/ruby/#{RUBY_VERSION[0,3]}"
$RUBY = "ruby"

while ARGV.length>0
  arg=ARGV.shift
  case arg
  when /=/
    i=arg.index '='
    ARGV.unshift arg[0..i-1], arg[i+1..-1]
  when "--prefix"
    $DESTDIR = ARGV.shift + "/lib/ruby/#{CONFIG["MAJOR"]}.#{CONFIG["MINOR"]}"
  end
end



def install_files(f,base,entries)
  entries.each {|type,name,*rest|
    case type
    when :ruby
      f.puts "\tinstall -m644 #{base+name} $(DESTDIR)/#{base+name}"
    when :directory
      f.puts "\t@mkdir $(DESTDIR)/#{base+name} || true"
      install_files(f,base+name,rest) 
    end
  }
end

def uninstall_files(f,base,entries)
  entries.each {|type,name,*rest|
    case type
    when :ruby
      f.puts "\trm $(DESTDIR)/#{base+name}"
    when :directory
      uninstall_files(f,base+name,rest) 
    end
  }
end

def make_makefile
  File.open("Makefile","w") {|f|
    f.puts "# Warning: this file is GENERATED by ./extconf.rb", ""
    f.puts "DESTDIR = #{$DESTDIR}", ""
    f.puts "RUBY = #{$RUBY}"
    f.puts "all::", ""
    f.puts "Makefile: extconf.rb"
    f.puts "\t$(RUBY) extconf.rb", ""

    f.puts "install::"
    f.puts "\t@mkdir -p $(DESTDIR)"
    install_files(f,"",FILES)
    f.puts
    f.puts "uninstall::"
    uninstall_files(f,"",FILES)
    f.puts
  }
  #FILES.each {|name|
  #  File.install "lib/#{name}", "#{DSTPATH}/#{name}", 0644, true   
  #end
end

#----------------------------------------------------------------#

$DESTDIR += "/linux/" #(HACK!)

FILES = [
#  [:directory, "linux/",
    [:ruby, "ioctl.rb"],
    [:ruby, "SoundPCM.rb"],
    [:ruby, "ParallelPort.rb"],
    [:ruby, "SoundMixer.rb"],
#  ]
]

make_makefile


__END__
### the following is discarded (just a test)

require "mkmf"

srcs = %w(
	termios
)

#have_library("m")
#have_func("sincos")
#have_func("asinh")

#if have_header("fftw.h")
#  if have_library("fftw", "fftwnd_create_plan")
#    srcs.push "na_fftw"
#  else
#    $defs.delete "-DHAVE_FFTW_H"
#  end
#end

$objs = srcs.map {|i| i+".o"}

#dir_config("linux")
create_makefile("linux")