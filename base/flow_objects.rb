=begin
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003,2004 by Mathieu Bouchard

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

#-------- fClasses for: control + misc

# a dummy class that gives access to any stuff global to GridFlow.
class GridGlobal < FObject
	def _0_bang
		GridFlow.tick
	end
	def _0_profiler_reset
		GridFlow.fobjects_set.each {|o,*| o.total_time = 0 }
		GridFlow.profiler_reset2 if GridFlow.respond_to? :profiler_reset2
	end
	def _0_profiler_dump
		ol = []
		total=0
		GridFlow.post "-"*32
		GridFlow.post "microseconds percent pointer  constructor"
		GridFlow.fobjects_set.each {|o,*| ol.push o }

		# HACK: BitPacking is not a real fobject
		# !@#$ is this still necessary?
		ol.delete_if {|o| not o.respond_to? :total_time }

		ol.sort! {|a,b| a.total_time <=> b.total_time }
		ol.each {|o| total += o.total_time }
		total=1 if total<1
		total_us = 0
		ol.each {|o|
			ppm = o.total_time * 1000000 / total
			us = (o.total_time*1E6/GridFlow.cpu_hertz).to_i
			total_us += us
			GridFlow.post "%12d %2d.%04d %08x %s", us,
				ppm/10000, ppm%10000, o.object_id, o.args
		}
		GridFlow.post "-"*32
		GridFlow.post "sum of accounted microseconds: #{total_us}"
		if GridFlow.respond_to? :memcpy_calls then
			GridFlow.post "memcpy calls: #{GridFlow.memcpy_calls} "+
				"; bytes: #{GridFlow.memcpy_bytes}"+
				"; time: #{GridFlow.memcpy_time}"
		end
		if GridFlow.respond_to? :malloc_calls then
			GridFlow.post "malloc calls: #{GridFlow.malloc_calls} "+
				"; bytes: #{GridFlow.malloc_bytes}"+
				"; time: #{GridFlow.malloc_time}"
		end
		GridFlow.post "-"*32
	end
	# security issue if patches shouldn't be allowed to do anything they want
	def _0_eval(*l)
		s = l.map{|x|x.to_i.chr}.join""
		GridFlow.post "ruby: %s", s
		GridFlow.post "returns: %s", eval(s).inspect
	end
	install "gridflow", 1, 1
	addcreator "@global"
	begin
		GridFlow.whatever :bind, "gridflow", "gridflow"
	rescue Exception
	end
end

class FPS < GridObject
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
	def _0_position(*); end
	def method_missing(*a) end # ignore non-bangs
	def _0_period x; @period=x end
	def publish
		if not @detailed then send_out 0, fps; return end
		@history.sort!
		n=@history.length
		fps = @history.length/@duration
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
	install "fps", 1, 1
end

# to see what the messages look like when they get on the Ruby side.
class RubyPrint < GridFlow::FObject
	def initialize(*a)
		super
		@time = !!(a.length and a[0]==:time)
	end

	def method_missing(s,*a)
		s=s.to_s
		pre = if @time then sprintf "%10.6f  ", Time.new.to_f else "" end
		case s
		when /^_0_/; GridFlow.post "%s","#{pre}#{s[3..-1]}: #{a.inspect}"
		else super
		end
	end

	install "rubyprint", 1, 0
end

class PrintArgs < GridFlow::FObject
	def initialize(*a)
		super
		GridFlow.post(a.inspect)
	end
	install "printargs", 0, 0
end

class GridPrint < GridFlow::GridObject
	attr_accessor :name

	def initialize(name=nil)
		super # don't forget super!!!
		if name then @name = name.to_s+": " else @name="" end
		@base=10; @format="d"
		@trunc=70
	end

	def format
		case @nt
			when :float32; '%6.6f'
			when :float64; '%14.14f'
			else "%#{@columns}#{@format}" end
	end

	def _0_base(x)
		@format = (case x
		when 2; "b"
		when 8; "o"
		when 10; "d"
		when 16; "x"
		else raise "base #{x} not supported" end)
		@base = x
	end

	def _0_trunc(x)
		0..240===x or raise "out of range (not in 0..240 range)"
		@trunc = x
	end

	def make_columns udata
		min = udata.min
		max = udata.max
		@columns = ""
		@columns = [
			sprintf(format,min).length,
			sprintf(format,max).length].max
	end

	def unpack data
		case @nt
		when :uint8; data.unpack("C*")
		when :int16; data.unpack("s*")
		when :int32; data.unpack("l*")
		when :float32; data.unpack("f*")
		when :float64; data.unpack("d*")
		else raise "#{self.class} doesn't know how to decode #{@nt}"
		end
	end

	def _0_rgrid_begin
		@dim = inlet_dim 0
		@nt = inlet_nt 0
		@data = ""
	end

	def _0_rgrid_flow(data) @data << data end

	def _0_rgrid_end
		head = "#{name}Dim[#{@dim.join','}]"
		head << "(#{@nt})" if @nt!=:int32
		head << ": "
		if @dim.length > 3 then
			GridFlow.post head+" (not printed)"
		elsif @dim.length < 2 then
			udata = unpack @data
			make_columns udata
			GridFlow.post trunc(head + dump(udata))
		elsif @dim.length == 2 then
			GridFlow.post head
			udata = unpack @data
			make_columns udata
			sz = udata.length/@dim[0]
			for row in 0...@dim[0]
				GridFlow.post trunc(dump(udata[sz*row,sz]))
			end
		elsif @dim.length == 3 then
			GridFlow.post head
			make_columns unpack(@data)
			sz = @data.length/@dim[0]
			sz2 = sz/@dim[1]
			for row in 0...@dim[0]
				column=0; str=""
				for col in 0...@dim[1]
					str << "{" << dump(unpack(@data[sz*row+sz2*col,sz2])) << "}"
					break if str.length>@trunc
				end
				GridFlow.post trunc(str)
				(GridFlow.post "[...]"; break) if row>100
			end
		end
		@data,@dim,@nt = nil
	end

	def dump(udata,sep=" ")
		f = format
		udata.map{|x| sprintf f,x }.join sep
	end

	def trunc s
		if s.length>@trunc then s[0...@trunc]+" [...]" else s end
	end

	install_rgrid 0, true
	install "#print", 1, 0
end

class GridPack < GridObject
	class<<self;attr_reader :ninlets;end
	def initialize(n=nil,cast=:int32)
		n||=self.class.ninlets
		n>=16 and raise "too many inlets"
		super
		@data=[0]*n
		@cast=cast
	end
	def initialize2
		return if self.class.ninlets>1
		GridFlow.whatever :addinlets, self, @data.length-1
	end
	def _0_cast(cast) @cast=cast end #!@#$ typecheck
	def self.define_inlet i
		module_eval "
			def _#{i}_int   x; @data[#{i}]=x; _0_bang; end
			def _#{i}_float x; @data[#{i}]=x; _0_bang; end
		"
	end
	(0...15).each {|x| define_inlet x }
	def _0_bang
		send_out_grid_begin 0, [@data.length], @cast
		send_out_grid_flow 0, @data.pack("l*")
	end
	install_rgrid 0
	install "#pack", 1, 1
end

# the install_rgrids in the following are hacks so that
# outlets can work. (install_rgrid is supposed to be for receiving)
# maybe GF-0.8 doesn't need that.
class GridTwo   < GridPack; install_rgrid 0; install "@two",   2, 1 end
class GridThree < GridPack; install_rgrid 0; install "@three", 3, 1 end
class GridFour  < GridPack; install_rgrid 0; install "@four",  4, 1 end
class GridEight < GridPack; install_rgrid 0; install "@eight", 8, 1 end

class GridUnpack < GridObject
  def initialize(n)
    @n=n
    n>=10 and raise "too many outlets"
    super
  end
  def initialize2
    GridFlow.whatever :addoutlets, self, @n
  end
  def _0_rgrid_begin
    inlet_dim(0)==[@n] or raise "expecting Dim[#{@n}], got Dim#{@dim}"
    inlet_set_factor @n
  end
  def _0_rgrid_flow data
    nt = inlet_nt
    data.unpack("l*").each_with_index{|x,i| send_out i,x }
  end
  install_rgrid 0
  install "#unpack", 1, 0
end

class GridExportSymbol < GridObject
	def _0_rgrid_begin; @data="" end
	def _0_rgrid_flow data; @data << data; end
	def _0_rgrid_end
		send_out 0, :symbol, @data.unpack("I*").pack("c*").intern
	end
	install_rgrid 0
	install "#export_symbol", 1, 1
end

#-------- fClasses for: math

class Messagebox<FPatcher
  def initialize(*a) @a=a end
  def _0_float(x)  send_out 0, *@a.map {|y| if y=="$1".intern then x else y end } end
  def _0_symbol(x) send_out 0, *@a.map {|y| if y=="$1".intern then x else y end } end
  install "gfmessagebox", 1, 1
end

class GridOp1<FPatcher
  @fobjects = ["# +","#type","gfmessagebox list $1 #"]
  @wires = [-1,0,1,0, 1,0,2,0, 2,0,0,1, -1,0,0,0, 0,0,-1,0]
  def initialize(sym)
	super
	@fobjects[0].send_in 0, case sym
		when :rand; "op rand"; when :sqrt; "op sqrt"
		when :abs;  "op abs-"; when :sq;   "op sq-"
		else raise "bork BORK bork" end
  end
  install "@!", 1, 1
end

class OldFold<FPatcher
  @fobjects = ["#fold +","gfmessagebox seed $1"]
  @wires = [-1,0,0,0, -1,1,1,0, 1,0,0,1, 0,0,-1,0]
  def initialize(op,seed=0) super; o=@fobjects[0]
	o.send_in 0, :op, op; o.send_in 0, :seed, seed end
  install "@fold", 2, 1
end
class OldScan<FPatcher
  @fobjects = ["#scan +","gfmessagebox seed $1"]
  @wires = [-1,0,0,0, -1,1,1,0, 1,0,0,1, 0,0,-1,0]
  def initialize(op,seed=0) super; o=@fobjects[0]
	o.send_in 0, :op, op; o.send_in 0, :seed, seed end
  install "@scan", 2, 1
end
class OldInner<FPatcher
  @fobjects = ["#inner","gfmessagebox seed $1"]
  @wires = [-1,0,0,0, -1,1,1,0, 1,0,0,0, 0,0,-1,0, -1,2,0,1]
  def initialize(op=:*,fold=:+,seed=0,r=0) super; o=@fobjects[0]
	o.send_in 0, :op, op; o.send_in 0, :fold, fold
	o.send_in 0, :seed, seed; o.send_in 1, r end
  install "@inner", 3, 1
end
class OldConvolve<FPatcher
  @fobjects = ["#convolve"]
  @wires = [-1,0,0,0, -1,2,0,1, 0,0,-1,0]
  def initialize(op=:*,fold=:+,seed=0,r=0) super; o=@fobjects[0]
	o.send_in 0, :op, op; o.send_in 0, :fold, fold
	o.send_in 0, :seed, seed; o.send_in 1, r end
  install "@convolve", 2, 1
end

#-------- fClasses for: video

class GridScaleTo < FPatcher
	@fobjects = [
		"@for {0 0} {42 42} {1 1}","@ *","@ /",
		"@store","#dim","@redim {2}","#finished",
	]
	@wires = []
	for i in 1..3 do @wires.concat [i-1,0,i,0] end
	@wires.concat [3,0,-1,0, 4,0,5,0, 5,0,1,1, 6,0,0,0,
		-1,0,4,0, -1,0,3,1, -1,0,6,0, -1,1,0,1, -1,1,2,1]
	def initialize(size)
		(size.length==2 and Numeric===size[0] and Numeric===size[1]) or
			raise "expecting {height width}"
		super
		send_in 1, size
	end
	install "@scale_to", 2, 1
end

#<vektor> told me to:
# RGBtoYUV : @fobjects = ["@inner * + 0 {3 3 # 66 -38 112 128 -74 -94 25 112 -18}",
#	"@ >> 8","@ + {16 128 128}"]
# YUVtoRGB : @fobjects = ["@ - {16 128 128}",
#	"@inner * + 0 {3 3 # 298 298 298 0 -100 516 409 -208 0}","@ >> 8"]

class GridRotate < FPatcher
	@fobjects = ["@inner * + 0","@ >> 8"]
	@wires = [-1,0,0,0, 0,0,1,0, 1,0,-1,0]
	def update_rotator
		rotator = (0...@axis[2]).map {|i|
			(0...@axis[2]).map {|j|
				if i==j then 256 else 0 end
			}
		}
		th = @angle * Math::PI / 18000
		scale = 1<<8
		(0...2).each {|i|
			(0...2).each {|j|
				rotator[@axis[i]][@axis[j]] =
					(scale*Math.cos(th+(j-i)*Math::PI/2)).to_i
			}
		}
		@fobjects[0].send_in 2,
			@axis[2], @axis[2], "#".intern, *rotator.flatten
	end
	def _0_axis(from,to,total)
		total>=0 or raise "total-axis number incorrect"
		from>=0 and from<total or raise "from-axis number incorrect"
		to  >=0 and to  <total or raise   "to-axis number incorrect"
		@axis = [from,to,total]
		update_rotator
	end
	def initialize(rot=0,axis=[0,1,2])
		super
		@angle=0
		_0_axis(*axis)
		send_in 1, rot
	end
	def _1_int(angle) @angle = angle; update_rotator end
	alias _1_float _1_int
	install "#rotate", 2, 1
end

class ForEach < FObject
	def initialize() super end
	def _0_list(*a)
		a.each {|e|
			if Symbol===e then
				send_out 0,:symbol,e
			else
				send_out 0,e
			end
		}
	end
	install "foreach", 1, 1
end

class ListFlatten < FObject
	def initialize() super end
	def _0_list(*a) send_out 0,:list,*a.flatten end
	install "listflatten", 1, 1
end

class Sprintf < FObject
  def initialize(format) _1_symbol(format) end
  def _0_list(*a)
    a.each {|x| x=x.to_s if Symbol===x }
    send_out 0, :symbol, (sprintf @format, *a).intern
  end
  alias _0_int _0_list
  alias _0_float _0_list
  alias _0_symbol _0_list
  def _1_symbol(format) @format = format.to_s end
  install "rubysprintf", 2, 1
end

#-------- fClasses for: jMax compatibility

class JMaxUDPSend < FObject
	def initialize(host,port)
		super
		@socket = UDPSocket.new
		@host,@port = host.to_s,port.to_i
	end
	def encode(x)
		case x
		when Integer; "\x03" + [x].pack("N")
		when Float; "\x04" + [x].pack("g")
		when Symbol, String; "\x01" + x.to_s + "\x02"
		end
	end
	def method_missing(sel,*args)
		sel=sel.to_s.sub(/^_\d_/, "")
		@socket.send encode(sel) +
			args.map{|arg| encode(arg) }.join("") + "\x0b",
			0, @host, @port
	end
	def delete; @socket.close end
	install "jmax_udpsend", 1, 0
end

class JMaxUDPReceive < FObject
	def initialize(port)
		super
		@socket = UDPSocket.new
		@port = port.to_i
		@socket.bind nil, @port
		$tasks[self] = proc {tick}
	end
	def decode s
		n = s.length
		i=0
		m = []
		case s[i]
		when 3; i+=5; m << s[i-4,4].unpack("N")[0]
		when 4; i+=5; m << s[i-4,4].unpack("g")[0]
		when 1; i2=s.index("\x02",i); m << s[i+1..i2-1].intern; i=i2+1
		when 11; break
		else raise "unknown code in udp packet"
		end while i<n
		m
	end
	def tick
		ready_to_read = IO.select [@socket],[],[],0
		return if not ready_to_read
		data,sender = @socket.recvfrom 1024
		return if not data
		send_out 1, sender.map {|x| x=x.intern if String===x; x }
		send_out 0, *(decode data)
	end
	def delete; $tasks.delete(self); @socket.close end
	install "jmax_udpreceive", 0, 2
end

class JMax4UDPSend < FObject
	def initialize(host,port)
		super
		@socket = UDPSocket.new
		@host,@port = host.to_s,port.to_i
		@symbols = {}
	end
	def encode(x)
		case x
		when Integer; "\x01" + [x].pack("N")
		when Float; "\x02" + [x].pack("G")
		when Symbol, String
			x = x.to_s
			y = x.intern
			if not @symbols[y]
				@symbols[y]=true
				"\x04" + [y].pack("N") + x + "\0"
			else
				"\x03" + [y].pack("N")
			end
		end
	end
	def method_missing(sel,*args)
		sel=sel.to_s.sub(/^_\d_/, "")
		sel=(case sel; when "int","float"; ""; else encode(sel) end)
		args=args.map{|arg| encode(arg) }.join("")
		@socket.send(sel+args+"\x0f", 0, @host, @port)
	end
	def delete; @socket.close end
	install "jmax4_udpsend", 1, 0
end

class JMax4UDPReceive < FObject
	def initialize(port)
		super
		@socket = UDPSocket.new
		@port = port.to_i
		@socket.bind nil, @port
		$tasks[self] = proc {tick}
		@symbols = {}
	end
	def decode s
		n = s.length
		i=0
		m = []
		case s[i]
		when 1; i+=5; m << s[i-4,4].unpack("N")[0]
		when 2; i+=9; m << s[i-8,8].unpack("G")[0]
		when 3
			i+=5; sid = s[i-4,4].unpack("N")[0]
			m << @symbols[sid]
		when 4
			i+=5; sid = s[i-4,4].unpack("N")[0]
			i2=s.index("\x00",i)
			@symbols[sid] = s[i..i2-1].intern
			m << @symbols[sid]
			i=i2+1
		when 15; break
		else GridFlow.post "unknown code %d in udp packet %s", s[i], s.inspect; return m
		end while i<n
		m
	end
	def tick
		ready_to_read = IO.select [@socket],[],[],0
		return if not ready_to_read
		data,sender = @socket.recvfrom 1024
		return if not data
		send_out 1, sender.map {|x| x=x.intern if String===x; x }
		send_out 0, *(decode data)
	end
	def delete; $tasks.delete(self); @socket.close end
	install "jmax4_udpreceive", 0, 2
end

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
			if host=="-" then
				@socket.bind nil, port
			end
		when :tcp
			if host=="-" then
				@server = TCPServer.new("localhost",port)
			else
				@socket = TCPSocket.new(host,port)
			end
			
		end
		$tasks[self] = proc {tick}
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
		GridFlow.post "encoding as: %s", msg.inspect if @options[:debug]
		case @protocol
		when :udp; @socket.send msg, 0, @host, @port
		when :tcp; @socket.send msg, 0
		end
	end
	def delete; $tasks.delete(self); @socket.close end
	def decode s
		GridFlow.post "decoding from: %s", s.inspect if @options[:debug]
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
	def tick
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
			#GridFlow.post "sysread_begin"
			@data << @socket.sysread(1024)
			#GridFlow.post "sysread: #{@data}"
			sender = @socket.peeraddr
			loop do
				n = /\n/ =~ @data
				break if not n
				send_out 1, sender.map {|x| x=x.intern if String===x; x }
				send_out 0, *(decode @data.slice!(0..n))
			end
		end
	end
	install "pd_netsocket", 2, 2
end

class PDNetReceive < PDNetSocket
	def initialize(port) super("-",port) end
	install "pd_netreceive", 0, 2
end

class PDNetSend < PDNetSocket
	install "pd_netsend", 1, 0
end

# bogus class for representing objects that have no recognized class.
class Broken < FObject
	def args; a=@args.dup; a[7,0] = " "+classname; a end
	install "broken", 0, 0
end

class Fork < FObject
  def method_missing(sel,*args)
    sel.to_s =~ /^_(\d)_(.*)$/ or super
	send_out 1,$2.intern,*args
	send_out 0,$2.intern,*args
  end
  install "fork", 1, 2 if GridFlow.bridge_name != "jmax"
end

class Shunt < FObject
	def initialize(n,i=0) super; @n=n; @i=i end
	def initialize2; GridFlow.whatever :addoutlets, self, @n end
	def method_missing(sel,*args)
		sel.to_s =~ /^_(\d)_(.*)$/ or super
		send_out @i,$2.intern,*args
	end
	def _1_int i; @i=i.to_i % @n end
	alias :_1_float :_1_int
	install "shunt", 2, 0
end

# hack: this is an alias.
if GridFlow.bridge_name != "jmax"
	class Demux < Shunt; install "demux", 2, 0; end
end

unless GridFlow.bridge_name =~ /jmax/
	class Button < FObject
		def method_missing(*) send_out 0 end
		install "button", 1, 1
	end
	class Toggle < FObject
		def _0_bang; @state ^= true; trigger end
		def _0_int x; @state = x!=0; trigger end
		def trigger; send_out 0, (if @state then 1 else 0 end) end
		install "toggle", 1, 1
	end
#	class Slider < FObject
#		install "slider", 1, 1
#	end
	class JPatcher < FObject
		def initialize(*a)
			super
			#STDERR.puts "JPATCHER: #{a.inspect}"
			@subobjects = {}
		end
		attr_accessor :subobjects
		install "jpatcher", 0, 0
	end
	class JComment < FObject
		install "jcomment", 0, 0
	end
	class LoadBang < FObject
		def trigger; send_out 0 end
		install "loadbang", 0, 1
	end
	class MessBox < FObject
		def _0_bang; send_out 0, *@argv end
		def clear; @argv=[]; end
		def append(*argv) @argv<<argv; end
		install "messbox", 1, 1
	end
	class OneShot < FObject
		def initialize(state=true) @state=state!=0 end
		def method_missing(sel,*a)
			m = /^_0_(.*)$/.match(sel.to_s) or return super
			send_out 0, m[1].intern, *a if @state
			@state=false
		end
		def _1_int(state) @state=state!=0 end
		alias _1_float _1_int
		def _1_bang; @state=true end
		install "oneshot", 2, 1
	end
end

# jMax List Classes

LPrefix = (if GridFlow.bridge_name == "jmax" then "ruby" else "" end)

	class List < FObject
		def initialize(*a) @a=a end
		def _0_list(*a) @a=a; _0_bang end
		def _1_list(*a) @a=a end
		def _0_bang; send_out 0, :list, *@a end
		install "#{LPrefix}listmake", 2, 1
	end

	class ListLength < FObject
		def initialize() super end
		def _0_list(*a) send_out 0, a.length end
		install "#{LPrefix}listlength", 1, 1
	end

	class ListElement < FObject
		def initialize(i=0) super; @i=i.to_i end
		def _1_int(i) @i=i.to_i end; alias _1_float _1_int
		def _0_list(*a)
			e=a[@i]
			if Symbol===e then
				send_out 0, :symbol, e
			else
				send_out 0, e
			end
		end
		install "#{LPrefix}listelement", 2, 1
	end

	class ListSublist < FObject
		def initialize(i=0,n=1) super; @i,@n=i.to_i,n.to_i end
		def _1_int(i) @i=i.to_i end; alias _1_float _1_int
		def _2_int(n) @n=n.to_i end; alias _2_float _2_int
		def _0_list(*a) send_out 0, :list, *a[@i,@n] end
		install "#{LPrefix}listsublist", 3, 1
	end

	class ListPrepend < FObject
		def initialize(*b) super; @b=b end
		def _0_list(*a) a[0,0]=@b; send_out 0, :list, *a end
		def _1_list(*b) @b=b end
		install "#{LPrefix}listprepend", 2, 1
	end

	class ListAppend < FObject
		def initialize(*b) super; @b=b end
		def _0_list(*a) a[a.length,0]=@b; send_out 0, :list, *a end
		def _1_list(*b) @b=b end
		install "#{LPrefix}listappend", 2, 1
	end

	class ListReverse < FObject
		def initialize() super end
		def _0_list(*a) send_out 0,:list,*a.reverse end
		install "#{LPrefix}listreverse", 1, 1
	end

	class MessagePrepend < FObject
		def initialize(*b) super; @b=b end
		def _0_(*a) a[0,0]=@b; send_out 0, *a end
		def _1_list(*b) @b=b end
		def method_missing(sym,*a)
			(m = /(_\d_)(.*)/.match sym.to_s) or return super
			_0_ m[2].intern, *a
		end
		install "messageprepend", 2, 1
	end

	class MessageAppend < FObject
		def initialize(*b) super; @b=b end
		def _0_(*a) a[a.length,0]=@b; send_out 0, *a end
		def _1_list(*b) @b=b end
		def method_missing(sym,*a)
			(m = /(_\d_)(.*)/.match sym.to_s) or return super
			_0_ m[2].intern, *a
		end
		install "messageappend", 2, 1
	end

unless GridFlow.bridge_name =~ /jmax/

# this is the demo and test for Ruby->jMax bridge
# FObject is a flow-object as found in jMax
# _0_bang means bang message on inlet 0
# FObject#send_out sends a message through an outlet
class RubyFor < GridFlow::FObject
	attr_accessor :start, :stop, :step
	def cast(key,val)
		val = Integer(val) if Float===val
		raise ArgumentError, "#{key} isn't a number" unless Integer===val
	end
	def initialize(start,stop,step)
		super
		cast("start",start)
		cast("stop",stop)
		cast("step",step)
		@start,@stop,@step = start,stop,step
	end
	def _0_bang
		x = start
		if step > 0
			(send_out 0, x; x += step) while x < stop
		elsif step < 0
			(send_out 0, x; x += step) while x > stop
		end
	end
	def _0_int(x)
		self.start = x
		_0_bang
	end
	alias _1_int stop=
	alias _2_int step=
	alias _1_float stop=
	alias _2_float stop=

	# FObject.install(name, inlets, outlets)
	# no support for metaclasses yet
	install "for", 3, 1
end

class InvPlus < FObject
  def initialize(b=0) @b=b end
  def _1_float(b) @b=b end
  def _0_float(a) send_out 0, :float, @b-a end
  install "inv+", 2, 1
end

class InvTimes < FObject
  def initialize(b=0) @b=b end
  def _1_float(b) @b=b end
  def _0_float(a) send_out 0, :float, @b/a end
  install "inv*", 2, 1
end

class JMaxRange<GridFlow::FObject
  def initialize(*a) @a=a end
  def initialize2
    GridFlow.whatever  :addinlets, self, @a.length
    GridFlow.whatever :addoutlets, self, @a.length
  end
  def _0_float(x) i=0; i+=1 until @a[i]==nil or x<@a[i]; send_out i,x end
  def method_missing(sym,*a)
    m = /^(_\d+_)(.*)/.match(sym.to_s) or return super
    m[2]=="float" or return super
    @a[m[1].to_i-1] = a[0]
    GridFlow.post "setting a[#{m[1].to_i-1}] = #{a[0]}"
  end
  install "range", 1, 1
end

end # if not =~ jmax

#-------- fClasses for: GUI

class Peephole < GridFlow::FPatcher
	@fobjects = ["#dim","#export_list","#downscale_by 1 smoothly","@out","#scale_by 1",
	proc{Demux.new(2)}]
	@wires = [-1,0,0,0, 0,0,1,0, -1,0,5,0, 2,0,3,0, 4,0,3,0, 5,0,2,0, 5,1,4,0, 3,0,-1,0]
	def initialize(sy=32,sx=32,*args)
		super
		@fobjects[1].connect 0,self,2
		GridFlow.post "Peephole#initialize: #{sx} #{sy} #{args.inspect}"
		@scale = 1
		@down = false
		@sy,@sx = sy,sx # size of the peephole widget
		@y,@x = 0,0     # topleft of the peephole widget
		@fy,@fx = 0,0   # size of last frame after downscale
	end
	def initialize2(*args)
		GridFlow.post "Peephole#initialize2: #{args.inspect}"
		return if GridFlow.bridge_name!="puredata"
		@rsym = "peephole#{self.id}".intern
		GridFlow.whatever :bind, self, @rsym.to_s
	end
	def pd_show(can)
		GridFlow.post "pd_show %d",can
		a=GridFlow.whatever:getpos,self,can
		GridFlow.post "getpos -> [%s]",a.inspect
		@x,@y=a
		@can=".x%x.c"%(can*4) if can
		if not @open
			can2=@can.gsub(/\.c$/,"")
			GridFlow.whatever :gui, %{
				pd \"#{@rsym} open [eval list [winfo id #{@can}]] 1;\n\";
			}
			@open=true
		end
		# round-trip to ensure this is done after the open
		GridFlow.whatever :gui, %{
			pd \"#{@rsym} set_geometry #{@y} #{@x} #{@sy} #{@sx};\n\";
		}
		color = "#A07467"
		GridFlow.whatever :gui, %{
			#{@can} delete #{@rsym}
			#{@can} create rectangle #{@x} #{@y} #{@x+@sx} #{@y+@sy} -fill #{color} -tags #{@rsym}
		}
		set_geometry_for_real_now
	end
	def pd_displace(can,x,y)
		GridFlow.post "pd_displace %d %d %d",can,x,y
		GridFlow.whatever :gui, "#{@can} move #{@rsym} #{x} #{y}\n"
		@x+=x
		@y+=y
		pd_show(can)
	end
	def pd_getrect(can)
		x,y=GridFlow.whatever :getpos,self,can
	#	GridFlow.post "pd_getrect %d %d %d",can,x,y
		if @x!=x or @y!=y then @x,@y=x,y end
		#GridFlow.post "%d %d %d %d", @x, @y, @sx, @sy
		[@x-1,@y-1,@x+@sx+1,@y+@sy+1]
	end
	def pd_vis(can,flag)
		GridFlow.post "pd_vis"
		pd_show(can) if flag!=0
	end
	def pd_click(can,xpix,ypix,shift,alt,dbl,doit)
		#GridFlow.post "click: %d %d %d %d %d %d",xpix,ypix,shift,alt,dbl,doit
		return 0
	end
	def pd_select(*a)
		GridFlow.post "pd_select "+a.join(", ")
	end
	def set_geometry_for_real_now
		@fy,@fx=@sy,@sx if @fy<1 or @fx<1
		if @fx>@sx or @fy>@sx then
			@down = true
			@scale = [(@fy+@sy-1)/@sy,(@fx+@sx-1)/@sx].max
			@scale=1 if @scale<1 # what???
			@fobjects[2].send_in 1, @scale
			sy2 = @fy/@scale
			sx2 = @fx/@scale
		else
			@down = false
			@scale = [@sy/@fy,@sx/@fx].min
			@fobjects[4].send_in 1, @scale
			sy2 = @fy*@scale
			sx2 = @fx*@scale
		end
		begin
			@fobjects[5].send_in 1, (if @down then 0 else 1 end)
			x2=@y+(@sy-sy2)/2
			y2=@x+(@sx-sx2)/2
			@fobjects[3].send_in 0, :set_geometry,
				x2, y2, sy2, sx2
		rescue StandardError => e
			GridFlow.post "peeperr: %s", e.inspect
		end
		GridFlow.post "set_geometry_for_real_now (%d,%d) (%d,%d) (%d,%d) (%d,%d) (%d,%d)",
			@x+1,@y+1,@sx,@sy,@fx,@fy,sx2,sy2,x2,y2
	end
	def _0_open(wid,use_subwindow)
		GridFlow.post "%s", [wid,use_subwindow].inspect
		@use_subwindow = use_subwindow==0 ? false : true
		#@fobjects[3].send_in 0, :open,:x11,:here
		if @use_subwindow then
			@fobjects[3].send_in 0, :open,:x11,:here,:embed_by_id,wid
		end
	end
	def _0_set_geometry(y,x,sy,sx)
		#GridFlow.post "_0_set_geometry %d %d %d %d",id,y,x,sy,sx
		@sy,@sx = sy,sx
		@y,@x = y,x
		set_geometry_for_real_now
	end
	def _0_fall_thru(flag)
		GridFlow.post "fall_thru: #{flag}"
		@fobjects[3].send_in 0, :fall_thru, flag
	end
	# note: the numbering here is a FPatcher gimmick... -1,0 goes to _1_.
	def _1_position(y,x,b)
		if @down then
			send_out 0,:position,y*@scale,x*@scale,b
		else
			send_out 0,:position,y/@scale,x/@scale,b
		end
	end
	def _2_list(sy,sx,chans)
		@fy,@fx = sy,sx
		set_geometry_for_real_now
	end
	def _0_paint()
		GridFlow.post "paint()"
		@fobjects[3].send_in 0, "draw"
	end
	def delete
		GridFlow.post "deleting peephole"
		GridFlow.whatever :gui, %{
			#{@can} delete #{@rsym}
		}
		@fobjects[3].send_in 0, :close
		super
	end

	def method_missing(s,*a)
		#GridFlow.post "%s: %s", s.to_s, a.inspect
		super rescue NameError
	end

	install "#peephole", 1, 1
end

#-------- fClasses for: Hardware

if const_defined? :USB

class<<USB
	attr_reader :busses
end

class DelcomUSB < GridFlow::FObject
	Vendor,Product=0x0FC5,0x1222
	def self.find
		r=[]
		USB.busses.each {|dir,bus|
			bus.each {|dev|
				r<<dev if dev.idVendor==Vendor and dev.idProduct==Product
			}
		}
		r
	end
	def initialize #(bus=nil,dev=nil)
		r=DelcomUSB.find
		raise "no such device" if r.length<1
		raise "#{r.length} such devices (which one???)" if r.length>1
		@usb=USB.new(r[0])
		if_num=nil
		r[0].config.each {|config|
			config.interface.each {|interface|
				if_num = interface.bInterfaceNumber
			}
		}
		# GridFlow.post "Interface # %i\n", if_num
		@usb.set_configuration 1
		@usb.claim_interface if_num
		@usb.set_altinterface 0 rescue ArgumentError
	end
	# libdelcom had this:
        # uint8 recipient, deviceModel, major, minor, dataL, dataM;
        # uint16 length; uint8[8] extension;
	def _0_send_command(major,minor,dataL,dataM,extension="\0\0\0\0\0\0\0\0")
		raise "closed" if not @usb
		raise "extension.length!=8" if extension.length!=8
		@usb.control_msg(
			0x000000c8, 0x00000012,
			minor*0x100+major,
			dataM*0x100+dataL,
			extension, 5000)
	end
	def delete; @usb.close; end
	install "delcomusb", 1, 1
end

# Klippeltronics
class IOBox < GridFlow::FObject
	Vendor,Product=0xDEAD,0xBEEF
	def self.find
	  r=[]
	  USB.busses.each {|dir,bus|
	    bus.each {|dev|
	      GridFlow.post "dir=%s, vendor=%x, product=%x",
		      dir, dev.idVendor, dev.idProduct
	      r<<dev if dev.idVendor==Vendor and dev.idProduct==Product
	    }
	  }
	  r
	end
	def initialize
		r=self.class.find
		raise "no such device" if r.length<1
		raise "#{r.length} such devices (which one???)" if r.length>1
		$iobox=@usb=USB.new(r[0])
		if_num=nil
		r[0].config.each {|config|
			config.interface.each {|interface|
				#GridFlow.post "interface=%s", interface.to_s
				if_num = interface.bInterfaceNumber
			}
		}
		# GridFlow.post "Interface # %i\n", if_num
		# @usb.set_configuration 0
		@usb.claim_interface if_num
		@usb.set_altinterface 0 rescue ArgumentError
	end
	#@usb.control_msg(0b10100001,0x01,0,0,"",1000)
	#@usb.control_msg(0b10100001,0x01,0,1," ",0)
	def delete; @usb.close; end
	install "iobox", 1, 1
end

end # if const_defined? :USB

# requires Ruby 1.8.0 because of bug in Ruby 1.6.x
class JoystickPort < FObject
  def initialize(port)
    raise "sorry, requires Ruby 1.8" if RUBY_VERSION<"1.8"
    @f = File.open(port.to_s,"r+")
    @status = nil
    $tasks[self] = proc {tick}
    @f.nonblock=true
  end
  def delete; $tasks.delete(self); @f.close end
  def tick
    loop{
      begin
        event = @f.read(8)
      rescue Errno::EAGAIN
        return
      end
      return if not event
      return if event.length<8
      send_out 0, *event.unpack("IsCC")
    }
  end
  install "joystick_port", 0, 1
end

end # module GridFlow
