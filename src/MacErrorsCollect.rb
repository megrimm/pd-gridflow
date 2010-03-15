# Copyright © 2010 Mathieu Bouchard

pattern = %r{(-\d+),\s+/\*\s*(.*)\s*\*/}

File.open("MacErrors.h") {|f|
  f.each {|line|
    if m = pattern.match(line)
      puts "erreur \# #{m[1]} est « #{m[2]} »"
    end
  }
}
