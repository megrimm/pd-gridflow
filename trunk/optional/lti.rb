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
  attr_writer    :inputs;  def inputs; if defined? @inputs then  @inputs else superclass. inputs end end
  attr_writer   :outputs; def outputs; if defined?@outputs then @outputs else superclass.outputs end end
  attr_writer    :inouts; def  inouts; if defined? @inouts then  @inouts else superclass. inouts end end
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
  def LTI.info(name,inputs=nil,outputs=nil, inouts=nil)
    return if not inputs
    Rblti.const_get(name).module_eval {
      #@inputs=inputs;  def self.inputs; @inputs end
      #@outputs=outputs def self.outputs; @outputs end
      self.inputs  =  inputs
      self.outputs = outputs
      self.inouts  =  inouts
    }
  end
end

def Functor.form=(*a)
  GridFlow.post "%s form: %s", self, a.inspect
end

class ArgMode
  def initialize x; @x=x end
  def inspect; "#{self.class}["+@x.inspect+"]" end
  class << self; alias [] new end
end
class    In<ArgMode; end
class   Out<ArgMode; end
class InOut<ArgMode; end

 LTI.info :Functor, [], []
 LTI.info     :ColorQuantization
 LTI.info         :KMColorQuantization
 LTI.info         :LkmColorQuantization
 LTI.info     :ChiSquareFunctor
 LTI.info     :IoFunctor
 LTI.info         :IoJPEG
 LTI.info         :IoBMP
 LTI.info         :IoImage
 LTI.info         :IoPNG
 LTI.info     :Interpolator
 LTI.info         :VariablySpacedSamplesInterpolator
 LTI.info     :RegionMerge
 LTI.info     :Transform
 LTI.info         :GradientFunctor
 LTI.info             :ColorContrastGradient
 LTI.info         :Skeleton
 LTI.info         :OrientedHLTransform
 LTI.info         :OrientationMap
 LTI.info         :RealFFT
 LTI.info     :ComputePalette
 LTI.info     :Modifier
 LTI.info         :FastRelabeling
 LTI.info             :GeometricFeaturesFromMask
 LTI.info                 :MultiGeometricFeaturesFromMask
 LTI.info         :Morphology
 LTI.info             :Erosion
 LTI.info             :DistanceTransform
 LTI.info             :Dilation
 LTI.info         :MeanshiftTracker, [Image], [], [Rect]
 LTI.info         :CornerDetector
 LTI.info         :GeometricTransform
 LTI.info         :Rotation
 LTI.info         :FlipImage
 LTI.info         :GHoughTransform
 LTI.info         :Thresholding
 LTI.info         :ConvexHull
 LTI.info         :EdgeDetector
 LTI.info             :CannyEdges
 LTI.info             :ClassicEdgeDetector
 LTI.info         :PolygonApproximation
 LTI.info         :BorderExtrema
 LTI.info         :Scaling
 LTI.info         :Filter
 LTI.info             :KNearestNeighFilter
 LTI.info             :Convolution
 LTI.info             :MedianFilter
 LTI.info     :Segmentation, [Image], [Imatrix,Palette]
 LTI.info         :ObjectsFromMask
 LTI.info         :CsPresegmentation
 LTI.info         :RegionGrowing
 LTI.info         :WhiteningSegmentation
 LTI.info         :WatershedSegmentation
 LTI.info         :MeanShiftSegmentation
 LTI.info         :Snake
 LTI.info         :KMeansSegmentation
 LTI.info         :BackgroundModel, [Image], [Channel8]
 LTI.info     :FeatureExtractor
 LTI.info         :LocalFeatureExtractor
 LTI.info             :LocalMoments
 LTI.info         :GlobalFeatureExtractor
 LTI.info             :ChromaticityHistogram
 LTI.info             :GeometricFeatures
 LTI.info     :SimilarityMatrix
 LTI.info     :UsePalette

require "gridflow/optional/lti_apply"

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
    def send_out_lti o,m
      case m
      when Imatrix: send_out_lti_imatrix o,m
      when Image:   send_out_lti_image   o,m
      when Palette: send_out_lti_palette o,m
      when Rect:    send_out_lti_rect    o,m
      else raise "don't know how to send_out a #{m.class}"
      end
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
  fui  = fuc. inputs || []
  fuo  = fuc.outputs || []
  fuio = fuc. inouts || []
  LTIGridObject.subclass("lti."+name,fui.length+fuio.length,fuo.length+fuio.length+1) {
    install_rgrid 0
    install_rgrid 1
    class << self
      attr_accessor  :param_class
      attr_accessor:functor_class
      attr_accessor:attrs
    end

    @functor_class = fuc
    @param_class = Rblti.const_get("R"+
	name[0..0].downcase+
	name[1..-1]+"_parameters")
    #subparams = instance_methods.grep(/^set.*Parameters$/)
    #GridFlow.post "%s has subparams: %s", @foreign_name, subparams if subparams.length>0
    sup=@functor_class.superclass; sup.subclasses << @functor_class if sup<=Functor
    @attrs = {}
    def self.lti_attr(name)
      tipe=nil
=begin
        @param_class.new.__send__(name+"=",Object.new)
      rescue Exception=>e
	  /of type '([^']*)'/ .match e.to_s and tipe=$1 or
	  GridFlow.post "%s",e.inspect
=end
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
    def _0_rgrid_end
	fuc = self.class.functor_class
        is = fuc.inputs + fuc.inouts
        os = fuc.outputs + fuc.inouts
	bufs = [@image]
	bufs << @arg1 if is[1]
	fuc.outputs.map{|o| o.new }
        @functor.setParameters @param
	#t=Time.new
	@functor.apply(*bufs)
	#t=Time.new-t
	#GridFlow.post "time for apply: %f",t
	k=os.length-1;
	j = bufs.length-os.length
	while k>=0 do send_out_lti k,bufs[j+k]; k-=1 end
    end

    def _1_rgrid_begin
	@dim1 = inlet_dim 1
	@nt1 = inlet_nt 1
	inlet_set_factor 1,@dim1.prod
	@arg1 = Rect.new
    end
    def _1_rgrid_flow data
	#@image_bp.pack3 @dim[0]*@dim[1],data.meat,@image.meat,@nt
	@arg1.ul.y,@arg1.ul.x,@arg1.br.y,@arg1.br.x = data.unpack("I4")
	if !(@arg1.isConsistent)
	   raise "Rectangle at inlet 1 is not consistent, does not follow (ul , br) format"   
	end
	@functor.reset
    end
    def _1_rgrid_end
    end
  }
rescue StandardError => e
  GridFlow.post "%s", e
  GridFlow.post "while handling lti class %s, which has ancestors %s",
    fuc,fuc.ancestors.join(' ')
end
}
