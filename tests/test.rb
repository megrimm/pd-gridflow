# $Id$

def test_load
	require "c/lib/i686-linux/gridflow.so"
end

test_load

include GridFlow

$imdir = "./images"
$animdir = "/opt/mex"
$port = 4200+rand(100)

def pressakey; puts "press return to continue."; readline; end

def test_gen
	f0 = FObject["@for 0 64 1"]
	f1 = FObject["@for 0 64 1"]
	f2 = FObject["@for 1 4 1"]
	t0 = FObject["@outer ^"]
	t1 = FObject["@outer *"]
	gout = FObject["@out 256 256"]
	f0.connect 0,t0,0
	f1.connect 0,t0,1
	t0.connect 0,t1,0
	f2.connect 0,t1,1
	t1.connect 0,gout,0
	f2.send_in 0
	f1.send_in 0
	f0.send_in 0
end

def test_ppm1
	gin = FObject["@in"]
	gout = FObject["@out 256 256"]
	gin.connect 0,gout,0
#	gin.send_in 0,"open ppm file #{$imdir}/g001.ppm"
	gin.send_in 0,"open grid file #{$imdir}/foo.grid"
	gin.send_in 0
end

def test_ppm2
	gin = FObject["@in"]
	pa = FObject["@convolve << + 0"]
	pb = FObject["@ / 9"]
	ra = FObject["@redim 3 3"]
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

def test_anim
	gin = FObject["@in"]
#	pa = FObject["@scale_by"]
	gout = FObject["@out 256 256"]
	v4j = FObject["@global"]
#	in.connect 0,pa,0
#	pa.connect 0,gout,0

	gin.connect 0,gout,0
	gin.send_in 0,"open ppm file #{$animdir}/b.ppm.cat"
#	gin.send_in 0,"open videodev /dev/video"
	gin.send_in 0,"option channel 1"
	gin.send_in 0,"option size 480 640"
	gout.send_in 0,"option timelog 1"
	50.times { gin.send_in 0 }
	v4j.send_in 0,"profiler_dump"
end

def test_anim2
	gin = FObject["@in"]
	gout = FObject["@out 256 256"]
	gin.connect 0,gout,0
	gin.send_in 0,"open mpeg file " \
		"/home/matju/net/Animations/washington_zoom_in.mpeg"
	loop { gin.send_in 0 }
end

def test_anim3
	gin = FObject["@in"]
	gout = FObject["@out 256 256"]
	gin.connect 0,gout,0
	gin.send_in 0,"open quicktime file #{$imdir}/gt.mov"
	loop { gin.send_in 0 }
end

def test_tcp
	if fork
		# client
		GridFlow.whine_header = "[client] "
		in1 = FObject["@in"]
		out = FObject["@out 240 320"]
		in1.connect 0,out,0
		out.send_in 0,"option autodraw 2"
		GridFlow.whine "test: waiting 2 seconds"
		sleep 2
		in1.send_in 0,"open grid tcp localhost #{$port}"

		$mainloop.timers.after(1) {
			GridFlow.whine "tick"
			in1.send_in 0 unless out.inlet_busy?(0)
		}
		in1.send_in 0
		GridFlow.whine "entering mainloop..."
		$mainloop.loop
	else
		# server
		GridFlow.whine_header = "[server] "
		in1 = FObject["@in"]
		in2 = FObject["@in"]
		out = FObject["@out"]
		toggle = 0
		in1.connect 0,out,0
		in2.connect 0,out,0
		in1.send_in 0,"open ppm file #{$imdir}/r001.ppm"
		in2.send_in 0,"open ppm file #{$imdir}/b001.ppm"
		out.send_in 0,"open grid tcpserver #{$port}"
		out.send_in 0,"option type uint8"
		GridFlow.whine "now setting up timer"
		$mainloop.timers.after(1) {
			GridFlow.whine "tick1"
			unless out.inlet_busy?(0)
				GridFlow.whine "tick2"
				if toggle==0
					in1.send_in 0
				else
					in2.send_in 0
				end
			end
			toggle ^= 1
		}
		GridFlow.whine "entering mainloop..."
		$mainloop.loop
	end
end

def test_videodev
	gin  = FObject["@in"]
	gout = FObject["@out 200 200"]
	gin.connect 0,gout,0
	gin.send_in 0,"open videodev /dev/video0"
	gin.send_in 0,"option channel 1"
	gin.send_in 0
	$mainloop.loop
end

def test_formats
	gin = FObject["@in"]
	gout = FObject["@out 256 256"]
	gin.connect 0,gout,0
	gin.send_in 0,"open ppm file #{$imdir}/g001.ppm"
	gin.send_in 0
	sleep 1
	gin.send_in 0,"open ppm file #{$imdir}/b001.ppm"
	gin.send_in 0
	sleep 1
	gin.send_in 0,"open ppm file #{$imdir}/r001.ppm"
	gin.send_in 0
	sleep 1
	gin.send_in 0,"open targa file #{$imdir}/teapot.tga"
	gin.send_in 0
	sleep 1
	gin.send_in 0,"open grid file #{$imdir}/foo.grid"
	gin.send_in 0
	sleep 1
#	gin.delete
#	gout.delete
end

#test_gen
#test_ppm1
#test_ppm2
#test_anim
#test_videodev
#test_anim2
#test_anim3
#test_formats
test_tcp

$mainloop.loop
