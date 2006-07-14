# $Id$
#
# LTIlib-for-GridFlow
# Copyright (c) 2006 by Mathieu Bouchard and Heriniaina Andrianirina
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
   
proc cool {cid} {
  set m "this would be a cool feature, eh?"
  tk_messageBox -message $m -type yesno -icon question -parent $cid
}
 
proc LTIPropertiesDialog_new {wid class formid forms} {
  global _
  set _($wid:formid) $formid
  if {[catch {toplevel $wid}]} {raise $wid; focus $wid; wm deiconify $wid; return}
  wm title $wid "\[$class\]"
  set i 0
  foreach form $forms {
    pack [radiobutton $wid.$i -text "$i: $form" -variable _($wid:formid) -value $i -anchor w] -fill x -expand 1
    incr i
  }
  pack [frame $wid.bar] -side bottom -fill x -pady 2m
  foreach {name Name} {cancel Cancel apply Apply ok Ok} {
    pack [button $wid.bar.$name -command "LTIPropertiesDialog_$name $wid" -text $Name] \
	-side left -expand 1
  }
  pack [frame $wid.sep -height 2 -borderwidth 1 -relief sunken] \
	-side bottom -fill x -expand 1
  wm protocol $wid WM_DELETE_WINDOW "destroy $wid"
}

proc LTIPropertiesDialog_cancel {wid} {
  destroy $wid
}

proc LTIPropertiesDialog_apply  {wid} {
  global _
  pd $wid formid $_($wid:formid) \;
}
proc LTIPropertiesDialog_ok     {wid} {
  LTIPropertiesDialog_apply  $wid
  LTIPropertiesDialog_cancel $wid
}
