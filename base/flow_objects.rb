=begin
	$Id$

	GridFlow
	Copyright (c) 2001-2007 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
=end
module GridFlow
GridFlow = ::GridFlow # ruby is nuts... sometimes

FObject.subclass("fps",1,1) {
	def initialize(*options)
		super
		@history = []   # list of delays between incoming messages
		@last = 0.0     # when was last time
		@duration = 0.0 # how much delay since last summary
		@period = 1     # minimum delay between summaries
		@detailed = false
		@mode = :real
		options.each {|o|
			case o
			when :detailed; @detailed=true
			when :real,:user,:system,:cpu; @mode=o
			end
		}
		def @history.moment(n=1)
			sum = 0
			each {|x| sum += x**n }
			sum/length
		end
	end
	def method_missing(*a) end # ignore non-bangs
	def _0_period x; @period=x end
	def publish
		@history.sort!
		n=@history.length
		fps = @history.length/@duration
		if not @detailed then send_out 0, fps; return end
		send_out 0, fps,
			1000*@history.min,
			500*(@history[n/2]+@history[(n-1)/2]),
			1000*@history.max,
			1000/fps,
			1000*(@history.moment(2) - @history.moment(1)**2)**0.5
	end
	def _0_bang
		t = case @mode
		when :real; Time.new.to_f
		when :user; Process.times.utime
		when :system; Process.times.stime
		when :cpu; GridFlow.rdtsc/GridFlow.cpu_hertz
		end
		@history.push t-@last
		@duration += t-@last
		@last = t
		return if @duration<@period
		fps = @history.length/@duration
		publish if fps>0.001
		@history.clear
		@duration = 0
	end
}

FObject.subclass("exec",1,0) {def _0_shell(*a) system(a.map!{|x| x.to_s }.join(" ")) end}
FObject.subclass("renamefile",1,0) {def initialize; end; def _0_list(a,b) File.rename(a.to_s,b.to_s) end}
FObject.subclass("ls",1,1) {
        def _0_symbol(s) send_out 0, :list, *Dir.new(s.to_s).map {|x| x.intern } end
        def _0_glob  (s) send_out 0, :list, *Dir[    s.to_s].map {|x| x.intern } end
}

FObject.subclass("rubysprintf",2,1) {
  def initialize(*format) _1_list(format) end
  def _0_list(*a) send_out 0, :symbol, (sprintf @format, *a).intern end
  alias _0_float _0_list
  alias _0_symbol _0_list
  def _1_list(*format) @format = format.join(" ") end
  alias _1_symbol _1_list
}

class PDNetSocket < FObject
	def initialize(host,port,protocol=:udp,*options)
		super
		_1_connect(host,port,protocol)
		@options = {}
		options.each {|k|
			k=k.intern if String===k
			@options[k]=true
		}
	end
	def _1_connect(host,port,protocol=:udp)
		host = host.to_s
		port = port.to_i
		@host,@port,@protocol = host.to_s,port.to_i,protocol
		case protocol
		when :udp
			@socket = UDPSocket.new
			if host=="-" then @socket.bind nil, port end
		when :tcp
 			if host=="-" then @server = TCPServer.new("localhost",port)
			else              @socket = TCPSocket.new(host,port) end
		end
		@clock = Clock.new self
		@clock.delay 0
		@data = ""
	end
	def encode(x)
		x=x.to_i if @options[:nofloat] and Float===x
		x.to_s
	end
	def method_missing(sel,*args)
		sel=sel.to_s
		sel.sub!(/^_\d_/, "") or return super
		sel=(case sel; when "int","float"; ""; else encode(sel) end)
		msg = [sel,*args.map{|arg| encode(arg) }].join(" ")
		if @options[:nosemicolon] then msg << "\n" else msg << ";\n" end
		post "encoding as: %s", msg.inspect if @options[:debug]
		case @protocol
		when :udp; @socket.send msg, 0, @host, @port
		when :tcp; @socket.send msg, 0
		end
	end
	def delete; @clock.unset; @socket.close end
	def decode s
		post "decoding from: %s", s.inspect if @options[:debug]
		s.chomp!("\n")
		s.chomp!("\r")
		s.chomp!(";")
		a=s.split(/[\s\0]+/)
		a.shift if a[0]==""
		a.map {|x|
			case x
			when /-?\d+$/; x.to_i
			when /-?\d/; x.to_f
			else x.intern
			end
		}
	end
	def call
		ready_to_accept = IO.select [@server],[],[],0 if @server
		if ready_to_accept
			@socket.close if @socket
			@socket = @server.accept
		end
		ready_to_read = IO.select [@socket],[],[],0 if @socket
		return if not ready_to_read
		case @protocol
		when :udp
			data,sender = @socket.recvfrom 1024
			send_out 1, sender.map {|x| x=x.intern if String===x; x }
			send_out 0, *(decode data)
		when :tcp
			@data << @socket.sysread(1024)
			sender = @socket.peeraddr
			loop do
				n = /\n/ =~ @data
				break if not n
				send_out 1, sender.map {|x| x=x.intern if String===x; x }
				send_out 0, *(decode @data.slice!(0..n))
			end
		end
		@clock.delay 50
	end
	install "pd_netsocket", 2, 2
end

FObject.subclass("printargs",1,0) {
  def initialize(*args)
    super
    @aaargh = args
  end
  def initialize2()
    @clock = Clock.new self
    @clock.delay 0
  end
  def call
    post "ruby=%s", @aaargh.inspect
    post "  pd=%s",    args.inspect
  end
  def method_missing(*a)
    post "message: %s", *a.inspect
  end
}

# plotter control (HPGL)
FObject.subclass("plotter_control",1,1) {
  def puts(x)
    x << "\n"
    x.each_byte {|b| send_out 0, b }
    send_out 0
  end
  def _0_pu; puts "PU;" end
  def _0_pd; puts "PD;" end
  def _0_pa x,y; puts "PA#{x},#{y};" end
  def _0_sp c; puts "SP#{c};"; end
  def _0_ip(*v) puts "IP#{v.join','};" end
  def _0_other(command,*v) puts "#{command.to_s.upcase}#{v.join','};" end
  def _0_print(*text) puts "LB#{text.join(' ')}\003;" end
  def _0_print_from_ascii(*codes)
    _0_print codes.map{|code| code.chr }.join("")
  end
}

# ASCII, useful for controlling pics
FObject.subclass("ascii",1,1) {
  def puts(x) x.each_byte {|b| send_out 0, b } end
  def _0_float x; puts "#{x.to_i}" end
}
# System, similar to shell
FObject.subclass("system",1,1) { def _0_system(*a) system(a.join(" ")) end }

module Ioctl
	def ioctl_intp_out(arg1,arg2) ioctl(arg1,[arg2].pack("l")) end
	def ioctl_intp_in(arg1)       ioctl(arg1,s="blah"); return s.unpack("l")[0] end
end

# experimental
FObject.subclass("rubyarray",2,2) {
  def initialize() @a=[]; @i=0; end
  def _0_get(s=nil)
    case s
    when :size; send_out 1,:size,@a.size
    when nil; _0_get :size
    end
  end
  def _0_clear; @a.clear end
  def _0_float i; @i=i; send_out 0, *@a[@i] if @a[@i]!=nil; end
  def _1_list(*l) @a[@i]=l; end
  def _0_save(filename,format=nil)
    f=File.open(filename.to_s,"w")
    if format then
      @a.each {|x| f.puts(format.to_s%x) }
    else
      @a.each {|x| f.puts(x.join(",")) }
    end
    f.close
  end
  def _0_load(filename)
    f=File.open(filename.to_s,"r")
    @a.clear
    f.each {|x| @a.push x.split(",").map {|y| Float(y) rescue y.intern }}
    f.close
  end
}

FObject.subclass("regsub",3,1) {
  def initialize(from,to) _1_symbol(from); _2_symbol(to) end
  def _0_symbol(s) send_out 0, :symbol, s.to_s.gsub(@from, @to).intern end
  def _1_symbol(from) @from = Regexp.new(from.to_s.gsub(/`/,"\\")) end
  def _2_symbol(to)   @to = to.to_s.gsub(/`/,"\\") end
  #doc:_0_symbol,"a string to transform"
  #doc:_1_symbol,"a regexp pattern to be found inside of the string"
  #doc:_2_symbol,"a replacement for the found pattern"
  #doc_out:_0_symbol,"the transformed string"
}

FObject.subclass("memstat",1,1) {
  def _0_bang
    f = File.open("/proc/#{$$}/stat")
    send_out 0, Float(f.gets.split(" ")[22]) / 1024.0
    f.close
  end
  #doc:_0_bang,"lookup process stats for the currently running pd+ruby and figure out how much RAM it uses."
  #doc_out:_0_float,"virtual size of RAM in kilobytes (includes swapped out and shared memory)"
}

end # module GridFlow

class IO; include GridFlow::Ioctl; end
