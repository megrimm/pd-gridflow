=begin
	$Id$

	LTIlib-for-GridFlow
	Copyright (c) 2006 by Mathieu Bouchard

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

require "rblti"
include GridFlow
include Rblti

FObject.subclass("lti",1,1) {
  @functors=[]
  @parameterses=[]
  cs=Rblti.constants
  cs.each{|c|
    next unless /^R(.)(\w+)_parameters/ =~ c
    @parameterses<<c
    @functors<<$1.upcase+$2
  }
  @others = (cs-(@functors+@parameterses)).sort
  def _0_help(a=nil) self.class.help(a) end
  def self.help(a=nil)
   case a
   when nil:
    GridFlow.post "try one of:"
    GridFlow.post "help functors"
    GridFlow.post "help others"
   when:functors:
    @functors.each{|x| GridFlow.post "functor %s", x }
    GridFlow.post "total %d functors",@functors.length
   when:others:
    @others.each{|x| GridFlow.post "other %s", x }
    GridFlow.post "total %d others",@others.length
   end
  end
}
