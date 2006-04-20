#!/usr/bin/env ruby
# $Id$

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

def fill_types lines
  inputs=Array.new
  outputs=Array.new
  for sl in lines
    if sl =~ /const/
      inputs.concat([get_type(sl)])
    else
      outputs.concat([get_type(sl)])
    end
  end #for
  return [inputs, outputs]
end  #function def

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

while tmp = get_next_apply_function(filehandle)
  functorname,functiontext=tmp

  if functorname != previousf
    puts "]\n\n" if init
    puts functorname+".form = ["
  end

  previousf=functorname
  errorlines = get_type_errors functiontext
  types=fill_types errorlines
  types.each {|typ| print typ.inspect, ",\n" }

  init=true
end #while
puts "]\n\n" if init

#puts head+":"
#text.scan(/bool\s+apply\s*([^;{])*[;{]/) {|m| puts $& }
#gets

