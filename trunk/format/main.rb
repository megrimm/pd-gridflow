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

class IO
  def nonblock= flag
    bit = Fcntl::O_NONBLOCK
    state = fcntl(Fcntl::F_GETFL, 0)
    fcntl(Fcntl::F_SETFL, (state & ~bit) |
      (if flag; bit else 0 end))
  end
end

module GridFlow

def max_rank; 16; end
def max_size; 64*1024**2; end
def max_packet; 1024; end

ENDIAN_BIG,ENDIAN_LITTLE,ENDIAN_SAME,ENDIAN_DIFF = 0,1,2,3

OurByteOrder = case [1].pack("L")
        when "\0\0\0\1"; ENDIAN_BIG     # Sun, SiliconGraphics
        when "\1\0\0\0"; ENDIAN_LITTLE  # Intel
        else raise "Cannot determine byte order" end

class Format
	FF_R = 4
	FF_W = 2
end

class BitPacking
	alias pack pack2
	alias unpack unpack2
end

# adding event-driven IO to a Format class
module EventIO
	def read_wait?; !!@action; end

	def on_read(n,&action)
		@action = action
		@chunksize = n
		$tasks[self] = proc { self.try_read(n) }
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
	rescue Errno::EWOULDBLOCK
#		p "wouldblock"
	end

	def try_read dummy
#		while @action
		@buffer ||= ""
#		p @chunksize-@buffer.length
		t = @stream.read(@chunksize-@buffer.length)
#		p t
		raise if not t
		@buffer << t
		if @buffer.length == @chunksize
			action,buffer = @action,@buffer
			@action,@buffer = nil,""
			$tasks.delete self
			action.call buffer
		end
#		end#while
	rescue Errno::EWOULDBLOCK
		GridFlow.whine "read would block"
	end

	def raw_open(mode,source,*args)
		mode = case mode
			when :in; "r"
			when :out; "w"
			else raise "bad mode" end
		case source
		when :file
			filename = GridFlow.find_file args[0].to_s
			@stream = File.open filename, mode
		when :tcp
			if VERSION < "1.6.6"
				raise "use at least 1.6.6 (reason: bug in socket code)"
			end
			GridFlow.whine "-----------"
			time = Time.new
			TCPSocket.do_not_reverse_lookup = true # hack
			@stream = TCPSocket.open(args[0].to_s,args[1])
			GridFlow.whine "----------- #{Time.new-time}"
			@stream.nonblock = true
			@stream.sync = true
			$tasks[self] = proc {self.try_read}
		when :tcpserver
			TCPSocket.do_not_reverse_lookup = true # hack
			TCPServer.do_not_reverse_lookup = true # hack
			GridFlow.whine "-----------"
			time = Time.new
			@acceptor = TCPServer.open(args[0].to_s)
			GridFlow.whine "----------- #{Time.new-time}"
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

	# little endian: smallest digit first, like i386
	attr_accessor :is_le

	# bits per value: 32 only
	attr_accessor :bpv # Fixnum

	def initialize(mode,source,*args)
		super
		@bpv = 32
		raw_open mode,source,*args
	end

	def rewind_if_needed
		thispos = @stream.seek 0,IO::SEEK_CUR
		lastpos = @stream.seek 0,IO::SEEK_END
		if thispos == lastpos then thispos = 0 end
		nextpos = @stream.seek 0,IO::SEEK_SET
	end

	# rewinding and starting
	def frame
		if not @stream
			raise "can't get frame when there is no connection"
		end
		if read_wait?
			raise "already waiting for input"
		end
		rewind_if_needed if not TCPSocket===@stream
		on_read(8) {|data| frame1 data }
		(try_read nil while read_wait?) if not TCPSocket===@stream
		GridFlow.whine "frame: finished"
	end

	# the header
	def frame1 data
		if is_le #!@#$ bug
			GridFlow.whine "we are smallest digit first"
		else
			GridFlow.whine "we are biggest digit first"
		end
		head,@bpv,reserved,@n_dim = data.unpack "a5ccc"
		case head
		when "\x7fGRID"
			@is_le = false
			GridFlow.whine "this file is biggest digit first"
		when "\x7fgrid"	
			@is_le = true
			GridFlow.whine "this file is smallest digit first"
		else
			raise "grid header: invalid (#{data.inspect})"
		end
		case bpv
		when 8, 32; # ok
		else raise "unsupported bpv (#{@bpv})"
		end
		if reserved!=0
			raise "reserved field is not zero"
		end
		GridFlow.whine "@n_dim=#{@n_dim}"
		if @n_dim > GridFlow.max_rank
			raise "too many dimensions (#{@n_dim})"
		end
		on_read(4*@n_dim) {|data| frame2 data }
	end

	# the dimension list
	def frame2 data
		@dim = data.unpack(if @is_le then "V*" else "N*" end)
		GridFlow.whine "dim=#{@dim.inspect}"
		@prod = 1
		@dim.each {|x| @prod *= x }
		if @prod > GridFlow.max_size
			raise "dimension list: invalid prod (#{@prod})"
		end
		send_out_grid_begin 0, @dim

		prod = 1
		@dim.each {|x| prod *= x }
		n = prod/@dim[0]
		k = 1 * GridFlow.max_packet / n
		k=1 if k<1
		@bufsize = k*n*@bpv/8

		on_read(bufsize) {|data| frame3 data }
		@dex = 0
	end

	attr_reader :bufsize

	# for each slice of the body
	def frame3 data
		n = data.length
		nn = n*8/@bpv
		case @bpv
		when 8
			send_out_grid_flow 0, data.unpack("c*").pack("i*")
			@dex += data.length
		when 32
			# hope for a multiple of 4 #!@#$
			send_out_grid_flow 0, data.unpack(
				if @is_le then "V*" else "N*" end).pack("i*")
			@dex += data.length/4
		end
		if @dex == @prod
			send_out_grid_end 0
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
		GridFlow.whine "@dim=#{@dim.inspect}"
		# header
		@stream.write(
			[if OurByteOrder==ENDIAN_LITTLE then "\x7fgrid" else "\x7fGRID" end,
			 @bpv,0,@dim.length].pack("a5ccc"))
		# dimension list
		@stream.write(@dim.to_a.pack("i*"))
	end

	def _0_rgrid_flow data
		case @bpv
		when 8
			@stream.write data.unpack("i*").pack("c*")
#			@stream.write BitPacking.new(ENDIAN_LITTLE,3,
#				data.pack "c*"
		when 32
			if GridFlow::OurByteOrder == is_le
				@stream.write data
			else
#				@stream.write data.swap32 #!@#$
				@stream.write data.unpack("N*").pack("V*")
			end
		end
	end

	def _0_rgrid_end
		case @stream
		when File; @stream.seek 0,IO::SEEK_SET # should be an option
		end
	end

	def option(name,*args)
		case name
		when :type
		# bug: should not be able to modify this _during_ a transfer
			case args[0]
			when :uint8; @bpv=8
			when :int32; @bpv=32
			else raise "unsupported number type"
			end
		else
			super
		end
	end

	install_rgrid 0
	install_format "FormatPPM", 1, 1, FF_R|FF_W, "grid", "GridFlow file format"
end

class FormatPPM < Format; include EventIO
	def initialize(mode,source,*args)
		super
		raw_open mode,source,*args
	end

	def frame
		metrics=[]
		line = @stream.gets
		if not line then @stream.seek 0,IO::SEEK_SET; line = @stream.gets.chomp end
		line.chomp!
		if line != "P6" then raise "Wrong format (needing PPM P6)" end
		while metrics.length<3
			line = @stream.gets
			next if line =~ /^#/
			metrics.push *(line.split(/\s+/).map{|x| Integer x })
		end
		if metrics[2] != 255 then raise \
			"Wrong color depth (max_value=#{metrics[2]} instead of 255)" end

		send_out_grid_begin 0, [metrics[1], metrics[0], 3]

		bp = BitPacking.new(ENDIAN_LITTLE,3,[0x0000ff,0x00ff00,0xff0000])
		bs = metrics[0]*3
		for y in 0...metrics[1] do
			data = @stream.read bs
			data or raise EOFError
			send_out_grid_flow 0, bp.unpack data
			#send_out_grid_flow 0, "\x80"*metrics[0]*12
		end
		p "HELLO"
		send_out_grid_end 0
	end

	def _0_rgrid_begin
		dim = inlet_dim 0
		@stream.write "P6\n" \
		"# generated using GridFlow #{GF_VERSION}\n#{dim[1]} #{dim[0]}\n255\n"
		@stream.flush
	end

	def _0_rgrid_flow data
		# !@#$ use BitPacking here
		# bp = BitPacking.new(ENDIAN_LITTLE,3,[0x0000ff,0x00ff00,0xff0000])
		@stream.write data.unpack("i*").pack("c*")
	end
	
	def _0_rgrid_end
		@stream.flush
		@stream.seek 0,SEEK_SET
	end

	install_rgrid 0
	install_format "FormatGrid", 1, 1, FF_R|FF_W, "ppm", "Portable PixMap"
end

class FormatTarga < Format; include EventIO

=begin
targa header is like:
	[:comment, Uint8, :length], 1,
	[:colors,  Uint8], 9,
	[:w, Uint16],
	[:h, Uint16],
	[:depth, Uint8], 1,
	[:comment, String8Unpadded, :data],
=end

	def initialize(mode,source,*args)
		super
		raw_open mode,source,*args
	end

	def frame
		comment_length,colors,w,h,depth = @stream.read(18).unpack("cxcx9vvcx")
		comment = @stream.read(comment_length)

		if colors != 2
			raise "unsupported color format: #{colors}"
		end

		GridFlow.whine sprintf "tga: size y=%d x=%d depth=%d",h,w,depth
		GridFlow.whine sprintf "tga: comment: %s", comment

		if depth != 24 and depth != 32
			raise sprintf "tga: wrong colour depth: %i\n", depth
		end
		send_out_grid_begin 0, [ h, w, depth/8 ]
		
		bs = w*depth/8

		bp = BitPacking.new(ENDIAN_BIG,3,[0x0000ff,0x00ff00,0xff0000])
		for y in 0...h do
			data = @stream.read bs
			send_out_grid_flow 0, bp.unpack data
		end
		send_out_grid_end 0
	end

	install_rgrid 0
	install_format "FormatTarga", 1, 1, FF_R, "targa", "TrueVision Targa"
end

#class FormatMulti < Format
#end

#class FormatSeq < Format
#end 

#class FormatTempFile < Format
#end 

end # module GridFlow

