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
	@dim
	@fold
	@outer
	@redim
	@template_contrast
	@template_outer
	@template_posterize
	@template_redim
	@template_scale_to
	@template_spread
	@template_identity_transform
	@template_three
	@template_two

} {
	helpPatch $name $dir/$name.help.jmax
}
