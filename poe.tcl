# $Id$
#----------------------------------------------------------------#
# POETCL
#
#   Copyright (c) 2005,2006 by Mathieu Bouchard
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# See file ../COPYING.desire-client.txt for further informations on licensing terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# Note that this is not under the same license as the rest of PureData.
# Even the DesireData server-side modifications stay on the same license
# as the rest of PureData.
#
#-----------------------------------------------------------------------------------#

# (please distinguish between what this is and what dataflow is)
# note, the toplevel class is called "thing".

package provide poe 0.1

if {$tcl_version < 8.5} {package require pre8.5}
set nextid 0
set _(Class:_class) Class
set _(Class:_super) {Thing}

proc proc* {name args body} {
	set argl {}
	foreach arg $args {set arg [lindex $arg 0]; lappend argl "$arg=\$$arg"}
	proc $name $args "puts \"\[VTgreen\]CALL TO PROC $name [join $argl " "]\[VTgrey\]\"; $body"
}

#proc Class_def {self selector args body} {
#	global _; if {![info exists _($self:_class)]} {error "unknown class '$self'"}
#	proc  ${self}_$selector "self $args" "global _; [regsub -all @(\[\\w\\?\]+) $body _(\$self:\\1)]"
#}
#proc def  {class selector args body}  {$class def  $selector $args $body}

proc expand_macros {body} {
	return [regsub -all @(\\\$?\[\\w\\?\]+) $body _(\$self:\\1)]
}

proc def {self selector args body} {
	global _ __trace
	if {![info exists _($self:_class)]} {error "unknown class '$self'"}
	if {[info exists __trace($self:$selector)]} {
		proc* ${self}_$selector "self $args" "global _; [expand_macros $body]"
	} {
		proc  ${self}_$selector "self $args" "global _; [expand_macros $body]"
	}
	#trace add execution ${self}_$selector enter dedebug
}

proc class_new {self {super {Thing}}} {
	global _
	set _($self:_class) Class
	set _($self:_super) $super
	set _($self:subclasses) {}
	foreach sup $super {lappend _($sup:subclasses) $self}
	proc ${self}_new {args} "
		global nextid _
		set self \[format o%07x \$nextid\]
		incr nextid
		set _(\$self:_class) $self
		eval [concat [list \$self init] \$args]
		return \$self
	"
	proc ${self}_new_as {self args} "
		global _
		if {[info exists _(\$self:_class)]} {error \"object '\\$self' already exists\" }
		set _(\$self:_class) $self
		eval [concat [list \$self init] \$args]
		return \$self
	"
}

proc lookup_method {class selector methodsv ancestorsv} {
	global _
	upvar $methodsv   methods
	upvar $ancestorsv ancestors
	set name ${class}_$selector
	if {[llength [info procs $name]]} {lappend methods $name}
	lappend ancestors $class
	foreach super $_($class:_super) {lookup_method $super $selector methods ancestors}
}

rename unknown unknown2
# TODO: remove duplicates in lookup
# TODO: cache lookup for greater speed
proc unknown {args} {
	global _ __
	set self [lindex $args 0]
	if {[llength [array names _ $self:_class]] == 0} {
		return [uplevel 1 [linsert $args 0 unknown2]]
	}
	set methods {}
	set ancestors {}
	set selector [lindex $args 1]
	set class $_($self:_class)
	if {[info exists __($class:$selector)]} {
		set methods $__($class:$selector)
	} {
		lookup_method $_($self:_class) $selector methods ancestors
		set __($class:$selector) $methods
	}
	set i 0
	if {![llength $methods]} {
		set ancestors [Class_ancestors $class]
		error "no such method '$selector' for object '$self'\nwith ancestors {$ancestors}"
	}
	eval [concat [list [lindex $methods $i] $self] [lrange $args 2 end]]
}

proc super {args} {
	upvar 2 methods methods self self i oi
	set i [expr 1+$oi]
	if {[llength $methods] < $i} {error "no more supermethods"}
	eval [concat [list [lindex $methods $i] $self] $args]
}

class_new Thing {}
#set _(Thing:_super) {}
def Thing init {} {}
def Thing == {other} {return [expr ![string compare $self $other]]}

# virtual destructor
def Thing delete {} {}

def Thing vars {} {
	set n [string length $self:]
	set ks [list]
	foreach k [array names _] {
		if {0==[string compare -length $n $self: $k]} {lappend ks [string range $k $n end]}
	}
	return $ks
}

def Thing inspect {} {
	set t [list "#<$self: "]
	foreach k [lsort [$self vars]] {lappend t "$k=[list $@$k] "}
	lappend t ">"
	return [join $t ""]
}

def Thing class {} {return $@_class}

class_new Class

# those return only the direct neighbours in the hierarchy
def Class superclasses {} {return $@_super}
def Class subclasses {} {return $@subclasses}

# those look recursively.
def Class ancestors {} {
	#if {[info exists @ancestors]} {}
	set r [list $self]
	foreach super $@_super {eval [concat [list lappend r] [$super ancestors]]}
	return $r
}
def Class <= {class} {return [expr [lsearch [$self ancestors] $class]>=0]}

# those are static methods, and objective.tcl doesn't distinguish them yet.
def Class new    {   args} {eval [concat [list ${self}_new       ] $args]}
def Class new_as {id args} {eval [concat [list ${self}_new_as $id] $args]}

#-----------------------------------------------------------------------------------#

proc VTgreen  {} {return "\x1b\[0;1;32m"}
proc VTred    {} {return "\x1b\[0;1;31m"}
proc VTyellow {} {return "\x1b\[0;1;33m"}
proc VTgrey   {} {return "\x1b\[0m"}

proc error_text {} {
	global errorInfo
	set e $errorInfo
	regsub -all "    invoked from within\n" $e "" e
	regsub -all "\n    \\(" $e " (" e
	regsub -all {\n[^\n]*procedure \"(::unknown|super)\"[^\n]*\n} $e "\n" e
	regsub -all {\s*while executing\s*\n} $e "\n" e
	#regsub {\n$} $e "" e
	return $e
}
proc error_dump {} {
	global errorCode
	puts "[VTred]Exception:[VTgrey] errorCode=$errorCode; errorInfo=[error_text]"
}

proc tracedef {class method {when enter}} {
	global __trace
	set __trace($class:$method) $when
}

proc yell {var key args} {
	global _ $key
	puts "[VTyellow]HEY! at [info level -1] set $key [list $_($key)][VTgrey]"
}
