# Copyright © 2010 Mathieu Bouchard

pattern = %r{\s*(\w+)\s*=\s*(-\d+),\s+/\*\s*(.*)\s*\*/}

File.open("MacErrors.h") {|f|
  f.each {|line|
    if m = pattern.match(line)
      puts "erreur \# #{m[2]} est #{m[1]} : « #{m[3]} »"
    end
  }
}
