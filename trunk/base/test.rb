# $Id$

require "gridflow"
include GridFlow
GridFlow.verbose=true

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
	def method_missing(s,*a)
		raise "stray message: #{s}: #{a.inspect}"
	end
	install "expect", 1, 0
end

def test_math
	e = FObject["@export_list"]
	x = Expect.new
	e.connect 0,x,0

	x.expect([2,3,5,7]) { e.send_in 0,"2 3 5 7" }
	x.expect([42]*10000) { e.send_in 0,"10000 # 42" }

	(a = FObject["@two"]).connect 0,e,0
	x.expect([42,0]) { a.send_in 0,42 }
	x.expect([42,28]) { a.send_in 1,28 }
	x.expect([1313,28]) { a.send_in 0,1313 }

	(a = FObject["@three"]).connect 0,e,0
	x.expect([42,0,0]) { a.send_in 0,42 }
	x.expect([42,28,0]) { a.send_in 1,28 }
	x.expect([42,28,-1]) { a.send_in 2,-1 }

	(a = FObject["@four"]).connect 0,e,0
	x.expect([42,0,0,0]) { a.send_in 0,42 }
	x.expect([42,0,0,-42]) { a.send_in 3,-42 }

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

	(a = FObject["@fold + {0 0}"]).connect 0,e,0
	x.expect([18,23]) { a.send_in 0, 3,2,"#".intern,2,3,5,7,11,13 }

	(a = FObject["@scan + {0 0}"]).connect 0,e,0
	x.expect([2,3,7,10,18,23]) { a.send_in 0, 3,2,"#".intern,2,3,5,7,11,13 }

	(a = FObject["@scan * 1"]).connect 0,e,0
	x.expect([2,6,30,210]) { a.send_in 0, 2,3,5,7 }
	x.expect([1,1,2,2,4,8,16,16,16,32,32,64,128]) {
		a.send_in 0, 1,1,2,1,2,2,2,1,1,2,1,2,2 }

	(a = FObject["@outer +"]).connect 0,e,0
	x.expect([9,10,12,17,18,20,33,34,36]) {
		a.send_in 1, 1,2,4
		a.send_in 0, 8,16,32 }

	(a = FObject["@outer",:%,[3,-3]]).connect 0,e,0
	x.expect([0,0,1,-2,2,-1,0,0,1,-2,2,-1,0,0]) {
		a.send_in 0, -30,-20,-10,0,+10,+20,+30 }

	(a = FObject["@outer","swap%".intern,[3,-3]]).connect 0,e,0
	x.expect([-27,-3,-17,-3,-7,-3,0,0,3,7,3,17,3,27]) {
		a.send_in 0, -30,-20,-10,0,+10,+20,+30 }

	(a = FObject["@inner2 * + 0 {2 2 # 2 3 5 7}"]).connect 0,e,0
	(i0 = FObject["@redim {2 2}"]).connect 0,a,0
	x.expect([8,19,32,76]) { i0.send_in 0, 1,2,4,8 }
	
	(a = FObject["@inner * + 0 {2 2 # 2 3 5 7}"]).connect 0,e,0
	(i0 = FObject["@redim {2 2}"]).connect 0,a,0
	x.expect([12,17,48,68]) { i0.send_in 0, 1,2,4,8 }

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
	i1.send_in 1, 3,3
	x.expect([5,6,5,4,4,4,6,7,6,4,3,3,6,7,5,4,2,3,6,6,5,4,3,4,5]) {
		i1.send_in 0, 1,1,1,1,1,1,1,1,1
		i0.send_in 0, 1,1,1,0,0,0 }

	(a = FObject["@import {4}"]).connect 0,e,0
	x.expect([2,3,5,7]) {
		[2,3,5,7].each {|v| a.send_in 0,v }}

	for o in ["@store", "@store uint8"]
		(a = FObject[o]).connect 0,e,0
		a.send_in 1, 5, 4, "#".intern, 1,2,3,4,5
		x.expect([1,2,3,4,4,5,1,2,2,3,4,5]) {
			a.send_in 0, 3,1, "#".intern, 0,2,4 }
		x.expect([1,2,3,4,5]*24) { a.send_in 0, 2,3,0, "#".intern }
		x.expect([1,2,3,4,5]*4)  { a.send_in 0, 0, "#".intern }
		x.expect([1,2,3,4,5]*4)  { a.send_in 0 }
	end

	b = FObject["@dim"]
	c = FObject["@export_list"]
	a.connect 0,b,0
	y = Expect.new
	b.connect 0,c,0
	c.connect 0,y,0

	(a = FObject["@for 0 10 1"]).connect 0,e,0
	a.connect 0,b,0
	y.expect([10]) {
		x.expect((0...10).to_a) {
			a.send_in 0 } }

	(a = FObject["@for {0} {10} {1}"]).connect 0,e,0
	a.connect 0,b,0
	y.expect([10,1]) {
		x.expect((0...10).to_a) {
			a.send_in 0 } }

	(a = FObject["@for {2 3} {5 7} {1 1}"]).connect 0,e,0
	a.connect 0,b,0
	y.expect([3,4,2]) {
		x.expect([2,3,2,4,2,5,2,6,3,3,3,4,3,5,3,6,4,3,4,4,4,5,4,6]) {
			a.send_in 0 } }
end

def test_rtmetro
	rt = FObject["rtmetro 1000"]
	pr = FObject["rubyprint"]
	rt.connect 0,pr,0
	GridFlow.post "trying to start the rtmetro"
	rt.send_in 0,1
	$mainloop.timers.after(10.0) {
		GridFlow.post "trying to stop the rtmetro (after 10 sec delay)"
		rt.send_in 0,0
	}
	$mainloop.loop
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

# generates recursive checkerboard pattern (munchies) in bluish colours.     
def test_munchies
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
	$mainloop.loop
end

def test_image command
	gin = FObject["@in"]
	gout = FObject["@out 256 256"]
	gin.connect 0,gout,0
#	31.times {
	3.times {
		gin.send_in 0,"open #{command}"
		gout.send_in 0,"option timelog 1"
		gin.send_in 0
	}
	FObject["@global"].send_in 0, "profiler_dump"
	$mainloop.loop
end

def test_ppm2
	gin = FObject["@in"]
	store = FObject["@store"]
	pa = FObject["@convolve << + 0"]
	pb = FObject["@ / 9"]
	ra = FObject["@redim {3 3}"]
#	gout = FObject["@out 256 256"]
	gout = FObject["@out"]
#	gout.send_in 0, "open sdl"
	gout.send_in 0, "open x11 here"
	gin.connect 0,store,1
	store.connect 0,pa,0
	pa.connect 0,pb,0
	pb.connect 0,gout,0
	ra.connect 0,pa,1
	ra.send_in 0,"0 0"
	gout.send_in 0,"option timelog 1"
	gin.send_in 0,"open ppm file #{$imdir}/teapot.ppm"
#	gin.send_in 0,"open ppm file #{$imdir}/g001.ppm"
	gin.send_in 0
#	40.times { store.send_in 0 }
	loop { store.send_in 0 }
	v4j = FObject["@global"]
	v4j.send_in 0,"profiler_dump"
#	$mainloop.loop
end

def test_foo
	foo = FObject["@for {0 234} {0 345} {1 1}"]
	che = FObject["@checkers"]
	out = FObject["@out x11"]
	foo.connect 0,che,0
	che.connect 0,out,0
end

def test_anim msgs
	gin = FObject["@in"]
	gout = FObject["@out 256 256"]
proc{
	gout = FObject["@fold + 0"]
	gout2 = FObject["@ / 3"]
	gout3 = FObject["@outer + {0}"]
	gout4 = FObject["@out"]
	gout4.send_in 0, "open aalib X11"
	gout.connect 0,gout2,0
	gout2.connect 0,gout3,0
	gout3.connect 0,gout4,0
}

	gin.connect 0,gout,0
#	pr = FObject["rubyprint time"]; gout.connect 0,pr,0
	msgs.each {|m| gin.send_in 0,m}
#	gout.send_in 0,"option timelog 1"
	d=Time.new
	frames=300
	frames.times { gin.send_in 0 }
#	loop { gin.send_in 0 }
#	metro = FObject["rtmetro 80"]
#	metro.connect 0,gin,0
#	metro.send_in 0,1
#	$mainloop.loop

	d=Time.new-d
	printf "%d frames in %.6f seconds (avg %.6f ms, %.6f fps)\n",
		frames, d, 1000*d/frames, frames/d
#	global.send_in 0,"dfgdfgdkfjgl"
	global = FObject["@global"]
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
		"jpeg file #{$imdir}/ruby0216.jpg",
		"ppm file #{$imdir}/g001.ppm",
		"targa file #{$imdir}/teapot.tga",
		"grid gzfile #{$imdir}/foo.grid.gz",
		"grid gzfile #{$imdir}/foo2.grid.gz",
#		"targa file #{$imdir}/tux.tga",
	].each {|command|
		gin.send_in 0,"open #{command}"
#		gin.send_in 0
		2.times { gin.send_in 0 } # test for load, rewind, load
		sleep 1
	}
	
#	gin.delete
#	gout.delete
end

def test_formats_write
	# read files, store and save them, reload, compare, expect identical
	a = FObject["@in"]
	b = FObject["@out"]; a.connect 0,b,0
	c = FObject["@ -"]; a.connect 0,c,1
	d = FObject["@in"]; d.connect 0,c,0
	e = FObject["@fold +"]; c.connect 0,e,0
	f = FObject["@fold +"]; e.connect 0,f,0
	g = FObject["@fold +"]; f.connect 0,g,0
	h = FObject["@export_list"]; g.connect 0,h,0
	x = Expect.new; h.connect 0,x,0
	[
		["ppm file", "#{$imdir}/g001.ppm"],
#		["targa file", "#{$imdir}/teapot.tga"],
		["grid gzfile", "#{$imdir}/foo.grid.gz"],
	].each {|type,file|
		a.send_in 0, "open #{type} #{file}"
		b.send_in 0, "open #{type} /tmp/patate"
		a.send_in 0
		b.send_in 0, "close"
		raise "written file does not exist" if not File.exist? "/tmp/patate"
		d.send_in 0, "open #{type} /tmp/patate"
		x.expect([0]) { d.send_in 0 }
	}	
end

def test_headerless
	gout = FObject["@out"]
	gout.send_in 0, "open grid file #{$imdir}/hello.txt"
	gout.send_in 0, "option headerless"
	gout.send_in 0, "option type uint8"
	gout.send_in 0, "104 101 108 108 111 32 119 111 114 108 100 33 10"
	gout.send_in 0, "close"
	gin = FObject["@in"]
	pr = FObject["@print"]
	gin.connect 0,pr,0
	gin.send_in 0, "open grid file #{$imdir}/hello.txt"
	gin.send_in 0, "option headerless 13"
	gin.send_in 0, "option type uint8"
	gin.send_in 0
end


def test_sound
#	o2 = FObject["@ * 359"] # @ 439.775 Hz
#	o1 = FObject["@for 0 44100 1"] # 1 sec @ 1.225 Hz ?
	o1 = FObject["@for 0 4500 1"]
	o2 = FObject["@ * 1600"] # @ 439.775 Hz
	o3 = FObject["@ sin* 255"]
	o4 = FObject["@ gamma 400"]
	o5 = FObject["@ << 7"]
	out = FObject["@out"]
	o1.connect 0,o2,0
	o2.connect 0,o3,0
	o3.connect 0,o4,0
	o4.connect 0,o5,0
	o5.connect 0,out,0
#	out.send_in 0,"open raw file /dev/dsp"
	out.send_in 0,"open grid file /dev/dsp"
	out.send_in 0,"option type int16"
	x=0
	loop {
		o4.send_in 1, x
		o1.send_in 0
		x+=10
	}
end

def test_polygon
	o1 = FObject["@for 0 5 1"]
	o2 = FObject["@ * 14400"]; o1.connect 0,o2,0
	o3 = FObject["@outer + {0 9000}"]; o2.connect 0,o3,0
	o4 = FObject["@ cos* 60"]; o3.connect 0,o4,0
	o5 = FObject["@ + {160 100}"]; o4.connect 0,o5,0
	poly = FObject["@polygon {3 # 255}"]; o5.connect 0,poly,2
	out = FObject["@out 240 320"]; poly.connect 0,out,0
	poly.send_in 0, "240 320 # 0"
end

def test_metro
	o1 = RtMetro.new(1000,:geiger)
	o2 = RubyPrint.new(:time)
	o1.connect 0,o2,0
	o1.send_in 0,1
	$mainloop.loop
end

if ARGV[0] then
	send "test_#{ARGV[0]}"
	exit 0
end

#test_polygon
#test_math
#test_munchies
#test_image "ppm file #{$imdir}/g001.ppm"
#test_image "ppm gzfile #{$imdir}/g001.ppm.gz"
#test_image "grid file #{$imdir}/foo.grid"
#test_image "grid gzfile #{$imdir}/foo.grid.gz"
#test_print
#test_nonsense
#test_ppm2
#test_anim ["open ppm file #{$imdir}/g001.ppm"]
#test_anim ["open ppm file #{$animdir}/b.ppm.cat"]
#test_anim ["open videodev /dev/video","option channel 1","option size 480 640"]
#test_anim ["open videodev /dev/video15 noinit","option transfer read"]
#test_anim ["open videodev /dev/video","option channel 1","option size 120 160"]
test_anim ["open mpeg file /home/matju/net/Animations/washington_zoom_in.mpeg"]
#test_anim ["open quicktime file #{$imdir}/gt.mov"]
#test_anim ["open quicktime file /home/matju/domopers_hi.mov"]
#test_anim ["open quicktime file /home/matju/net/c.mov"]
#test_formats
#test_tcp
#test_sound
#test_metro
#$mainloop.loop

