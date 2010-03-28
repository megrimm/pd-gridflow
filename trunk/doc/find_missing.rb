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
].split(/\s+/)

b=Dir["../abstractions/*.pd"].map{|x|
  x.gsub(/^\.\.\/abstractions\//,"").gsub(/\.pd$/,"")
}

c      = Dir["flow_classes/*-help.pd"   ].map{|x| x.gsub(/^flow_classes\//,"").gsub(/-help\.pd$/,"") }
c.concat Dir["flow_classes/cv/*-help.pd"].map{|x| x.gsub(/^flow_classes\//,"").gsub(/-help\.pd$/,"") }
c.concat Dir["flow_classes/gf/*-help.pd"].map{|x| x.gsub(/^flow_classes\//,"").gsub(/-help\.pd$/,"") }
ab=a+b

d=[]
File.open("index.pd") {|f|
  f.each {|line|
    m=/obj -?\d+ -?\d+ ([^ ;]+)/.match(line)
    d<<m[1] if m
  }
}

def report text,list
  puts text
  puts list.sort.join" " if list.length>0
  puts "(#{list.size})"
end

report "missing from help files: ", ab-c
puts ""
report "orphan help files:", c-ab
puts ""
report "missing from index but not from help files:", ab-d-(ab-c)
puts ""
report "orphan index entries: ", d-ab
