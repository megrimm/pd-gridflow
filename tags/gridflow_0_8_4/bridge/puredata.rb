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

module GridFlow
  class FObject
    def  add_inlets(n) self. ninlets += n end
    def add_outlets(n) self.noutlets += n end
  end
end

#  <matju> alx1: in puredata.rb, just after the header, you have a %w() block,
#  and in it you write the name of your object, and if your helpfile is not
#  named like your object, then you add an equal sign and the filename

#!@#$ DON'T PUT ABSTRACTIONS IN THE %w() !!!
# @mouse=help_mouse @motion_detection=help_motion_detect @fade=help_fade
# @apply_colormap_channelwise @checkers @complex_sq @contrast
# @posterize @ravel @greyscale_to_rgb @rgb_to_greyscale @solarize @spread
#rgb_to_yuv=#rgb_to_yuv_and_#yuv_to_rgb
#yuv_to_rgb=#rgb_to_yuv_and_#yuv_to_rgb
#clip #contrast #fade #numop #remap_image

# NEW help files
#!@#$ (what's #+-help.pd ? #print-help2.pd ?)
#%w(
#	# #cast #dim #reverse
#	  #pack=#unpack-#pack
#	#unpack=#unpack-#pack
#	renamefile
#	#in plotter_control
#	listelement exec ls #print unix_time
#	lti
#).each {|name|
#	if name =~ /=/ then name,file = name.split(/=/) else file = name end
#	begin
#		x = GridFlow.fclasses[name]
#		x.set_help "gridflow/flow_classes/#{file}-help.pd"
#	rescue Exception => e
#		GridFlow.post "for [#{name}], #{e.class}: #{e}" # + ":\n" + e.backtrace.join("\n")
#	end
#}

#GridFlow.gui "frame .controls.gridflow -relief ridge -borderwidth 2\n"
#GridFlow.gui "button .controls.gridflow.button -text FOO\n"
#GridFlow.gui "pack .controls.gridflow.button -side left\n"
#GridFlow.gui "pack .controls.gridflow -side right\n"

GridFlow.gui "source #{GridFlow::DIR}/bridge/puredata.tcl\n"
