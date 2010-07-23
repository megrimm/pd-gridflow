#!/usr/bin/env tclsh

proc mset {vars list} {uplevel 1 "foreach {$vars} {$list} {break}"}
proc p {text} {write [list #X text 10 $::y $text]; incr ::y 60}
proc write {list} {
	set v [join $list " "]
	regsub -all "," $v " \\, " v
	regsub -all ";" $v " \\; " v
	regsub -all "\\$" $v "\\$" v
	puts $::fh "$v;"
}
proc obj  {args} {write [concat [list #X obj ] $args]; incr ::oid}
proc msg  {args} {write [concat [list #X msg ] $args]; incr ::oid}
proc text {args} {write [concat [list #X text] $args]; incr ::oid}

set fh [open numop.pd w]
write [list #N canvas 0 0 1024 768 10]
write [list #X obj 0 0 doc_demo]
set oid 1
set y 30
set row 0
set msgboxes {}
set col1 96
set col2 [expr $col1+350]
set col3 [expr $col2+230]
set col4 [expr $col3+230]
set rowsize 26

obj 0 $y cnv 15 $col4 22 empty empty empty 20 12 0 14 20 -66577 0
text 10 $y op name
text $col1 $y description
text $col2 $y "effect on pixels"
text $col3 $y "effect on coords"
incr y 24

# onpixels = meaning in pixel context (pictures, palettes)
# oncoords = meaning in spatial context (indexmaps, polygons)

# for vecops, the two analogy-columns were labelled:
#   meaning in geometric context (indexmaps, polygons, in which each complex number is a point)
#   meaning in spectrum context (FFT) in which each number is a (cosine,sine) pair
proc op {op desc {extra1 ""} {extra2 ""}} {
	global y
	if {$::row&1} {set bg -233280} {set bg -249792}
	obj 0 $y cnv 15 $::col4 [expr $::rowsize-2] empty empty empty 20 12 0 14 $bg -66577 0
	lappend ::msgboxes $::oid
	set x 10
	foreach op1 $op {msg $x $y op $op; incr 50}
	text                     $::col1 [expr {$y-4}] $desc
	if {$extra1 != ""} {text $::col2 [expr {$y-4}] $extra1}
	if {$extra2 != ""} {text $::col3 [expr {$y-4}] $extra2}
	incr ::row
	incr ::y $::rowsize
}

proc draw_columns {} {
	set y 30
	obj [expr $::col1-1] $y cnv 0 0 $::y empty empty empty -1 12 0 14 0 -66577 0
	obj [expr $::col2-1] $y cnv 0 0 $::y empty empty empty -1 12 0 14 0 -66577 0
	obj [expr $::col3-1] $y cnv 0 0 $::y empty empty empty -1 12 0 14 0 -66577 0
}

proc numbertype {op desc {extra1 ""} {extra2 ""}} {op $op $desc $extra1 $extra2}

set sections {}
proc section {desc} {
	lappend ::sections [list $::y $desc]
	incr ::y 16
}

section {numops}
op {ignore} { A } {no effect} {no effect}
op {put} { B } {replace by} {replace by}
op {+} { A + B } {brightness, crossfade} {move, morph}
op {-} { A - B } {brightness, motion detection} {move, motion detection}
op {inv+} { B - A } {negate then contrast} {180 degree rotate then move}
op {*} { A * B } {contrast} {zoom out}
op {*>>8} { (A * B) >> 8 } {contrast} {zoom out}
op {/} { A / B, rounded towards zero } {contrast} {zoom in}
op {div} { A / B, rounded downwards } {contrast} {zoom in}
op {inv*} { B / A, rounded towards zero }
op {swapdiv} { B / A, rounded downwards }
op {%} { A % B, modulo (goes with div) } {--} {tile}
op {swap%} { B % A, modulo (goes with div) }
op {rem} { A % B, remainder (goes with /) }
op {swaprem} { B % A, remainder (goes with /) }
op {gcd} {greatest common divisor}
op {lcm} {least common multiple}
op {|} { A or B, bitwise } {bright munchies} {bottomright munchies}
op {^} { A xor B, bitwise } {symmetric munchies (fractal checkers)} {symmetric munchies (fractal checkers)}
op {&} { A and B, bitwise } {dark munchies} {topleft munchies}
op {<<} { A * pow(2,mod(B,32)), which is left-shifting } {like *} {like *}
op {>>} { A / pow(2,mod(B,32)), which is right-shifting } {like /,div} {like /,div}
op {||} { if A is zero then B else A }
op {&&} { if A is zero then zero else B}
op {min} { the lowest value in A,B } {clipping} {clipping (of individual points)}
op {max} { the highest value in A,B } {clipping} {clipping (of individual points)}
op {cmp} { -1 when A<B, 0 when A=B, 1 when A>B }
op {==} { is A equal to B ? 1=true, 0=false }
op {!=} { is A not equal to B ? }
op {>} { is A greater than B ? }
op {<=} { is A not greater than B ? }
op {<} { is A less than B ? }
op {>=} {is A not less than B ? }
op {sin*} { B * sin(A) in centidegrees } {--} {waves, rotations}
op {cos*} { B * cos(A) in centidegrees } {--} {waves, rotations}
op {atan} { arctan(A/B) in centidegrees } {--} {find angle to origin; (part of polar transform)}
op {tanh*} { B * tanh(A) in centidegrees } {smooth clipping} {smooth clipping (of individual points); neural sigmoid, fuzzy logic}
op {log*} { B * log(A) (in base e) }
op {gamma} { floor(pow(a/256.0,256.0/b)*256.0) } {gamma correction}
op {**} { A**B, that is, A raised to power B } {gamma correction}
op {abs-} { absolute value of (A-B) }
op {rand} { randomly produces a non-negative number below A }
op {sqrt} { square root of A, rounded downwards }
op {sq-} { (A-B) times (A-B) }
op {avg} { (A+B)/2 }
op {hypot} { distance function: square root of (A*A+B*B) }
op {clip+} { like A+B but overflow causes clipping instead of wrapping around }
op {clip-} { like A-B but overflow causes clipping instead of wrapping around }
op {erf*} { integral of e^(-x*x) dx ... (coming soon; what ought to be the scaling factor?) }
op {weight} { number of "1" bits in an integer}
op {sin} {sin(A-B) in radians, float only}
op {cos} {cos(A-B) in radians, float only}
op {atan2} {atan2(A,B) in radians, float only}
op {tanh} {tanh(A-B) in radians, float only}
op {exp} {exp(A-B) in radians, float only}
op {log} {log(A-B) in radians, float only}

section {vecops for complex numbers}
op {C.*    } {A*B}
op {C.*conj} {A*conj(B)}
op {C./    } {A/B}
op {C./conj} {A/conj(B)}
op {C.sq-  } {(A-B)*(A-B)}
op {C.abs- } {abs(A-B)}
op {C.sin  } {sin(A-B)}
op {C.cos  } {cos(A-B)}
op {C.tanh } {tanh(A-B)}
op {C.exp  } {exp(A-B)}
op {C.log  } {log(A-B)}

#section {vecops for other things}
#op {cart2pol}
#op {pol2cart}

incr y 10
set outletid $oid
obj 10 $y outlet
incr y 20

foreach msgbox $msgboxes {write [list #X connect $msgbox 0 $outletid 0]}

draw_columns

foreach section $sections {
	mset {y1 desc} $section
	obj 0 $y1 cnv 15 $::col4 14 empty empty empty 20 12 0 14 -248881 -66577 0
	text 10 [expr $y1-2] $desc
}

p {
	note: a centidegree is 0.01 degree. There are 36000 centidegrees in a circle.
        Some angle operators use centidegrees, while some others use radians. To
        convert degrees into centidegrees, multiply by 100.
        To convert degrees into radians, divide by 57.2957 .
        Thus, to convert centidegrees into radians, divide by 5729.57 .
}

close $fh

