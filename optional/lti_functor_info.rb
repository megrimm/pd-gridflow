#!/usr/bin/env ruby
# $Id$

$: << "rblti"
require "rblti"
include Rblti

#LTI=ENV["HOME"]+"/Documents/ltilib_1.9.12"
$wrap= "rblti/rblti_wrap.cxx"

def get_next_apply_function f
  while line=f.gets
    text=""
    line.gsub!(/\r?\n$/,"")
    line.gsub!(/\/\/.*$/,"")

    openbraces=0

    if line =~/^_wrap_.+_apply.+/
      #here we get every version of apply
      funcname=(line.split "_")[2]
      funcname[0,1]=funcname[0,1].upcase

      text=text+line

      #test for end of function by counting curly braces
      openbraces+=line.scan("{").size - line.scan("}").size
      while (line=f.gets) and (openbraces > 0)
        text=text+line
        openbraces+=line.scan("{").size - line.scan("}").size
      end #while

      #text contains the whole function text at this point
      return [funcname, text]
    end #if
  end #while
  f.close
  return nil
end  #function def

def get_type_errors text
  #nargs=text.scan(/^\s*if\s*\(\(argc\s*<\s*\d{1,2}/)[0].scan(/\d{1,2}/)[0].to_i   #now this is an abuse
  argnum=2
  funclines = text.split(/^/)
  typelines=Array.new
  rxp=Regexp.new("argument \" \"" + argnum.to_s + "\"\" of type")
  for singleline in funclines
    singleline.gsub!(/\r?\n$/,"")
    if singleline =~ rxp
      typelines.concat([singleline])
      argnum+=1
      rxp=Regexp.new("argument \" \"" + argnum.to_s + "\"\" of type")
    end
  end #for
  return typelines
end  #function def

class ArgMode
  def initialize x; @x=x end
  def inspect; "#{self.class}["+@x.inspect+"]" end
  class << self; alias [] new end
end
class    In<ArgMode; end
class   Out<ArgMode; end
class InOut<ArgMode; end

def fill_types(lines,functorname)
#  STDERR.puts "fill_types on #{functorname}"
  args=[]
  for sl in lines
    t = adapt_type(get_type(sl))
    if sl =~ /const/
      args << In[t]
    else
      case functorname
      when "MeanshiftTracker": args << InOut[t]
      else args << Out[t]
      end
    end
  end #for
  return args
end  #function def

$stats={}
def adapt_type type
  type = adapt_type2 type
  if $init
    $stats[type] ||= 0
    $stats[type] += 1
  end
  type
end

def adapt_type2 type
  type=type.gsub(/lti::/,"")
  type=type.sub(/unsigned char/,"ubyte")
  type=type.sub(/channel8::value_type/,"ubyte")
  type=type.sub(/channel::value_type/,"float")
  case type
  when "Matrix<ubyte >":  Umatrix; when "Vector<ubyte >":  Uvector
  when "Matrix<int >"  :  Imatrix; when "Vector<int >":    Ivector
  when "Matrix<float >":  Fmatrix; when "Vector<float >":  Fvector
  when "Matrix<double >": Dmatrix; when "Vector<double >": Dvector
  when "Trectangle<int >":    Rect
  when "Trectangle<float >":  Frect
  when "Trectangle<double >": Drect
  when "float": Float
  else Rblti.const_get(type)
  end
rescue NameError
    type
end

def get_type line
  type=line.scan(/type[\s.]*.*;/)[0]
  type.delete! "\"';"
  type[-1]=""
  type[0,6]=""
  type.chomp!(" const &")  #ugly
  type.chomp!(" &")
  if type.slice!(/^lti::/)
    type[0,1]=type[0,1].upcase!
  end
  return type
end  #function def

filehandle=File.open $wrap
previousf=""
$init=false

#termin="]\n\n"
#termin="] rescue nil\n\n"
 termin="]; rescue Exception=>e; GridFlow.post \"form error: %s\", e.inspect end\n\n"


while tmp = get_next_apply_function(filehandle)
  functorname,functiontext=tmp

  if functorname != previousf
    puts termin if $init
    if Rblti.const_defined?(functorname) and Rblti.const_get(functorname) < Functor then
      puts "begin"
      puts functorname+".forms = ["
      $init=true
    else
      $init=false
    end
  end

  previousf=functorname
  errorlines = get_type_errors functiontext
  types = fill_types(errorlines,functorname)
  print types.inspect,",\n" if $init

end #while
puts termin if $init

#puts head+":"
#text.scan(/bool\s+apply\s*([^;{])*[;{]/) {|m| puts $& }
#gets

$stats.keys.sort_by{|k|-$stats[k]}.each {|k|
  printf "\# %3d %s\n", $stats[k], k
}

=begin
 bool
 Ubyte       int          Float     double      RgbPixel         TrgbPixel(f)         DrgbPixel
 Uvector     Ivector      Fvector   Dvector                      Vector<trgbPixel(f)>
 Channel8    Channel32    Channel               Image
 Umatrix     Imatrix      Fmatrix   Dmatrix     Matrix<rgbPixel>
                          Tpoint(f)
             PointList
             TpointList(i)      TpointList(f)     TpointList(d)
                                TpolygonPoints(f) TpolygonPoints(d)
             Trectangle(i)
 Palette
 AreaPoints
 PolygonPoints
 BorderPoints
 Location
 IoPoints
 
 std::list<Fvector>
 std::list<dvector>
 std::list<location>
 std::list<areaPoints>
 std::vector<areaPoints>
 std::map<std::string,double>
 std::vector<std::vector<rectangle>>
 std::vector<matrix<float>>
 std::vector<std::vector<geometricFeatureGroup0>>
 std::vector<geometricFeatureGroup0>
 kdTree<rgbPixel,int>
 tree<objectStruct>
 std::list<borderPoints>
 std::list<ioPoints>
 std::vector<rectangle>
 std::list<channel>
 principalComponents(f)
 int (*)(int const &,int const &,int const &)
 int (*)(int const &,int const &)
=end
