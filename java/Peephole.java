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

import java.awt.*;
import java.awt.event.*;
//import java.awt.geom.*;
//import java.awt.image.*;
//import java.awt.image.ImageObserver;
import java.util.*;
import javax.swing.*;
//import java.beans.*;

import ircam.jmax.fts.*;
import ircam.jmax.utils.*;

import ircam.jmax.editors.patcher.*;
import ircam.jmax.editors.patcher.interactions.*;
import ircam.jmax.editors.patcher.menus.*;
import ircam.jmax.editors.patcher.objects.*;

import ircam.jmax.guiobj.*;

public class Peephole extends GraphicObject
implements FtsIntValueListener//, ImageObserver
{
	public static native int getWindowXID( String sFrameName_, int debug );
	private static boolean _bLibLoaded = false;
/*
	static {
		_bLibLoaded = false;
		String soname = PathInfo.jmax_path + "/java/classes/libPeephole.so";
		System.out.println("soname:"+soname);
		Runtime.getRuntime().load(soname);
		_bLibLoaded = true;
		System.out.println( "gridflow.Peephole.<static>._bLibLoaded: " + _bLibLoaded );
	}
*/
	private static final int DEFAULT_WIDTH = 64;
	private static final int DEFAULT_HEIGHT = 48;
	private static final int MINIMUM_SIZE = 32;
	private static final int BORDER = 4;
	
	private static Hashtable imageTable = new Hashtable();
	public static PeepholeControlPanel controlPanel = new PeepholeControlPanel();

	public ErmesSketchPad sketchpad;
	public boolean dragging;
	public boolean createdSubwindow;
	
	public Peephole( ErmesSketchPad theSketchPad, FtsObject theFtsObject) {
		super(theSketchPad, theFtsObject);
		sketchpad = theSketchPad;

		int w = getWidth();
		if (w == -1) setWidth(DEFAULT_WIDTH);
		else if (w <= MINIMUM_SIZE) setWidth(MINIMUM_SIZE);

		int h = getHeight();
		if (h == -1) setHeight(DEFAULT_HEIGHT);
		else if (h <= MINIMUM_SIZE) setHeight(MINIMUM_SIZE);
	}

	public void setGeometry () {
		if (dragging || !createdSubwindow) return;
		FtsAtom a[] = new FtsAtom[4];
		for (int i=0; i<4; i++) a[i] = new FtsAtom();
		//int y = getY()-getHeight()/2+BORDER;
		int y = getY()+BORDER;
		int x = getX()+BORDER;
		
		for (java.awt.Container ct=sketchpad; ct!=null; ct=ct.getParent()) {
			if (ct.getParent()==null) break; // hack
			if (ct.getParent().getParent()==null) break; // hack hack
			x += ct.getX();
			y += ct.getY();
		}
		a[0].setInt(y);
		a[1].setInt(x);
		a[2].setInt(getHeight()-2*BORDER);
		a[3].setInt(getWidth()-2*BORDER);
		ftsObject.sendMessage(0, "set_geometry", 4, a);
	}
	
	public void setWidth (int w) {
		if (w < MINIMUM_SIZE) w = MINIMUM_SIZE;
		super.setWidth(w);
	}

	public void setHeight (int h) {
		if (h < MINIMUM_SIZE) h = MINIMUM_SIZE;
		super.setHeight(h);
		setGeometry();
	}

	public void setX (int x) { super.setX(x); setGeometry(); }
	public void setY (int y) { super.setY(y); setGeometry(); }

	public boolean isSquare() {return true;}

	public void gotSqueack(int squeack, Point mouse, Point oldMouse) {
		System.out.println("mouse "+squeack+","+mouse+","+oldMouse);
		super.gotSqueack(squeack,mouse,oldMouse);
	}

	public void resizing(boolean v) {
		System.out.println("resizing("+v+") (was "+dragging+")");
		if (dragging && !v) {
			dragging=v; setGeometry();
		} else {
			dragging=v;
		}
	}

	public void createSubwindow() {
		try {
		Component window = sketchpad;
		for (int i=0; i<10; i++) {
			String name = window.getName();
			//int xid = name==null ? 0xFadedF00 : getWindowXID(window.getName(),1);
			//System.out.println("step #"+i+": name='"+name+"'; ");
			//"window_id="+Integer.toHexString(xid));
			if (window.getParent()==null) break;
			window = window.getParent();
			//System.out.println("frame="+window.toString());
		}
		int windowid = (int) Math.round(Math.random()*0x40000000);
		String title = ((Frame)window).getTitle();
		if (title==null) title="";
		System.out.println("title was: "+title+" ("+title.length()+" bytes)");
		if (title.indexOf("\t\t\t")<0) title+="\t\t\t"+windowid;
		System.out.println("title is now: "+title+" ("+title.length()+" bytes)");
		((Frame)window).setTitle(title);
		windowid = Integer.valueOf(title.substring(title.indexOf("\t\t\t")+3)).intValue();

		FtsAtom a[] = new FtsAtom[1];
		for (int i=0; i<1; i++) a[i] = new FtsAtom();
		// apparently you can't pass symbols in those arglists???
		// a[0].setSymbol(new FtsSymbol("patate"));
		a[0].setInt(windowid);
		ftsObject.sendMessage(0, "open", 1, a);
		} catch (Throwable e) {
			System.out.println("Peephole exception: "+e.toString()+": "+e.getMessage());
			e.printStackTrace();
		}

		createdSubwindow = true;
		setGeometry();
	}
	
	public void valueChanged(int value) { updateRedraw(); }
	
	// ImageObserver interface
	/*public boolean imageUpdate(Image img, int infoflags, int x, int y, int width, int height) {
		return true;
	}*/

	public void paint(Graphics g) {
		if (!createdSubwindow) createSubwindow();
		int x = getX(), y = getY();
		int w = getWidth(), h = getHeight();
		//Image image;
		Color c = Settings.sharedInstance().getUIColor();
		if(isSelected()) c = c.darker();
		g.setColor(c);
		g.fill3DRect(x+1, y+1, w-2, h-2, true);
		super.paint(g);
	}

	public void updatePaint(Graphics g) { paint(g); }

	public ObjectControlPanel getControlPanel() { return this.controlPanel; }

	//popup interaction 
	public void popUpUpdate(boolean onInlet, boolean onOutlet, SensibilityArea area) {
		super.popUpUpdate(onInlet, onOutlet, area);
		ObjectPopUp.addSeparation();
		ObjectPopUp.getInstance().add(ColorPopUpMenu.getInstance());    
		getControlPanel().update(this);
		ObjectPopUp.getInstance().add((JPanel)getControlPanel());
	}

	public void popUpReset() {
		super.popUpReset();
		ObjectPopUp.getInstance().remove(ColorPopUpMenu.getInstance());
		ObjectPopUp.getInstance().remove((JPanel)getControlPanel());
		ObjectPopUp.removeSeparation();
	}
	protected SensibilityArea findSensibilityArea (int mouseX, int mouseY) {
		if ((mouseY >= getY() + getHeight() - ObjectGeometry.V_RESIZE_SENSIBLE_HEIGHT)
		&& (mouseX >= getX() + getWidth() / 2)) {
			return SensibilityArea.get(this, Squeack.VRESIZE_HANDLE);
		} else
		return super.findSensibilityArea( mouseX, mouseY);
	}
}
