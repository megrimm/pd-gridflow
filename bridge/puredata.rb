%w(
	@apply_colormap_channelwise @cast @checkers @complex_sq @contrast
	@convolve @dim @downscale_by @draw_polygon @export=@importexport
	@finished @fold @for @four=@twothreefour @global @grade
	@greyscale_to_rgb @import=@importexport @inner2 @inner=@foldinnerouter
	@in=@inout @join @layer @outer=@foldinnerouter @out=@inout
	@! @ @perspective @posterize printargs @print @ravel @redim
	@rgb_to_greyscale rubyprint @scale_by @scale_to @scan @solarize
	@spread @store @three=@twothreefour @two=@twothreefour
).each {|name|
	if name =~ /=/ then name,file = name.split(/=/) else file = name end
	begin
		GridFlow.whatever(:help,name,"gridflow/#{file}.pd")
	rescue Exception => e
		GridFlow.post "ruby #{e.class}: #{e}:\n" + e.backtrace.join("\n")
	end
}

#GridFlow.whatever(:gui,"frame .controls.gridflow -relief ridge -borderwidth 2\n")
#GridFlow.whatever(:gui,"button .controls.gridflow.button -text FOO\n")
#GridFlow.whatever(:gui,"pack .controls.gridflow.button -side left\n")
#GridFlow.whatever(:gui,"pack .controls.gridflow -side right\n")

GridFlow.whatever(:gui,%q{
menu .mbar.gridflow -tearoff $pd_tearoff
.mbar add cascade -label "GridFlow" -menu .mbar.gridflow
.mbar.gridflow add command -label "profiler_dump" -command {pd "gridflow profiler_dump;"}
.mbar.gridflow add command -label "profiler_reset" -command {pd "gridflow profiler_reset;"}
frame .ruby
label .ruby.label -text "Ruby: "
entry .ruby.eval -width 40
pack .ruby.label -side left
pack .ruby.eval -side left -fill x -expand yes
pack .ruby -fill x -expand yes
proc ruby_eval {} {
	set l {}
	foreach c [split [.ruby.eval get] ""] {lappend l [scan $c %c]}
	pd "gridflow eval $l;"
	.ruby.eval delete 0 end
}
bind .ruby.eval <Return> {ruby_eval}
})

#GridFlow.post "BLAH START"
#GridFlow.whatever(:gui,"TEST\n")
#GridFlow.whatever(:gui,".log.1 insert end {This is GridFlow\n}\n")
#GridFlow.post "BLAH END"

#Thread.new { loop { GridFlow.post "thread test"; sleep 1 }}