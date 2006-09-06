#!/usr/bin/env ruby

while gets
  $_.gsub! /int >>/, 'int> >'
  $_.gsub! /RSTRING\(obj\)->len/, "StringValueLen(obj)"
  if /^#define StringValueLen\(s\)/ =~ $_
    $_ = "#define StringValueLen(s) RSTRING_LEN(s)\n"
  end
  print
end
