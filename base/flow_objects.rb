=begin
	$Id$

	GridFlow
	Copyright (c) 2001,2002 by Mathieu Bouchard

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
	def _0_profiler_reset
		GridFlow.fobjects_set.each {|o,*| o.profiler_cumul = 0 }
		GridFlow.profiler_reset2 if GridFlow.respond_to? :profiler_reset2
	end
	def _0_profiler_dump
#		GridFlow.post "sorry, the profiler is broken. please someone fix it."
#		return

		ol = []
		total=0
		GridFlow.post "-"*32
		GridFlow.post "microseconds percent pointer  constructor"
		GridFlow.fobjects_set.each {|o,*| ol.push o }

		# HACK: BitPacking is not a real gridobject
		ol.delete_if {|o| not o.respond_to? :profiler_cumul }

		ol.sort! {|a,b| a.profiler_cumul <=> b.profiler_cumul }
		ol.each {|o| total += o.profiler_cumul }
		total=1 if total<1
		total_us = 0
		ol.each {|o|
			ppm = o.profiler_cumul * 1000000 / total
			us = (o.profiler_cumul*1E6/GridFlow.cpu_hertz).to_i
			total_us += us
			GridFlow.post "%12d %2d.%04d %08x %s", us,
				ppm/10000, ppm%10000, o.object_id, o.args
		}
		GridFlow.post "-"*32
		GridFlow.post "sum of accounted microseconds: #{total_us}"
		if GridFlow.respond_to? :memcpy_calls then
			GridFlow.post "memcpy_calls: #{GridFlow.memcpy_calls}"
			GridFlow.post "memcpy_bytes: #{GridFlow.memcpy_bytes}"
		end
		GridFlow.post "-"*32
	end

	install "@global", 1, 1
end

class FPS < GridObject
	@handlers = [] # for GridObject (BLETCH)
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
		!!detailed
		def @history.moment(n=1)
			sum = 0
			each {|x| sum += x**n }
			sum/length
		end
	end
	def method_missing(*)
		# ignore
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
		n=@history.length
		fps = n/@duration
		@duration = 0
		if fps>0.001 then
			if @detailed
				@history.sort!
				send_out 0, fps,
					1000*@history.min,
					500*(@history[n/2]+@history[(n-1)/2]),
					1000*@history.max,
					1000/fps,
					1000*(@history.moment(2) - @history.moment(1)**2)**0.5
			else
				send_out 0, fps
			end
		end
		@history.clear
	end
	install "fps", 1, 1
end

# to see what the messages look like when they get on the Ruby side.
class RubyPrint < GridFlow::GridObject
	def initialize(*a)
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

	install_rgrid 0
	install "rubyprint", 1, 0
end

class PrintArgs < GridFlow::GridObject
	def initialize(*a)
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
	end

	def _0_base(x)
		case x
		when 2; @format="b"
		when 8; @format="o"
		when 10; @format="d"
		when 16; @format="x"
		else raise "base #{x} not supported" end
		@base = x
	end

	def make_columns udata
		min = udata.min
		max = udata.max
		@columns = [
			sprintf("%#{@format}",min).length,
			sprintf("%#{@format}",max).length].max
	end

	def unpack data
		case @nt
		when :uint8; data.unpack("C*")
		when :int16; data.unpack("s*")
		when :int32; data.unpack("l*")
		when :float32; data.unpack("f*")
		else raise "#{self.class} doesn't know how to decode this"
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
			udata = unpack @data
			make_columns udata
			sz = udata.length/@dim[0]
			sz2 = sz/@dim[1]
			for row in 0...@dim[0]
				column=0; str=""
				for column in 0...@dim[1]
					str << "{" << dump(udata[sz*row+sz2*column,sz2]) << "} "
				end
				GridFlow.post trunc(str)
			end
		end
		@data,@dim,@nt = nil
	end

	def dump udata
		f = "%#{@columns}#{case @nt; when :float32; 'f'; else @format end}"
		udata.map{|x| sprintf f,x }.join(" ")
	end

	def trunc s
		if s.length>240 then s[0...240]+" [...]" else s end
	end

	install_rgrid 0, true
	install "@print", 1, 0
end

class GridPack < GridObject
	class<<self;attr_reader :ninlets;end
	def initialize
		super
		@data=[0]*self.class.ninlets
	end
	def self.define_inlet i
		module_eval "
			def _#{i}_int x; @data[#{i}]=x; _0_bang; end
			def _#{i}_float x; @data[#{i}]=x.to_i; _0_bang; end
		"
	end

	def _0_bang
		send_out_grid_begin 0, [self.class.ninlets]
		send_out_grid_flow 0, @data.pack("l*")
	end
end

# the install_rgrids in the following are hacks so that
# outlets can work. (install_rgrid is supposed to be for receiving)

class GridTwo < GridPack
	(0...2).each {|x| define_inlet x }
	install_rgrid 0
	install "@two", 2, 1
end
class GridThree < GridPack
	(0...3).each {|x| define_inlet x }
	install_rgrid 0
	install "@three", 3, 1
end
class GridFour < GridPack
	(0...4).each {|x| define_inlet x }
	install_rgrid 0
	install "@four", 4, 1
end

class GridExportSymbol < GridObject
	def _0_rgrid_begin; @data="" end
	def _0_rgrid_flow data; @data << data; end
	def _0_rgrid_end
		send_out 0, :symbol, @data.unpack("I*").pack("c*").intern
	end
	install_rgrid 0
	install "@export_symbol", 1, 1
end

#-------- fClasses for: math

class GridComplexSq < FPatcher
  FObjects = ["@inner2 * + 0 {2 2 2 # 0 1 2 0 1 1 -1 1}","@fold * 1"]
  Wires = [-1,0,0,0, 0,0,1,0, 1,0,-1,0]
  def initialize() super(FObjects,Wires,1) end
  install "@complex_sq", 1, 1
end

class GridRavel < FPatcher
	FObjects = ["@dim","@fold * 1","@redim {1}","@redim {42}"]
	Wires = [-1,0,0,0, 0,0,1,0, 1,0,2,0, 2,0,3,1, -1,0,3,0, 3,0,-1,0]
	def initialize; super(FObjects,Wires,1) end
	install "@ravel", 1, 1
end

#-------- fClasses for: video

# linear solarization
class GridSolarize < FPatcher
	FObjects = ["@ & 255","@ << 1","@ inv+ 255","@! abs","@ inv+ 255"]
	Wires = [4,0,-1,0]
	for i in 0..4 do Wires.concat [i-1,0,i,0] end
	def initialize; super(FObjects,Wires,1) end
	install "@solarize", 1, 1
end		

class GridCheckers < GridObject
	def initialize
		@chain =
		[
			"@ >> 3","@ & 1","@fold ^","@ inv+ 0","@ & 63",
			"@ + 128","@outer + {0 0 0}"
		].map {|o| FObject[o] }
		(0..@chain.length-2).each {|i| @chain[i].connect 0,@chain[i+1],0 }
		@chain[-1].connect 0,self,1
	end
	def _0_grid(*a) @chain[0]._0_grid(*a) end
	def _1_grid(*a) send_out(0, :grid, *a) end
	install "@checkers", 1, 1
end

class GridScaleTo < FPatcher
	FObjects = [
			"@for {0 0} {42 42} {1 1}",
			"@ *",
			"@ /",
			"@store",
			"@dim",
			"@redim {2}",
			"@finished",
	]
	Wires = []
	for i in 1..3 do Wires.concat [i-1,0,i,0] end
	Wires.concat [3,0,-1,0, 4,0,5,0, 5,0,1,1, 6,0,0,0,
		-1,0,4,0, -1,0,3,1, -1,0,6,0, -1,1,0,1, -1,1,2,1]
	def initialize(size)
		(size.length==2 and Numeric===size[0] and Numeric===size[1]) or
			raise "expecting {height width}"
		super(FObjects,Wires,2)
		send_in 1, size
	end
	install "@scale_to", 2, 1
end

class GridContrast < FPatcher
	FObjects = [
		"@ inv+ 255",
		"@ *",
		"@ >> 8",
		"@ inv+ 255",
		"@ *",
		"@ >> 8",
		"@ min 255",
		"@ max 0",
	] 
	Wires = []
	for i in 0..7 do Wires.concat [i-1,0,i,0] end
	Wires.concat [7,0,-1,0, -1,1,1,1, -1,2,4,1]
	def initialize(iwhiteness=256,contrast=256)
		super(FObjects,Wires,3)
		send_in 1, iwhiteness
		send_in 2, contrast
	end
	install "@contrast", 3, 1
end

class GridSpread < FPatcher
	FObjects = [
		"@ & 0",
		"@ + 5",
		"@! rand",
		"@ - 2",
		"@ +",
		"@ >> 1",
	]
	Wires = []
	for i in 0..3 do Wires.concat [i-1,0,i,0] end
	Wires.concat [3,0,4,1, -1,0,4,0, 4,0,-1,0, -1,1,1,1, -1,1,5,0, 5,0,3,1]
	def initialize(factor)
		super(FObjects,Wires,2)
		send_in 1, factor
	end
	install "@spread", 2, 1
end

class GridPosterize < FPatcher
	FObjects =  [
		"@ *",
		"@ >> 8",
		"@ * 255",
		"@ /",
		"@ - 1",
	]
	Wires = []
	for i in 0..3 do Wires.concat [i-1,0,i,0] end
	Wires.concat [3,0,-1,0, -1,1,0,1, -1,1,4,0, 4,0,3,1]
	def initialize(factor=2)
		super(FObjects,Wires,2)
		send_in 1, factor
	end
	install "@posterize", 2, 1
end

class RGBtoGreyscale < FPatcher
  FObjects = ["@ * {77 151 28}","@fold +","@outer >> {8}"]
  Wires = [-1,0,0,0, 0,0,1,0, 1,0,2,0, 2,0,-1,0]
  def initialize() super(FObjects,Wires,1) end
  install "@rgb_to_greyscale", 1, 1
end

class GreyscaleToRGB < FPatcher
  FObjects = ["@fold put 0","@outer ignore {0 0 0}"]
  Wires = [-1,0,0,0, 0,0,1,0, 1,0,-1,0]
  def initialize() super(FObjects,Wires,1) end
  install "@greyscale_to_rgb", 1, 1
end

class GridApplyColormapChannelwise < FPatcher
	FObjects = ["@outer & {-1 0}","@ + {3 2 # 0 0 0 1 0 2}","@store"]
	Wires = [-1,1,2,1,2,0,-1,0]
	for i in 0..2 do Wires.concat [i-1,0,i,0] end
	def initialize; super(FObjects,Wires,2) end
	install "@apply_colormap_channelwise", 2, 1
end

class GridRotate < FPatcher
  FObjects = ["@outer + {2 2 # 0 9000 -9000 0}","@ cos* 256",
  "@inner * + 0","@ >> 8"]
  Wires = [-1,0,2,0, 2,0,3,0, 3,0,-1,0, -1,1,0,0, 0,0,1,0, 1,0,2,2]
  def initialize(rot=0) super(FObjects,Wires,2); send_in 1, rot end
  install "@rotate", 2, 1
end

class GridRemapImage < FPatcher
  FObjects = ["@dim","@inner * + 0 {3 2 # 1 0 0}","@for {0 0} {0 0} {1 1}",
  "@store","@finished"]
  Wires = [-1,0,3,1, -1,0,0,0, 0,0,1,0, 1,0,2,1,
  -1,0,4,0, 4,0,2,0, 2,0,-1,1, -1,1,3,0, 3,0,-1,0]
  def initialize() super(FObjects,Wires,2) end
  install "@remap_image", 2, 2
end

#-------- fClasses for: jMax compatibility

# this is the demo and test for Ruby->jMax bridge
# FObject is a flow-object as found in jMax
# _0_bang means bang message on inlet 0
# FObject#send_out sends a message through an outlet
class RubyFor < GridFlow::FObject
	attr_accessor :start, :stop, :step
	def initialize(start,stop,step)
		raise ArgumentError, "@start not integer" unless Integer===start
		raise ArgumentError, "@stop not integer" unless Integer===stop
		raise ArgumentError, "@step not integer" unless Integer===step
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
	alias :_1_int :stop=
	alias :_2_int :step=

	# FObject.install(name, inlets, outlets)
	# no support for metaclasses yet
	install "rubyfor", 3, 1
end

class JMaxUDPSend < FObject
	def initialize(host,port)
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

	def delete
		@socket.close
	end

	install "jmax_udpsend", 1, 0
end

class JMaxUDPReceive < FObject
	def initialize(port)
		@socket = UDPSocket.new
		@port = port.to_i
		@socket.bind "localhost", @port
#		@socket.nonblock = true
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
#		GridFlow.post "recvfrom: before"
		data,sender = @socket.recvfrom 1024
#		GridFlow.post "recvfrom: after"
		return if not data
#		GridFlow.post "#{data.inspect}"
		send_out 1, sender.map {|x| x=x.intern if String===x; x }
		send_out 0, *(decode data)
	end

	def delete
		@socket.close
	end

	install "jmax_udpreceive", 0, 2
end

class Broken < FObject
	install "broken", 0, 0
end

if GridFlow.bridge_name != "jmax"
	class Fork < FObject
	  def method_missing(sel,*args)
	    sel.to_s =~ /^_(\d)_(.*)$/ or super
		send_out 1,$2.intern,*args
		send_out 0,$2.intern,*args
	  end
	  install "fork", 1, 2
	end
	class Demux < FObject
		N=2
		def initialize(n)
			super
			@n=n
			@i=0
			raise "sorry, maximum #{N}" if n>N
		end
		def method_missing(sel,*args)
			sel.to_s =~ /^_(\d)_(.*)$/ or super
			send_out @i,$2.intern,*args
		end
		def _1_int i; @i=i end
		alias :_1_float :_1_int
		install "demux", 2, N
	end
end

if GridFlow.bridge_name == nil
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
		def initialize
			super
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
end # if "jmax"

end # module GridFlow
