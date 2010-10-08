#!/usr/bin/env ruby

a=[]
`grep -nri install ../src/*.[ch]xx ../src/*.m`.each {|line|
  m=/install(?:_format)?\(\"([^"]+)/.match(line)
  a<<m[1] if m
}
a-=%[
  cv/#CalcOpticalFlowBM
  cv/#CalcOpticalFlowHS
  cv/#CalcOpticalFlowLK
  cv/#CalcOpticalFlowPyrLK
  inv0x2a
  inv0x2b
  gf/mouse_spy_proxy
  #io.libv4l
].split(/\s+/)

#  cv/#Add cv/#Sub cv/#Mul cv/#Div
#  cv/#And cv/#Or cv/#Xor

b=(Dir["../abstractions/*.pd"]+Dir["../abstractions/gf/*.pd"]).map{|x|
  x.gsub(/^\.\.\/abstractions\//,"").gsub(/\.pd$/,"")
}
b-=%[
  #camera2
  #camera3
].split(/\s+/)

c      = Dir["flow_classes/*-help.pd"   ].map{|x| x.gsub(/^flow_classes\//,"").gsub(/-help\.pd$/,"") }
c.concat Dir["flow_classes/cv/*-help.pd"].map{|x| x.gsub(/^flow_classes\//,"").gsub(/-help\.pd$/,"") }
c.concat Dir["flow_classes/gf/*-help.pd"].map{|x| x.gsub(/^flow_classes\//,"").gsub(/-help\.pd$/,"") }
ab=a+b

d=[]
File.open("index.pd") {|f|
  f.each {|line|
    m= /obj -?\d+ -?\d+ doc_link ([^ ;]+)/ .match(line); if m then d<<m[1] else
    m= /obj -?\d+ -?\d+ ([^ ;]+)/          .match(line); if m then d<<m[1] end end
  }
}

def report text,list
  puts text
  puts list.sort.join" " if list.length>0
  puts "(#{list.size})"
end

report "missing from help files: ", ab-c-%w[cv/#Add cv/#And cv/#Div cv/#Mul cv/#Or cv/#Sub cv/#Xor]
puts ""
report "orphan help files:", c-ab
puts ""
report "missing from index but not from help files:", ab-d-(ab-c)
puts ""
report "orphan index entries: ", d-ab
