# $Id$
# this is a telnet-accessible line-oriented server for accessing
# the ruby part of a running GridFlow.
# it must not use threads because Ruby-threads don't work with jMax.

require "socket"
require "fcntl"

class IO
  def nonblock= flag
    bit = Fcntl::O_NONBLOCK
    state = fcntl(Fcntl::F_GETFL, 0)
    fcntl(Fcntl::F_SETFL, (state & ~bit) |
      (if flag; bit else 0 end))
  end
end

# this will accept and manage connections
class EvalServerManager
  def initialize
    @conns = {}
    for @port in 6400..6409
      begin
        @sock = TCPServer.new(@port)
      rescue StandardError => e; p e
      else break
      end
    end
    raise "can't connect on any port in 6400..6409" if not @sock
    STDERR.puts "eval-server launched on port #{@port}"
    @sock.nonblock = true
  end
  def tick
    begin
      @conns[EvalServer.new(@sock.accept)]=1
      STDERR.puts "NEW CONNECTION"
    rescue Errno::EWOULDBLOCK
    end
    for s in @conns.keys do
      begin s.tick
      rescue Errno::EWOULDBLOCK
      rescue EOFError
        @conns.delete(s)
      rescue Errno::E000
        @conns.delete(s)
        STDERR.puts "Error: Success (this is a bug in Ruby)"
      end
    end
  end
end

# this handles one server-side regular socket
class EvalServer
  def initialize sock
    @sock = sock
    @sock.nonblock = true
  end
  def tick
    @sock.print "\e[0;36m"
    @sock.flush
    line = @sock.readline
    begin
      @sock.puts "\e[0;1;32m#{eval(line).inspect}"
    rescue Exception => e
      @sock.print "\e[0;1;33m"+["#{e.class}: #{e}",*e.backtrace].join("\n")
    end
  end
end

def protect
  yield
rescue Exception => e
  STDERR.puts "#{e.class}: #{e}"
  STDERR.puts e.backtrace
end

if $0 == __FILE__
  e = EvalServerManager.new
  protect { loop { e.tick; sleep 0.1 }}
end
