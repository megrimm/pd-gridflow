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

# ENV["RUBY_VERBOSE_GC"]="yes"

# this file gets loaded by main.c upon startup
# module GridFlow is supposed to be created by main.c
# this includes GridFlow.post_string(s)

#class NotImplementedError
#	def initialize(*)
#		#GridFlow.post "HELLO"
#		Process.kill $$, 6
#	end
#end

#verbose,$VERBOSE=$VERBOSE,false
for victim in [TCPSocket, TCPServer]
	def victim.new
		raise NotImplementedError, "upgrade to Ruby 1.6.6 "+
		"(disabled because of bug in threadless sockets)"
	end
end if VERSION < "1.6.6"

# this should be done in base/bridge_jmax.c
for victim in [Thread, Continuation]
	def victim.new
		raise NotImplementedError, "class #{self} disabled because of jMax incompatibility"
	end
end
#$VERBOSE=verbose

require "gridflow/base/MainLoop.rb"
$mainloop = MainLoop.new
$tasks = {}

module GridFlow
	def esmtick
		$esm.tick
		$mainloop.timers.after(0.1) { esmtick }
	end
	def start_eval_server
		require "gridflow/extra/eval_server.rb"
		$esm = EvalServerManager.new
		esmtick
	end
end

#$post_log = File.open "/tmp/gridflow.log", "w"
$post_log = nil

module GridFlow #------------------

def self.post(s,*a)
	post_string(sprintf("%s"+s,post_header,*a))
	($post_log << sprintf(s,*a); $post_log.flush) if $post_log
end

class<<self
	attr_accessor :post_header
	attr_accessor :verbose
	attr_reader :alloc_set
	attr_reader :fobjects_set
	alias gfpost post
end

@verbose=false

self.post_header = "[gf] "

def self.gfpost2(fmt,s); post("%s",s) end

self.post "This is GridFlow #{GridFlow::GF_VERSION} within Ruby version #{VERSION}"
self.post "base/main.c was compiled on #{GridFlow::GF_COMPILE_TIME}"
self.post "Please use at least 1.6.6 if you plan to use sockets" \
	if VERSION < "1.6.6"

def self.parse(m)
	m = m.gsub(/(\{|\})/," \\1 ").split(/\s+/)
	m.map! {|x| case x
		when Integer, Symbol; x
		when /^[+-]?[0-9]+$/; x.to_i
		when String; x.intern
		end
	}
	i=0
	while i<m.length
		if m[i]=="{".intern
			j=i+1
			j+=1 until m[j]=="}".intern
			a = [m[i+1..j-1]]
			m[i..j] = a
		else
			i+=1
		end
	end
	m
end

# adding some functionality to that:
class FObject
	attr_writer :args # String
	attr_reader :outlets
	def args
		if defined? @args
			@args
		else
			"[#{self.class} ...]"
		end
	end
	def connect outlet, object, inlet
		#puts "connect: #{self} #{outlet} #{object} #{inlet}"
		@outlets ||= []
		@outlets[outlet] ||= []
		@outlets[outlet].push [object, inlet]
	end
	def send_in(inlet, *m)
		m=GridFlow.parse(m[0]) if m.length==1 and m[0] =~ / /
		sym = if m.length==0 then :bang
		elsif Symbol===m[0] then m.shift
		elsif String===m[0] then m.shift.intern
		elsif m.length>1 then :list
		elsif Integer===m[0] then :int
		elsif Float===m[0] then :float
		elsif Array===m[0] then m=m[0];:list
		else raise "don't know how to deal with #{m.inspect}"
		end
		GridFlow.post "%s",m.inspect if GridFlow.verbose
		send("_#{inlet}_#{sym}".intern,*m)
	end
	def self.name_lookup sym
		qlasses = GridFlow.instance_eval{@fclasses_set}
		qlass = qlasses[sym.to_s]
		raise "object class '#{sym}' not found" if not qlass
		qlass
	end
	def self.[](*m)
		o=nil
		if m.length==1 and m[0] =~ / /
			o="[#{m[0]}]"
			m=GridFlow.parse(m[0]) 
		else
			o=m.inspect
		end
		sym = m.shift
		sym = sym.to_s if Symbol===sym
		qlass = name_lookup sym
		GridFlow.post "%s",o.inspect if GridFlow.verbose
		r = qlass.new(*m)
		r.args = o
		r
	end
	def inspect
		if args then "#<#{self.class} #{args}>" else super end
	end
	def initialize
		super
		self.args ||= "[#{self.class} ;-)]"
	end
end

class FPatcher < FObject
	def initialize(fobjects,wires,ninlets)
		@fobjects = fobjects.map {|x| if String===x then FObject[x] else x end }
		@inlets = []
		@ninlets = ninlets
		i=0
		@fobjects << self
		while i<wires.length do
			a,b,c,d = wires[i,4]
			if a==-1 then
				a=self
				@inlets[b]||=[]
				@inlets[b] << [@fobjects[c],d]
			else
				if c==-1 then
					@fobjects[a].connect b,self,d+ninlets
				else
					@fobjects[a].connect b,@fobjects[c],d
				end
			end
			i+=4
		end
	end
	def method_missing(sym,*args)
		sym=sym.to_s
		if sym =~ /^_(\d)_(.*)/ then
			inl = Integer $1
			sym = $2.intern
			if inl<@ninlets then
				for x in @inlets[inl] do
				 x[0].send_in x[1],sym,*args end
			else
				send_out(inl-@ninlets,sym,*args)
			end
		else super end
	end
end

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

# FObject features I want to see:
#
#	self.ninlets #=> 3
#	self.noutlets #=> 1
#	self.external_name #=> "rubyfor"

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
		if name then @name = name+": " else @name="" end
	end

	def make_columns data
		min = data.unpack("l*").min
		max = data.unpack("l*").max
		@columns = [sprintf("%d",min).length,sprintf("%d",max).length].max
	end

	def _0_rgrid_begin; @dim = inlet_dim 0; @data = ""; end
	def _0_rgrid_flow data; @data << data end
	def _0_rgrid_end
		make_columns @data
		GridFlow.post("#{name}Dim[#{@dim.join','}]: " + case @dim.length
		when 0,1; dump @data
		when 2;   ""
		else      "(not printed)"
		end)
		case @dim.length
		when 2
			sz = @data.length/@dim[0]
			for row in 0...@dim[0]
				GridFlow.post dump(@data[sz*row,sz])
			end
		end
	end
	def dump data
		# data.unpack("l*").join(",")
		f = "%#{@columns}d"
		data.unpack("l*").map{|x| sprintf f,x }.join(" ")
	end
	install_rgrid 0
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
			def _#{i}_int x; @data[#{i}]=x; trigger; end
			def _#{i}_float x; @data[#{i}]=x.to_i; trigger; end
		"
	end
	def trigger
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

# a dummy class that gives access to any stuff global to GridFlow.
class GridGlobal < FObject
	def _0_profiler_reset
		GridFlow.fobjects_set.each {|o,*| o.profiler_cumul = 0 }
	end
	def _0_profiler_dump
		GridFlow.post "sorry, the profiler is broken. please someone fix it."
		return

		ol = []
		total=0
		GridFlow.post "-"*32
		GridFlow.post "         clock-ticks percent pointer  constructor"
		GridFlow.fobjects_set.each {|o,*| ol.push o }

		# HACK: BitPacking is not a real gridobject
		ol.delete_if {|o| not o.respond_to? :profiler_cumul }

		ol.sort {|a,b| a.profiler_cumul <=> b.profiler_cumul }
		ol.each {|o| total += o.profiler_cumul }
		total=1 if total<1
		ol.each {|o|
			ppm = o.profiler_cumul * 1000000 / total
			GridFlow.post "%20d %2d.%04d %08x %s",
				o.profiler_cumul, ppm/10000, ppm%10000,
				o.id, o.args
		}
		GridFlow.post "-"*32
	end

	install "@global", 1, 1
end

class FPS < GridObject
	def initialize(*options)
		@history = []   # list of delays between incoming messages
		@last = 0.0     # when was last time
		@duration = 0.0 # how much delay since last summary
		@period = 1     # minimum delay between summaries
		@detailed = false
		@mode = :real
		options.each {|o|
			case o
			when :detailed; @detailed=true
			when :user,:system,:cpu; @mode=o
			end
		}
		!!detailed
		def @history.moment(n=1)
			sum = 0
			each {|x| sum += x**n }
			sum/length
		end
		if @mode==:cpu
			u0,t0=GridFlow.rdtsc,Time.new.to_f
			sleep .01
			u1,t1=GridFlow.rdtsc,Time.new.to_f
			@hertz = (u1-u0)/(t1-t0)
			GridFlow.post "estimating cpu clock at #{@hertz} Hz"
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
		when :cpu; GridFlow.rdtsc/@hertz
		end
		@history.push t-@last
		@duration += t-@last
		@last = t
		return if @duration<@period
		n=@history.length
		fps = n/@duration
		@duration = 0
		if fps>.001 then
			if @detailed
				@history.sort!
				send_out 0, fps,
					1000*@history.min,
					500*(@history[n/2]+@history[(n-1)/2]),
					1000*@history.max,
					1000/fps,
					1000*(@history.moment(2) - @history.moment(1)**2)**.5
			else
				send_out 0, fps
			end
		end
		@history.clear
	end
	install "fps", 1, 1
end

#class RtMetro < GridFlow::FObject
#	attr_accessor 
#
#end

class RGBtoGreyscale < FPatcher
  FObjects = ["@ * {77 151 28}","@fold +","@outer >> {8}"]
  Wires = [-1,0,0,0, 0,0,1,0, 1,0,2,0, 2,0,-1,0]
  def initialize() super(FObjects,Wires,1) end
  install "@rgb_to_greyscale", 1, 1
end

class GreyscaleToRGB < FPatcher
  FObjects = ["@fold +","@outer + {0 0 0}"]
  Wires = [-1,0,0,0, 0,0,1,0, 1,0,-1,0]
  def initialize() super(FObjects,Wires,1) end
  install "@greyscale_to_rgb", 1, 1
end

class GridComplexSq < FPatcher
  FObjects = ["@inner2 * + 0 {2 2 2 # 0 1 2 0 1 1 -1 1}","@fold * 1"]
  Wires = [-1,0,0,0, 0,0,1,0, 1,0,-1,0]
  def initialize() super(FObjects,Wires,1) end
  install "@complex_sq", 1, 1
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

# linear solarization
class GridSolarize < FPatcher
	FObjects = ["@ & 255","@ << 1","@ inv+ 255","@! abs","@ inv+ 255"]
	Wires = [4,0,-1,0]
	for i in 0..4 do Wires.concat [i-1,0,i,0] end
	def initialize; super(FObjects,Wires,1) end
	install "@solarize", 1, 1
end		

=begin
class GridApplyColormap < FPatcher
	FObjects = ["@outer + {0}","@store"]
	Wires = [-1,1,2,1,1,0,-1,0]
	for i in 0..1 do Wires.concat [i-1,0,i,0] end
	def initialize; super(FObjects,Wires,2) end
	install "@apply_colormap", 2, 1
end
=end

class GridApplyColormapChannelwise < FPatcher
	FObjects = ["@outer & {-1 0}","@ + {3 2 # 0 0 0 1 0 2}","@store"]
	Wires = [-1,1,2,1,2,0,-1,0]
	for i in 0..2 do Wires.concat [i-1,0,i,0] end
	def initialize; super(FObjects,Wires,2) end
	install "@apply_colormap_channelwise", 2, 1
end

def self.routine
#	post "hello"
	$tasks.each {|k,v|
#		puts "#{k} #{v}"
		case v
		when Integer; GridFlow.exec k,v
		when Proc; v[k]
		else raise "problem"
		end
	}
	# mainloop is not used as much as it could.
	# it should eventually play a more central role,
	# but for now I rely on GridFlow.routine
	$mainloop.timers.after(0.025) { routine }
#	if not @nextgc or Time.new > @nextgc then
#		GC.start
#		@nextgc = Time.new + 5
#	end
end

def GridFlow.find_file s
#	post "find_file: #{s}"
	s
end

def GridFlow.tick
	$mainloop.one(0)
#	self.routine
#	GC.start
rescue Exception => e
	GridFlow.post "ruby #{e.class}: #{e}:\n" + e.backtrace.join("\n")
end

end # module GridFlow
#----------------------------------------------------------------#

#!@#$ this should be done _after_ formats/main.rb is loaded
user_config_file = ENV["HOME"] + "/.gridflow_startup"
load user_config_file if File.exist? user_config_file

END {
#	puts "This is an END block"
	GridFlow.fobjects_set.each {|k,v| k.delete if k.respond_to? :delete }
	GridFlow.fobjects_set.clear
	GC.start
}

GridFlow.routine

# set_trace_func proc {|a| STDERR.puts  }
