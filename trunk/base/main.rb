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

# this file gets loaded by main.c upon startup
# module GridFlow is supposed to be created by main.c
# this includes GridFlow.post_string(s)

class NotImplementedError
	def initialize(*)
		GridFlow.gfpost "HELLO"
		Process.kill $$, 6
	end
end

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
		raise NotImplementedError, "disabled because of jMax incompatibility"
	end
end
#$VERBOSE=verbose

require "gridflow/base/MainLoop.rb"
$mainloop = MainLoop.new
$tasks = {}

module GridFlow
	GF_VERSION = "0.6.2"
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

if true
	$post_log = File.open "/tmp/gridflow.log", "w"
end

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
end

@verbose=false

self.post_header = "[gf] "

def self.gfpost2(fmt,s); post("%s",s) end
def self.gfpost(fmt,*a); post(fmt,*a) end

self.gfpost "This is GridFlow #{GridFlow::GF_VERSION} within Ruby version #{VERSION}"
self.gfpost "Please use at least 1.6.6 if you plan to use sockets" \
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
	alias profiler_cumul= profiler_cumul_assign
	attr_writer :args
	def args; @args || "[#{self.class} ...]"; end
	def connect outlet, object, inlet
		@outlets ||= []
		@outlets.push [object, inlet]
	end
	def send_in(inlet, *m)
		m=GridFlow.parse(m[0]) if m.length==1 and m[0] =~ / /
		sym = if m.length==0
			:bang
		elsif Symbol===m[0]
			m.shift
		elsif String===m[0]
			m.shift.intern
		elsif m.length>1 and String===m[0] #???
			m.shift.intern
		elsif m.length>1
			:list
		elsif Integer===m[0]
			:int
		elsif Float===m[0]
			:float
		else
			raise "don't know how to deal with #{m.inspect}"
		end
		p m if GridFlow.verbose
		send("_#{inlet}_#{sym}".intern,*m)
	end
	def self.name_lookup sym
		qlasses = GridFlow.instance_eval{@fclasses_set}
#		p qlasses
		qlass = qlasses[sym.to_s]
#		p qlass
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
		r = qlass.new(*m)
		r.args = o
		p o if GridFlow.verbose
		r
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
		if a.length and a[0]==:time then
			@time=true
		end	
	end

	def method_missing(s,*a)
		s=s.to_s
		pre = if @time then sprintf "%10.6f  ", Time.new.to_f else "" end
		case s
		when /^_0_/; GridFlow.gfpost "#{pre}#{s[3..-1]}: #{a.inspect}"
		else super
		end
	end

	install_rgrid 0
	install "rubyprint", 1, 0
end

class PrintArgs < GridFlow::GridObject
	def initialize(*a)
		GridFlow.gfpost(a.inspect)
	end
	install "printargs", 0, 0
end

class GridPrint < GridFlow::GridObject
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
		GridFlow.gfpost("#{name}dim(#{@dim.join','}): " + case @dim.length
		when 0,1; dump @data
		when 2;   ""
		else      "(not printed)"
		end)
		case @dim.length
		when 2
			sz = @data.length/@dim[0]
			for row in 0...@dim[0]
				GridFlow.gfpost dump(@data[sz*row,sz])
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

class GridGlobal
	def _0_profiler_reset
		GridFlow.fobjects_set.each {|o,*| o.profiler_cumul = 0 }
	end
	def _0_profiler_dump
		ol = []
		total=0
		GridFlow.gfpost "-"*32
		GridFlow.gfpost "         clock-ticks percent pointer  constructor"
		GridFlow.fobjects_set.each {|o,*| ol.push o }
		ol.sort {|a,b| a.profiler_cumul <=> b.profiler_cumul }
		ol.each {|o| total += o.profiler_cumul }
		total=1 if total<1
		ol.each {|o|
			int ppm = o.profiler_cumul * 1000000 / total
			GridFlow.gfpost "%20d %2d.%04d %08x %s",
				o.profiler_cumul, ppm/10000, ppm%10000,
				o.id, o.args
		}
		GridFlow.gfpost "-"*32
	end
end

#class RtMetro < GridFlow::FObject
#	attr_accessor 
#
#end

def self.routine
	$tasks.each {|k,v|
#		puts "#{k} #{v}"
		case v
		when Integer; GridFlow.exec k,v
		when Proc; v[k]
		else raise "problem"
		end
	}
	$mainloop.timers.after(0.025) { routine }
#	if not @nextgc or Time.new > @nextgc then
#		GC.start
#		@nextgc = Time.new + 5
#	end
end

def GridFlow.find_file s
	gfpost "find_file: #{s}"
	s
end

def GridFlow.tick
	$mainloop.one(0)
#	GC.start
rescue Exception => e
	GridFlow.gfpost "ruby #{e.class}: #{e}:\n" + backtrace.join("\n")
end

end # module GridFlow
#----------------------------------------------------------------#

user_config_file = ENV["HOME"] + "/.gridflow_startup"
load user_config_file if File.exist? user_config_file

END {
#	puts "This is an END block"
	GridFlow.fobjects_set.each {|k,v| k.delete }
	GridFlow.fobjects_set.clear
}

GridFlow.routine

#set_trace_func proc {|a| p a }
