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

require "gridflow/base/MainLoop.rb"
$mainloop = MainLoop.new
$tasks = {}

module GridFlow
	GF_VERSION = "0.6.1"
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
	$whine_log = File.open "/tmp/gridflow.log", "w"
end

p GridFlow::GridOuter.instance_methods

def GridFlow.post(s,*a)
#	printf s,*a
	GridFlow.post_string(sprintf s,*a)
	($whine_log << (sprintf s,*a); $whine_log.flush) if $whine_log
end

class<<GridFlow; attr_accessor :whine_header end
GridFlow.whine_header = "[whine] "

def GridFlow.whine2(fmt,s)
	@whine_last ||= ""
	@whine_count ||= 0

	if @whine_last==fmt
		@whine_count+=1
		#if @whine_count >= 64 and @whine_count/(@whine_count&-@whine_count)<4
		#	post "[too many similar posts. this is # %d]\n", @whine_count
		#	return
		#end
	else
		@whine_last = fmt
		@whine_count += 1
	end

	s<<"\n" if s[-1]!=10
	post(GridFlow.whine_header)
	post(s)
end

# a slightly friendlier version of post(...)
# it removes redundant messages.
# it also ensures that a \n is added at the end.
def GridFlow.whine(fmt,*a)
	fmt=fmt.to_s
	whine2(fmt,(sprintf fmt, *a))
end

GridFlow.whine "This is GridFlow 0.6.0 within Ruby version #{VERSION}"
GridFlow.whine "Please use at least 1.6.6 if you plan to use sockets" \
	if VERSION < "1.6.6"

GridFlow.whine "hello???"
GridFlow.whine GridFlow.constants.inspect

for victim in [TCPSocket, TCPServer]
	def victim.new
		raise NotImplementedError, "upgrade to Ruby 1.6.6 "+
		"(disabled because of bug in threadless sockets)"
	end
end if VERSION < "1.6.6"

for victim in [Thread, Continuation]
	def victim.new
		raise NotImplementedError, "disabled because of jMax incompatibility"
	end
end

# simple dataflow engine mixin for Ruby
#module GridFlow; module RubyFlow
#	def initialize
#		...
#		super
#	end
#	...
#end end

# dual system that can work with both Ruby dataflow and jMax dataflow
#module GridFlow; class DFObject; include RubyFlow
#	...
#end end

def GridFlow.parse(m)
	m = m.split(/\s+/)
	m.map! {|x| case x
		when Integer, Symbol; x
		when /^[0-9]+$/; x.to_i
		when String; x.intern
		end
	}
	m
end

# adding some functionality to that:
module GridFlow; class FObject
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
		elsif m.length>1 and String===m[0]
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
		send("_#{inlet}_#{sym}".intern,*m)
	end
	def self.[](*m)
		m=GridFlow.parse(m[0]) if m.length==1 and m[0] =~ / /
		sym = m.shift
		sym = sym.to_s if Symbol===sym
		p sym
		qlass = case sym
			when "@"; GridFlow::GridOp2
			when "@!"; GridFlow::GridOp1
			when /^@/; GridFlow.const_get("Grid"+
				("_"+sym[1..-1]).gsub("_[a-z]") {|s| s[1..1].upcase})
			else
				raise ArgumentError, "GF names begin with @"
			end
		qlass.new(*m)
	end
end end

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

# FObject features I want to see:
#
#	self.ninlets #=> 3
#	self.noutlets #=> 1
#	self.external_name #=> "rubyfor"
end

#class RtMetro < GridFlow::FObject
#	attr_accessor 
#
#end

#----------------------------------------------------------------#

def routine
#	GridFlow.whine "hello"
	$tasks.each {|k,v|
		case v
		when Integer; GridFlow.exec k,v
		when Proc; v[k]
		else raise "problem"
		end
	}
	$mainloop.timers.after(0.025) { routine }
#	GC.start
#	GridFlow.whine "bye"
end

def GridFlow.find_file s
	whine "find_file: #{s}"
	s
end

user_config_file = ENV["HOME"] + "/.gridflow_startup"
load user_config_file if File.exist? user_config_file

routine
