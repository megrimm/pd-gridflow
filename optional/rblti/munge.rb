#!/usr/bin/env ruby

while gets
  $_.gsub! /int >>/, 'int> >'
  $_.gsub! /RSTRING\((\w+)\)->len/, "StringValueLen(\\1)"
  $_.gsub! /RSTRING\((\w+)\)->ptr/, "StringValuePtr(\\1)"
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
