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

class String
  # returns a Fixnum that is a pointer divided by 4  
  def meat # warning: not 64-bit-safe
    [self].pack("p").unpack("I")[0]>>2
  end
end

Rblti.const_set(:RkMeansSegmentation_parameters,Class.new(RkMeansSegmentation_parameters))
RkMeansSegmentation_parameters.module_eval {
  def initialize
    super
    @cq_params = RkMColorQuantization_parameters.new
  end
  RkMColorQuantization_parameters.instance_methods.grep(/\w=$/).each{|writer|
    att=writer.chop
    module_eval "def #{att}    ; @cq_params.#{att} end"
    module_eval "def #{att}=(v); @cq_params.#{att}=(v); setQuantParameters @cq_params end"
  }
}

def GridFlow.post2(*a)
  s = sprintf(*a)
  t = []
  i = 0
  loop do
    j = i
    i = s.index(/ /,64)
    t<<s[j...(i||s.length)]
    break if not i
  end
  GridFlow.post "%s",t
end

class<<Functor
  attr_accessor :inputs
  attr_accessor:outputs
end

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
    fucn = $1.upcase+$2
    @parameterses<<c
    @functors<<fucn if const_get(fucn).ancestors.include?(Functor)
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
  def LTI.from_pd datum, tipe
    case tipe
    when "float": datum.to_f
    when "int":   datum.to_i
    when "bool":  datum.to_i!=0
    else raise "LTI.from_pd can't convert #{datum}, a #{datum.class}, to type '#{tipe}'"
    end
  end
  def LTI.info(name,inputs,outputs)
    Rblti.const_get(name).module_eval {
      #@inputs=inputs;  def self.inputs; @inputs end
      #@outputs=outputs def self.outputs; @outputs end
      self.inputs = inputs
      self.outputs = outputs
    }
  end
end

LTI.info         :Segmentation, [Image], [Imatrix,Palette]
LTI.info   :KMeansSegmentation, [Image], [Imatrix,Palette]
LTI.info:MeanShiftSegmentation, [Image], [Imatrix,Palette]

class LTIGridObject < GridObject
    def initialize
      super
      c=self.class
      @functor = c.functor_class.new
      @param   = c.  param_class.new
      @functor.setParameters @param
      @image_bp=BitPacking.new(ENDIAN_LITTLE,4,[0xff0000,0x00ff00,0x0000ff])
      @imatrix_bp=BitPacking.new(ENDIAN_LITTLE,1,[0xff])
    end
    def send_out_lti_image o,m
        GridFlow.post "4*meat=0x%08x",4*m.meat
	send_out_grid_begin o,[m.rows,m.columns,3]
	ps=GridFlow.packstring_for_nt(@nt)
	sz=GridFlow.sizeof_nt(@nt)
	a=[0,0,0]
	for y in 0...m.rows do ro=m.getRow(y)
	  for x in 0...m.columns do px=ro.at(x)
	    a[0]=px.getRed
	    a[1]=px.getGreen
	    a[2]=px.getBlue
	    send_out_grid_flow o, a.pack(ps)
	  end
	end
    end
    def send_out_lti_palette o,m
        #GridFlow.post "4*meat=0x%08x",4*m.meat
	send_out_grid_begin o,[m.size,3]
	ps=GridFlow.packstring_for_nt(@nt)
	sz=GridFlow.sizeof_nt(@nt)
	a=[0,0,0]
	for x in 0...m.size do px=m.at(x)
	  a[0]=px.getRed
	  a[1]=px.getGreen
	  a[2]=px.getBlue
	  send_out_grid_flow o,a.pack(ps)
	end
    end
    def send_out_lti_imatrix o,m
	send_out_grid_begin o,[m.rows,m.columns]#,@out_nt
	send_out_grid_flow_3 o, m.rows*m.columns, m.meat, :int32
    end
end

LTI.functors.each {|name|
fuc = Rblti.const_get name
begin
  fui = fuc. inputs || [Image]
  fuo = fuc.outputs || [Image]
  LTIGridObject.subclass("lti."+name,fui.length,fuo.length+1) {
    install_rgrid 0
    class << self
      attr_accessor  :param_class
      attr_accessor:functor_class
      attr_accessor:attrs
    end

    @functor_class = fuc
    @param_class = Rblti.const_get("R"+
	name[0..0].downcase+
	name[1..-1]+"_parameters")
    subparams = instance_methods.grep(/^set.*Parameters$/)
    GridFlow.post "%s has subparams: %s", @foreign_name, subparams if subparams.length>0
    @attrs = {}
    def self.lti_attr(name)
      tipe=nil
      begin
        @param_class.new.__send__(name+"=",Object.new)
      rescue Exception=>e
	  /of type '([^']*)'/ .match e.to_s and tipe=$1 or
	  GridFlow.post "%s",e.inspect
      end
      @attrs[name]=[tipe]
      #GridFlow.post "%s", "defining #{name} for #{functor_class}"
      module_eval "def _0_#{name}(value) @param.#{name} = LTI.from_pd(value,'#{tipe}'); end"
    end
    param_class.instance_methods.grep(/\w=$/).each{|x| self.lti_attr x.chop }
    def _0_help() self.class.help end
    def self.help()
      @attrs.each{|x,v| GridFlow.post "attribute %s: %s",x,v.inspect}
      GridFlow.post "total %d attributes", @attrs.length
      #---
      pmo=param_class.instance_methods-attrs.keys-attrs.keys.map{|x|x+"="}-Object.instance_methods
      pmo.each{|x| GridFlow.post "other param method %s",x}
      GridFlow.post "total %d other param methods", pmo.length
      #---
      fm=functor_class.instance_methods-Object.instance_methods
      fm.each{|x| GridFlow.post "functor method %s",x}
      GridFlow.post "total %d functor methods", fm.length
      #anc = ancestors
      anc = functor_class.ancestors
      anc.each{|x|
	y=x.to_s
        GridFlow.post "ancestor class %s", (if y[0]==35 then "["+x.foreign_name+"]" else y end)
      }
      GridFlow.post "total %d ancestor classes", anc.length
      GridFlow.post "input types: %s",  (@functor_class. inputs.inspect rescue "(#{$!})")
      GridFlow.post "output types: %s", (@functor_class.outputs.inspect rescue "(#{$!})")
    end
    def _0_get(sel=nil)
      return @param.__send__(sel) if sel
      no=self.class.noutlets
      self.class.attrs.each_key {|sel|
        v=_0_get(sel)
        begin
          send_out no-1, sel.intern, LTI.to_pd(v)
	rescue StandardError=>e
	  GridFlow.post "%s", e.inspect
	  send_out no-1, sel.intern, :some, v.class.to_s.intern
	end
      }
    end

    def _0_rgrid_begin
	@dim = inlet_dim 0
	@nt = inlet_nt 0
	@dim.length!=3 and raise "expecting 3 dims (rows,columns,channels) but got #{@dim.inspect}"
	@dim[2]!=3 and raise "expecting 3 channels, got #{@dim.inspect}"
	@image = Image.new @dim[0],@dim[1]
	inlet_set_factor 0,@dim[0]*@dim[1]*@dim[2]
    end
    def _0_rgrid_flow data
	@image_bp.pack3 @dim[0]*@dim[1],data.meat,@image.meat,@nt
    end
    def _0_rgrid_end # to be generalized...
	@imatrix = Rblti::Imatrix.new
	@palette = Rblti::Palette.new
	#t=Time.new
        @functor.setParameters @param
	@functor.apply @image, @imatrix, @palette
	#t=Time.new-t
	#GridFlow.post "time for apply: %f",t
	send_out_lti_palette 1,@palette
	send_out_lti_imatrix 0,@imatrix
    end
  }
rescue StandardError => e
  GridFlow.post "%s", e
  GridFlow.post "while handling lti class %s, which has ancestors %s",
    fuc,fuc.ancestors.join(' ')
end
}
