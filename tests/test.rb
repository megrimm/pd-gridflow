# $Id$

require "gridflow"
include GridFlow

$imdir = "./images"
$animdir = "/opt/mex"
srand Time.new.to_i
$port = 4200+rand(100)

def pressakey; puts "press return to continue."; readline; end

class Expect < FObject
	def expect(v)
		@count=0
		@v=v
		yield
		raise "wrong number of messages (#{@count})" if @count!=1
	end
	def _0_list(*l)
		raise "got #{l.inspect} expecting #{@v.inspect}" if @v!=l
		@count+=1
	end
	install "expect", 1, 0
end

def test_math
	e = FObject["@export_list"]
	x = Expect.new
	e.connect 0,x,0

	x.expect([2,3,5,7]) { e.send_in 0,"2 3 5 7" }

	(a = FObject["@ + {0 10}"]).connect 0,e,0
	x.expect([1,12,4,18]) {
		a.send_in 0, 1,2,4,8 }

	(a = FObject["@ + 42"]).connect 0,e,0
	x.expect([43,44,46,50]) {
		a.send_in 0, 1,2,4,8 }

	x.expect([3,5,9,15]) {
		a.send_in 1, 2,3,5,7
		a.send_in 0, 1,2,4,8 }
	x.expect([11,12,14,18]) {
		a.send_in 1, 10
		a.send_in 0, 1,2,4,8 }

	(a = FObject["@! abs"]).connect 0,e,0
	x.expect([2,3,5,7]) {
		a.send_in 0, -2,3,-5,7 }

	(a = FObject["@fold * 1"]).connect 0,e,0
	x.expect([210]) { a.send_in 0, 2,3,5,7 }
	x.expect([128]) { a.send_in 0, 1,1,2,1,2,2,2,1,1,2,1,2,2 }

	(a = FObject["@scan * 1"]).connect 0,e,0
	x.expect([2,6,30,210]) { a.send_in 0, 2,3,5,7 }
	x.expect([1,1,2,2,4,8,16,16,16,32,32,64,128]) {
		a.send_in 0, 1,1,2,1,2,2,2,1,1,2,1,2,2 }

	(a = FObject["@outer +"]).connect 0,e,0
	x.expect([9,10,12,17,18,20,33,34,36]) {
		a.send_in 1, 1,2,4
		a.send_in 0, 8,16,32 }

	(a = FObject["@inner2 * + 0"]).connect 0,e,0
	(i0 = FObject["@redim {2 2}"]).connect 0,a,0
	(i1 = FObject["@redim {2 2}"]).connect 0,a,2
	x.expect([8,19,32,76]) {
		i1.send_in 0, 2,3,5,7
		i0.send_in 0, 1,2,4,8 }
	
	(a = FObject["@inner * + 0"]).connect 0,e,0
	(i0 = FObject["@redim {2 2}"]).connect 0,a,0
	(i1 = FObject["@redim {2 2}"]).connect 0,a,2
	x.expect([12,17,48,68]) {
		i1.send_in 0, 2,3,5,7
		i0.send_in 0, 1,2,4,8 }

	a = FObject["@print"]
	a.send_in 0, "3 3 # 1 0 0 0"

	(a = FObject["@outer * {3 2 # 1 2 3}"]).connect 0,e,0
	b = FObject["@dim"]
	c = FObject["@export_list"]
	a.connect 0,b,0
	y = Expect.new
	b.connect 0,c,0
	c.connect 0,y,0

	y.expect([2,3,2]) {
		x.expect([1,2,3,1,2,3,10,20,30,10,20,30]) {
			a.send_in 0, 1, 10 }}

	#pr=GridPrint.new
	(b = FObject["@redim {5 5}"]).connect 0,e,0
	(a = FObject["@convolve"]).connect 0,b,0
	(i0 = FObject["@redim {5 5 1}"]).connect 0,a,0
	(i1 = FObject["@redim {3 1}"]).connect 0,a,1
#	i1.send_in 0, 0,1,0
#	i0.send_in 0, 1,1,1,0,0,0
#	i1.send_in 0, 1,0,0
#	i0.send_in 0, 1,1,1,0,0,0
	i1.send_in 1, 3,3
	x.expect([5,6,5,4,4,4,6,7,6,4,3,3,6,7,5,4,2,3,6,6,5,4,3,4,5]) {
		i1.send_in 0, 1,1,1,1,1,1,1,1,1
		i0.send_in 0, 1,1,1,0,0,0 }
end

def test_print
	i = FObject["@redim {3}"]
	pr = FObject["@print"]
#	pr = GridFlow::RubyPrint.new
	i.connect 0,pr,0
	i.send_in 0, 85, 170, 255
	i.send_in 1, 3, 3
	i.send_in 0, 1, 0, 0, 0
	i.send_in 1, 2, 2, 2
	i.send_in 0, 2, 3, 5, 7, 11, 13, 17, 19
end

class Barf < GridObject
	def _0_rgrid_begin
		raise "barf"
	end
	install_rgrid 0
	install "barf", 1, 0
end

def test_nonsense
#	(a = FObject["@! abs"]).connect 0,e,0
#	x.expect_error {
#		a.send_in 1, 42,42 }

	a = FObject["@import {3}"]
	b = Barf.new
	a.connect 0,b,0
	begin
	  a.send_in 0, 1, 2, 3
	rescue StandardError
	  nil
	else
	  raise "Expected StandardError"
	end
	p b.inlet_dim(0)
end

def test_gen
	f0,f1 = (0..1).map { FObject["@for 0 64 1"] } # two identical objects
	f2 = FObject["@for 2 5 1"] # make vector (2,3,4)
	t0 = FObject["@outer ^"] # for combining all rows and columns
	t1 = FObject["@outer *"] # for generating colours
	gout = FObject["@out 64 64"] # open window

	# syntax: source.connect outlet,destination,inlet
	f0.connect 0,t0,0
	f1.connect 0,t0,1
	t0.connect 0,t1,0
	f2.connect 0,t1,1
	t1.connect 0,gout,0

	[f2,f1,f0].each {|o| o.send_in 0 } # sending three bangs
end

def test_image command
	gin = FObject["@in"]
	gout = FObject["@out 256 256"]
	gin.connect 0,gout,0
	31.times {
		gin.send_in 0,"open #{command}"
		gout.send_in 0,"option timelog 1"
		gin.send_in 0
	}
	FObject["@global"].send_in 0, "profiler_dump"
#	$mainloop.loop
end

def test_ppm2
	gin = FObject["@in"]
	pa = FObject["@convolve << + 0"]
	pb = FObject["@ / 9"]
	ra = FObject["@redim {3 3}"]
	gout = FObject["@out 256 256"]
	v4j = FObject["@global"]
	gin.connect 0,pa,0
	pa.connect 0,pb,0
	pb.connect 0,gout,0
	ra.connect 0,pa,1
	ra.send_in 0,"0 0"
	gout.send_in 0,"option timelog 1"
	gin.send_in 0,"open ppm file #{$imdir}/teapot.ppm"
#	gin.send_in 0,"open ppm file #{$imdir}/g001.ppm"
	30.times { gin.send_in 0 }
	v4j.send_in 0,"profiler_dump"
end

def test_anim msgs
	gin = FObject["@in"]
#	pa = FObject["@scale_by"]
	gout = FObject["@out 256 256"]
	global = FObject["@global"]
#	in.connect 0,pa,0
#	pa.connect 0,gout,0

	gin.connect 0,gout,0
	msgs.each {|m| gin.send_in 0,m}
#	gout.send_in 0,"option timelog 1"
	d=Time.new
	frames=125
	frames.times { gin.send_in 0 }
#	loop { gin.send_in 0 }
	d=Time.new-d
	printf "%d frames in %.6f seconds (avg %.6f ms, %.6f fps)\n",
		frames, d, 1000*d/frames, frames/d
	global.send_in 0,"profiler_dump"
end

def test_tcp
	if fork
		# client
		GridFlow.gfpost_header = "[client] "
		$in_client = in1 = FObject["@in"]
		out = FObject["@out 240 320"]
		in1.connect 0,out,0
		out.send_in 0,"option timelog 1"
		out.send_in 0,"option autodraw 2"
		GridFlow.gfpost "test: waiting 1 seconds"
		sleep 1
		p "HI"
		#in1.send_in 0,"open grid tcp localhost #{$port}"
		in1.send_in 0,"open grid tcp 127.0.0.1 #{$port}"
		p "HI"

		test_tcp = GridFlow::FObject.new
		def test_tcp._0_bang
			# GridFlow.gfpost "tick"
			# avoid recursion
			$mainloop.timers.after(0) {$in_client.send_in 0}
		end
		out.connect 0,test_tcp,0

		in1.send_in 0
		GridFlow.gfpost "entering mainloop..."
		$mainloop.loop
	else
		# server
		GridFlow.gfpost_header = "[server] "
		$in1_server = in1 = FObject["@in"]
		$in2_server = in2 = FObject["@in"]
		$out = out = FObject["@out"]
		toggle = 0
		in1.connect 0,out,0
		in2.connect 0,out,0
		in1.send_in 0,"open ppm file #{$imdir}/r001.ppm"
		in2.send_in 0,"open ppm file #{$imdir}/b001.ppm"
		out.send_in 0,"open grid tcpserver #{$port}"
		out.send_in 0,"option type uint8"
		test_tcp = GridFlow::FObject.new
		def test_tcp._0_bang
			# GridFlow.gfpost "tick"
			@toggle ||= 0
			# avoid recursion
			$mainloop.timers.after(0.01) {
				if $out.format.stream
					if @toggle==0; $in1_server else $in2_server end.send_in 0
					@toggle ^= 1
				end
				_0_bang
			}
		end
		out.connect 0,test_tcp,0
		test_tcp.send_in 0
		GridFlow.gfpost "entering mainloop..."
		$mainloop.loop
	end
end

def test_formats
	gin = FObject["@in"]
	gout = FObject["@out 256 256"]
	gin.connect 0,gout,0
	[
		"ppm file #{$imdir}/g001.ppm",
		"ppm file #{$imdir}/b001.ppm",
		"ppm file #{$imdir}/r001.ppm",
		"targa file #{$imdir}/teapot.tga",
		"grid gzfile #{$imdir}/foo.grid.gz",
		"grid gzfile #{$imdir}/foo2.grid.gz",
	].each {|command|
		gin.send_in 0,"open #{command}"
		gin.send_in 0
		sleep 1
	}
	
#	gin.delete
#	gout.delete
end

def test_sound
	o1 = FObject["@for 0 44100 1"] # 1 sec @ 1.225 Hz ?
	o2 = FObject["@ * 359"] # @ 439.775 Hz
	o3 = FObject["@ sin* 127"]
	o4 = FObject["@in"]
	o1.connect 0,o2,0
	o2.connect 0,o3,0
	o3.connect 0,o4,0
	o4.send_in 0,"open raw file /dev/dsp"
	o4.send_in 0,"option type int16"
	o1.send_in 0
end

def test_metro
	o1 = RtMetro.new(1000,:geiger)
	o2 = RubyPrint.new(:time)
	o1.connect 0,o2,0
	o1.send_in 0,1
	$mainloop.loop
end

test_math
test_gen
#test_image "ppm file #{$imdir}/g001.ppm"
#test_image "ppm gzfile #{$imdir}/g001.ppm.gz"
#test_image "grid file #{$imdir}/foo.grid"
#test_image "grid gzfile #{$imdir}/foo.grid.gz"
#test_print
#test_nonsense
#test_ppm2
#test_anim ["open ppm file #{$animdir}/b.ppm.cat"]
#test_anim ["open videodev /dev/video","option channel 1","option size 480 640"]
#test_anim ["open videodev /dev/video","option channel 1","option size 240 320"]
#test_anim ["open videodev /dev/video","option channel 1","option size 120 160"]
#test_anim ["open mpeg file /home/matju/net/Animations/washington_zoom_in.mpeg"]
#test_anim ["open quicktime file #{$imdir}/gt.mov"]
#test_anim ["open quicktime file /home/matju/domopers_hi.mov"]
#test_anim ["open quicktime file /home/matju/net/c.mov"]
#test_formats
#test_tcp
#test_sound
#test_metro
$mainloop.loop

