=begin
	$Id$

	GridFlow
	Copyright (c) 2001-2007 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
=end

module GridFlow
GridFlow = ::GridFlow # ruby is nuts... sometimes

# a dummy class that gives access to any stuff global to GridFlow.
FObject.subclass("gridflow",1,1) {
	def _0_formats
		post "-"*32
		GridFlow.fclasses.each {|k,v|
			next if not /#io:/ =~ k
			modes = case v.flags
			when 2; "#out"
			when 4; "#in"
			when 6; "#in/#out"
			else "??? #{v.flags}"
			end
			post "%s %s: %s", modes, k, v.description
			post "-> %s", v.args
		}
		post "-"*32
	end
	# security issue if patches shouldn't be allowed to do anything they want
	def _0_eval(*l)
		s = l.map{|x|x.to_i.chr}.join""
		post "ruby: %s", s
		post "returns: %s", eval(s).inspect
	end
	def _0_load(s) load s.to_s end
	add_creator "@global"
	GridFlow.bind "gridflow", "gridflow" rescue Exception
}
FObject.subclass("fps",1,1) {
	def initialize(*options)
		super
		@history = []   # list of delays between incoming messages
		@last = 0.0     # when was last time
		@duration = 0.0 # how much delay since last summary
		@period = 1     # minimum delay between summaries
		@detailed = false
		@mode = :real
		options.each {|o|
			case o
			when :detailed; @detailed=true
			when :real,:user,:system,:cpu; @mode=o
			end
		}
		def @history.moment(n=1)
			sum = 0
			each {|x| sum += x**n }
			sum/length
		end
	end
	def method_missing(*a) end # ignore non-bangs
	def _0_period x; @period=x end
	def publish
		@history.sort!
		n=@history.length
		fps = @history.length/@duration
		if not @detailed then send_out 0, fps; return end
		send_out 0, fps,
			1000*@history.min,
			500*(@history[n/2]+@history[(n-1)/2]),
			1000*@history.max,
			1000/fps,
			1000*(@history.moment(2) - @history.moment(1)**2)**0.5
	end
	def _0_bang
		t = case @mode
		when :real; Time.new.to_f
		when :user; Process.times.utime
		when :system; Process.times.stime
		when :cpu; GridFlow.rdtsc/GridFlow.cpu_hertz
		end
		@history.push t-@last
		@duration += t-@last
		@last = t
		return if @duration<@period
		fps = @history.length/@duration
		publish if fps>0.001
		@history.clear
		@duration = 0
	end
}

FObject.subclass("unix_time",1,3) {
  def _0_bang
    t = Time.new
    tt = t.to_s
    send_out 2, t.year, t.month, t.day, t.hour, t.min, t.day
    send_out 1, t.to_i/86400, t.to_i%86400,
	((t.to_f-t.to_f.floor)*1000000).to_i
    a=[]
    for char in tt.split("") do a << char[0] end
    send_out 0, :list, *a
  end
}
### test with "shell xlogo &" -> [exec]
FObject.subclass("exec",1,0) {
        def _0_shell(*a) system(a.map!{|x| x.to_s }.join(" ")) end
}
FObject.subclass("renamefile",1,0) {
        def initialize; end
        def _0_list(a,b) File.rename(a.to_s,b.to_s) end
}
FObject.subclass("ls",1,1) {
        def _0_symbol(s) send_out 0, :list, *Dir.new(s.to_s).map {|x| x.intern } end
        def _0_glob  (s) send_out 0, :list, *Dir[    s.to_s].map {|x| x.intern } end
}

#<vektor> told me to:
# RGBtoYUV : @fobjects = ["#inner (3 3 # 66 -38 112 128 -74 -94 25 112 -18)",
#	"@ >> 8","@ + {16 128 128}"]
# YUVtoRGB : @fobjects = ["@ - (16 128 128)",
#	"#inner (3 3 # 298 298 298 0 -100 516 409 -208 0)","@ >> 8"]

FObject.subclass("args",1,1) {
	def initialize(*argspecs) @argspecs = argspecs end
	#def initialize2; add_outlets @argspecs.length end
	def initialize2; self.noutlets = @argspecs.length+1 end
#	def split(ary,sep)
#		i=0; r=[[]]
#		ary.each {|x| if sep===x then r.last << x else r << [] end }
#		r
#	end
	def _0_bang
		pa,*loadbang=patcherargs.split(:",")
		GridFlow.handle_braces! pa
		#GridFlow.post "patcherargs=%s", pa.inspect
		i=@argspecs.length-1
		while i>=0 do
			#GridFlow.post "argspecs[%i]=%s", i, @argspecs[i].inspect
			#GridFlow.post "argspecs[%i] isa %s", i, @argspecs[i].class
			#GridFlow.post "      pa[%i]=%s", i, pa[i].inspect
			v = pa[i]
			if not v then
				if Array===@argspecs[i] and @argspecs[i][2] then
					v = @argspecs[i][2]
				elsif @argspecs[i].to_s!="*"
					#raise "missing argument!"
					GridFlow.post "missing %d argument", i+1
					next
				end
			end
			if @argspecs[i].to_s=="*" then
				rest = pa[i..-1]||[]
				send_out i,:list,*rest
				break
			end
			case pa[i]
			when Symbol; send_out i,:symbol,v
			when  Array; send_out i,:list, *v
			else         send_out i,        v
			end
			i-=1
		end
		if pa.length > @argspecs.length then
			GridFlow.post "warning: too many args (%d for %d)", pa.length, @argspecs.length
			GridFlow.post "... argspecs=%s",@argspecs.inspect
			GridFlow.post "... but got %s",pa.inspect
		end
		loadbang.each {|m| send_out @argspecs.length, *m }
	end
}

FObject.subclass("#rotatificator",2,1) {
	def _0_float(scale)
		n = @axis[2]
		rotator = (0...n).map {|i| (0...n).map {|j| if i==j then scale else 0 end }}
		th = @angle * Math::PI / 18000
		(0...2).each {|i| (0...2).each {|j|
				a = @axis[i].to_i
				b = @axis[j].to_i
				rotator[a][b] = (scale*Math.cos(th+(j-i)*Math::PI/2)).to_i
		}}
		send_out 0,:list,n,n,:"#",*rotator.flatten
	end
	def _0_axis(from,to,total)
		total>=0 or raise "total-axis number incorrect"
		from>=0 and from<total or raise "from-axis number incorrect"
		to  >=0 and to  <total or raise   "to-axis number incorrect"
		@axis = [from.to_i,to.to_i,total.to_i]
	end
	def initialize(axis=[0,1,2])
		super
		@angle=0
		_0_axis(*axis)
	end
	def _1_float(angle) @angle = angle end
}

FObject.subclass("listflatten",1,1) {
	def initialize() super end
	def _0_list(*a) send_out 0,:list,*a.flatten end
}
FObject.subclass("rubysprintf",2,1) {
  def initialize(*format) _1_list(format) end
  def _0_list(*a) send_out 0, :symbol, (sprintf @format, *a).intern end
  alias _0_float _0_list
  alias _0_symbol _0_list
  def _1_list(*format) @format = format.join(" ") end
  alias _1_symbol _1_list
}

class PDNetSocket < FObject
	def initialize(host,port,protocol=:udp,*options)
		super
		_1_connect(host,port,protocol)
		@options = {}
		options.each {|k|
			k=k.intern if String===k
			@options[k]=true
		}
	end
	def _1_connect(host,port,protocol=:udp)
		host = host.to_s
		port = port.to_i
		@host,@port,@protocol = host.to_s,port.to_i,protocol
		case protocol
		when :udp
			@socket = UDPSocket.new
			if host=="-" then
				@socket.bind nil, port
			end
		when :tcp
 			if host=="-" then
				@server = TCPServer.new("localhost",port)
			else
				@socket = TCPSocket.new(host,port)
			end
			
		end
		@clock = Clock.new self
		@clock.delay 0
		@data = ""
	end
	def encode(x)
		x=x.to_i if @options[:nofloat] and Float===x
		x.to_s
	end
	def method_missing(sel,*args)
		sel=sel.to_s
		sel.sub!(/^_\d_/, "") or return super
		sel=(case sel; when "int","float"; ""; else encode(sel) end)
		msg = [sel,*args.map{|arg| encode(arg) }].join(" ")
		if @options[:nosemicolon] then msg << "\n" else msg << ";\n" end
		post "encoding as: %s", msg.inspect if @options[:debug]
		case @protocol
		when :udp; @socket.send msg, 0, @host, @port
		when :tcp; @socket.send msg, 0
		end
	end
	def delete; @clock.unset; @socket.close end
	def decode s
		post "decoding from: %s", s.inspect if @options[:debug]
		s.chomp!("\n")
		s.chomp!("\r")
		s.chomp!(";")
		a=s.split(/[\s\0]+/)
		a.shift if a[0]==""
		a.map {|x|
			case x
			when /-?\d+$/; x.to_i
			when /-?\d/; x.to_f
			else x.intern
			end
		}
	end
	def call
		ready_to_accept = IO.select [@server],[],[],0 if @server
		if ready_to_accept
			@socket.close if @socket
			@socket = @server.accept
		end
		ready_to_read = IO.select [@socket],[],[],0 if @socket
		return if not ready_to_read
		case @protocol
		when :udp
			data,sender = @socket.recvfrom 1024
			send_out 1, sender.map {|x| x=x.intern if String===x; x }
			send_out 0, *(decode data)
		when :tcp
			@data << @socket.sysread(1024)
			sender = @socket.peeraddr
			loop do
				n = /\n/ =~ @data
				break if not n
				send_out 1, sender.map {|x| x=x.intern if String===x; x }
				send_out 0, *(decode @data.slice!(0..n))
			end
		end
		@clock.delay 50
	end
	install "pd_netsocket", 2, 2
end

# make this with variable number of outlets?
# what about moving this to desiredata?
FObject.subclass("fork",1,2) {
  def method_missing(sel,*args)
    sel.to_s =~ /^_(\d)_(.*)$/ or super
	send_out 1,$2.intern,*args
	send_out 0,$2.intern,*args
  end
}
FObject.subclass("shunt",2,0) {
	gfattr :index
	gfattr :mode
	gfattr :hi
	gfattr :lo
	def initialize(n=2,i=0) super; @n=n; @hi=n-1; @lo=0; @mode=0; @index=i end
	def initialize2; add_outlets @n end
	def method_missing(sel,*args)
		sel.to_s =~ /^_(\d)_(.*)$/ or super
		send_out @index,$2.intern,*args
		if @mode!=0 then
			@index += @mode<=>0
			if @index<@lo or @index>@hi then
				k = @hi-@lo+1
				m = @mode.abs
				if m==1 then @index = ((@index-@lo) % k)+@lo
				else @mode=-@mode; @index+=@mode end
			end
		end
	end
	def _1_float i; @index=i.to_i % @n end
	# hack: this is an alias.
	class Demux < self; install "demux", 2, 0; end
}

# maybe that some of them could be replaced by 0.39 abstractions
	FObject.subclass("listmake",2,1) {
		def initialize(*a) @a=a end
		def _0_list(*a) @a=a; _0_bang end
		def _1_list(*a) @a=a end
		def _0_bang; send_out 0, :list, *@a end
	}
	FObject.subclass("listlength",1,1) {
		def initialize() super end
		def _0_list(*a) send_out 0, a.length end
	}
	FObject.subclass("listelement",2,1) {
		def initialize(i=0) super; @i=i.to_i end
		def _1_float(i) @i=i.to_i end
		def _0_list(*a)
			e=a[@i]
			if Symbol===e then
				send_out 0, :symbol, e
			else
				send_out 0, e
			end
		end
	}
	FObject.subclass("listsublist",3,1) {
		def initialize(i=0,n=1) super; @i,@n=i.to_i,n.to_i end
		def _1_float(i) @i=i.to_i end
		def _2_float(n) @n=n.to_i end
		def _0_list(*a) send_out 0, :list, *a[@i,@n] end
	}
	FObject.subclass("listprepend",2,1) {
		def initialize(*b) super; @b=b end
		def _0_list(*a) a[0,0]=@b; send_out 0, :list, *a end
		def _1_list(*b) @b=b end
	}
	FObject.subclass("listappend",2,1) {
		def initialize(*b) super; @b=b end
		def _0_list(*a) a[a.length,0]=@b; send_out 0, :list, *a end
		def _1_list(*b) @b=b end
	}
	FObject.subclass("listreverse",1,1) {
		def initialize() super end
		def _0_list(*a) send_out 0,:list,*a.reverse end
	}
	FObject.subclass("messageprepend",2,1) {
		def initialize(*b) super; @b=b end
		def _0_(*a) a[0,0]=@b; send_out 0, *a end
		def _1_list(*b) @b=b end
		def method_missing(sym,*a)
			(m = /(_\d_)(.*)/.match sym.to_s) or return super
			_0_ m[2].intern, *a
		end
	}
	FObject.subclass("messageappend",2,1) {
		def initialize(*b) super; @b=b end
		def _0_(*a) a[a.length,0]=@b; send_out 0, *a end
		def _1_list(*b) @b=b end
		def method_missing(sym,*a)
			(m = /(_\d_)(.*)/.match sym.to_s) or return super
			_0_ m[2].intern, *a
		end
	}

FObject.subclass("route2",1,1) {
	def initialize(*b)
		super; @b=b; @bh={}
		b.each_with_index {|x,i| @bh[x]=i }
	end
	def initialize2(*b) add_outlets @b.length end
	def _0_(*a) i=@bh[a[0]]||@b.length; send_out i,*a end
	def _1_list(*b) @b=b end
	def method_missing(sym,*a)
		(m = /(_\d_)(.*)/.match sym.to_s) or return super
		_0_ m[2].intern, *a
	end
}

FObject.subclass("range",1,1) {
  def initialize(*a) @a=a end
  def initialize2
    add_inlets  @a.length
    add_outlets @a.length
  end
  def _0_list(x) _0_float(x) end
  def _0_float(x) i=0; i+=1 until @a[i]==nil or x<@a[i]; send_out i,x end
  def method_missing(sel,*a)
    m = /^(_\d+_)(.*)/.match(sel.to_s) or return super
    m[2]=="float" or return super
    @a[m[1].to_i-1] = a[0]
    post "setting a[#{m[1].to_i-1}] = #{a[0]}"
  end
}
FObject.subclass("listfind",2,1) {
  def initialize(*a) _1_list(*a) end
  def _1_list(*a) @a = a end
  def _0_float(x)
    i=0
    while i<@a.length
      (send_out 0,i; return) if @a[i]==x
      i+=1
    end
    send_out 0,-1
  end
  #doc:_1_list,"list to search into"
  #doc:_0_float,"float to find in that list"
  #doc_out:_0_float,"position of the incoming float in the stored list"
}

module Gooey # to be included in any FObject class
	def initialize(*)
		super
		@selected=false
		@bg  = "#ffffff" # white background
		@bgb = "#000000" # black border
		@bgs = "#0000ff" # blue border when selected
		@fg  = "#000000" # black foreground
		@rsym = (self.class.to_s+":"+"%08x"%(2*self.object_id)).intern # unique id for use in Tcl
		@can = nil    # the canvas number
		@canvas = nil # the canvas string
		@y,@x = 0,0 # position on canvas
		@sy,@sx = 16,16 # size on canvas
		@font = "Courier -12"
		@vis = nil
	end
	attr_reader :canvas
	attr_reader :selected
	def canvas=(can)
		@can = can if Integer===can
		@canvas = case can
		  when String; can
		  when Integer; ".x%x.c"%(4*can)
		  else raise "huh?"
		end
	end
	def initialize2(*) GridFlow.bind self, @rsym.to_s end
	def pd_displace(can,x,y) self.canvas||=can; @x+=x; @y+=y; pd_show(can) end
	def pd_activate(can,*) self.canvas||=can end
	def quote(text)
		#STDERR.puts "quote  in: "+text
		text=text.gsub(/["\[\]\n\$]/m) {|x| if x=="\n" then "\\n" else "\\"+x end }
		text="\"#{text}\""
		#STDERR.puts "quote out: "+text
		text
	end
	def pd_vis(can,vis)
	self.canvas||=can; @vis=vis!=0; update end
	def update; pd_show @can if @vis end
	def pd_getrect(can)
		self.canvas||=can
		@x,@y = get_position(can)
		# the extra one-pixel on each side was for #peephole only
		# not sure what to do with this
		[@x-1,@y-1,@x+@sx+1,@y+@sy+1]
	end
	def pd_click(can,x,y,shift,alt,dbl,doit) return 0 end
	def outline; if selected then @bgs else "#000000" end end
	def pd_select(can,sel)
		self.canvas||=can
		@selected=sel!=0
		GridFlow.gui %{ #{canvas} itemconfigure #{@rsym} -outline #{outline} \n }
	end
	def pd_delete(can) end
	def pd_show(can)
		self.canvas||=can
		@x,@y = get_position can if can
	end
	def highlight(color,ratio) # doesn't use self
		c = /^#(..)(..)(..)/.match(color)[1..3].map {|x| x.hex }
		c.map! {|x| [255,(x*ratio).to_i].min }
		"#%02x%02x%02x" % c
	end
end

if FObject.respond_to?(:gui_enable)
class Display < FObject; include Gooey
	attr_accessor :text
	def initialize()
		super
		@sel = nil; @args = [] # contents of last received message
		@text = "..."
		@sy,@sx = 16,80 # default size of the widget
		# @bg,@bgs,@fg = "#6774A0","#00ff80","#ffff80"
		@bg = "#cccccc"
		# hijacking a [#print]
		@gp = Pd.objectmaker(:"#print")
		#@gp.send_in 0, :trunc, 70
		Pd.send_in @gp, 0, :maxrows, 20
		@clock = Clock.new self
	end
	def initialize2()
		super
		b=bself
		#STDERR.puts "initialize2: bself=#{b.inspect}"
		Pd.send_in @gp, 0, :dest, b
	end
	def _0_set_size(sy,sx) @sy,@sx=sy,sx end
	def atom_to_s a
		# careful with the namespaces! apparently, Array is not ::Array in this context (i don't know why)
		case a
		when   Float; sprintf("%.5f",a).gsub(/\.?0+$/, "")
		when ::Array; "(" + a.map{|x| atom_to_s x }.join(" ") + ")"
		else a.to_s
		end
	end
	def method_missing(sel,*args)
		m = /^(_\d+_)(.*)/.match(sel.to_s) or return super
		@sel,@args = m[2].intern,args
		@text = case @sel
			when nil; "..."
			when :float; atom_to_s @args[0]
			else @sel.to_s + ": " + @args.map{|a| atom_to_s a }.join(' ')
			end
		@clock.delay 0
	end
	def pd_show(can)
		super
		return if not canvas or not @vis # can't show for now...
		cmd = "display_update #{@rsym} #{@x} #{@y} #{@fg} #{@bg} #{outline} #{quote @font} #{canvas} #{quote @text}\n"
		#puts "CMD=#{cmd}"
		GridFlow.gui cmd
	end
	def pd_delete(can)
		GridFlow.gui %{ #{canvas} delete #{@rsym} #{@rsym}TEXT \n} if @vis
		super
	end
	def _0_grid(*foo) @text=""; Pd.send_in @gp, 0, :grid, *foo; @clock.delay 0 end
	def call; update; end
	def _0_very_long_name_that_nobody_uses(*list)
		@text << "\n" if @text.length>0
		list.each {|x| @text << x.to_i }
	end
	install "display", 1, 1
	gui_enable

	GridFlow.gui %{
		proc display_update {self x y fg bg outline font canvas text} {
			$canvas delete ${self}TEXT
			$canvas create text [expr $x+2] [expr $y+2] -fill $fg -font $font -text $text -anchor nw -tag ${self}TEXT
			foreach {x1 y1 x2 y2} [$canvas bbox ${self}TEXT] {}
			incr x -1
			incr y -1
			set sx [expr $x2-$x1+2]
			set sy [expr $y2-$y1+4]
			$canvas delete ${self}
			$canvas create rectangle $x $y [expr $x+$sx] [expr $y+$sy] -fill $bg -tags $self -outline $outline
			$canvas create rectangle $x $y [expr $x+7]         $y      -fill red -tags $self -outline $outline
			$canvas lower $self ${self}TEXT
			pd \"$self set_size $sy $sx;\\n\"
		}
	}
end
end # respond to gui_enable

FObject.subclass("printargs",1,0) {
  def initialize(*args)
    @aaargh = args
    @clock = Clock.new self
    @clock.delay 0
  end
  def call
    post "ruby=%s", @aaargh.inspect
    post "  pd=%s",    args.inspect
  end
}

FObject.subclass("joystick_port",0,1) {
  def initialize(port)
    @f = File.open(port.to_s,"r+")
    @status = nil
    @clock = Clock.new self
    @clock.delay 0
    @f.nonblock=true
  end
  def delete; @clock.unset; @f.close end
  def call
    loop{
      begin
        event = @f.read(8)
      rescue Errno::EAGAIN
	@clock.delay 0
        return
      end
      return if not event
      return if event.length<8
      send_out 0, *event.unpack("IsCC")
    }
  end
}

# plotter control (HPGL)
FObject.subclass("plotter_control",1,1) {
  def puts(x)
    x << "\n"
    x.each_byte {|b| send_out 0, b }
    send_out 0
  end
  def _0_pu; puts "PU;" end
  def _0_pd; puts "PD;" end
  def _0_pa x,y; puts "PA#{x},#{y};" end
  def _0_sp c; puts "SP#{c};"; end
  def _0_ip(*v) puts "IP#{v.join','};" end
  def _0_other(command,*v) puts "#{command.to_s.upcase}#{v.join','};" end
  def _0_print(*text) puts "LB#{text.join(' ')}\003;" end
  def _0_print_from_ascii(*codes)
    _0_print codes.map{|code| code.chr }.join("")
  end
}

# ASCII, useful for controlling pics
FObject.subclass("ascii",1,1) {
  def puts(x) x.each_byte {|b| send_out 0, b } end
  def _0_float x; puts "#{x.to_i}" end
}
# System, similar to shell
FObject.subclass("system",1,1) { def _0_system(*a) system(a.join(" ")) end }

=begin
devices/linux

AUTHOR
        Mathieu Bouchard <matju@artengine.ca>
        irc: irc.freenode.net / #ruby-lang / matju
        (note: I can't read Japanese; write in French or English please)

OVERVIEW
This is a collection of simple modules that you extend IO objects with, to
give them support for specific devices.  For example:

        require "linux/SoundMixer"
        f = File.open "/dev/mixer"
        f.extend Linux::SoundMixer

        # f now has special accessors for driver variables, e.g:

        f.treble = left_speaker_percent + 256 * right_speaker_percent

The modules are made of automatically generated methods, much like Ruby's
accessors. those generators are called ioctl_reader, ioctl_writer,
ioctl_accessor. Writing expects an integer in -2**31...2**31; reading will
return the same. You may browse the source to find out which accessors are
available, and it's easy to add support for more features.
=end

# general-purpose code for performing less-than-trivial IOCTL operations. (was part of devices4ruby)
module IoctlClass
	def ioctl_reader(sym,cmd_in)
		module_eval %{def #{sym}
			ioctl_intp_in(#{cmd_in})
		end}
	end
	def ioctl_writer(sym,cmd_out)
		module_eval %{def #{sym}=(v)
			ioctl_intp_out(#{cmd_out},v)
			#{sym} if respond_to? :#{sym}
		end}
	end
	def ioctl_accessor(sym,cmd_in,cmd_out)
		ioctl_reader(sym,cmd_in)
		ioctl_writer(sym,cmd_out)
	end
end
module Ioctl
	# this method is not used anymore
	def int_from_4(foo)
		# if it crashes, just insert foo=foo.reverse here.
		(foo[0]+0x100*foo[1])+0x10000*(foo[2]+0x100*foo[3])
	end
	def ioctl_intp_out(arg1,arg2)
		ioctl(arg1,[arg2].pack("l"))
	end
	def ioctl_intp_in(arg1)
		ioctl(arg1,s="blah")
		return s.unpack("l")[0]
	end
end

# Linux::ParallelPort
# Copyright (c) 2001, 2003 by Mathieu Bouchard
# this is published under the Ruby license

=begin
  if using a DB-25 female connector as found on a PC,
  then the pin numbering is like:
  13 _____ 1
  25 \___/ 14

  1 = STROBE = the clock line is a square wave, often at 9600 Hz,
      which determines the data rate in usual circumstances.
  2..9 = D0..D7 = the eight ordinary data bits
  10 = -ACK (status bit 6 ?)
  11 = BUSY (status bit 7)
  12 = PAPER_END (status bit 5)
  13 = SELECT (status bit 4 ?)
  14 = -AUTOFD
  15 = -ERROR (status bit 3 ?)
  16 = -INIT
  17 = -SELECT_IN
  18..25 = GROUND
=end

module Linux; module ParallelPort
	extend IoctlClass
	@port_flags = %w[
	LP_EXIST
	LP_SELEC
	LP_BUSY
	LP_OFFL
	LP_NOPA
	LP_ERR
	LP_ABORT
	LP_CAREFUL
	LP_ABORTOPEN
	LP_TRUST_IRQ
	]
	@port_status = %w[
		nil,
		nil,
		nil,
		LP_PERRORP  # unchanged input, active low
		LP_PSELECD  # unchanged input, active high
		LP_POUTPA   # unchanged input, active high
		LP_PACK     # unchanged input, active low
		LP_PBUSY    # inverted input, active high
	]
	LPCHAR = 0x0601
	LPTIME = 0x0602
	LPABORT = 0x0604
	LPSETIRQ = 0x0605
	LPGETIRQ = 0x0606
	LPWAIT = 0x0608
	LPCAREFUL = 0x0609 # obsoleted??? wtf?
	LPABORTOPEN = 0x060a
	LPGETSTATUS = 0x060b # return LP_S(minor)
	LPRESET = 0x060c # reset printer
	LPGETSTATS = 0x060d # struct lp_stats (most likely turned off)
	LPGETFLAGS = 0x060e # get status flags
	LPTRUSTIRQ = 0x060f # set/unset the LP_TRUST_IRQ flag
	ioctl_reader :port_flags , :LPGETFLAGS
	ioctl_reader :port_status, :LPGETSTATUS
	ioctl_writer :port_careful,:LPCAREFUL
	ioctl_writer :port_char,   :LPCHAR
end end

FObject.subclass("parallel_port",1,3) {
  def initialize(port,manually=0)
    @f = File.open(port.to_s,"r+")
    @f.extend Linux::ParallelPort
    @status = nil
    @flags = nil
    @manually = manually!=0
    @clock = (if @manually then nil else Clock.new self end)
    @clock.delay 0 if @clock
  end
  def delete; @clock.unset unless @manually; @f.close end
  def _0_float(x) @f.write x.to_i.chr; @f.flush end
  def call
    flags = @f.port_flags
    send_out 2, flags if @flags != flags
    @flags = flags
    status = @f.port_status
    send_out 1, status if @status != status
    @status = status
    @clock.delay 20 if @clock
  end
  def _0_bang
    @status = @flags = nil
    call
  end
  # outlet 0 reserved (future use)
}

module Linux; module SoundMixer
	extend IoctlClass
	MIXER_NRDEVICES    = 0x00000019
	MIXER_VOLUME       = 0x00000000
	MIXER_BASS         = 0x00000001
	MIXER_TREBLE       = 0x00000002
	MIXER_SYNTH        = 0x00000003
	MIXER_PCM          = 0x00000004
	MIXER_SPEAKER      = 0x00000005
	MIXER_LINE         = 0x00000006
	MIXER_MIC          = 0x00000007
	MIXER_CD           = 0x00000008
	MIXER_IMIX         = 0x00000009
	MIXER_ALTPCM       = 0x0000000a
	MIXER_RECLEV       = 0x0000000b
	MIXER_IGAIN        = 0x0000000c
	MIXER_OGAIN        = 0x0000000d
	MIXER_LINE1        = 0x0000000e
	MIXER_LINE2        = 0x0000000f
	MIXER_LINE3        = 0x00000010
	MIXER_DIGITAL1     = 0x00000011
	MIXER_DIGITAL2     = 0x00000012
	MIXER_DIGITAL3     = 0x00000013
	MIXER_PHONEIN      = 0x00000014
	MIXER_PHONEOUT     = 0x00000015
	MIXER_VIDEO        = 0x00000016
	MIXER_RADIO        = 0x00000017
	MIXER_MONITOR      = 0x00000018
	ONOFF_MIN          = 0x0000001c
	ONOFF_MAX          = 0x0000001e
	MIXER_NONE         = 0x0000001f
	MIXER_ENHANCE      = 0x0000001f
	MIXER_MUTE         = 0x0000001f
	MIXER_LOUD         = 0x0000001f
	MIXER_RECSRC       = 0x000000ff
	MIXER_DEVMASK      = 0x000000fe
	MIXER_RECMASK      = 0x000000fd
	MIXER_CAPS         = 0x000000fc
	MIXER_STEREODEVS   = 0x000000fb
	MIXER_OUTSRC       = 0x000000fa
	MIXER_OUTMASK      = 0x000000f9
	MASK_VOLUME        = 0x00000001
	MASK_BASS          = 0x00000002
	MASK_TREBLE        = 0x00000004
	MASK_SYNTH         = 0x00000008
	MASK_PCM           = 0x00000010
	MASK_SPEAKER       = 0x00000020
	MASK_LINE          = 0x00000040
	MASK_MIC           = 0x00000080
	MASK_CD            = 0x00000100
	MASK_IMIX          = 0x00000200
	MASK_ALTPCM        = 0x00000400
	MASK_RECLEV        = 0x00000800
	MASK_IGAIN         = 0x00001000
	MASK_OGAIN         = 0x00002000
	MASK_LINE1         = 0x00004000
	MASK_LINE2         = 0x00008000
	MASK_LINE3         = 0x00010000
	MASK_DIGITAL1      = 0x00020000
	MASK_DIGITAL2      = 0x00040000
	MASK_DIGITAL3      = 0x00080000
	MASK_PHONEIN       = 0x00100000
	MASK_PHONEOUT      = 0x00200000
	MASK_RADIO         = 0x00800000
	MASK_VIDEO         = 0x00400000
	MASK_MONITOR       = 0x01000000
	MASK_MUTE          = 0x80000000
	MASK_ENHANCE       = 0x80000000
	MASK_LOUD          = 0x80000000
	MIXER_READ_VOLUME  = 0x80044d00
	MIXER_READ_BASS    = 0x80044d01
	MIXER_READ_TREBLE  = 0x80044d02
	MIXER_READ_SYNTH   = 0x80044d03
	MIXER_READ_PCM     = 0x80044d04
	MIXER_READ_SPEAKER = 0x80044d05
	MIXER_READ_LINE    = 0x80044d06
	MIXER_READ_MIC     = 0x80044d07
	MIXER_READ_CD      = 0x80044d08
	MIXER_READ_IMIX    = 0x80044d09
	MIXER_READ_ALTPCM  = 0x80044d0a
	MIXER_READ_RECLEV  = 0x80044d0b
	MIXER_READ_IGAIN   = 0x80044d0c
	MIXER_READ_OGAIN   = 0x80044d0d
	MIXER_READ_LINE1   = 0x80044d0e
	MIXER_READ_LINE2   = 0x80044d0f
	MIXER_READ_LINE3   = 0x80044d10
	MIXER_READ_MUTE    = 0x80044d1f
	MIXER_READ_ENHANCE = 0x80044d1f
	MIXER_READ_LOUD    = 0x80044d1f
	MIXER_READ_RECSRC  = 0x80044dff
	MIXER_READ_DEVMASK = 0x80044dfe
	MIXER_READ_RECMASK = 0x80044dfd
	MIXER_READ_STEREODEVS = 0x80044dfb
	MIXER_READ_CAPS    = 0x80044dfc
	MIXER_WRITE_VOLUME = 0xc0044d00
	MIXER_WRITE_BASS   = 0xc0044d01
	MIXER_WRITE_TREBLE = 0xc0044d02
	MIXER_WRITE_SYNTH  = 0xc0044d03
	MIXER_WRITE_PCM    = 0xc0044d04
	MIXER_WRITE_SPEAKER = 0xc0044d05
	MIXER_WRITE_LINE   = 0xc0044d06
	MIXER_WRITE_MIC    = 0xc0044d07
	MIXER_WRITE_CD     = 0xc0044d08
	MIXER_WRITE_IMIX   = 0xc0044d09
	MIXER_WRITE_ALTPCM = 0xc0044d0a
	MIXER_WRITE_RECLEV = 0xc0044d0b
	MIXER_WRITE_IGAIN  = 0xc0044d0c
	MIXER_WRITE_OGAIN  = 0xc0044d0d
	MIXER_WRITE_LINE1  = 0xc0044d0e
	MIXER_WRITE_LINE2  = 0xc0044d0f
	MIXER_WRITE_LINE3  = 0xc0044d10
	MIXER_WRITE_MUTE   = 0xc0044d1f
	MIXER_WRITE_ENHANCE = 0xc0044d1f
	MIXER_WRITE_LOUD   = 0xc0044d1f
	MIXER_WRITE_RECSRC = 0xc0044dff
	MIXER_INFO         = 0x805c4d65
	MIXER_ACCESS       = 0xc0804d66
	MIXER_AGC          = 0xc0044d67
	MIXER_3DSE         = 0xc0044d68
	MIXER_PRIVATE1     = 0xc0044d6f
	MIXER_PRIVATE2     = 0xc0044d70
	MIXER_PRIVATE3     = 0xc0044d71
	MIXER_PRIVATE4     = 0xc0044d72
	MIXER_PRIVATE5     = 0xc0044d73
	MIXER_GETLEVELS    = 0xc0a44d74
	MIXER_SETLEVELS    = 0xc0a44d75
	DEVICE_LABELS = [
		"Vol ", "Bass ", "Trebl", "Synth", "Pcm ",
		"Spkr ","Line ", "Mic ",  "CD ",   "Mix ",
		"Pcm2 ","Rec ",  "IGain", "OGain",
		"Line1", "Line2", "Line3", "Digital1", "Digital2", "Digital3",
		"PhoneIn", "PhoneOut", "Video", "Radio", "Monitor"
	]
	DEVICE_NAMES = [
		"vol", "bass", "treble", "synth", "pcm", "speaker", "line",
		"mic", "cd", "mix", "pcm2", "rec", "igain", "ogain",
		"line1", "line2", "line3", "dig1", "dig2", "dig3",
		"phin", "phout", "video", "radio", "monitor"
	]
	DEVICE_NAMES.each_with_index {|name,i|
		ioctl_accessor name,
			MIXER_READ_VOLUME+i,
			MIXER_WRITE_VOLUME+i
	}
end end

#FObject.subclass("SoundMixer",1,1) {
# using 'class' because in Ruby 1.8 (but not 1.6 nor 1.9) scoping rules are different for Class#instance_eval.
class GFSoundMixer < FObject; install "SoundMixer",1,1
  # BUG? i may have the channels (left,right) backwards
  def initialize(filename)
    super
    @file = File.open filename.to_s, 0
    @file.extend Linux::SoundMixer
    $sm = self
  end
  @@vars = Linux::SoundMixer.instance_methods.grep(/=/)
  @@vars_h = {}
  @@vars.each {|attr|
    attr=attr.chop
    eval %{ def _0_#{attr}(x) @file.#{attr} = x[0]*256+x[1] end }
    @@vars_h[attr]=true
  }
  def _0_get(sel=nil)
    if sel then
      sels=sel.to_s
      sel=sels.intern
      raise if not @@vars_h.include? sel.to_s
      begin
        x = @file.send sel
        send_out 0, sel, :"(", (x>>8)&255, x&255, :")"
      rescue
        send_out 0, sel, :"(", -1, -1, :")"
      end
    else
      @@vars.each {|var| _0_get var }
    end
  end
end#}

# end of devices4ruby

# experimental
FObject.subclass("rubyarray",2,2) {
  def initialize() @a=[]; @i=0; end
  def _0_get(s=nil)
    case s
    when :size; send_out 1,:size,@a.size
    when nil; _0_get :size
    end
  end
  def _0_clear; @a.clear end
  def _0_float i; @i=i; send_out 0, *@a[@i] if @a[@i]!=nil; end
  def _1_list(*l) @a[@i]=l; end
  def _0_save(filename,format=nil)
    f=File.open(filename.to_s,"w")
    if format then
      @a.each {|x| f.puts(format.to_s%x) }
    else
      @a.each {|x| f.puts(x.join(",")) }
    end
    f.close
  end
  def _0_load(filename)
    f=File.open(filename.to_s,"r")
    @a.clear
    f.each {|x| @a.push x.split(",").map {|y| Float(y) rescue y.intern }}
    f.close
  end
}

FObject.subclass("regsub",3,1) {
  def initialize(from,to) _1_symbol(from); _2_symbol(to) end
  def _0_symbol(s) send_out 0, :symbol, s.to_s.gsub(@from, @to).intern end
  def _1_symbol(from) @from = Regexp.new(from.to_s.gsub(/`/,"\\")) end
  def _2_symbol(to)   @to = to.to_s.gsub(/`/,"\\") end
  #doc:_0_symbol,"a string to transform"
  #doc:_1_symbol,"a regexp pattern to be found inside of the string"
  #doc:_2_symbol,"a replacement for the found pattern"
  #doc_out:_0_symbol,"the transformed string"
}

FObject.subclass("memstat",1,1) {
  def _0_bang
    f = File.open("/proc/#{$$}/stat")
    send_out 0, Float(f.gets.split(" ")[22]) / 1024.0
    f.close
  end
  #doc:_0_bang,"lookup process stats for the currently running pd+ruby and figure out how much RAM it uses."
  #doc_out:_0_float,"virtual size of RAM in kilobytes (includes swapped out and shared memory)"
}

FObject.subclass("sendgui",1,0) {
  def _0_list(*x)
    GridFlow.gui x.join(" ").gsub(/`/,";")+"\n"
  end
  install "sys_vgui", 1, 0
  #doc:_0_list,"a Tcl/Tk command to send to the pd client."
}

end # module GridFlow

class IO; include GridFlow::Ioctl; end

begin
#  require "optional/lti"
  GridFlow.post "LTI support loaded."
rescue Exception => e
  GridFlow.post "LTI support not found (%s)", e.message
end

