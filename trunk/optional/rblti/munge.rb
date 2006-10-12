#!/usr/bin/env ruby

while gets
  $_.gsub! /int >>/, 'int> >'
  $_.gsub! /RSTRING\(([^\)]+)\)->len/, "StringValueLen(\\1)"
  $_.gsub! /RSTRING\(([^\)]+)\)->ptr/, "StringValuePtr(\\1)"
  $_.gsub!  /RARRAY\(([^\)]+)\)->len/, "RARRAY_LEN(\\1)"
  $_.gsub!  /RARRAY\(([^\)]+)\)->ptr/, "RARRAY_PTR(\\1)"
  $_.gsub!  /lti::std::/, "std::"
  $_.gsub!  /lti::vector<lti::_fastLine/, "std::vector<lti::_fastLine" 

  if /^#include <ruby.h>/ =~ $_
    $_ << %{
#ifndef RARRAY_LEN
#define RARRAY_LEN(a) RARRAY(a)->len
#define RARRAY_PTR(a) RARRAY(a)->ptr
#endif
}
  end

  if /^#define StringValueLen\(s\)/ =~ $_
    $_ = %{
#ifdef RSTRING_LEN
#define StringValueLen(s) RSTRING_LEN(s)
#else
#define StringValueLen(s) RSTRING(s)->len
#endif
}
  end
  if /^#define StringValuePtr\(s\)/ =~ $_
    $_ = %{
#ifdef RSTRING_PTR
#define StringValuePtr(s) RSTRING_PTR(s)
#else
#define StringValuePtr(s) RSTRING(s)->ptr
#endif
}
  end
  print
end
