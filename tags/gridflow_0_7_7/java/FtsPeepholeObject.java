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
// Authors: Maurizio De Cecco, Francois Dechelle, Enzo Maggi, Norbert Schnell.
// 

package gridflow;

import ircam.jmax.*;
import ircam.jmax.fts.*;

/**
 * A generic FTS object with an int value.
 * Used for intbox and sliders, for example.
 * If the listener of this object is an instance
 * of FtsIntValueListener, fire it when the we got a new value
 * from the server.
 */

public class FtsPeepholeObject extends FtsIntValueObject
{
  /*****************************************************************************/
  /*                                                                           */
  /*                               CONSTRUCTORS                                */
  /*                                                                           */
  /*****************************************************************************/

  int flashDuration;

  /* for the message box */
    public FtsPeepholeObject(Fts fts, FtsObject parent, String variable, String className, int nArgs, FtsAtom args[]) {
	super( fts, parent, variable, className, nArgs, args);
    }

    public void setFlashDuration(int fd)
    {
	flashDuration = fd;
	getFts().getServer().putObjectProperty(this, "flash", fd);
	setDirty();
    }

    public int getFlashDuration()
    {
	return flashDuration;
    }

    /* Over write the localPut message to handle value changes;
     */

    protected void localPut(String name, int newValue)
    {
	if (name == "flash")
	    flashDuration = newValue;
	else
	    super.localPut(name, newValue);
    }
}
