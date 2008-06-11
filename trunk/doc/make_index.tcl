proc say {k v} {set ::say($k) $v}
source locale/english.tcl
puts "#N canvas 0 0 640 480 10 ;"
set y 50
foreach k [lsort [array names ::say *]] {
	puts "#X obj 40 $y $k;"
	puts "#X text 160 $y $::say($k);"
	incr y 32
}
