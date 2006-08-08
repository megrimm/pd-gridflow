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

GridFlow.gui %q{
############ properties_dialog #########

proc gf_color_popup_select {self name c} {
	global _ preset_colors
	set _($self:$name) $c
	.$self.$name.color configure \
		-background [format #%6.6x $_($self:$name)] \
		-highlightbackground [format #%6.6x $_($self:$name)]
}

proc gf_color_popup {self w name i} {
	set w $w.$name.popup
	if [winfo exists $w] {destroy $w}
	menu $w -tearoff false
	global preset_colors
	for {set i 0} {$i<[llength $preset_colors]} {incr i} {
		set c [lindex $preset_colors $i]
		$w add command -label "     " \
		-background "#$c" -foreground "#$c" \
		-command [list gf_color_popup_select $self $name [expr 0x$c]]
	}
	tk_popup $w \
		[expr [winfo rootx .$self.$name]] \
		[expr [winfo rooty .$self.$name]] 0
}

proc iemgui_choose_col {id var title} {
    set self [string trimleft $id .]
    global _
    set c $_($self:$var)
    if {[string index $c 0]=="#"} {set c [string replace $c 0 0 0x]}
    set color [tk_chooseColor -title $title \
	    -initialcolor [format "#%6.6x" [expr $c&0xFFFFFF]]]
    if {$color != ""} {
	gf_color_popup_select $self $var [expr [string replace $color 0 0 "0x"]&0xFFFFFF]
    }
}

proc gf_change_entry {self val} {
	set v [expr [$self get]+$val]
	$self delete 0 end
	$self insert 0 $v
}

proc gf_properties_dialog {self w ok which struct} {
    global _ look
    foreach {name label type options} $struct {
		set f $w.$name
        switch -- $type {
	    side {
		frame $f
		pack [label $f.label -text $label] -side left
		frame $f.side -relief ridge -borderwidth 2
		foreach {i side} {0 left 1 right 2 top 3 bottom} {
			radiobutton $f.side.$side -value $i \
				-variable _($self:$name) -text $side
		}
		pack $f.side.left   -side   left -fill y
		pack $f.side.right  -side  right -fill y
		pack $f.side.top    -side    top
		pack $f.side.bottom -side bottom
		pack  $f.side -side left
	    }
	    color {
		frame $f
		label $f.label -text $label
		# wtf, %6.6x sometimes gives me _8_ chars !?!
		set c $_($self:$name)
		switch -regexp -- $c { ^# { set c 0x[string trimleft $c #] } }
		set c [expr $c & 0xFCFCFC]
		button $f.color -text "      " -font {Courier 8} -width 10 -pady 2 \
			-command [list iemgui_choose_col $w $name $label] \
			-relief sunken -background [format #%6.6x $c] \
			-highlightbackground       [format #%6.6x $c]
		
		button $f.preset -text "..." -pady 2 -font {Helvetica 8} -command [list gf_color_popup $self $w $name $i]
		pack  $f.label $f.color $f.preset -side left
	    }
	    choice {
		frame $f
		set i 0
		pack [label $f.label -text [lindex $label 0]] $f -side left
		# this which is not good, need to sort it out...
		set m [menu $f.$name -tearoff 0]
		foreach part [lrange $label 1 end] {
			$m add command -label $part -command [list dropmenu_set $f $part]
			
		}
		label $f.label_set -text "default" -relief raised
		balloon $f.label_set "click to change setting"
		pack $f.label_set -side top -fill x
		# need to sort this out.......
		bind $f.label_set <1> "tk_popup $f.$name \[expr %X - 100\] %Y"
		
	    }
	    choice_color {
		frame $f
		pack [label $f.label -text $label] -side left
		button $f.color_chooser -bg $look($name) -activebackground $look($name) -command "gf_color_popup $f $look($name)" \
					-width 17 
		pack $f.color_chooser -side left
		
	    }
	    subsection {
	    	#frame $f
		puts "path ===== $f"
		label $f -text $label -bg "#0000aa" -fg "#ffff55" -font {helvetica -10 bold}
	    	#$self add_subsection $f $name
	    }
	    toggle {
		frame $f
		label $f.label -text $label
		checkbutton $f.toggle -variable _($self:name)
	    }
	    section {
	        label $f -text $label -bg "#0000aa" -fg "#ffff55" -font {helvetica -10 bold}
	    }
	    default {
		frame $f
		#pack [label $f.label -text $label] -side left
		#label $f.label -text $label -wraplength 200 -justify left
		
		pack [label $f.label -text $label] -side left
		balloon $f.label $name

		eval "entry $f.entry -textvariable _($self:$name) $options"
		pack $f.entry -side left
		bind $f.entry <Return> "$ok $self"
		switch -regexp -- $type {
		    integer|float|fontsize {
			frame $f.b -borderwidth 0
			button $f.b.1 -image icon_uparrow   -command "gf_change_entry $f.entry +1"
			button $f.b.2 -image icon_downarrow -command "gf_change_entry $f.entry -1"
			pack $f.b.1 $f.b.2 -side top
			pack $f.b -side left
			bind $f.entry <Button-4> "gf_change_entry $f.entry +1"
			bind $f.entry <Button-5> "gf_change_entry $f.entry -1"
		    }
		    entry {}
		    default {
			label $f.type -text "($type)" -fg "#808080"
			pack $f.type -side right -anchor e
		    }
		}
	    }
	}
	pack $f -side top -fill x
	catch {$f.label configure -width 45 -anchor e}
    }
}
} # GridFlow.gui
