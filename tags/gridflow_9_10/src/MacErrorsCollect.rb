# Copyright © 2010 Mathieu Bouchard

pattern = %r{\s*(\w+)\s*=\s*(-\d+),(?:\s+/\*\s*(.*)\s*\*/)?}

f = File.open("MacErrors.h")
g = File.open("MacErrors2.i","w")
g.puts "// automatically made from a headerfile copyrighted by Apple."

table = {}

f.each {|line|
  if m = pattern.match(line)
    if table[m[2]] then
      puts "#{m[2]} est en double (#{table[m[2]].inspect})"
    else
      g.puts "OSERR(#{m[2]},\"#{m[1]}\",#{(m[3]||'').inspect})"
      table[m[2]] = [m[1],m[3]]
    end
  end
}

