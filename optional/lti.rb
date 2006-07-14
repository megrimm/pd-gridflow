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
	- Reset for BackgroundModel (DONE: please see method "clear" in [lti.BackgroundModel])
	- Mask functors, operations
	- Try to predict segfaults, e.g. in operations with arguments of the wrong sizes
=end

#$lti_debug = true

require "rblti"
include GridFlow
include Rblti

class String
  # returns a Fixnum that is a pointer divided by 4
  def meat
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
    attr_accessor:have_param
  end
  @functors=[]
  @parameterses=[]
  @have_param=Hash.new(false)

  cs=Rblti.constants
  iii=0
  cs.sort!
  cs.each{|c|
    #GridFlow.post "%d Processing %s", iii, c
    iii=iii+1
    #next unless const_get(c).respond_to? "instance_methods"
    next unless Class===Rblti.const_get(c)
    next unless const_get(c).ancestors.include?(Functor)
    parm = 'R'+c.sub(/^./) {|s| s.downcase}+ '_parameters'

    if (cs.grep Regexp.new(parm)).size != 0
       @have_param[c]=parm
       @parameterses << parm
    end
    @functors << c
  }
  @others = (cs-(@functors+@parameterses)).sort
  def _0_help(a=nil) self.class.help(a) end
  def self.help(a=nil)
    case a
    when nil
      GridFlow.post "try one of:"
      GridFlow.post "  help functors"
      GridFlow.post "  help others"
    when :functors
      #@functors.each{|x| GridFlow.post "  functor %s", x }
      GridFlow.post "total %d functors (post_hierarchy)",Functor.post_hierarchy
      GridFlow.post "total %d functors (@functors)",     @functors.length
    when :others
      @others.each{|x|
        v = const_get(x)
	info = if Class===v then "ancestors="+v.ancestors.inspect else v.inspect end
	GridFlow.post "  other %s (%s)", x, info }
      GridFlow.post "total %d others",@others.length
    end
  end
  def LTI.to_pd datum
    case datum
    when String; datum.intern
    when Float,Integer; datum
    when true; 1
    when false; 0
    else raise "LTI.to_pd can't convert #{datum}, a #{datum.class}"
    end
  end
  def LTI.from_pd datum, tipe
    case tipe
    when "float"; datum.to_f
    when "int";   datum.to_i
    when "bool";  datum.to_i!=0
    else raise "LTI.from_pd can't convert #{datum}, a #{datum.class}, to type '#{tipe}'"
    end
  end
end

class ArgMode
  def initialize(of) @of=of end
  def inspect; "#{self.class}["+@of.inspect+"]" end
  def to_s; inspect; end
  class << self; alias [] new end
  def of; @of end
end
class    In<ArgMode; def in?;  true end; def out?; false end end
class   Out<ArgMode; def in?; false end; def out?;  true end end
class InOut<ArgMode; def in?;  true end; def out?;  true end end

require "gridflow/optional/lti_apply"

class LTIGridObject < GridObject
    ImageBP   = BitPacking.new(GridFlow::ENDIAN_LITTLE,4,[0xff0000,0x00ff00,0x0000ff])
    UmatrixBP = BitPacking.new(GridFlow::ENDIAN_LITTLE,1,[0xff])

    class << self
      attr_accessor  :param_class
      attr_accessor:functor_class
      attr_accessor:attrs
      attr_accessor:funcname
    end
    
    def initialize(formid=0)
	super()
	c=self.class
	@functor = c.functor_class.new
	@formid = formid.to_i
	@funcname = c.funcname
	GridFlow.post "@funcname = %s\n", @funcname.inspect if $lti_debug
        @param=nil
        if LTI.have_param[@funcname]
	   @param   = c.  param_class.new
	   @functor.setParameters @param
	end
    end
    def initialize2; initialize3 end

    def formid=(f); @formid=f; initialize3 end
    def formid; @formid end
    
    # wtf is this?...
    def _0_formid(f); self.formid=f end
    def _0_form i
	n = self.class.functor_class.forms.length
	if i<0 or i>=n then raise "no form numbered %d (try help)", i end
	@formid = form
    end

    # initialize  : every ruby object, called by .new
    # initialize2 : every GF object, just after the object is registered in Pd
    # initialize3 : called by initialize and every time the form (the chosen "apply" type) changes
    def initialize3
	c = self.class
	GridFlow.post "this is class %s, functor_class %s", c.inspect, c.functor_class.inspect if $lti_debug
	form = c.functor_class.forms[@formid]
	GridFlow.post "will use form %s: %s", @formid.inspect, form.inspect if $lti_debug
	@dims = form.map{[]}
	GridFlow.post "@dims = %s", @dims.inspect if $lti_debug
	@nts = form.map{:int32}
	GridFlow.post " @nts = %s",  @nts.inspect if $lti_debug
	@stuffs = form.map{|stuffclass| stuffclass.of.new }
	GridFlow.post " @stuffs = %s",  @stuffs.inspect if $lti_debug
	@inletmap = []
	@outletmap = []
	form.each_with_index {|slot,i|
		case slot; when  In,InOut;  @inletmap << i end
		case slot; when Out,InOut; @outletmap << i end
	}
	GridFlow.post " @inletmap = %s",  @inletmap.inspect if $lti_debug
	GridFlow.post "@outletmap = %s", @outletmap.inspect if $lti_debug
	add_inlets @inletmap.length-1
	add_outlets @outletmap.length
    end

    def self.lti_attr(name)
      tipe=nil
      begin
        if LTI.have_param[name]
           @param_class.new.__send__(name+"=",Object.new)
        end
      rescue Exception=>e
          /of type '([^']*)'/ .match e.to_s and tipe=$1 or
          GridFlow.post "%s",e.inspect if $lti_debug
      end
      @attrs[name]=[tipe]
      #GridFlow.post "%s", "defining #{name} for #{functor_class}" if $lti_debug
      if LTI.have_param[name]
         module_eval "def _0_#{name}(value) @param.#{name} = LTI.from_pd(value,'#{tipe}'); end"
      end
    end

    def _0_help() self.class.help end
    def self.help()
      GridFlow.post "\n\nFunctor %s", @funcname
      @attrs.each{|x,v| GridFlow.post "attribute %s: %s",x,v.inspect}
      GridFlow.post "total %d attributes", @attrs.length
      #---
      
      if LTI.have_param[@funcname]
         pmo=param_class.instance_methods-attrs.keys-attrs.keys.map{|x|x+"="}-Object.instance_methods
         pmo.each{|x| GridFlow.post "other param method %s",x}
         GridFlow.post "total %d other param methods", pmo.length
      else
         GridFlow.post "This functor has no parameter"
      end

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
      @functor_class.forms.each_with_index {|form,i|
	GridFlow.post "form %d: %s", i, form.inspect
      }
    end

    def _0_get(sel=nil)
      printf "_0_get(%s)\n", sel.inspect
      if LTI.have_param[@funcname]
         return @param.__send__(sel.to_s) if sel
      end
      no=self.class.noutlets
      if sel==:form then send_out no-1, :form; return end
      if sel!=nil then raise "unknown parameter '#{sel}'" end
      # here's the code to get ALL parameters (sel=nil)
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
        
    def _n_rgrid_begin(inlet)
	dim = inlet_dim inlet
	nt  = inlet_nt  inlet
	slot = @inletmap[inlet] # slot into the apply()
#        GridFlow.post "Functor %s inlet=%d slot=%d stuff=%s", @funcname, inlet, slot, @stuffs[slot]
        @stuffs[slot] =
        case @stuffs[slot]
        when Image
	  dim.length!=3 and raise "expecting 3 dims (rows,columns,channels) but got #{dim.inspect}"
	  dim[2]!=3 and raise "expecting 3 channels, got #{dim.inspect}"
          Image.new dim[0],dim[1]
        when Umatrix,Channel8, Imatrix,Channel32, Fmatrix,Channel
	  dim.length!=3 and raise "expecting 3 dims (rows,columns,channels) but got #{dim.inspect}"
	  dim[2]!=1 and raise "expecting 1 channel, got #{dim.inspect}"
          case @stuffs[slot]
	  when Fmatrix,Channel; nt==:float32 or raise "wanted float32"
	  end
          @stuffs[slot].class.new dim[0],dim[1]
        when Irect
	  dim.length!=2 and raise "expecting 2 dims (points,axes) but got #{dim.inspect}"
          dim[0]!=2 and raise "expecting 2 points"
          dim[1]!=2 and raise "expecting 2 axes"
          Irect.new
        when Rblti::Ubyte, Rblti::Integer, Rblti::Float, Rblti::Double
          dim.length!=0 and raise "expecting 0 dims (scalar) at inlet #{inlet} but got #{dim.inspect}"
          case @stuffs[slot]
          when Rblti::Ubyte
             nt==:uint8 or raise "wanted uint8"
          when Rblti::Integer
             nt==:int32 or raise "wanted int32"
          when Rblti::Float
             nt==:float32 or raise "wanted float32"
          when Rblti::Double
             nt==:float64 or raise "wanted float64"
          end
          @stuffs[slot].class.new
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
	case st # we might pretend that N floats or N int32 are 4N uint8 instead, because UmatrixBP is really a memcpy
	when Umatrix, Channel8; LTIGridObject::UmatrixBP.pack3 dim[0]*dim[1]  ,data.meat,st.meat,nt
	when Imatrix,Channel32; LTIGridObject::UmatrixBP.pack3 dim[0]*dim[1]*4,data.meat,st.meat,:uint8
	when Fmatrix,  Channel; LTIGridObject::UmatrixBP.pack3 dim[0]*dim[1]*4,data.meat,st.meat,:uint8
	when Image            ; LTIGridObject::  ImageBP.pack3 dim[0]*dim[1]  ,data.meat,st.meat,nt
        when Irect
          d = data.unpack("I4")
          #GridFlow.post "d=%s", d.inspect
          st.ul.x, st.br.x = [d[1],d[3]].sort!
          st.ul.y, st.br.y = [d[0],d[2]].sort!
        when Rblti::Ubyte;   st.val= data.unpack("C")[0]
        when Rblti::Integer; st.val= data.unpack("I")[0]
        when Rblti::Float;   st.val= data.unpack("f")[0]
        when Rblti::Double;  st.val= data.unpack("d")[0]
	else raise "don't know how to write into a #{st.class} for inlet #{inlet}"
	end
    end

    def apply
	#GridFlow.post "would apply %s",@stuffs.inspect
        realstuff=[]
        for i in @outletmap
           case @stuffs[i]
           when Rblti::Ubyte, Rblti::Integer, Rblti::Float, Rblti::Double
              realstuff[i]=@stuffs[i].getPtr
           else
              realstuff[i]=@stuffs[i]
           end
        end

        for i in @inletmap
           case @stuffs[i]
           when Rblti::Ubyte, Rblti::Integer, Rblti::Float, Rblti::Double
              realstuff[i]=@stuffs[i].val
           else
              realstuff[i]=@stuffs[i]
           end
        end
        
	@functor.apply(*realstuff)
    end

    def _n_rgrid_end(inlet) end

    def _0_rgrid_end
        if LTI.have_param[@funcname]
           @functor.setParameters @param
	end
#	t=Time.new
	apply
#	t=Time.new-t; GridFlow.post "time for apply: %f",t
        GridFlow.post "@outletmap = %s", @outletmap.inspect if $lti_debug
        GridFlow.post "@stuffs = %s", @stuffs.inspect if $lti_debug
        i=@outletmap.length-1
        while i>=0
          slot = @outletmap[i]
          send_out_lti i,@stuffs[slot]
          i-=1
        end
    end

    def send_out_lti o,m
      case m
      when Umatrix, Channel8;  send_out_lti_umatrix o,m
      when Imatrix, Channel32; send_out_lti_imatrix o,m
      when Fmatrix, Channel;   send_out_lti_fmatrix o,m
      when Image;   send_out_lti_image   o,m
      when Palette; send_out_lti_palette o,m
      when Irect;   send_out_lti_rect    o,m
      when Rblti::Double;  send_out_lti_double o,m
      when Rblti::Float;   send_out_lti_float o,m
      when Rblti::Integer; send_out_lti_int o,m
      when Rblti::Ubyte;   send_out_lti_ubyte o,m
      else raise "don't know how to send_out a #{m.class}"
      end
    end

    def send_out_lti_image o,m
	foo="666"*m.columns*m.rows
	LTIGridObject::ImageBP.unpack3 m.rows*m.columns,m.meat,foo.meat,:uint8
	send_out_grid_begin o,[m.rows,m.columns,3] #,@out_nt
	send_out_grid_flow_3 o, m.rows*m.columns*3, foo.meat, :uint8
    end
    def send_out_lti_palette o,m
	foo="666"*m.size
	LTIGridObject::ImageBP.unpack3 m.size,m.meat,foo.meat,:uint8
	send_out_grid_begin o,[m.size,3]
	send_out_grid_flow_3 o, m.size*3, foo.meat, :uint8
    end
    def send_out_lti_umatrix o,m
	send_out_grid_begin o,[m.rows,m.columns,1] #,@out_nt
	send_out_grid_flow_3 o, m.rows*m.columns, m.meat, :uint8
    end
    def send_out_lti_imatrix o,m
	send_out_grid_begin o,[m.rows,m.columns,1] #,@out_nt
	send_out_grid_flow_3 o, m.rows*m.columns, m.meat, :int32
    end
    def send_out_lti_fmatrix o,m
	send_out_grid_begin o,[m.rows,m.columns,1], :float32
	send_out_grid_flow_3 o, m.rows*m.columns, m.meat, :float32
    end
    def send_out_lti_rect o,m
	send_out_grid_begin o,[2,2]#,@out_nt
	send_out_grid_flow o, [m.ul.y, m.ul.x, m.br.y, m.br.x].pack("I4"), :int32
    end
    def send_out_lti_double o,m
	send_out_grid_begin o, [], :float64
	send_out_grid_flow o, [m.val].pack("d"), :float64
	#send_out o, m.val
    end
    def send_out_lti_float o,m
	send_out_grid_begin o, [], :float32
	send_out_grid_flow o, [m.val].pack("f"), :float32
	#send_out o, m.val
    end
    def send_out_lti_int o,m
	send_out_grid_begin o, [], :int32
	send_out_grid_flow o, [m.val].pack("I"), :int32
	#send_out o, m.val
    end
    def send_out_lti_ubyte o,m
	send_out_grid_begin o, [], :uint8
	send_out_grid_flow o, [m.val].pack("C"), :uint8
	#send_out o, m.val
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
  	GridFlow.post "no forms for %s ?", fuc if $lti_debug
	next
  end
  LTIGridObject.subclass("lti."+name,1,1) {
    install_rgrid 0, true
    install_rgrid 1, true
    install_rgrid 2, true
    @functor_class = fuc
    if LTI.have_param[name]
       @param_class = Rblti.const_get("R"+
	   name[0..0].downcase+
	   name[1..-1]+"_parameters")
    end
    @funcname = name
    sup=@functor_class.superclass; sup.subclasses << @functor_class if sup<=Functor
    @attrs = {}
    
    if LTI.have_param[name]
       param_class.instance_methods.grep(/\w=$/).each{|x| self.lti_attr x.chop }
    end

    (0..2).each {|i|
      eval"
	def _#{i}_rgrid_begin;     _n_rgrid_begin #{i} end
	def _#{i}_rgrid_flow data; _n_rgrid_flow  #{i},data end
	def _#{i}_rgrid_end;       _n_rgrid_end   #{i} end
      "
    }

    def pd_properties canvas
      cid = ".x%x"%(4*canvas)
      wid = ".x%x"%(4*self.object_id)
      fc = self.class.functor_class
      GridFlow.gui %{
        # tk_messageBox -message "this would be a cool feature, eh?" -type yesno -icon question -parent #{cid}
	toplevel #{wid}
      }
      fc.forms.each_with_index {|f,i|
        GridFlow.gui %{
	  pack [radiobutton #{wid}.#{i} -text {#{f}} -variable var#{wid} -anchor w] -fill x -expand 1
        }
      }
      GridFlow.gui %{
        pack [frame #{wid}.bar] -side bottom -fill x -pady 2m
	foreach {name Name} {cancel Cancel apply Apply ok Ok} {
		pack [button #{wid}.bar.$name -command "pd #{wid} props_$name" -text $Name] \
			-side left -expand 1
	}
	pack [frame #{wid}.sep -height 2 -borderwidth 1 -relief sunken] \
		-side bottom -fill x -expand 1
        wm protocol #{wid} WM_DELETE_WINDOW "destroy #{wid}"
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

# Special Cases:

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

GridFlow.fclasses["lti.MeanshiftTracker"].module_eval {
#  def apply
  def _1_rgrid_end
    @functor.reset if @functor.isInitialized
    super
  end
  def _0_reset; @functor.reset end
}
