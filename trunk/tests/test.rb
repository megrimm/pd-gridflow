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
end

def test_math
	e = FObject["@export_list"]
	x = Expect.new
	e.connect 0,x,0
	x.expect([2,3,5,7]) { e.send_in 0,"2 3 5 7" }

	(a = FObject["@ +"]).connect 0,e,0
	x.expect([3,5,9,15]) {
		a.send_in 1, 2,3,5,7
		a.send_in 0, 1,2,4,8 }
end

def test_gen
	f0 = FObject["@for 0 64 1"]
	f1 = FObject["@for 0 64 1"]
	f2 = FObject["@for 2 5 1"]
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
	gin.send_in 0,"open ppm file #{$imdir}/g001.ppm"
#	gin.send_in 0,"open grid file #{$imdir}/foo.grid"
	gin.send_in 0
	$mainloop.loop
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
#	gin.send_in 0,"open ppm file #{$animdir}/b.ppm.cat"
	gin.send_in 0,"open videodev /dev/video"
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
		$in_client = in1 = FObject["@in"]
		out = FObject["@out 240 320"]
		in1.connect 0,out,0
		out.send_in 0,"option timelog 1"
		out.send_in 0,"option autodraw 2"
		GridFlow.whine "test: waiting 1 seconds"
		sleep 1
		p "HI"
		#in1.send_in 0,"open grid tcp localhost #{$port}"
		in1.send_in 0,"open grid tcp 127.0.0.1 #{$port}"
		p "HI"

		test_tcp = GridFlow::FObject.new
		def test_tcp._0_bang
			# GridFlow.whine "tick"
			# avoid recursion
			$mainloop.timers.after(0) {$in_client.send_in 0}
		end
		out.connect 0,test_tcp,0

		in1.send_in 0
		GridFlow.whine "entering mainloop..."
		$mainloop.loop
	else
		# server
		GridFlow.whine_header = "[server] "
		$in1_server = in1 = FObject["@in"]
		$in2_server = in2 = FObject["@in"]
		out = FObject["@out"]
		toggle = 0
		in1.connect 0,out,0
		in2.connect 0,out,0
		in1.send_in 0,"open ppm file #{$imdir}/r001.ppm"
		in2.send_in 0,"open ppm file #{$imdir}/b001.ppm"
		out.send_in 0,"open grid tcpserver #{$port}"
		out.send_in 0,"option type uint8"
		test_tcp = GridFlow::FObject.new
		def test_tcp._0_bang
			# GridFlow.whine "tick"
			@toggle ||= 0
			# avoid recursion
			$mainloop.timers.after(0) {
				if @toggle==0; $in1_server else $in2_server end.send_in 0
				@toggle ^= 1
			}
		end
		out.connect 0,test_tcp,0
		test_tcp.send_in 0
		GridFlow.whine "entering mainloop..."
		$mainloop.loop
	end
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

def test_sound
	o1 = FObject["@for 0 44100 1"] # 1 sec @ 1.225 Hz ?
	o2 = FObject["@ * 359"] # @ 439.775 Hz
	o3 = FObject["@ sin* 127"]
	o4 = FObject["@in"]
	o4.send_in 0,"open raw file /dev/dsp"
	o4.send_in 0,"option type int16"
	o1.connect 0,o2,0
	o2.connect 0,o3,0
	o3.connect 0,o4,0
end

#test_math
test_gen
#test_ppm1
#test_ppm2
#test_anim
#test_anim2
#test_anim3
#test_formats
#test_tcp
#test_sound
$mainloop.loop

