proc say {k v} {set ::say($k) $v}
source locale/english.tcl
puts "#N canvas 0 0 640 480 10 ;"
set y 50
foreach k [lsort [array names ::say *]] {
	set w [string length $k]
	if {$w<3} {set w 3}
	set w [expr {$w*7}]
	if {$k == "#color"} {set w 171}
	puts "#X obj [expr 180-$w] $y $k;"
	set v $::say($k)
	regsub "," $v " \\, " v
	regsub ";" $v " \\; " v
	puts "#X text 200 $y $v;"
	if {$k == "#color"} {incr y 40}
	incr y 32
}
