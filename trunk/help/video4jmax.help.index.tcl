#!/usr/bin/tclsh

catch {
if {[string compare [lindex $argv 0] generate] == 0} {
	# should generate Scheme code here (or something)

	proc helpSummary {name file} {
		# write me
	}

	proc helpPatch {name file} {
		# write me
	}
}
}

helpSummary "video4jmax summary" $dir/video4jmax.summary.jmax

foreach name {
	@
	@store
	@import
	@export
	@video_out
	@video_out_file
	@video_in_file
} {
	helpPatch $name $dir/$name.help.jmax
}
