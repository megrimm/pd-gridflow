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

#GridFlow.post "BLAH START"
#GridFlow.whatever(:gui,"TEST\n")
#GridFlow.whatever(:gui,".log.1 insert end {This is GridFlow\n}\n")
#GridFlow.post "BLAH END"

#Thread.new { loop { GridFlow.post "thread test"; sleep 1 }}