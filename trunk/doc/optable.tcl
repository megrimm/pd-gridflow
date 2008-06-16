#!/usr/bin/env tclsh

proc mset {vars list} {uplevel 1 "foreach {$vars} {$list} {break}"}

proc write {list} {
	set v [join $list " "]
	regsub "," $v " \\, " v
	regsub ";" $v " \\; " v
	regsub "\\$" $v "\\$" v
	puts "$v;"
}

write [list #N canvas 0 0 1024 768 10]
set y 0
set row 0
set oid 0
set msgboxes {}
set col1 96
set col2 512
set col3 768
set col4 1024

write [list #X obj 0 $y cnv 15 $col4 30 empty empty empty 20 12 0 14 20 -66577 0]
write [list #X text 10 $y op name]
write [list #X text $col1 $y description]
write [list #X text $col2 $y "effect on pixels"]
write [list #X text $col3 $y "effect on coords"]
incr y 32
incr oid 5

proc op {op desc onpixels oncoords} {
	global y
	if {$::row&1} {set bg -233280} {set bg -249792}
	write [list #X obj 0 $y cnv 15 $::col4 30 empty empty empty 20 12 0 14 $bg -66577 0]
	write [list #X msg 10 $y op $op]
	write [list #X text $::col1 $y $desc]
	write [list #X text $::col2 $y $onpixels]
	write [list #X text $::col3 $y $oncoords]
	lappend msgboxes [expr $::oid+1]
	incr ::row
	incr ::y 32
	incr ::oid 5
}

set sections {}
proc section {desc} {
	global y
	lappend ::sections [list $y $desc]
	incr ::y 16
}

section {numops}
op {ignore} { A } {no effect} {no effect}
op {put} { B } {replace by} {replace by}
op {+} { A + B } {brightness, crossfade} {move, morph}
op {-} { A - B } {brightness, motion detection} {move, motion detection}
op {inv+} { B - A } {negate then contrast} {180 degree rotate then move}
op {*} { A * B } {contrast} {zoom out}
op {/} { A / B, rounded towards zero } {contrast} {zoom in}
op {div} { A / B, rounded downwards } {contrast} {zoom in}
op {inv*} { B / A, rounded towards zero } {--} {--}
op {swapdiv} { B / A, rounded downwards } {--} {--}
op {%} { A % B, modulo (goes with div) } {--} {tile}
op {swap%} { B % A, modulo (goes with div) } {--} {--}
op {rem} { A % B, remainder (goes with /) } {--} {--}
op {swaprem} { B % A, remainder (goes with /) } {--} {--}
op {gcd} {greatest common divisor} {--} {--}
op {lcm} {least common multiple} {--} {--}
op {|} { A or B, bitwise } {bright munchies} {bottomright munchies}
op {^} { A xor B, bitwise } {symmetric munchies (fractal checkers)} {symmetric munchies (fractal checkers)}
op {&} { A and B, bitwise } {dark munchies} {topleft munchies}
op {<<} { A * (2**(B % 32)), which is left-shifting } {like *} {like *}
op {>>} { A / (2**(B % 32)), which is right-shifting } {like /,div} {like /,div}
op {||} { if A is zero then B else A } {--} {--}
op {&&} { if A is zero then zero else B} {--} {--}
op {min} { the lowest value in A,B } {clipping} {clipping (of individual points)}
op {max} { the highest value in A,B } {clipping} {clipping (of individual points)}
op {cmp} { -1 when A&lt;B; 0 when A=B; 1 when A&gt;B. } {--} {--}
op {==} { is A equal to B ? 1=true, 0=false } {--} {--}
op {!=} { is A not equal to B ? } {--} {--}
op {>} { is A greater than B ? } {--} {--}
op {<=} { is A not greater than B ? } {--} {--}
op {<} { is A less than B ? } {--} {--}
op {>=} {is A not less than B ? } {--} {--}
op {sin*} { B * sin(A) in centidegrees } {--} {waves, rotations}
op {cos*} { B * cos(A) in centidegrees } {--} {waves, rotations}
op {atan} { arctan(A/B) in centidegrees } {--} {find angle to origin (part of polar transform)}
op {tanh*} { B * tanh(A) in centidegrees } {smooth clipping} {smooth clipping (of individual points), neural sigmoid, fuzzy logic}
op {log*} { B * log(A) (in base e) } {--} {--}
op {gamma} { floor(pow(a/256.0,256.0/b)*256.0) } {gamma correction} {--}
op {**} { A**B, that is, A raised to power B } {gamma correction} {--}
op {abs-} { absolute value of (A-B) } {--} {--}
op {rand} { randomly produces a non-negative number below A } {--} {--}
op {sqrt} { square root of A, rounded downwards } {--} {--}
op {sq-} { (A-B) times (A-B) } {--} {--}
op {avg} { (A+B)/2 } {--} {--}
op {hypot} { square root of (A*A+B*B) } {--} {--}
op {clip+} { like A+B but overflow causes clipping instead of wrapping around (coming soon) } {--} {--}
op {clip-} { like A-B but overflow causes clipping instead of wrapping around (coming soon) } {--} {--}
op {erf*} { integral of e^(-x*x) dx ... (coming soon; what ought to be the scaling factor?) } {--} {--}
op {weight} { number of "1" bits in an integer} {--} {--}
op {sin} {sin(A-B) in radians, float only} {--} {--}
op {cos} {cos(A-B) in radians, float only} {--} {--}
op {atan2} {atan2(A,B) in radians, float only} {--} {--}
op {tanh} {tanh(A-B) in radians, float only} {--} {--}
op {exp} {exp(A-B) in radians, float only} {--} {--}
op {log} {log(A-B) in radians, float only} {--} {--}

section {vecops for complex numbers}
op {C.*    } {A*B} {--} {--}
op {C.*conj} {A*conj(B)} {--} {--}
op {C./    } {A/B} {--} {--}
op {C./conj} {A/conj(B)} {--} {--}
op {C.sq-  } {(A-B)*(A-B)} {--} {--}
op {C.abs- } {abs(A-B)} {--} {--}
op {C.sin  } {sin(A-B)} {--} {--}
op {C.cos  } {cos(A-B)} {--} {--}
op {C.tanh } {tanh(A-B)} {--} {--}
op {C.exp  } {exp(A-B)} {--} {--}
op {C.log  } {log(A-B)} {--} {--}

write [list #X obj 10 $y outlet]
set outletid $oid
incr oid

foreach msgbox $msgboxes {
	write [list #X connect $msgbox 0 $outletid 0]
}

write [list #X obj [expr $col1-1] 0 cnv 0 0 $y empty empty empty -1 12 0 14 0 -66577 0]
write [list #X obj [expr $col2-1] 0 cnv 0 0 $y empty empty empty -1 12 0 14 0 -66577 0]
write [list #X obj [expr $col3-1] 0 cnv 0 0 $y empty empty empty -1 12 0 14 0 -66577 0]

foreach section $sections {
	mset {y desc} $section
	write [list #X obj 0 $y cnv 15 $::col4 14 empty empty empty 20 12 0 14 -248881 -66577 0]
	write [list #X text 10 $y $desc]
	incr oid 2
}
