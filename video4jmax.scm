(setq pname "video4jmax")

(provide-package pname "0.0.0")

(ucs "load" "module" pname
  (file-cat dir "c" "lib" jmax-arch jmax-mode "libvideo4jmax.so"))

(println "package: Video4jmax")

;; help
(sshh-load (file-cat dir "help" "video4jmax.help.index.scm"))
(help-summary pname (file-cat dir "help" "video4jmax.summary.jmax"))

