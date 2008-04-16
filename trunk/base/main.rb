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
def self.post(s,*a) post_string(sprintf(s,*a)) end
class<<self
	attr_accessor :data_path
	attr_reader :fobjects
end
@data_path=[GridFlow::DIR+"/images"]
class ::Object; def FloatOrSymbol(x) Float(x) rescue x.intern end end
class FObject
	class << self
		# per-class
		def inspect; @pdname or super; end
		# should it recurse into superclasses?
		def gfattrs; @gfattrs={} if not defined? @gfattrs; @gfattrs end
		def gfattr(s,*v)
			s=s.intern if String===s
			gfattrs[s]=v
			attr_accessor s
			module_eval "def _0_#{s}(o) self.#{s}=o end"
		end
	end
	def self.help
		gfattrs.each{|x,v|
			s =      "attr=%-8s" % x
			s <<    " type=%-8s" % v[0] if v[0]
			s << " default=%-8s" % v[1] if v[1] # what default="false" ?
			GridFlow.post "%s", s
		}
		GridFlow.post "total %d attributes", gfattrs.length
	end
	def post(*a) GridFlow.post(*a) end
	def self.subclass(*args,&b)
		qlass = Class.new self
		qlass.install(*args)
		qlass.instance_eval{qlass.module_eval(&b)}
	end
	def initialize2; end
	def initialize(*) end
	def _0_help; self.class.help end
	def _0_get(s=nil)
		s=s.intern if String===s
		if s then
			if respond_to? s then send_out noutlets-1,s,__send__(s) else ___get s end
		else
			self.class.gfattrs.each_key{|k| _0_get k }
		end
	end
end
def GridFlow.find_file s
	s=s.to_s
	if s==File.basename(s) then
		dir = GridFlow.data_path.find {|x| File.exist? "#{x}/#{s}" }
		if dir then "#{dir}/#{s}" else s end
	else s end
end
end # module GridFlow
def GridFlow.load_user_config
	user_config_file = ENV["HOME"] + "/.gridflow_startup"
	begin
		load user_config_file if File.exist? user_config_file
	rescue Exception => e
		GridFlow.post "#{e.class}: #{e}:\n" + e.backtrace.join("\n")
		GridFlow.post "while loading ~/.gridflow_startup"
	end
end
class Object
  def method_missing(name,*)
    oc = GridFlow::FObject
    obj = (case obj; when oc; self.args; else self      .inspect end)
    qla = (case obj; when oc; self.args; else self.class.inspect end)
    raise NameError, "undefined method \"#{name}\" for #{obj} in class #{qla}"#, ["hello"]+e.backtrace
  end
end
