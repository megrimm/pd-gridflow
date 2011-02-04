proc say {k v} {set ::say($k) $v}
proc category {k} {}
source locale/english.tcl
puts "#N canvas 0 0 632 659 10;"
set y 0
puts "#X obj 0 $y doc_demo;"; incr y 32
puts "#X obj 3 $y doc_section General Topics;"; incr y 32

foreach {k v} {
  about    {about GridFlow (credits, etc)}
  intro    {the main goal of this library (but not the only one)}

  attr     {attributes, the 'get' method, etc}
  canvas   {methods added by GridFlow to pd's canvas class accessible through \[namecanvas\])}
  grid     {what is a grid ?}
  image    {an image of pixels is just one way to use a grid}
  indexmap {goes with \[#store\]}
  numop1   {a numop1 is a transformation made on one number (usually)}
  numop2   {a numop2 represents a way of combining two numbers (usually)}
  numtype  {numtypes allow you to balance precision, range, headroom and efficiency ("speed")}

  hpgl     {about <hpgl> inlets and outlets}
} {
  set k "doc_link $k 1"
  set w [string length $k]
  set w [expr {$w*6+4}]
  incr w -36 ;# à cause de doc_link
  incr w -12 ;# à cause du $2 de doc_link
  regsub -all {\s?,} $v { \, } v
  puts "#X obj [expr 160-$w] $y $k;"
  puts "#X text 180 $y $v;"
  incr y 32
}

incr y 20
puts "#X obj 3 $y doc_section Object Classes;"; incr y 32
set n 0

foreach k [lsort [array names ::say *]] {
	set h 0 ;# gets clipped to 32 later, or replaced by whatever
	set v $::say($k)
	set ok $k ;# original k
	set sel obj
	if {$k == "#"}          {set k "$k"}
	if {$k == "#fold"}      {set k "$k +"}
	if {$k == "#scan"}      {set k "$k +"}
	if {$k == "#outer"}     {set k "$k +"}
	if {$k == "hpgl_op"}    {set k "$k +"}
	if {$k == "#for"}       {set k "$k 0 4"}
	if {$k == "receives"}   {set k "$k \$0-"}
	if {$k=="#cluster_avg"} {set k "$k 4"}
	if {$k=="cv/#KMeans"}   {set k "$k 4"}
	if {$k == "#many"}      {set k "$k tgl 3 3"}
	if {$k == "gf/lol"}     {set k "$k 0"}
	if {$k == "gf/canvas_hehehe"} {set k "$k 0"}
	if {$k == "gf/canvas_hohoho"} {set k "$k 0"}

	if {[regexp ^doc $k]}      {set k "doc_link $k"}
	if {[regexp ^#io $k]}      {set k "doc_link $k"}
	if {$k == "parallel_port"} {set k "doc_link $k"}
	if {$k == "gf/nbxhsl"}     {set k "doc_link $k"}
	if {$k == "#to~"}          {set k "doc_link $k"} ;# until crash gets fixed
	if {$k == "#in~"}          {set k "doc_link $k"} ;# until crash in [#to~] gets fixed
	if {$k == "gf/tk_button"}  {set k "doc_link $k"}

	set w [string length $k]
	regsub "\\$" $k "\\$" k
	if {$w<3} {set w 3}
	set w [expr {$w*6+4}]

	if {$ok == "#many"}  {set w  53; set h 53}
	if {$ok == "#see"}   {set w  69; set h 56}
	if {$ok == "doremi"} {set w 105; set h 73}
	if {$ok == "#color"} {set w 156; set h 55}
	if {$ok == "#type-gui"} {set w 64; set h 80}
	if {$ok == "display"} {set w 22}
	if {[regexp ^doc_link $k]} {incr w -36}

	puts "#X $sel [expr 160-$w] $y $k;"
	regsub -all "," $v " \\, " v
	regsub -all ";" $v " \\; " v
	regsub -all "\\$" $v "\\$" v
	puts "#X text 180 $y $v;"
	incr h 16 ;# interligne
	if {$h > 32} {incr y $h} {incr y 32}
	incr n
}

puts "#X text 0 $y $n classes are listed in this index.;"
