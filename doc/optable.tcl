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
set i 0
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

foreach opdesc {
{{ignore} { A } {no effect} {no effect} }
{{put} { B } {replace by} {replace by} }
{{+} { A + B } {brightness, crossfade} {move, morph} }
{{-} { A - B } {brightness, motion detection} {move, motion detection} }
{{inv+} { B - A } {negate then contrast} {180 degree rotate then move} }
{{*} { A * B } {contrast} {zoom out} }
{{/} { A / B, rounded towards zero } {contrast} {zoom in} }
{{div} { A / B, rounded downwards } {contrast} {zoom in} }
{{inv*} { B / A, rounded towards zero } {--} {--} }
{{swapdiv} { B / A, rounded downwards } {--} {--} }
{{%} { A % B, modulo (goes with div) } {--} {tile} }
{{swap%} { B % A, modulo (goes with div) } {--} {--} }
{{rem} { A % B, remainder (goes with /) } {--} {--} }
{{swaprem} { B % A, remainder (goes with /) } {--} {--} }
{{gcd} {greatest common divisor} {--} {--} }
{{lcm} {least common multiple} {--} {--} }
{{|} { A or B, bitwise } {bright munchies} {bottomright munchies} }
{{^} { A xor B, bitwise } {symmetric munchies (fractal checkers)} {symmetric munchies (fractal checkers)} }
{{&} { A and B, bitwise } {dark munchies} {topleft munchies} }
{{<<} { A * (2**(B % 32)), which is left-shifting } {like *} {like *} }
{{>>} { A / (2**(B % 32)), which is right-shifting } {like /,div} {like /,div} }
{{||} { if A is zero then B else A } {--} {--} }
{{&&} { if A is zero then zero else B} {--} {--} }
{{min} { the lowest value in A,B } {clipping} {clipping (of individual points)} }
{{max} { the highest value in A,B } {clipping} {clipping (of individual points)} }
{{cmp} { -1 when A&lt;B; 0 when A=B; 1 when A&gt;B. } {--} {--} }
{{==} { is A equal to B ? 1=true, 0=false } {--} {--} }
{{!=} { is A not equal to B ? } {--} {--} }
{{>} { is A greater than B ? } {--} {--} }
{{<=} { is A not greater than B ? } {--} {--} }
{{<} { is A less than B ? } {--} {--} }
{{>=} {is A not less than B ? } {--} {--} }
{{sin*} { B * sin(A) in centidegrees } {--} {waves, rotations} }
{{cos*} { B * cos(A) in centidegrees } {--} {waves, rotations} }
{{atan} { arctan(A/B) in centidegrees } {--} {find angle to origin (part of polar transform)} }
{{tanh*} { B * tanh(A) in centidegrees } {smooth clipping} {smooth clipping (of individual points), neural sigmoid, fuzzy logic} }
{{log*} { B * log(A) (in base e) } {--} {--} }
{{gamma} { floor(pow(a/256.0,256.0/b)*256.0) } {gamma correction} {--} }
{{**} { A**B, that is, A raised to power B } {gamma correction} {--} }
{{abs-} { absolute value of (A-B) } {--} {--} }
{{rand} { randomly produces a non-negative number below A } {--} {--} }
{{sqrt} { square root of A, rounded downwards } {--} {--} }
{{sq-} { (A-B) times (A-B) } {--} {--} }
{{avg} { (A+B)/2 } {--} {--} }
{{hypot} { square root of (A*A+B*B) } {--} {--} }
{{clip+} { like A+B but overflow causes clipping instead of wrapping around (coming soon) } {--} {--} }
{{clip-} { like A-B but overflow causes clipping instead of wrapping around (coming soon) } {--} {--} }
{{erf*} { integral of e^(-x*x) dx ... (coming soon; what ought to be the scaling factor?) } {--} {--} }
{{weight} { number of "1" bits in an integer} {--} {--} }
{{sin} {sin(A-B) in radians, float only} {--} {--} }
{{cos} {cos(A-B) in radians, float only} {--} {--} }
{{atan2} {atan2(A,B) in radians, float only} {--} {--} }
{{tanh} {tanh(A-B) in radians, float only} {--} {--} }
{{exp} {exp(A-B) in radians, float only} {--} {--} }
{{log} {log(A-B) in radians, float only} {--} {--} }
} {
	mset {op desc onpixels oncoords} $opdesc
	if {$i&1} {set bg -233280} {set bg -249792}
	write [list #X obj 0 $y cnv 15 $col4 30 empty empty empty 20 12 0 14 $bg -66577 0]
	write [list #X msg 10 $y op $op]
	write [list #X text $col1 $y $desc]
	write [list #X text $col2 $y $onpixels]
	write [list #X text $col3 $y $oncoords]
	lappend msgboxes [expr $oid+1]
	incr i
	incr y 32
	incr oid 5
}

write [list #X obj 10 $y outlet]
set outletid $oid
incr oid

foreach msgbox $msgboxes {
	write [list #X connect $msgbox 0 $outletid 0]
}

write [list #X obj [expr $col1-1] 0 cnv 0 0 $y empty empty empty -1 12 0 14 0 -66577 0]
write [list #X obj [expr $col2-1] 0 cnv 0 0 $y empty empty empty -1 12 0 14 0 -66577 0]
write [list #X obj [expr $col3-1] 0 cnv 0 0 $y empty empty empty -1 12 0 14 0 -66577 0]
