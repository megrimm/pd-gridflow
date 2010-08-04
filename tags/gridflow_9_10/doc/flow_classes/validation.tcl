#!/usr/bin/env tclsh

source ../../tcl/pd_parser.tcl

foreach fname [glob *-help.pd] {
  set f [open $fname]
  set elements {doc_h doc_c doc_i doc_o doc_also doc_f}
  foreach e $elements {set count($e) 0}
  foreach line [pd_mess_split [read $f]] {
    foreach e $elements {
      if {[lindex $line 1] == "obj" && [lindex $line 4] == $e} {
	incr count($e)
      }
    }
  }
  foreach e $elements {if {$count($e) != 1} {puts "$fname $e: $count($e)"}}
}

