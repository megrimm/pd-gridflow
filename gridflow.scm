;; $Id$

(setq pname "gridflow")

(provide-package pname "0.3.0")

(ucs "load" "module" pname
  (file-cat dir "c" "lib" jmax-arch jmax-mode "libgridflow.so"))
;; no java here!!!


(template-directory (file-cat dir "templates"))

(sshh-load (file-cat dir "help" "gridflow.help.index.scm"))

(println "package: GridFlow")
