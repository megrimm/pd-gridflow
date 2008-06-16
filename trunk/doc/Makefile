all:: index.pd numop.pd numbertype.pd

index.pd: make_index.tcl locale/english.tcl
	tclsh make_index.tcl > index.pd

numop.pd numbertype.pd: moulinette.tcl
	tclsh moulinette.tcl
