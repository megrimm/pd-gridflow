proc pd_mess_split {s} {
        set s [regsub -all {\\\\} $s "\x03"]
        set s [regsub -all {(^|[^\\]);}          $s "\\1\x02"]
        set s [regsub -all {(^|[^\\])[\s\n]+}    $s "\\1\x01"]
        set s [regsub -all {^\n}                 $s "\x01"] ;# oops
        set s [regsub -all {\\([\\; \{\}])} $s "\\1\\2"]
        set s [regsub -all \x03 $s \\]
        set r {}
        foreach m [split $s \x02] {
                set m [regsub -all "\x01+" $m "\x01"]
                set m [regsub -all "^\x01" $m ""]
                set t [split $m \x01]
                lappend r $t
        }
        return $r
}
