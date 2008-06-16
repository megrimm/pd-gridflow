all:: index.pd optable.pd

index.pd: make_index.tcl locale/english.tcl
	tclsh make_index.tcl > index.pd

optable.pd: optable.tcl
	tclsh optable.tcl > optable.pd
