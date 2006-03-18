require 'mkmf'

dir_config("patched")
dir_config("ltiheaders")
dir_config("ltilib")

#$libs = append_library("ltilib7_rmfcd")
$libs = append_library($libs, Config::CONFIG['RUBY_INSTALL_NAME'])
$libs = append_library($libs, "ltir")

create_makefile("lti")

#have_library("ltilib7_rmfcd")