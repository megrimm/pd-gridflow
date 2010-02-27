proc say {k v} {set ::say($k) $v}
proc category {k} {}
source locale/english.tcl
puts "#N canvas 0 0 560 480 10 ;"
set y 50
set n 0
foreach k [lsort [array names ::say *]] {
	set h 0 ;# gets clipped to 32 later, or replaced by whatever
	set v $::say($k)
	set ok $k ;# original k
	if {$k == "#"}          {set k "$k +"}
	if {$k == "#fold"}      {set k "$k +"}
	if {$k == "#scan"}      {set k "$k +"}
	if {$k == "#outer"}     {set k "$k +"}
	if {$k == "#cast"}      {set k "$k i"}
	if {$k == "#for"}       {set k "$k 0 4 1"}
	if {$k == "#redim"}     {set k "$k ()"}
	if {$k == "receives"}   {set k "$k \$0-"}
	if {$k=="#cluster_avg"} {set k "$k 4"}
	if {$k == "#many"}      {set k "$k tgl 3 3"}

	set w [string length $k]
	regsub "\\$" $k "\\$" k
	if {$w<3} {set w 3}
	set w [expr {$w*6+4}]

	if {$ok == "#many"}  {set w  53; set h 53}
	if {$ok == "#see"}   {set w  69; set h 56}
	if {$ok == "doremi"} {set w 105; set h 73}
	if {$ok == "#color"} {set w 156; set h 55}
	if {$ok == "display"} {set w 22}

	puts "#X obj [expr 160-$w] $y $k;"
	regsub "," $v " \\, " v
	regsub ";" $v " \\; " v
	regsub "\\$" $v "\\$" v
	puts "#X text 180 $y $v;"
	incr h 16 ;# interligne
	if {$h > 32} {incr y $h} {incr y 32}
	incr n
}

puts "#X text 0 $y $n classes are listed in this index.;"
