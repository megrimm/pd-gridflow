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

# this should be done in base/bridge.c
for victim in [Thread, Continuation]
	def victim.new
		raise NotImplementedError, "class #{self} disabled because of jMax incompatibility"
	end
end
#$VERBOSE=verbose

# because Ruby1.6 has no #object_id and Ruby1.8 warns on #id
unless Object.instance_methods(true).include? "object_id"
	class Object
		alias object_id id
	end
end

require "gridflow/base/MainLoop.rb"
$mainloop = MainLoop.new
$tasks = {}

#$post_log = File.open "/tmp/gridflow.log", "w"
$post_log = nil

class Array
	def split(elem)
		r=[]
		j=0
		for i in 0...length
			(r<<self[j,i-j]; j=i+1) if self[i]==elem
		end
		r<<self[j,length-j]
	end
end

module GridFlow #------------------

def self.esmtick
	$esm.tick
	$mainloop.timers.after(0.1) { esmtick }
end

def self.start_eval_server
	require "gridflow/extra/eval_server.rb"
	$esm = EvalServerManager.new
	esmtick
end

def self.post(s,*a)
	post_string(sprintf("%s"+s,post_header,*a))
	($post_log << sprintf(s,*a); $post_log.flush) if $post_log
end

class<<self
	attr_accessor :data_path
	attr_accessor :post_header
	attr_accessor :verbose
	attr_reader :alloc_set
	attr_reader :fobjects_set
	attr_reader :cpu_hertz
	attr_reader :subprocesses
	alias gfpost post
end

@subprocesses={}
@verbose=false
@data_path=[]

def self.hunt_zombies
	#STDERR.puts "GridFlow.hunt_zombies"
	# the $$ value is bogus
	begin
		died = []
		subprocesses.each {|x,v|
			Process.waitpid2(x,Process::WNOHANG) and died<<x
		}
	rescue Errno::ECHILD
	end
	STDERR.puts died.inspect
	died.each {|x| subprocesses.delete x }
end

self.post_header = "[gf] "

def self.gfpost2(fmt,s); post("%s",s) end

self.post "This is GridFlow #{GridFlow::GF_VERSION} within Ruby version #{VERSION}"
self.post "base/main.c was compiled on #{GridFlow::GF_COMPILE_TIME}"
self.post "Please use at least 1.6.6 if you plan to use sockets" \
	if VERSION < "1.6.6"

Brace1 = "{".intern
Brace2 = "}".intern
Paren1 = "(".intern
Paren2 = ")".intern

def self.parse(m)
	m = m.gsub(/(\{|\})/," \\1 ").split(/\s+/)
	m.map! {|x| case x
		when Integer, Symbol; x
		when /^[+\-]?[0-9]+$/; x.to_i
		when String; x.intern
		end
	}
	m
end

def self.stringify_list(argv)
	argv.map {|x| stringify x }.join(" ")
end

def self.stringify(arg)
	case arg
	when Integer, Float, Symbol; arg.to_s
	when Array; "{#{stringify_list arg}}"
	end
end

# adding some functionality to that:
class FObject
	@broken_ok = false
	class<<self
		# global
		attr_accessor :broken_ok
		# per-class
		attr_reader :ninlets
		attr_reader :noutlets
	end

	alias :total_time :total_time_get
	alias :total_time= :total_time_set
	attr_writer :args # String
	attr_accessor :argv # Array
	attr_reader :outlets
	attr_accessor :parent_patcher
	attr_accessor :properties
	attr_accessor :classname
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
	def self.name_lookup sym
		qlasses = GridFlow.instance_eval{@fclasses_set}
		qlass = qlasses[sym.to_s]
		if not qlass
			return qlasses['broken'] if @broken_ok
			raise "object class '#{sym}' not found"
		end
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
		GridFlow.handle_braces!(m)
		ms = m.split ','.intern
		m = ms.shift
		qlass = m.shift
		qlassname = qlass.to_s
		qlass = name_lookup qlass.to_s unless Class===qlass
		r = qlass.new(*m)
		GridFlow.post "%s",r.args if GridFlow.verbose
		r.classname = qlassname
		for x in ms do r.send_in(-2, *x) end
		r
	end
	def inspect
		if args then "#<#{self.class} #{args}>" else super end
	end

	def initialize(*argv)
		s = GridFlow.stringify_list argv
		@argv = argv
		@args = "["
		@args << (self.class.instance_eval{
			@foreign_name if defined? @foreign_name} ||
			self.class.to_s)
		@args << " " if s.length>0
		@args << s << "]"
		@parent_patcher = nil
		@properties = {}
		@init_messages = []
	end
end

class FPatcher < FObject
	class << self
		attr_reader :fobjects
		attr_reader :wires
	end
	def initialize(*)
		super
		fobjects = self.class.fobjects
		wires = self.class.wires
		@fobjects = fobjects.map {|x| if String===x then FObject[x] else x end }
		@inlets = []
		@ninlets = self.class.ninlets or raise "oops"
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
					@fobjects[a].connect b,self,d+@ninlets
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

def GridFlow.estimate_cpu_clock
	u0,t0=GridFlow.rdtsc,Time.new.to_f
	sleep 0.01
	u1,t1=GridFlow.rdtsc,Time.new.to_f
	(u1-u0)/(t1-t0)
end

begin
	@cpu_hertz = (0...3).map {
		GridFlow.estimate_cpu_clock
	}.sort[1] # median of three tries
rescue
	GridFlow.post $!
end

def self.routine
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
	s=s.to_s
	if s==File.basename(s) then
		dir = GridFlow.data_path.find {|x| File.exist?("#{x}/#{s}") }
		if dir then "#{dir}/#{s}" else s end
	else
		s
	end
end

def GridFlow.tick
begin
	$mainloop.one(0)
#	self.routine
#	GC.start
rescue Exception => e
	GridFlow.post "ruby #{e.class}: #{e}:\n" + e.backtrace.join("\n")
end end

end # module GridFlow

#----------------------------------------------------------------#

def GridFlow.load_user_config
	require "gridflow/bridge/puredata.rb" if GridFlow.bridge_name == "puredata"
	user_config_file = ENV["HOME"] + "/.gridflow_startup"
	begin
		load user_config_file if File.exist? user_config_file
	rescue Exception => e
		GridFlow.post "ruby #{e.class}: #{e}:\n" + e.backtrace.join("\n")
		GridFlow.post "while loading ~/.gridflow_startup"
	end
end

END {
	puts "This is an END block"
	GridFlow.fobjects_set.each {|k,v| k.delete if k.respond_to? :delete }
	GridFlow.fobjects_set.clear
	GC.start
}

GridFlow.routine

require "gridflow/base/flow_objects.rb"
