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

#include "gridflow.hxx.fcs"
#include <jni.h> // for running Java code
//extern pthread_key_t __envkey; // from z_jni.c
#define GET_ENV JNIEnv &env = *(JNIEnv *) pthread_getspecific(__envkey);

void startup_android () {
/*
    GET_ENV
    jclass gfjava = env.FindClass("org/puredata/android/gridflow/GridFlow");
    post("gfjava=%p",gfjava);
    if (!gfjava) {post("d'la marde"); return;}
    jmethodID gridmethod = env.GetStaticMethodID(gfjava, "", "()[I");
    //jstring jmsg = env->NewStringUTF(env, msg);
	jobject a = env.CallObjectMethod(gfjava,gridmethod);
	post("a=%p",a);
*/
	//\startall
}
