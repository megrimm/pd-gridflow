#!/usr/bin/tclsh

#set dir .
#set argv generate

catch {
if {[string compare [lindex $argv 0] generate] == 0} {
	# should generate Scheme code here (or something)

	proc helpSummary {name file} {
		# write me
	}

	proc helpPatch {name file} {
		# write me
		puts "helpPatch $name $file"
	}
}
}

helpSummary "GridFlow summary" $dir/gridflow.summary.jmax

foreach entry {
	{@inout {@in @out}}
	{@importexport {@import @export}}
	{@twothreefour {@two @three @four}}

	@for

	@
	@!
	{@foldinnerouter {@fold @inner @outer}}

	@redim
	@store
	@identity_transform
	@scale_to
	@spread

	@dim

	@convolve
	@scale_by
	@contrast
	@posterize

	rtmetro
	@global
} {
	if {[llength $entry] == 1} {
		helpPatch $entry $dir/$entry.help.jmax

	} else {
		foreach name [lindex $entry 1] {
			helpPatch $name $dir/[lindex $entry 0].help.jmax
		}
	}
}
