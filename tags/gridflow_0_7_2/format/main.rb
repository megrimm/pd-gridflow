=begin
	$Id$

	GridFlow
	Copyright (c) 2001,2002 by Mathieu Bouchard

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

require "socket"
require "fcntl"

class IO
  def nonblock= flag
    bit = Fcntl::O_NONBLOCK
    state = fcntl(Fcntl::F_GETFL, 0)
    fcntl(Fcntl::F_SETFL, (state & ~bit) |
      (if flag; bit else 0 end))
  end
end

module GridFlow

class<<self
	attr_reader :formats
	def max_rank; 16; end
	def max_size; 64*1024**2; end
	def max_packet; 1024*2; end
end

ENDIAN_BIG,ENDIAN_LITTLE,ENDIAN_SAME,ENDIAN_DIFF = 0,1,2,3

OurByteOrder = case [1].pack("L")
        when "\0\0\0\1"; ENDIAN_BIG     # Mac, Sun, SiliconGraphics
        when "\1\0\0\0"; ENDIAN_LITTLE  # Intel
        else raise "Cannot determine byte order" end

class Format #< GridObject
	FF_R,FF_W = 4,2

	attr_accessor :parent

=begin
	NEW FORMAT API (0.6.x)
	mode is :in or :out

	def initialize(mode,*args) :
		open a file handler (do it via .new of class)

	def frame() :
		read one frame, send through outlet 0

	def seek(Integer i) :
		select one frame to be read next (by number)

	def length() : ^Integer
		returns number of frames (never implemented ?)

	def option(Symbol name, *args) :
		miscellaneous options

	def close() :
		close a handler

	AMENDMENT (0.7.1):
	def frame() now has return values :
		Integer >= 0 : frame number of frame read.
		false : no frame was read : end of sequence.
		nil : a frame was read, but can't say its number.
	note that trying to read a nonexistent frame should no longer
	rewind automatically (@in handles that part), nor re-read the
	last frame (mpeg/quicktime used to do this)
=end

	def initialize(mode,*)
		super
		@cast = :int32
		@mode = mode
		@frame = 0
		@parent = nil
		flags = self.class.instance_eval{@flags}
		# FF_W, FF_R, FF_RW
		case mode
		when  :in; flags[2]==1
		when :out; flags[1]==1
		else raise "Format opening mode is incorrect"
		end or raise \
			"Format '#{self.class.instance_eval{@symbol_name}}'"\
			" does not support mode '#{mode}'"
	end

	def close; end

	def self.install_format(name,inlets,outlets,flags,symbol_name,description)
		install(name,inlets,outlets)
		conf_format(flags,symbol_name,description)
		nil
	end

	def seek frame
		(rewind; return) if frame == 0
		raise "don't know how to seek for frame other than # 0"
	end

	# this is what you should use to rewind
	# different file-sources may redefine this as something else
	# (eg: gzip)
	def rewind
		raise "Nothing to rewind about..." if not @stream
		@stream.seek 0,IO::SEEK_SET
		@frame = 0
	end

	# This is different from IO#eof, which waits until a read has failed
	# doesn't work in nonblocking mode? (I don't recall why)
	def eof?
		thispos = (@stream.seek 0,IO::SEEK_CUR; @stream.tell)
		lastpos = (@stream.seek 0,IO::SEEK_END; @stream.tell)
		@stream.seek thispos,IO::SEEK_SET
		return thispos == lastpos
	rescue Errno::ESPIPE # just ignore if seek is not possible
		return false
	end

	# "ideal" buffer size or something
	# the buffer may be bigger than this but usually not by much.
	def self.buffersize; 16384 end

	#!@#$ what's this doing here?
	def option(name,*args)
		case name
		when :rewind
			rewind
		when :headerless
			args=args[0] if Array===args[0]
			#raise "expecting dimension list..."
			args.each {|a|
				Integer===a or raise "expecting dimension list..."
			}
			@headerless = args
		when :headerful
			@headerless = nil
		when :type
		# bug: should not be able to modify this _during_ a transfer
		# bug: does not exist for formats other than "grid".
			case args[0]
			when :uint8; @bpv=8
				@bp=BitPacking.new(ENDIAN_LITTLE,1,[0xff])
			when :int16; @bpv=16
				@bp=BitPacking.new(ENDIAN_LITTLE,1,[0xffff])
			when :int32; @bpv=32
			else raise "unsupported number type"
			end
		when :cast
			case args[0]
			when :uint8, :int16, :int32, :float32
				@cast = args[0]
			else raise "unsupported number type"
			end
		else
			raise "option #{name} not supported"
		end
	end

	def frame
		@frame+=1
		@frame-1
	end
end

# common parts between GridIn and GridOut
module GridIO
	def check_file_open
		if not @format then raise "can't do that: file not open" end
	end

	attr_reader :format

	def _0_open(sym,*a)
		qlass = GridFlow.formats[sym]
		if not qlass then raise "unknown file format identifier: #{sym}" end
		@format.close if @format
		@format = nil
		@format = qlass.new @mode, *a
		@format.connect 0,self,1
		#@format.connect 1,self,2
		@format.parent = self
		@loop = true
	end

	def _0_timelog flag; @timelog = Integer(flag)!=0 end
	def _0_loop flag; @loop = a[1].to_i != 0 end

	def _0_option(*message)
		case message[0]
		when :timelog, :loop
			send_in 0, *message
		else
			check_file_open
			if @format.respond_to? "_0_#{message[0]}" #hack
				@format.send_in 0, :option, *message
			elsif @format.respond_to? "#{message[0]}" #hack
				@format.send(*message)
			else
				@format.option(*message)
			end
		end
	end

	def method_missing(*message)
		sel = message[0].to_s
		sel =~ /^_0_/ or return super
		message[0] = sel.sub(/^_0_/,"").intern
		_0_option(*message)
	end

	def _0_close
		check_file_open
		@format.close
		@format = nil
	end

	def delete
		@format.close if @format
		@format = nil
		super
	end
end

class GridIn < GridObject
	include GridIO

	def initialize(*a)
		super
		@format = nil
		@timelog = false
		@framecount = 0
		@time = Time.new
		@mode = :in
		return if a.length==0
		_0_open(*a)
	end

	def _0_bang
		check_file_open
		framenum = @format.frame
		if framenum == false
			send_out 1
			return if not @loop
			@format.seek 0
			framenum = @format.frame
			if framenum == false
				raise "can't read frame: the end is at the beginning???"
			end
		end
		send_out 1, framenum if framenum
	end

	def _0_int frame; _0_set frame; _0_bang end
	def _0_set frame; check_file_open; @format.seek frame end
	def _0_reset; check_file_open; @format.seek 0; end

	alias _0_float _0_int

	def _1_grid(*a)
		send_out 0,:grid,*a
	end

	install_rgrid 0
	install "@in", 1, 2
end

class GridOut < GridObject
	include GridIO

	def initialize(*a)
		super
		@format = nil
		@timelog = false
		@framecount = 0
		@time = Time.new
		@mode = :out
		return if a.length==0
		if Integer===a[0] or Float===a[0]
			_0_open :x11,:here
			_0_option :out_size,a[0],a[1]
		else
			_0_open(*a)
		end
	end

	def _0_list(*a) @format._0_list(*a) end
	def _1_grid(*a) send_out 0,:grid,*a end # for aalib
	def _1_position(*a) send_out 0,:position,*a end

	def _0_grid(*a)
		check_file_open
		@format._0_grid(*a)
		send_out 0,:bang
		log if @timelog
		@framecount+=1
	end

	def log
		time = Time.new
		GridFlow.post(
			"@out: frame#%04d time: %10.3f s; diff: %5d ms",
			@framecount, time, ((time-@time)*1000).to_i)
		@time = time
	end

	install_rgrid 0
	install "@out", 1, 1
end

class BitPacking
	alias pack pack2
	alias unpack unpack2
end

# adding event-driven IO to a Format class
module EventIO
	def read_wait?; !!@action; end

	def initialize(*)
		@acceptor = nil
		@buffer = nil
		@action = nil
		@chunksize = nil
		super
	end

	def on_read(n,&action)
		@action = action
		@chunksize = n
		$tasks[self] = proc { self.try_read }
	end

	def try_accept
		TCPSocket.do_not_reverse_lookup = true # hack
		@acceptor.nonblock = true
		@stream = @acceptor.accept
		@stream.nonblock = true
		@stream.sync = true
#		p "GOT IT"
		$tasks.delete self
#		send_out 0, :accept # does not work
	rescue Errno::EAGAIN
#		p "wouldblock"
	end

	def try_read(dummy=nil)
#		while @action
#		p @chunksize-@buffer.length
		n = @chunksize-(if @buffer then @buffer.length else 0 end)
#		puts "tell: #{@stream.tell}"
		t = @stream.read(n) # or raise EOFError
		if not t
			raise "heck" if not @stream.eof?
			rewind
			t = @stream.read(n) or raise "can't read any of #{n} bytes?"
		end
#		p t
		#return if not t
		if @buffer then @buffer << t else @buffer = t end
		if @buffer.length == @chunksize
			action,buffer = @action,@buffer
			@action,@buffer = nil,""
			$tasks.delete self
			action.call buffer
		end
#		end#while
	rescue Errno::EAGAIN
		GridFlow.post "read would block"
	end

	def raw_open(mode,source,*args)
		@raw_open_args = mode,source,*args
		fmode = case mode
			when :in; "r"
			when :out; "w"
			else raise "bad mode" end
		case source
		when :file
			filename = args[0].to_s
			filename = GridFlow.find_file filename if mode==:in
			@stream = File.open filename, fmode
#		when :files
#			if mode==:in then
#			@glob = Dir[]
		when :gzfile
			filename = args[0].to_s
			filename = GridFlow.find_file filename if mode==:in
			@stream = File.open filename, fmode
			if mode==:in then
				r,w = IO.pipe
				if fork
					w.close
					@stream = r
				else
					r.close
					STDOUT.reopen w
					STDIN.reopen @stream
					@stream = File.open filename, "r"
					exec "gzip", "-dc"
				end
			else # mode==:out
				r,w = IO.pipe
				if fork
					r.close
					@stream = w
				else
					w.close
					STDIN.reopen r
					STDOUT.reopen @stream
					@stream = File.open filename, "w"
					exec "gzip", "-c"
				end
			end
			def self.rewind
				@stream.close
				raw_open(*@raw_open_args)
				@frame = 0
			end unless singleton_methods.include? "rewind"
		when :tcp
			if VERSION < "1.6.6"
				raise "use at least 1.6.6 (reason: bug in socket code)"
			end
			GridFlow.post "-----------"
			time = Time.new
			TCPSocket.do_not_reverse_lookup = true # hack
			@stream = TCPSocket.open(args[0].to_s,args[1])
			GridFlow.post "----------- #{Time.new-time}"
			@stream.nonblock = true
			@stream.sync = true
			$tasks[self] = proc {self.try_read}
		when :tcpserver
			TCPSocket.do_not_reverse_lookup = true # hack
			TCPServer.do_not_reverse_lookup = true # hack
			GridFlow.post "-----------"
			time = Time.new
			@acceptor = TCPServer.open(args[0].to_s)
			GridFlow.post "----------- #{Time.new-time}"
			@acceptor.nonblock = true
			$tasks[self] = proc {self.try_accept}
		else
			raise "unknown access method '#{source}'"
		end
	end
	def close
		@acceptor.close if @acceptor
		@stream.close if @stream
	end
end

class FormatGrid < Format; include EventIO
=begin
	This is the Grid format I defined:
	1 uint8: 0x7f
	4 uint8: "GRID" big endian | "grid" little endian
	1 uint8: bits per value (supported: 32)
	1 uint8: reserved (supported: 0)
	1 uint8: number of dimensions N (supported: at least 0..4)
	N uint32: number of elements per dimension D[0]..D[N-1]
	raw data goes there.
=end

	# bits per value: 32 only
	attr_accessor :bpv # Fixnum: bits-per-value

	# endianness
	# attr_accessor :endian # ENDIAN_LITTLE or ENDIAN_BIG

	# IO or File or TCPSocket
	attr_reader :stream

	# nil=headerful; array=assumed dimensions of received grids
	#attr_accessor :headerless

	def initialize(mode,source,*args)
		super
		@bpv = 32
		@headerless = nil
		@endian = OurByteOrder
		raw_open mode,source,*args
	end

	# rewinding and starting
	def frame
		if not @stream
			raise "can't get frame when there is no connection"
		end
		if read_wait?
			raise "already waiting for input"
		end
		return false if eof?

		if @headerless then
			@n_dim=@headerless.length
			@dim = @headerless
			@dex = 0
			set_bufsize
			send_out_grid_begin 0, @dim
			on_read(bufsize) {|data| frame3 data }
		else
			on_read(8) {|data| frame1 data }
		end
		(try_read nil while read_wait?) if not TCPSocket===@stream
#		GridFlow.post "frame: finished"
		super
	end

	def set_bufsize
		@prod = 1
		@dim.each {|x| @prod *= x }
		n = @prod/@dim[0]
		k = GridFlow.max_packet / n
		k=1 if k<1
		@bufsize = k*n*@bpv/8
		@bufsize = @prod if @bufsize > @prod
	end

	# the header
	def frame1 data
#		GridFlow.post("we are " + if OurByteOrder == ENDIAN_LITTLE
#			then "smallest digit first"
#			else "biggest digit first" end)
		head,@bpv,reserved,@n_dim = data.unpack "a5ccc"
		@endian = case head
			when "\x7fGRID"; ENDIAN_BIG
			when "\x7fgrid"; ENDIAN_LITTLE
			else raise "grid header: invalid (#{data.inspect})" end
#		GridFlow.post("this file is " + if @endian
#			then "biggest digit first"
#			else "smallest digit first" end)

		case bpv
		when 8, 16, 32; # ok
		else raise "unsupported bpv (#{@bpv})"
		end
		if reserved!=0
			raise "reserved field is not zero"
		end
#		GridFlow.post "@n_dim=#{@n_dim}"
		if @n_dim > GridFlow.max_rank
			raise "too many dimensions (#{@n_dim})"
		end
		on_read(4*@n_dim) {|data| frame2 data }
	end

	# the dimension list
	def frame2 data
		@dim = data.unpack(if @endian==ENDIAN_LITTLE then "V*" else "N*" end)
#		GridFlow.post "dim=#{@dim.inspect}"
		set_bufsize
		if @prod > GridFlow.max_size
			raise "dimension list: invalid prod (#{@prod})"
		end
		send_out_grid_begin 0, @dim, @cast

		on_read(bufsize) {|data| frame3 data }
		@dex = 0
	end

	attr_reader :bufsize

	# for each slice of the body
	def frame3 data
		n = data.length
		nn = n*8/@bpv
		# is/was there a problem with the size of the data being read?
		case @bpv
		when 8
			@bp = BitPacking.new(@endian,1,[0xff])
			send_out_grid_flow(0, @bp.unpack(data))
			@dex += data.length
		when 16
			@bp = BitPacking.new(@endian,2,[0xffff])
			send_out_grid_flow(0, @bp.unpack(data))
			@dex += data.length
		when 32
			data.swap32! if @endian!=OurByteOrder
			send_out_grid_flow 0, data
			@dex += data.length/4
		end
		if @dex == @prod
			$tasks.delete self
		else
			on_read(bufsize) {|data| frame3 data }
		end
	end

	def _0_rgrid_begin
		if not @stream
			raise "can't send frame when there is no connection"
		end
		@dim = inlet_dim 0
		GridFlow.post "@dim=#{@dim.inspect}"
		return if @headerless
		# header
		@stream.write(
			[if @endian==ENDIAN_LITTLE then "\x7fgrid" else "\x7fGRID" end,
			 @bpv,0,@dim.length].pack("a5ccc"))
		# dimension list
		@stream.write(
			@dim.to_a.pack(if @endian==ENDIAN_LITTLE then "V*" else "N*" end))
	end

	def _0_rgrid_flow data
		case @bpv
		when 8, 16
			@stream.write @bp.pack(data)
		when 32
			data.swap32! if GridFlow::OurByteOrder != @endian
			@stream.write data
		end
	end

	def _0_rgrid_end
		@stream.flush
	end

	def endian(a)
		case a
		when :little; @endian = ENDIAN_LITTLE
		when :big;    @endian = ENDIAN_BIG
		when :same;   @endian = ENDIAN_SAME
		else raise "argh"
		end
	end

	def headerless(*args)
		args=args[0] if Array===args[0]
		#raise "expecting dimension list..."
		args.each {|a|
			Integer===a or raise "expecting dimension list..."
		}
		@headerless = args
	end

	def headerful; @headerless = nil end

	#!@#$ method name conflict ?
	def type(nt)
		#!@#$ bug: should not be able to modify this _during_ a transfer
		case nt
		when :uint8; @bpv=8
			@bp=BitPacking.new(ENDIAN_LITTLE,1,[0xff])
		when :int16; @bpv=16
			@bp=BitPacking.new(ENDIAN_LITTLE,1,[0xffff])
		when :int32; @bpv=32
		else raise "unsupported number type"
		end
	end

	install_rgrid 0
	install_format "FormatGrid", 1, 1, FF_R|FF_W, "grid", "GridFlow file format"
end

module PPMandTarga
	# "and false" disables features that may cause crashes and don't
	# accelerate gridflow that much.
	def frame_read_body height, width, channels
		bs = width*channels
		n = bs*height
		bs = (self.class.buffersize/bs)*bs+bs # smallest multiple of bs over BufferSize
		buf = ""
		if VERSION >= "1.8.0" and false
			data = "x"*bs # must preallocate (bug in 1.8.0.pre1-3)
			while n>0 do
				bs=n if bs>n
				@stream.read(bs,data) or raise EOFError
				if @bp then
					send_out_grid_flow 0, @bp.unpack(data,buf)
				else
					send_out_grid_flow 0, data, :uint8
				end
				n-=bs
			end
		else
			nothing = ""
			while n>0 do
				bs=n if bs>n
				data = @stream.read(bs) or raise EOFError
				if @bp then
					send_out_grid_flow 0, @bp.unpack(data,buf)
				else
					send_out_grid_flow 0, data, :uint8
				end
				data.replace nothing and false # prevent clogging memory
				n-=bs
			end
		end
	end
end

class FormatPPM < Format; include EventIO, PPMandTarga
	def initialize(mode,source,*args)
		if mode==:out
			@bp = BitPacking.new(ENDIAN_LITTLE,3,[0x0000ff,0x00ff00,0xff0000])
		else
			@bp = nil
		end
		super
		raw_open mode,source,*args
	end

	def frame
		#@stream.sync = false
		metrics=[]
		return false if eof?
		line = @stream.gets
		(rewind; line = @stream.gets) if not line # hack
		line.chomp!
		if line != "P6" then raise "Wrong format (needing PPM P6)" end
		while metrics.length<3
			line = @stream.gets
			next if line =~ /^#/
			metrics.push(*(line.split(/\s+/).map{|x| Integer x }))
		end
		metrics[2]==255 or
			raise "Wrong color depth (max_value=#{metrics[2]} instead of 255)"

		send_out_grid_begin 0, [metrics[1], metrics[0], 3], @cast
		frame_read_body metrics[1], metrics[0], 3
		super
	end

	def _0_rgrid_begin
		dim = inlet_dim 0
		raise "expecting (rows,columns,channels)" if dim.length!=3
		raise "expecting channels=3" if dim[2]!=3
		@stream.write "P6\n" \
		"# generated using GridFlow #{GF_VERSION}\n#{dim[1]} #{dim[0]}\n255\n"
		@stream.flush
		inlet_set_factor 0, 3
	end
	def _0_rgrid_flow data
		@stream.write @bp.pack(data)
	end
	def _0_rgrid_end
		@stream.flush
	end

	install_rgrid 0
	install_format "FormatPPM", 1, 1, FF_R|FF_W, "ppm", "Portable PixMap"
end

class FormatTarga < Format; include EventIO, PPMandTarga

=begin
targa header is like:
	[:comment, Uint8, :length],
	[:colortype, Uint8],
	[:colors,  Uint8], 5,
	[:origin_x, Int16],
	[:origin_y, Int16],
	[:w, Uint16],
	[:h, Uint16],
	[:depth, Uint8], 1,
	[:comment, String8Unpadded, :data],
=end

	def initialize(mode,source,*args)
		super
		raw_open mode,source,*args
	end

	def set_bitpacking depth
		@bp = case depth
		when 24
			# endian here doesn't seem to be changing much ?
			BitPacking.new(ENDIAN_LITTLE,3,[0xff0000,0x00ff00,0x0000ff])
		when 32
			BitPacking.new(ENDIAN_LITTLE,4,
				[0x00ff0000,0x0000ff00,0x000000ff,0xff000000])
		else
			raise "tga: unsupported colour depth: #{depth}\n"
		end
	end

	def frame
		return false if eof?
		head = @stream.read(18)
		comment_length,colortype,colors,w,h,depth = head.unpack("cccx9vvcx")
		comment = @stream.read(comment_length)

		if colors != 2
			raise "unsupported color format: #{colors}"
		end

#		GridFlow.post "tga: size y=#{h} x=#{w} depth=#{depth} colortype=#{colortype}"
#		GridFlow.post "tga: comment: \"#{comment}\""

		set_bitpacking depth
		send_out_grid_begin 0, [ h, w, depth/8 ], @cast
		frame_read_body h, w, depth/8
		super
	end

	def _0_rgrid_begin
		dim = inlet_dim 0
		raise "expecting (rows,columns,channels)" if dim.length!=3
		raise "expecting channels=3 or 4" if dim[2]!=3 and dim[2]!=4
		# comment = "" # "created using GridFlow"
		comment = "CREATOR: The GIMP's TGA Filter Version 1.2"
		@stream.write [comment.length,colortype=0,colors=2,"\0"*9,
		dim[1],dim[0],8*dim[2],(8*(dim[2]-3))|32,comment].pack("ccca9vvcca*")
		set_bitpacking 8*dim[2]
		inlet_set_factor 0, dim[2]
	end

	def _0_rgrid_flow data
		@stream.write @bp.pack(data)
	end
	
	def _0_rgrid_end
		@stream.flush
	end

	install_rgrid 0
	install_format "FormatTarga", 1, 1, FF_R|FF_W, "targa", "TrueVision Targa"
end

=begin
class FormatMulti < Format
	install_rgrid 0
	install_format "FormatMulti", 1, 1, FF_R, "multi", "joining multiple files"
end

class FormatTempFile < Format
	install_rgrid 0
	install_format "FormatTempFile", 1, 1, FF_R|FF_W, "tempfile", "huh..."
end 
=end

end # module GridFlow
