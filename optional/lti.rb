=begin
	$Id$

	LTIlib-for-GridFlow
	Copyright (c) 2006 by Mathieu Bouchard

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

require "rblti"
include GridFlow
include Rblti

class LTI<FObject; install "lti",1,1
  class << self
    attr_accessor:functors
    attr_accessor:parameterses
    attr_accessor:others
  end
  @functors=[]
  @parameterses=[]
  cs=Rblti.constants
  cs.each{|c|
    next unless /^R(.)(\w+)_parameters/ =~ c
    @parameterses<<c
    @functors<<$1.upcase+$2
  }
  @others = (cs-(@functors+@parameterses)).sort
  def _0_help(a=nil) self.class.help(a) end
  def self.help(a=nil)
    case a
    when nil:
      GridFlow.post "try one of:"
      GridFlow.post "  help functors"
      GridFlow.post "  help others"
    when:functors:
      @functors.each{|x| GridFlow.post "  functor %s", x }
      GridFlow.post "total %d functors",@functors.length
    when:others:
      @others.each{|x| GridFlow.post "  other %s", x }
      GridFlow.post "total %d others",@others.length
    end
  end
  def LTI.to_pd datum
    case datum
    when String: datum.intern
    when Float,Integer; datum
    when true: 1
    when false: 0
    else raise "LTI.to_pd can't convert #{datum}, a #{datum.class}"
    end
  end
end

class LTIGridObject < GridObject

end

LTI.functors.each {|name|
  LTIGridObject.subclass("lti."+name,1,2) {
    class << self
      attr_accessor  :param_class
      attr_accessor:functor_class
      attr_accessor:attrs
      attr_accessor:writers
    end

    @functor_class = const_get name
    @param_class = Rblti.const_get("R"+
	name[0..0].downcase+
	name[1..-1]+"_parameters")
    @writers = param_class.instance_methods.grep(/\w=$/)
    @attrs = writers.map{|x|x.chop}

    def _0_help() self.class.help end
    def self.help()
      @attrs.each{|x| GridFlow.post "attribute %s",x}
      GridFlow.post "total %d attributes", @attrs.length
      #---
      pmo=param_class.instance_methods-attrs-writers-self.attrs-Object.instance_methods
      pmo.each{|x| GridFlow.post "other param method %s",x}
      GridFlow.post "total %d other param methods", pmo.length
      #---
      fm=functor_class.instance_methods-Object.instance_methods
      fm.each{|x| GridFlow.post "functor method %s",x}
      GridFlow.post "total %d functor methods", fm.length
    end
    def initialize
      c=self.class
      @functor = c.functor_class.new
      @param   = c.  param_class.new
    end
    def _0_get(sel=nil)
      return @param.__send__(sel) if sel
      self.class.attrs.each {|sel|
        v=_0_get(sel)
        begin
          send_out 1, sel.intern, LTI.to_pd(v)
	rescue StandardError=>e
	  GridFlow.post "%s", e.inspect
	  send_out 1, sel.intern, :some, v.class.to_s.intern
	end
      }
    end
    @attrs.each {|name|
      module_eval "def _0_#{name}(value) @param.#{name} = value end"
    }

    def _0_rgrid_begin
	@dim = inlet_dim 0
	@nt = inlet_nt 0
	GridFlow.post "%s %s",@dim,@nt
	@dim.length!=3 and raise "expecting 3 dims (rows,columns,channels) but got #{@dim.inspect}"
	@dim[2]!=3 and raise "expecting 3 channels, got #{@dim.inspect}"
	@image = Image.new @dim[0],@dim[1]
	inlet_set_factor 0,@dim[0]*@dim[1]*@dim[2]
    end
    def _0_rgrid_flow data
        i=0
	ps=GridFlow.packstring_for_nt(@nt)+"3"
	sz=GridFlow.sizeof_nt(@nt)*3
        for y in 0...@dim[0]
	  for x in 0...@dim[1]
	    r,g,b = data[i,sz].unpack(ps)
	    pixel=@image.getRow(y).at(x)
	    pixel.setRed r
	    pixel.setGreen g
	    pixel.setBlue b
	    i+=sz
	  end
	end
    end
    def _0_rgrid_end # to be generalized...
	@imatrix = Rblti::Imatrix.new
	@palette = Rblti::Palette.new
	@functor.apply @image, @imatrix, @palette
	send_out_lti_image 0,@image
	#send_out_lti_imatrix 0,@imatrix
	#send_out_lti_palette 0,@palette
    end
    def send_out_lti_image o,m
	send_out_grid_begin o,[m.rows,m.columns,3]
	ps=GridFlow.packstring_for_nt(@nt)
	sz=GridFlow.sizeof_nt(@nt)
	for y in 0...@dim[0] do ro=m.getRow(y)
	  for x in 0...@dim[1] do px=ro.at(x)
	    send_out_grid_flow o,
	      [px.getRed,px.getGreen,px.getBlue].pack(ps)
	  end
	end
    end
    def send_out_lti_imatrix o,m
	send_out_grid_begin o,[m.rows,m.columns],:float32
	ps=GridFlow.packstring_for_nt(@nt)
	sz=GridFlow.sizeof_nt(@nt)
	for y in 0...@dim[0] do ro=m.getRow(y)
	  for x in 0...@dim[1] do px=ro.at(x)
	    send_out_grid_flow o, [px].pack(ps)
	  end
	end
    end
    install_rgrid 0
  }
}
