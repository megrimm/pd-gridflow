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

import java.util.*;
import java.awt.*;
import java.awt.event.*;

import javax.swing.*;
import javax.swing.event.*;

import ircam.jmax.toolkit.*;
import ircam.jmax.toolkit.actions.*;

import ircam.jmax.editors.patcher.objects.*;

import ircam.jmax.guiobj.*;

//
// The graphic pop-up menu used to change the number of an inlet or an outlet in a subpatcher.
//

public class PeepholeControlPanel extends JPanel implements ObjectControlPanel
{
  GraphicObject target = null;
  JSlider durationSlider;
  JLabel durationLabel;

  public PeepholeControlPanel()
  {
    super();
    setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

    JLabel titleLabel = new JLabel("Flash Duration", JLabel.CENTER);
    titleLabel.setForeground(Color.black);
    durationLabel = new JLabel("", JLabel.CENTER);

    JPanel labelBox = new JPanel();
    labelBox.setLayout(new BoxLayout(labelBox, BoxLayout.X_AXIS));
    labelBox.add(Box.createRigidArea(new Dimension(20, 0)));    
    labelBox.add(titleLabel);    
    labelBox.add(Box.createHorizontalGlue());    
    labelBox.add(durationLabel);    
    labelBox.add(Box.createRigidArea(new Dimension(20, 0)));    

    durationSlider = new JSlider(JSlider.HORIZONTAL, 10, 500, 125);
    durationSlider.setBorder(BorderFactory.createEmptyBorder(0,0,0,10));
    durationSlider.addChangeListener(new ChangeListener(){
	public void stateChanged(ChangeEvent e) {
	  JSlider source = (JSlider)e.getSource();
	  int duration = (int)source.getValue();
	    
	  if(!source.getValueIsAdjusting())
	    {
	      if((target!=null)&&(((FtsPeepholeObject)target.getFtsObject()).getFlashDuration()!=duration))
		((FtsPeepholeObject)target.getFtsObject()).setFlashDuration(duration);
	    }
      
	  durationLabel.setText(""+(int)duration);
	}
      });

    add(labelBox);    
    add(durationSlider);        
    validate();
  }

  public void update(GraphicObject obj)
  {
    target = obj;
    int duration = ((FtsPeepholeObject)obj.getFtsObject()).getFlashDuration();
    durationSlider.setValue(duration);
    durationLabel.setText(""+duration);    

    //revalidate();
  }
}
