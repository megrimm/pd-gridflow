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

# btw here's a useful line for debugging ltilib's makefile:
# ruby -i.bak -ne 'print $_.gsub(/^ {8}/,"\t") unless /@echo.*\\$/' Makefile

=begin
        TODO list
	- Learning period for BackgroundModel before apply can be used
	- Reset for BackgroundModel
	- Mask functors, operations

=end

require "rblti"
include GridFlow
include Rblti

class String
  # returns a Fixnum that is a pointer divided by 4
  def meat # warning: not 64-bit-safe
    stuff = [self].pack("p")
    if stuff.length == 4 then # 32 bit...
      stuff.unpack("I")[0]>>2
    elsif GridFlow::OurByteOrder==1 # 64 bit LE (AMD K8)
      a,b=stuff.unpack("I2")
      (a >> 2)|(b << 30)
    else # 64 bit BE (Apple G5)
      b,a=stuff.unpack("I2")
      (a >> 2)|(b << 30)
    end
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
    t << s[j...(i||s.length)]
    break if not i
  end
  GridFlow.post "%s",t
end

class << Functor
  def forms=(a) @forms=a end
  def forms()
    if defined? @forms then @forms elsif self.class<=Functor then superclass.forms else [] end
  end
  attr_accessor :subclasses # only the adjacent ones
end

def Functor.post_hierarchy(level=0)
  GridFlow.post "%*s%s", 4*level, "", self.inspect
  n=1; level+=1
  (subclasses||[]).each {|x| begin
  	n += x.post_hierarchy level
	rescue Exception=>e; GridFlow.post "%*s%s (doesn't have post_hierarchy)", 4*level, "", x.inspect
	end
  }
  n
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
    @parameterses << c
    @functors << fucn if const_get(fucn).ancestors.include?(Functor)
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
      #@functors.each{|x| GridFlow.post "  functor %s", x }
      GridFlow.post "total %d functors (post_hierarchy)",Functor.post_hierarchy
      GridFlow.post "total %d functors (@functors)",     @functors.length
    when:others:
      @others.each{|x|
        v = const_get(x)
	info = if Class===v then "ancestors="+v.ancestors.inspect else v.inspect end
	GridFlow.post "  other %s (%s)", x, info }
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
end

class ArgMode
  def initialize(of) @of=of end
  def inspect; "#{self.class}["+@of.inspect+"]" end
  class << self; alias [] new end
  def of; @of end
end
class    In<ArgMode; def in?;  true end; def out?; false end end
class   Out<ArgMode; def in?; false end; def out?;  true end end
class InOut<ArgMode; def in?;  true end; def out?;  true end end

require "gridflow/optional/lti_apply"

class LTIGridObject < GridObject
    ImageBP   = BitPacking.new(GridFlow::ENDIAN_LITTLE,4,[0xff0000,0x00ff00,0x0000ff])
    ImatrixBP = BitPacking.new(GridFlow::ENDIAN_LITTLE,1,[0xff]) # actually this would be for Umatrix

    def initialize(formid=0)
	super()
	c=self.class
	@functor = c.functor_class.new
	@param   = c.  param_class.new
	@formid = formid.to_i
	@functor.setParameters @param
	#form = c.functor_class.forms[@formid]
    end
    def initialize2; initialize3 end

    def _0_formid(f); self.formid=f end
    def formid=(f); @formid=f; initialize3 end
    def formid; @formid end

    # initialize  : every ruby object, called by .new
    # initialize2 : every GF object, just after the object is registered in Pd
    # initialize3 : called by initialize and every time the form (the chosen "apply" type) changes
    def initialize3
	c = self.class
	GridFlow.post "this is class %s, functor_class %s", c.inspect, c.functor_class.inspect
	form = c.functor_class.forms[@formid]
	GridFlow.post "will use form %s: %s", @formid.inspect, form.inspect
	@dims = form.map{[]}
	GridFlow.post "@dims = %s", @dims.inspect
	@nts = form.map{:int32}
	GridFlow.post " @nts = %s",  @nts.inspect
	@stuffs = form.map{|stuffclass| stuffclass.of.new }
	GridFlow.post " @stuffs = %s",  @stuffs.inspect
	@inletmap = []
	@outletmap = []
	form.each_with_index {|slot,i|
		case slot; when  In,InOut;  @inletmap << i end
		case slot; when Out,InOut; @outletmap << i end
	}
	GridFlow.post " @inletmap = %s",  @inletmap.inspect
	GridFlow.post "@outletmap = %s", @outletmap.inspect
	add_inlets @inletmap.length-1
	add_outlets @outletmap.length
    end

    def _n_rgrid_begin(inlet)
	dim = inlet_dim inlet
	nt  = inlet_nt  inlet
	slot = @inletmap[inlet] # slot into the apply()
#        GridFlow.post "inlet=%d slot=%d stuff=%s", inlet, slot, @stuffs[slot].inspect
        @stuffs[slot] = 
        case @stuffs[slot]
        when Image:
	  dim.length!=3 and raise "expecting 3 dims (rows,columns,channels) but got #{dim.inspect}"
	  dim[2]!=3 and raise "expecting 3 channels, got #{dim.inspect}"
          Image.new dim[0],dim[1]
        when Imatrix:
	  dim.length!=2 and raise "expecting 2 dims (rows,columns) but got #{dim.inspect}"
          Imatrix.new dim[0],dim[1]
        when Irect:
	  dim.length!=2 and raise "expecting 2 dims (points,axes) but got #{dim.inspect}"
          dim[0]!=2 and raise "expecting 2 points"
          dim[1]!=2 and raise "expecting 2 axes"
          Irect.new
	else raise "don't know how to validate a #{st.class} for inlet #{inlet}"
        end
	@dims[  slot] = dim
	@nts[   slot] = nt
	inlet_set_factor inlet,dim.prod
    end

    def _n_rgrid_flow(inlet,data)
	slot = @inletmap[inlet] # slot into the apply()
	st=@stuffs[slot]
	dim= @dims[slot]
	nt =  @nts[slot]
	GridFlow.post "dim=%s", dim.inspect
	GridFlow.post "data.meat=%s", data.meat.inspect
	GridFlow.post "data.inspect=%s", data.inspect
	GridFlow.post "data.pack=%s", [data].pack("p").inspect
	GridFlow.post "data.meat2=%s", ([data].pack("p").unpack("I")[0]>>2).inspect
	case st
	when Imatrix; LTIGridObject::ImatrixBP.pack3 dim[0]*dim[1],data.meat,st.meat,nt
	when Image  ; LTIGridObject::  ImageBP.pack3 dim[0]*dim[1],data.meat,st.meat,nt
        when Irect
          d = data.unpack("I4")
          #GridFlow.post "d=%s", d.inspect
          dax = [d[1],d[3]].sort!
          day = [d[0],d[2]].sort!
          st.ul.x, st.ul.y = dax
          st.br.x, st.br.y = day
	else raise "don't know how to write into a #{st.class} for inlet #{inlet}"
	end
    end

    def apply
	#GridFlow.post "would apply %s",@stuffs.inspect
	@functor.apply(*@stuffs)
    end

    def _n_rgrid_end(inlet)
        @functor.setParameters @param
#	t=Time.new
	apply
#	t=Time.new-t; GridFlow.post "time for apply: %f",t
        GridFlow.post "@outletmap = %s", @outletmap.inspect
        GridFlow.post "@stuffs = %s", @stuffs.inspect
        i=@outletmap.length-1
        while i>=0
          slot = @outletmap[i]
          send_out_lti i,@stuffs[slot]
          i-=1
        end
    end

    def send_out_lti o,m
      case m
      when Imatrix: send_out_lti_imatrix o,m
      when Image:   send_out_lti_image   o,m
      when Palette: send_out_lti_palette o,m
      when Irect:   send_out_lti_rect    o,m
      else raise "don't know how to send_out a #{m.class}"
      end
    end

#-=-=-=-=-#-=-=-=-=-#-=-=-=-=-#-=-=-=-=-#-=-=-=-=-#-=-=-=-=-#-=-=-=-=-#-=-=-=-=-#-=-=-=-=-#-=-=-=-=-#-=-=-=-=-#

    def send_out_lti_image o,m
        GridFlow.post "4*meat=0x%08x",4*m.meat
	send_out_grid_begin o,[m.rows,m.columns,3]
	nt = :int32
	ps=GridFlow.packstring_for_nt(nt)
	sz=GridFlow.sizeof_nt(nt)
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
    def rgrid_begin_lti_image o,m
    end
    def send_out_lti_palette o,m
        #GridFlow.post "4*meat=0x%08x",4*m.meat
	send_out_grid_begin o,[m.size,3]
	nt = :int32
	ps=GridFlow.packstring_for_nt(nt)
	sz=GridFlow.sizeof_nt(nt)
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
    def send_out_lti_rect o,m
	send_out_grid_begin o,[2,2]#,@out_nt
	send_out_grid_flow o, [m.ul.y, m.ul.x, m.br.y, m.br.x].pack("I4"), :int32
    end
end

class Array; def prod() r=1; each{|x| r*=x }; r end end

LTI.functors.each {|name|
  qlas = Rblti.const_get(name)
  qlas.subclasses = [] if qlas<=Functor
}

LTI.functors.each {|name|
fuc = Rblti.const_get name
begin
  if fuc.forms.length==0 then
  	GridFlow.post "no forms for %s ?", fuc
	next
  end
  LTIGridObject.subclass("lti."+name,1,1) {
    install_rgrid 0
    install_rgrid 1
    install_rgrid 2
    class << self
      attr_accessor  :param_class
      attr_accessor:functor_class
      attr_accessor:attrs
    end

    @functor_class = fuc
    @param_class = Rblti.const_get("R"+
	name[0..0].downcase+
	name[1..-1]+"_parameters")
    sup=@functor_class.superclass; sup.subclasses << @functor_class if sup<=Functor
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
      #GridFlow.post "input types: %s",  (@functor_class. inputs.inspect rescue "(#{$!})")
      #GridFlow.post "output types: %s", (@functor_class.outputs.inspect rescue "(#{$!})")
      @functor_class.forms.each_with_index {|form,i|
	GridFlow.post "form %d: %s", i, form.inspect
      }
    end
    def _0_get(sel=nil)
      return @param.__send__(sel) if sel
      no=self.class.noutlets
      if sel==:form or not sel then send_out no-1, :form end
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
    def _0_form i
	n = self.class.functor_class.forms.length
	if i<0 or i>=n then raise "no form numbered %d (try help)", i end
	@formid = form
    end
    [0,1,2].each {|i|
      eval"
	def _#{i}_rgrid_begin;     _n_rgrid_begin #{i} end
	def _#{i}_rgrid_flow data; _n_rgrid_flow  #{i},data end
	def _#{i}_rgrid_end;       _n_rgrid_end   #{i} end
      "
    }

#    def _1_rgrid_begin
#	@dim1 = inlet_dim 1
#	@nt1 = inlet_nt 1
#	inlet_set_factor 1,@dim1.prod
#	@arg1 = Irect.new
#    end
#    def _1_rgrid_flow data
#	#@image_bp.pack3 @dim[0]*@dim[1],data.meat,@image.meat,@nt
#	@arg1.ul.y,@arg1.ul.x,@arg1.br.y,@arg1.br.x = data.unpack("I4")
#	if !(@arg1.isConsistent)
#	   raise "Rectangle at inlet 1 is not consistent, does not follow (ul , br) format"   
#	end
#	@functor.reset
#    end
#    def _1_rgrid_end
#    end
    def pd_properties canvas
      cid = ".x%x"%(4*canvas)
      GridFlow.gui %{
        tk_messageBox -message "this would be a cool feature, eh?" -type yesno -icon question -parent #{cid}
      }
    end
    properties_enable
  }
rescue StandardError => e
  GridFlow.post "%s", e
  GridFlow.post "%s", e.backtrace.join("\n")
  GridFlow.post "while handling lti class %s, which has ancestors %s",
    fuc,fuc.ancestors.join(' ')
end
}

GridFlow.fclasses["lti.BackgroundModel"].module_eval {
  def initialize
    super
    @mode = :apply
  end
  def _0_mode(mode)
    case mode
    when :addBackground; #ok
    when :adaptBackground; #ok
    when :apply; #ok
    else raise "unknown mode: #{mode}"
    end
    @mode = mode
  end
  def apply
    case @mode
    when :addBackground
      @functor.addBackground(@stuffs[0])
    when :adaptBackground
      @functor.adaptBackground(@stuffs[0],@stuffs[1])
    when :apply; super
    else raise "unknown mode: #{mode}"
    end
  end
  def _0_clear; @functor.clearMoldel end # really. that's a typo in the lti API
}
