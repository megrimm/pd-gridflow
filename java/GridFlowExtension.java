//
// jMax
// Copyright (C) 1994, 1995, 1998, 1999 by IRCAM-Centre Georges Pompidou, Paris, France.
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// See file LICENSE for further informations on licensing terms.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
// Based on Max/ISPW by Miller Puckette.
//
// Authors: Riccardo Borghesi, Francois Dechelle, Norbert Schnell.
// 

package gridflow;

import ircam.jmax.script.pkg.*;
import ircam.jmax.script.*;
import ircam.jmax.editors.patcher.objects.*;
import ircam.jmax.*;
import ircam.jmax.mda.*;

import java.io.*;

/**
 * The table extension; install the table data type
 * and the table file data handler
 */
public class GridFlowExtension extends tcl.lang.Extension implements JavaExtension
{
  public void init(Interpreter interp)
  {
	System.out.println("HELLO WORLD FROM GRIDFLOW.JAR !");
	ObjectCreatorManager.registerFtsClass("peephole",
		gridflow.FtsPeepholeObject.class);
	ObjectCreatorManager.registerGraphicClass("peephole",
		gridflow.Peephole.class, "gridflow");
  }

    /* this method should be removed as soon as jacl is completely forgotten about */
  public void init(tcl.lang.Interp interp)
  {
	System.out.println("HELLO WORLD FROM GRIDFLOW.JAR ! (2)");
	ObjectCreatorManager.registerFtsClass("peephole",
		gridflow.FtsPeepholeObject.class);
	ObjectCreatorManager.registerGraphicClass("peephole",
		gridflow.Peephole.class, "gridflow");
  }
}
