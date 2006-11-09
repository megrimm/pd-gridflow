proc post {args} {
  poststring2 [eval [concat [list format] $args]]
}

#--------#--------#--------#--------#--------#--------#--------#--------#
# this is a personalized helloworld using tcl.pd_linux but without objective.tcl

tclpd_class_new helloworld 0

proc helloworld_0_bang {self} {
  global _
  post "Hello world (and especially %s)!" $_($self:yourname)
}

proc helloworld {self yourname} {
  global _
  post "helloworld self=%s yourname=%s" $self $yourname
  set _($self:yourname) $yourname
  proc $self {selector args} "eval \[concat \[list helloworld_\$selector $self\] \$args\]"
  return $self
}
