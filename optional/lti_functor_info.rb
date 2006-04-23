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

def fill_types lines
  args=[]
  for sl in lines
    t = adapt_type(get_type(sl))
    if sl =~ /const/
      args << In[t]
    else
      args << Out[t]
    end
  end #for
  return args
end  #function def

def adapt_type type
  case type
  when "Matrix<lti::ubyte >": Umatrix
  when "Matrix<int >"  :      Imatrix
  when "Matrix<float >":      Fmatrix
  when "Matrix<double >":     Dmatrix
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
init=false

#termin="]\n\n"
#termin="] rescue nil\n\n"
 termin="]; rescue Exception=>e; GridFlow.post \"form error: %s\", e.inspect end\n\n"

while tmp = get_next_apply_function(filehandle)
  functorname,functiontext=tmp

  if functorname != previousf
    puts termin if init
    puts "begin"
    puts functorname+".form = ["
  end

  previousf=functorname
  errorlines = get_type_errors functiontext
  types = fill_types errorlines
  print types.inspect,",\n"

  init=true
end #while
puts termin if init

#puts head+":"
#text.scan(/bool\s+apply\s*([^;{])*[;{]/) {|m| puts $& }
#gets

