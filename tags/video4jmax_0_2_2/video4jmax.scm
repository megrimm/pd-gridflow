;; $Id$

(setq pname "video4jmax")

(provide-package pname "0.2.2")

(ucs "load" "module" pname
  (file-cat dir "c" "lib" jmax-arch jmax-mode "libvideo4jmax.so"))

(template-directory (file-cat dir "templates"))

(sshh-load (file-cat dir "help" "video4jmax.help.index.scm"))

(println "package: Video4jmax")
