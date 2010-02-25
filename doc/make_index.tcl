proc say {k v} {set ::say($k) $v}
proc category {k} {}
source locale/english.tcl
puts "#N canvas 0 0 560 480 10 ;"
set y 50
set n 0
foreach k [lsort [array names ::say *]] {
	set v $::say($k)
	if {$k == "#"}          {set k "$k +"}
	if {$k == "#fold"}      {set k "$k +"}
	if {$k == "#scan"}      {set k "$k +"}
	if {$k == "#outer"}     {set k "$k +"}
	if {$k == "#cast"}      {set k "$k i"}
	if {$k == "#for"}       {set k "$k 0 4 1"}
	if {$k == "#redim"}     {set k "$k ()"}
	if {$k == "receives"}   {set k "$k \$0-"}
	if {$k=="#cluster_avg"} {set k "$k 4"}
	set w [string length $k]
	regsub "\\$" $k "\\$" k
	if {$w<3} {set w 3}
	set w [expr {$w*6+2}]
	if {$k == "#color"} {set w 154}
	puts "#X obj [expr 160-$w] $y $k;"
	regsub "," $v " \\, " v
	regsub ";" $v " \\; " v
	regsub "\\$" $v "\\$" v
	puts "#X text 180 $y $v;"
	if {$k == "#color"} {incr y 40}
	incr y 32
	incr n
}

puts "#X text 0 $y $n classes are listed in this index.;"
