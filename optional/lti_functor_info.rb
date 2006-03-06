#!/usr/bin/env ruby
# $Id$

LTI="."
Headers=`find #{LTI} -iname '*.cxx'`
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

}
