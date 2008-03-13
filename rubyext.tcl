# $Id$
#
# GridFlow
# Copyright (c) 2001-2006 by Mathieu Bouchard
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# See file ../COPYING for further informations on licensing terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

if {[info command post] == ""} {
	proc post {x} {puts $x}
}

# if not running Impd:
if {[string length [info command listener_new]] == 0} {
# start of part duplicated from Impd
proc listener_new {self name} {
	global _
	set _($self:hist) {}
	set _($self:histi) 0
	frame $self
	label $self.label -text "$name: "
	entry $self.entry -width 40
#	entry $self.count -width 5
	pack $self.label -side left
	pack $self.entry -side left -fill x -expand yes
#	pack $self.count -side left
	pack $self -fill x -expand no
	bind $self.entry <Up>     "listener_up   $self"
	bind $self.entry <Down>   "listener_down $self"
}

proc listener_up {self} {
	global _
	if {$_($self:histi) > 0} {set _($self:histi) [expr -1+$_($self:histi)]}
	$self.entry delete 0 end
	$self.entry insert 0 [lindex $_($self:hist) $_($self:histi)]
	$self.entry icursor end
#	$self.count delete 0 end
#	$self.count insert 0 "$_($self:histi)/[llength $_($self:hist)]"
}

proc listener_down {self} {
	global _
	if {$_($self:histi) < [llength $_($self:hist)]} {incr _($self:histi)}
	$self.entry delete 0 end
	$self.entry insert 0 [lindex $_($self:hist) $_($self:histi)]
	$self.entry icursor end
#	$self.count delete 0 end
#	$self.count insert 0 "$_($self:histi)/[llength $_($self:hist)]"
}

proc listener_append {self v} {
	global _
	lappend _($self:hist) $v
	set _($self:histi) [llength $_($self:hist)]
}

proc gf_tcl_eval {} {
	set l [.tcl.entry get]
	post "tcl: $l"
	post "returns: [eval $l]"
	listener_append .tcl [.tcl.entry get]
	.tcl.entry delete 0 end

}

}
# end of part duplicated from Impd

proc gf_ruby_eval {} {
	set l {}
	foreach c [split [.ruby.entry get] ""] {lappend l [scan $c %c]}
	pd "gridflow eval $l;"
	listener_append .ruby [.ruby.entry get]
	.ruby.entry delete 0 end
}

if {[catch {
  # DesireData
  Listener new .ruby "Ruby" {gf_ruby_eval}
}]} {
  if {[catch {
    # ImpureData (new)
    listener_new .ruby "Ruby" {gf_ruby_eval}
    listener_new  .tcl "Tcl"   {gf_tcl_eval}
  }]} {
    # ImpureData (old)
    listener_new .ruby "Ruby"; bind .ruby.entry <Return> {gf_ruby_eval}
    listener_new .tcl  "Tcl" ; bind  .tcl.entry <Return>  {gf_tcl_eval}
  }
}

catch {
	if {[file exists ${pd_guidir}/lib/gridflow/icons/peephole.gif]} {
		global pd_guidir
		image create photo icon_peephole -file ${pd_guidir}/lib/gridflow/icons/peephole.gif
		global butt
		button_bar_add peephole {guiext peephole}
	} {
		puts $stderr GAAAH
	}
}

