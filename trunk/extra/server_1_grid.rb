# $Id$

require "socket"
require "smpte" # in this folder

picture = "\x7fGRID \000\003"
picture << [240,320,3].pack("N*")
make_smpte(picture) {|*rgb| rgb.pack "N*" }

# File.open("blah.grid","w") {|f| f.write picture }

serv = TCPServer.new 4242
loop {
  puts "waiting for connection (port 4242)"
  sock = serv.accept
  puts "incoming connection"
  begin
    loop {
      sock.write picture
      puts "wrote one picture"
    }
  rescue Errno::EPIPE # Broken Pipe
    puts "connection closed (by client)"
    # it's ok, go back to waiting.
  end
}
