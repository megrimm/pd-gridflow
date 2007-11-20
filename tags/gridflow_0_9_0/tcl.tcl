#set i 0
#foreach c [info commands] {if {[string compare [string range $c 0 1] t_]} {incr i}}
#puts $i

package require poe
#load ./gridflow2.so

proc post {args} {
  poststring2 [eval [concat [list format] $args]]
}

tclpd_class_new tcl 0
proc tcl {args} {}

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

#--------#--------#--------#--------#--------#--------#--------#--------#
# this comes from federico

tclpd_class_new list_change 0

proc list_change {self args} {
  global _
  set _($self:oldlist) [list]
  set _($self:curlist) [list]
  proc $self {selector args} \
    "eval \[concat \[list list_change_\$selector $self\] \$args\]"
  return $self
}

proc list_change_0_list {self args} {
  global _
  set _($self:oldlist) $_($self:curlist)
  set _($self:curlist) $args
  list_change_check $self
}

proc list_change_check {self} {
  global _
  set l [llength $_($self:curlist)]
  for {set i 0} {$i < $l} {incr i} {
    if {[lindex $_($self:curlist) $i] != [lindex $_($self:oldlist) $i]} {
      outlet_list 0 [list change $i $_($self:curlist)]
    }
  }
}
