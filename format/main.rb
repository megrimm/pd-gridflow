=begin
	$Id$

	GridFlow
	Copyright (c) 2001-2006 by Mathieu Bouchard

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

require "fcntl"
module GridFlow
class<< self
	def max_rank; 16; end
	def max_size; 64*1024**2; end
	def max_packet; 1024*2; end
end
=begin API (version 0.8)
	mode is :in or :out
	def initialize(mode,*args) :
		open a file handler (do it via .new of class)
	attr_reader :description :
		a _literal_ (constant) string describing the format handler
	def self.info() optional :
		return a string describing the format handler differently
		than self.description(). in particular, it can list
		compile-time options and similar things. for example,
		quicktime returns a list of codecs.
	def frame() :
		read one frame, send through outlet 0
		return values :
			Integer >= 0 : frame number of frame read.
			false : no frame was read : end of sequence.
			nil : a frame was read, but can't say its number.
		note that trying to read a nonexistent frame should no longer
		rewind automatically (@in handles that part), nor re-read the
		last frame (mpeg/quicktime used to do this)
	def seek(Integer i) :     select one frame to be read next (by number)
	def length() : ^Integer   returns number of frames (never implemented ?)
	def close() :             close a handler
	inlet 0 :
		grid : frame to write
		other : special options
	outlet 0 : grid : frame just read
	outlet 1 : everything else
=end
class Format < GridObject
	FF_R,FF_W = 4,2 # flags indicating support of :in and :out respectively.
	attr_accessor :parent
	def initialize(mode,*)
		super
		@cast = :int32
		@colorspace = :rgb
		@mode = mode
		@frame = 0
		@parent = nil
		@stream = nil
		flags = self.class.instance_eval{if defined?@flags then @flags else 6 end}
		# FF_W, FF_R, FF_RW
		case mode
		when  :in; flags[2]==1
		when :out; flags[1]==1
		else raise "Format opening mode is incorrect"
		end or raise \
			"Format '#{self.class.instance_eval{@symbol_name}}'"\
			" does not support mode '#{mode}'"
	end
	def close; @stream.close if defined? @stream and @stream end
	def self.suffixes_are(*suffixes)
		suffixes.map{|s|s.split(/[, ]/)}.flatten.each {|suffix|
			Format.suffixes[suffix] = self
		}
	end
	class<< self
		attr_reader :flags
		attr_reader :suffixes
	end
	@suffixes = {}
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
	def _0_type arg
		#!@#$ goes in FormatGrid ?
		#!@#$ bug: should not be able to modify this _during_ a transfer
		case arg
		when :uint8; @bpv=8; @bp=BitPacking.new(ENDIAN_LITTLE,1,[0xff])
		when :int16; @bpv=16; @bp=BitPacking.new(ENDIAN_LITTLE,1,[0xffff])
		when :int32; @bpv=32; @bp=nil
		else raise "unsupported number type: #{arg}"
		end
	end
	def _0_cast arg
		case arg
		when :uint8, :int16, :int32, :int64, :float32, :float64
			@cast = arg
		else raise "unsupported number type: #{arg}"
		end
	end
	def frame; @frame+=1; @frame-1 end
	def self.install_format(a,b,c,d,e) install(a,b,c); @mode=d; suffixes_are e.split(" ") end
end

# common parts between GridIn and GridOut
class GridIO < GridObject
	def check_file_open; if not @format then raise "can't do that: file not open" end end
	def _0_close; (@format.close; @format = nil) if @format end
	def delete; @format.close if @format; @format = nil; super end
	attr_reader :format
	def initialize(*)
		super
		@format = nil
		@timelog = false
		@framecount = 0
		@time = Time.new
	end
	def _0_open(sym,*a)
		sym = sym.to_s
		if a.length==0 and /\./ =~ sym then
			a = [mode,sym]
			suf=sym.split(/\./)[-1]
			#if suf=="gz" then a[1]=:gzfile; suf=file.split(/\./)[-2] end
			h=Format.suffixes[suf]
			if not h then raise "unknown suffix '.#{suf}'" end
			@format = h.new(*a)
		else
			qlass = GridFlow.fclasses["\#io:#{sym}"]
			if not qlass then raise "unknown file format identifier: #{sym}" end
		end
		_0_close if @format
		@format = qlass.new @mode, *a
		@format.connect 0,self,1
		@format.connect 1,self,2
		@format.parent = self
		@loop = true
	end

	def _0_timelog flag; @timelog = Integer(flag)!=0 end
	def _0_loop    flag;    @loop = Integer(flag)!=0 end
	def method_missing(*message)
		sel = message[0].to_s
		if sel =~ /^_0_/
			message[0] = sel.sub(/^_0_/,"").intern
			@format.send_in 0, *message
		elsif sel =~ /^_2_/
			sel = sel.sub(/^_2_/,"").intern
			message.shift
			send_out 1, sel, *message
		else
			return super
		end
	end
end

GridIO.subclass("#in",1,2) {
	install_rgrid 0
	def initialize(*a)
		super
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
	def _0_float frame; _0_set frame; _0_bang end
	def _0_set frame; check_file_open; @format.seek frame end
	def _0_reset; check_file_open; @format.seek 0; end
	def _1_grid(*a) send_out 0,:grid,*a end
	def _0_load(*a); _0_open(*a); _0_bang; _0_close end
	def _0_get(s=nil)
		super
		return if not @format
		if s then @format._0_get s else @format._0_get end
	end
	def _0_help()
		super
		if not @format then GridFlow.post "no file or device currently open." end
		@format._0_help
	end

}

GridIO.subclass("#out",1,1) {
	def initialize(*a)
		super
		@mode = :out
		return if a.length==0
		if Numeric===a[0]
			_0_open :window
			_0_out_size a[0],a[1]
		else
			_0_open(*a)
		end
		@finished = FObject["#finished"]
		@finished.connect 0, self, 1
	end
	def _0_autoclose(v=1) @autoclose = !!v end
	def _0_list(*a) @format._0_list(*a); _0_close if @autoclose end
	def _1_grid(*a) send_out 0,:grid,*a end # for aalib
	def _1_position(*a) send_out 0,:position,*a end
	def _1_keypress(*a) send_out 0,:keypress,*a end
	def _1_keyrelease(*a) send_out 0,:keyrelease,*a end
	def _1_bang(*a) _0_close if @autoclose end
	def _0_grid(*a)
		check_file_open
		@format._0_grid(*a)
		send_out 0,:bang
		log if @timelog
		@framecount+=1
		@finished.send_in 0, :grid, *a if @finished ;# why do i need this condition?
	end
	def log
		time = Time.new
		post("\#out: frame#%04d time: %10.3f s; diff: %5d ms", @framecount, time, ((time-@time)*1000).to_i)
		@time = time
	end
	install_rgrid 0
}
class BitPacking
	alias pack pack2
	alias unpack unpack2
end

=begin
FormatPPM.subclass("#io:tk",1,1) {
	install_rgrid 0
	def initialize(mode)
		@id = sprintf("x%08x",object_id)
		@filename = "/tmp/tk-#{$$}-#{@id}.ppm"
		if mode!=:out then raise "only #out" end
		super(mode,:file,@filename)
		GridFlow.gui "toplevel .#{@id}\n"
		GridFlow.gui "wm title . GridFlow/Tk\n"
		GridFlow.gui "image create photo gf#{@id} -width 320 -height 240\n"
		GridFlow.gui "pack [label .#{@id}.im -image #{@id}]\n"
	end
	def _0_rgrid_end
		super
		@stream.seek 0,IO::SEEK_SET
		GridFlow.gui "image create photo #{@id} -file #{@filename}\n"
	end
	def delete
		GridFlow.gui "destroy .#{@id}\n"
		GridFlow.gui "image delete #{@id}\n"
	end
	alias close delete
}
=end

end # module GridFlow
