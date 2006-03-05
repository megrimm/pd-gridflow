#!/usr/bin/env ruby
# $Id$

LTI=ENV["HOME"]+"/Documents/ltilib_1.9.12"
Headers=`find #{LTI}/src -iname '*.h'`
Headers.each {|head|
  head.chomp!
  f=File.open head
  text=""
  while line=f.gets
    line.gsub!(/\r?\n$/,"")
    line.gsub!(/\/\/.*$/,"")
    text << line << " "
  end
  f.close
  puts head+":"
  text.scan(/bool\s+apply\s*([^;{])*[;{]/) {|m| puts $& }
  gets
}
