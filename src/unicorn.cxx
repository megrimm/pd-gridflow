/*
	GridFlow
	Copyright (c) 2001-2011 by Mathieu Bouchard

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
*/

#include "bundled/m_pd.h"

extern "C" void gridflow_unicorn_setup () {
	sys_gui((char *)"catch {rename pdtk_canvas_sendkey pdtk_canvas_sendqui\n"
	  "proc pdtk_canvas_sendkey {name state key iso shift} {\n"
	  "if {$iso != \"\" && [lsearch {BackSpace Tab Return Escape Space Delete KP_Delete} $iso]<0} {\n"
	  "  binary scan [encoding convertto $iso] c* bytes\n"
	  "  foreach byte $bytes {pd [canvastosym $name] key $state [expr {$byte & 255}] $shift \\;}\n"
	  "} else {pdtk_canvas_sendqui $name $state $key $iso $shift}}}\n");
#if 0
	sys_gui("rename pdtk_text_new pdtk_text_nous\n"
	        "proc pdtk_text_new {a b c d e f g} {pdtk_text_nous $a $b $c $d [encoding convertfrom $e] $f $g}\n"
		"rename pdtk_text_set pdtk_text_sept\n"
	        "proc pdtk_text_set {a b e        } {pdtk_text_sept $a $b       [encoding convertfrom $e]      }\n");
#endif
}
