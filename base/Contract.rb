#!/usr/bin/env ruby
# $Id$
=begin

	MetaRuby
	file: Contract & Assertions

	Copyright (c) 2001 by Mathieu Bouchard
	Licensed under the same license as Ruby.

=end
module GridFlow
#--------------------------------------------------------------------------#
# This was really itching me.
# This is the best spot to put it for now.

unless Object.const_defined? :Boolean
	module Boolean; end
	[TrueClass,FalseClass].each{|x| x.instance_eval { include Boolean }}
end

#--------------------------------------------------------------------------#
# this was called CommonAssert long ago.
# this is a bunch of tools to write unit-tests and contracts.

module Contract

	def assert_type(name,value,type)
		type===value or raise TypeError,
			"%s=%s is of type %s, not %s"%[name,value.inspect,value.type,type]
	end

	def assert_range(name,value,range)
		range===value or raise TypeError,
			"%s=%s is not in range %s" % [name,value,range]
	end

	def assert_nonneg(name,value)
		value >= 0 or raise TypeError,
			"%s = %s is negative" % [name,value]
	end

	def assert_index_excl(i); assert_range("i",i,0...length); end
	def assert_index_incl(i); assert_range("i",i,0..length); end
	def assert_extent(i,n); assert_range("%d+%d"%[i,n],i+n,0..length); end

	def assert_eq(name,value,expected)
		value == expected or \
			raise "%s = %s, expected %s" % [name, value, expected]
	end
	# private :assert_i, :assert_n, :assert_i_n

	def assert_not_frozen
#		frozen? and raise TypeError, "can't modify frozen array"
		frozen? and raise "can't modify frozen array"
	end
end

end
