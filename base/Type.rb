=begin
	MetaRuby
	Copyright 2001 by Mathieu Bouchard

	Type.rb: Type System (related to RubySchema, RubyX11)

	$Id$

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=end

require "gridflow/base/Contract"
module GridFlow
#----------------------------------------------------------------#

Fast = (::ENV["X11_LOG"].to_i < 0)
DoCheckTypes = (not Fast)

def self.add(sym,object)
	const_set(sym,object)
	if object.respond_to? :const_name_is then
		object.const_name_is(sym,self)
	end
end

#----------------------------------------------------------------#

=begin module Type

	a Type object represents a set of potential and/or existing objects
	that share common characteristics. This is a more general concept than
	module or class, in that the thing that is of a certain type need not
	even know about it (so you can have a type for arbitrary subsets eg:
	odd Integers)

	include Type

		this declares a class as a metatype. Instances of it are types.

	extend Type

		this declares an object as a type.

	#===(object) ^Boolean

		The type-checking method.
		Is set inclusion predicate just like Module#===.


=end

module Type
	module Contract
		def ===(object)
			result = super
			assert_type("post: result",result,Boolean)
			result
		end
	end
end

[Module,Class].each {|c| c.class_eval { include Type }}

=begin module Template
	hmmm, revise this...
=end

module Template
	attr_reader :specializations
	def of(*args)
		((@specializations ||= {})[normalize_args(args)] ||=
			instantiate_template(*args))
	end
	def normalize_args(*args) args end
end

=begin module Nameable
=end

module Nameable
	attr_accessor :const_name
	def const_name_is(sym,holder); @const_name = "#{holder}::#{sym}"; end
	def inspect; to_s; end
	def to_s
		if const_name
			const_name
		elsif @template_of then
			"#{@template_of.to_s}.of(#{@template_by.map do|x|x.inspect end.join','})"
		else
			super
		end
	end
#	def dup; r=super; r.instance_eval{@const_name=nil}; r; end
end

#----------------------------------------------------------------#
# ChoiceTypes and MultiChoiceTypes

=begin class ChoiceTypeBase
=end

#!@#$ templatize
class ChoiceTypeBase; include Type, Nameable
	attr_reader :index_to_val, :val_to_index

	class<<self
		alias [] new
		alias of new
	end
	def initialize(*args)
		@val_to_index = {}
		if args.length==1 and Hash===args[0] then
			@index_to_val = args[0]
			@index_to_val.each {|i,v| @val_to_index[v] = i }
		else		
			@index_to_val = args
			@index_to_val.each_with_index {|v,i| @val_to_index[v] = i }
		end
		[@index_to_val,@val_to_index].each{|x|x.freeze}
	end

end

=begin class ChoiceType
	a ChoiceType object represents a set of symbols and a
	bijection of that set to integers 0...n. The mapping is
	applied back and forth during transmissions to the X11
	server.
	
	In C a choice is called an enum, but MIT-X11 uses macros instead.
=end

class ChoiceType < ChoiceTypeBase; #extend Template
#	class<<self; alias instantiate_template new; end
	def []    (v); @index_to_val[v]; end
	def lookup(v)
		@val_to_index[v] or
			(raise TypeError, "value #{v.inspect} not in type #{self}")
	end
	def ===(v); !! @val_to_index[v]; end
end

# MultiChoice objects are similar but instead of dealing with single
# elements, they deal with subsets. The power-set of the set has a
# bijection to integers 0...2**n. In X11 they are called masks; others
# call them bitsets.

# MultiChoice objects are directly integers, not lists of symbols,
# but you can get the underlying list of symbols.

=begin class MultiChoiceType
=end

class MultiChoiceType < ChoiceTypeBase; #extend Template
#	class<<self; alias instantiate_template new; end
	attr_accessor :int_type

	def initialize(*a)
		super
		@int_type = Uint32
		@range = 0...1<<@index_to_val.length
	end

	def lookup(*v)
		k=0
		v.each {|i|
			k |= 1 << (@val_to_index[i] ||
				(raise TypeError, "value #{i.inspect} not in type #{self}"))
		}; k
	end

	def ===(v)
		int_type===v && @range===v
	end
end

#----------------------------------------------------------------#
# Any, List

class Any; include Type, Nameable, Contract; extend Template
	class<<self
		alias instantiate_template new
	end
	def initialize(*alternatives)
		alternatives.each{|a| assert_type("a",a,Type) } if DoCheckTypes
		@alternatives = alternatives.freeze
	end
	def ===(o)
		!! @alternatives.find {|t| t===o }
	end
	def to_s
		if @const_name then
			super
		else
			"#{Any}.of(%s)" % [@alternatives.join(",")]
		end
	end
	alias inspect to_s
end

# type-template
class List; include Type, Nameable, Contract; extend Template
	attr_reader :element_type, :length, :options

	class<<self
		alias instantiate_template new
	end
	def initialize(element_type,length=nil,options=nil)
		@template_of = List
		@element_type,@length,@options = @template_by =
		[element_type, length, options]
		assert_type "element_type", element_type, Type if DoCheckTypes
		Integer===length or NilClass===length or raise "type error"
	end

	def ===(o)
		return false if not Array===o
		return false if not length and o.length == length
		o.each {|e|
			return false unless @element_type===e
		} if DoCheckTypes
		true
	end
end

#----------------------------------------------------------------#
# FormBase, Form, FormField

# subclasses must define #form.
class FormBase
	class<<self
		alias [] new
		attr_reader :fields
	end
	def method_missing(sym,*args)
		if self.class.compile_fields then send(sym) else super end
	end
	def to_s; "#{self.class}[#{form.join ', '}]"; end
	def inspect; "#{self.class}#{form.inspect}"; end
end

class Form < FormBase; extend Type; end

class<<Form # metaclass

	attr_reader :minimum_args

	def fields_are(*fs)
		raise "Redefinition of a Form class' fields" if @fields
		@fields = fs.map {|sym,type,opt| FormField[sym,type,opt] }
		@minimum_args = @fields.length
#		_{"@fields"}
#		_{"@minimum_args"}
		while @minimum_args > 0 and @fields[@minimum_args-1].has_default?
			@minimum_args -= 1
		end
		compile_fields
	end

	def compile_fields
		return false if @compiled_fields
		e=""
		fields.each_with_index {|f,i| e << "def #{f.sym};@form[#{i}]end;" }
		class_eval e
		@compiled_fields = true
	end

	def check_types(*args)
#		p self.fields
		args.length >= @minimum_args or raise ArgumentError,
			"wrong number of args: got #{args.length} "\
			"expecting #{fields.length} "\
			"while constructing a #{self}"
		args.each_with_index {|a,i|
			f = fields[i]
			if not f.vtype === a then
				raise TypeError, "for an #{self}, "\
					"supplied #{f.sym}=#{a.inspect} "\
					"is not in type #{f.vtype.inspect}"
			end
		}
		args
	end

	#!@#$ are those really useful?
	def field_names; fields.map{|f| f.sym   }; end
	def field_types; fields.map{|f| f.vtype }; end
end

class Form # first level

	def initialize(*args)
		# this part (the single Hash case) should be X11-specific.
		if args.length==1 and Hash===args[0] then
			h = args[0]
			f=nil
			args=[]
			for f in self.class.fields; args << h[f.sym]; end
		end
		self.class.check_types(*args) if DoCheckTypes
		while args.length < self.class.fields.length
			args << self.class.fields[args.length].options[:default]
		end
		@form = args
	end

	attr_reader :form # Array

	def [](i); @form[i]; end
	def hash; form.hash; end
	def to_s
#		pa = @form.length
#		while pa>self.class.minimum_args and @form[pa-1]==self.class.fields[pa-1].options[:default]
#			pa -= 1
#		end
#		"#{self.class}[#{@form[0...pa].join ', '}]"
		"#{self.class}[#{@form.join ', '}]"
	end

	def inspect; "#{self.class}#{@form.inspect}"; end
	def to_a  ; form.dup; end
	def to_ary; form.dup; end

	def each
		self.class.fields.each_with_index {|f,i| yield f.sym, @form[i] }
	end

	def to_h
		foo={}
		self.class.fields.each_with_index {|(sym,vtype),i| foo[sym]=form[i] }
		foo
	end

	def ==(other); self.class == other.class && form == other.form; end
	alias eql? ==
end

# this form class is not defined by the same means than the others;
# this is because of a chicken-and-egg issue.
#!@#$ really? something can be done here.
class Slot < Form
	class<<self; alias [] new; end
	def sym    ; @form[0]; end
	def vtype  ; @form[1]; end
	def options; @form[2]; end
	def initialize(sym,vtype,options=nil)
		@form = sym,vtype,options
	end
	@minimum_args = 3
	@fields = [
		Slot[:sym,Symbol],
		Slot[:vtype,Type],
		Slot[:options,Any.of(Hash,NilClass)],
	]

	def has_default?; @form[2] && @form[2].has_key?(:default); end
	def default; @form[2][:default]; end
end

# older class names
TupleField = FormField = Slot
TupleBase = FormBase
Tuple = Form

#----------------------------------------------------------------#
end # of module GridFlow
