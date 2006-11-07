proc post {args} {
  poststring2 [eval [concat [list format] $args]]
}

tclpd_class_new meuh 0
