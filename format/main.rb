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
