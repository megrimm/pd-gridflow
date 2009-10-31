#!/usr/bin/env ruby

a=[]
`grep -nriw install ../*.[ch] ../*/*.[chm]`.each {|line|
  m=/install\(\"([^"]+)/.match(line)
  a<<m[1] if m
}

b=Dir["../abstractions/*.pd"].map{|x|
  x.gsub(/^\.\.\/abstractions\//,"").gsub(/\.pd$/,"")
}

c      = Dir["flow_classes/*-help.pd"   ].map{|x| x.gsub(/^flow_classes\//,"").gsub(/-help\.pd$/,"") }
c.concat Dir["flow_classes/cv/*-help.pd"].map{|x| x.gsub(/^flow_classes\//,"").gsub(/-help\.pd$/,"") }
ab=a+b

 puts (ab-c).sort.join" "
 puts (ab-c).size

#puts (c-ab).sort.join" "
