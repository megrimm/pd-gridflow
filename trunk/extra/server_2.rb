# a server program to connect 2 or more clients together.
# by Mathieu Bouchard

# this script may not work for more than 2 clients,
# due to bugs in Ruby's non-blocking mode.

require "socket"

class IO
  def nonblock=flag
    bit = Fcntl::O_NONBLOCK
    fcntl(Fcntl::F_SETFL, (fcntl(Fcntl::F_GETFL) & ~bit) |
      if flag then bit else 0 end)
  end
end

serv = TCPServer.new 4242
socks = [serv]

loop {
  puts "waiting for connection (port 4242)"
  begin
    loop {
      puts "waiting"
      ready,blah,crap = IO.select socks, [], socks, 1
      (ready||[]).each {|s|
	if s==serv then
          sock = serv.accept
          sock.nonblock=true
          socks << sock
          puts "incoming connection (total: #{socks.length-1})"
	else
          other = socks - [s]
          stuff = s.read 1
          (s.close;socks.delete s) if not stuff
          other.each {|x| x.write stuff }
        end
      }
    }
  rescue Errno::EPIPE # Broken Pipe
    puts "connection closed (by client)"
    # it's ok, go back to waiting.
  end
}
