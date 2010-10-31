/*
	GridFlow
	Copyright (c) 2001-2010 by Mathieu Bouchard

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
// cd PdCore/src/org/puredata/android
// mkdir gridflow
// cd gridflow
// ln -s ../../../../../jni/extra/gridflow/src/GridFlow.java .
// and replace PdTest.java's initGui's contents by : org.puredata.android.gridflow.GridFlow.init(this);
// (or you can also just add the line at the end of the inside of initGui)

package org.puredata.android.gridflow;
import android.app.Activity;
import android.graphics.*;
import android.widget.*;
import android.widget.Gallery.LayoutParams;

public class GridFlow {
	public static Activity activity;
	public static ImageView iv;
	public static int[] a;
	public static void init (Activity acti) {
		activity = acti;
		Bitmap b = Bitmap.createBitmap(320, 240, Bitmap.Config.ARGB_8888);
		iv = new ImageView(activity);
		int[] a = new int[240*320];
		int k=0;
		for (int i=0; i<240; i++) {
			for (int j=0; j<320; j++, k++) {
				a[k] = 0xff000000 + (((i^j)&255)<<16) + ((j&255)<<8) + i;
			}
		}
		b.setPixels(a,0,320,0,0,320,240);
		iv.setImageBitmap(b);
		//iv.setMaxWidth(320);
		//iv.setMaxHeight(240);
		iv.setAdjustViewBounds(true);
		LinearLayout ll = new LinearLayout(activity);
		iv.setLayoutParams(new Gallery.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
		ll.addView(iv);
		//activity.setContentView(ll);

		//TextView tv = new TextView(activity);
		//tv.setText("This is a test from the Emergency Broadcast System");
		//activity.setContentView(tv);
	}
/*
 	public int[] getArray () {
		iv.setImageDrawable(activity.getResources().getDrawable(0x7f020000));		
		return a;
	}
	public void apply () {

	}
*/
}
