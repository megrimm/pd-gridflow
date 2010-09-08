#!/usr/bin/env ruby
=begin
	GridFlow
	Copyright (c) 2001-2010 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
=end

$stack = []
$classes = []
$exit = 0

ClassDecl = Struct.new(:name,:supername,:methods,:grins,:attrs,:info)
MethodDecl = Struct.new(:rettype,:inlet,:selector,:args,:minargs,:maxargs,:where,:continue)
Arg  = Struct.new(:type,:name,:default)
Attr = Struct.new(:type,:name,:default,:virtual)

class MethodDecl
	def ==(o)
		return false unless rettype==o.rettype && maxargs==o.maxargs
		args.each_index{|i| args[i] == o.args[i] or return false }
		return true
	end
	def ===(o)
		return false unless rettype==o.rettype && maxargs==o.maxargs
		args.each_index{|i| args[i].type == o.args[i].type and args[i].default == o.args[i].default or return false }
		return true
	end
	attr_accessor :done
	def selector2; if inlet==-2 then selector elsif inlet==-1 then "_n_#{selector}" else "_#{inlet}_#{selector}" end end
end

class Arg
  def canon(type) if type=="PtrGrid" then type="Grid *" else type end end
  def ==(o) canon(type)==canon(o.type) && name==o.name end
end

In  = File.open ARGV[0], "r"
Out = File.open ARGV[1], "w"

def handle_class(line)
	raise "already in class #{where}" if $stack[-1] and ClassDecl===$stack[-1]
	/^(\w+)(?:\s*[:<]\s*(\w+))?\s*(\{.*)?/.match line or raise "syntax error #{where}"
	classname = $1
	superclassname = $2
	rest = $3
	q=ClassDecl.new(classname,superclassname,{},{},{},false)
	$stack << q
	$classes << q
	Out.print "#define THISCLASS #{classname}\n\# #{$linenumber}\n"
	if rest and /^\{/ =~ rest then
		Out.print "struct #{classname} "
		Out.print ": #{superclassname}" if superclassname
		Out.print rest
	end
end

def parse_methoddecl(line,term)
	/^(\w+(?:\s*\*)?)\s+(\w+)\s*\(([^\)]*)\)\s*(#{term})/.match line or raise "syntax error #{where}"
	rettype,selector,args,continue = $1,$2,$3,$4,$6
	if /^\d+$/ =~ rettype then inlet = rettype; rettype = "void"
	elsif    rettype=="n" then inlet = -1     ; rettype = "void" else inlet = -2 end
	args,minargs,maxargs = parse_args args
	MethodDecl.new(rettype,inlet,selector,args,minargs,maxargs,where,continue)
end

def parse_args(args)
	args = args.split(/,/)
	maxargs = args.length
	args = args.map {|arg|
		if /^\s*\.\.\.\s*$/.match arg then maxargs=-1; next end
		/^\s*([\w\s\*<>]+)\s*\b(\w+)\s*(?:\=(.*))?/.match arg or raise "syntax error in \"#{arg}\" #{where}"
		type,name,default=$1,$2,$3
		Arg.new(type.sub(/\s+$/,""),name,default)
	}.compact
	minargs = args.length
	minargs-=1 while minargs>0 and args[minargs-1].default
	[args,minargs,maxargs]
end

def unparse_args(args,with_default=true)
	args.map {|arg|
		x="#{arg.type} #{arg.name}"
		x << '=' << arg.default if with_default and arg.default
		x
	}.join(", ")
end

def where; "[#{ARGV[0]}:#{$linenumber}]" end

def handle_attr(line)
	line.gsub!(/\/\/.*$/,"") # remove comment
	frame = $stack[-1]; raise "missing \\class #{where}" if not frame or not ClassDecl===frame
	type = line.gsub(%r"//.*$","").gsub(%r"/\*.*\*/","").gsub(%r";?\s*$","")
	virtual = !!type.slice!(/\(\)$/)
	name = type.slice!(/\w+$/)
	handle_decl "void ___get(t_symbol *s);" if frame.attrs.size==0
	frame.attrs[name]=Attr.new(type,name,nil,virtual)
	if virtual then handle_decl "#{type} #{name}();"
	else		Out.print line end
	type.gsub!(/\s+$/,"")
	type.gsub!(/^\s+/,"")
	if type=="bool" then handle_decl "0 #{name} (#{type} #{name}=true);"
	else		     handle_decl "0 #{name} (#{type} #{name});" end
end

def handle_decl(line)
	qlass = $stack[-1]
	raise "missing \\class #{where}" if not qlass or not ClassDecl===qlass
	classname = qlass.name
	m = parse_methoddecl(line,"(;\s*|\\{?.*)$")
	qlass.methods[m.selector2] = m
	if /^\{/ =~ m.continue
	  handle_def(line,true)
	else
	  Out.print "#{m.rettype} #{m.selector2}("
	  Out.print "VA" if m.maxargs<0
	  Out.print unparse_args m.args if m.args.length>0
	  Out.print "); static void #{m.selector2}_wrap(#{classname} *self, VA);"
	end
end

def check_argc(m)
	fudge=0
	fudge=1 if m.inlet==-1
	fudge=2 if m.selector2=="_0_anything"
	if    m.maxargs==m.minargs then Out.print  "EXACTARGS(#{fudge},#{m.minargs})"
	elsif m.maxargs==-1        then Out.print    "MINARGS(#{fudge},#{m.minargs})"
	else                            Out.print "MINMAXARGS(#{fudge},#{m.minargs},#{m.maxargs})" end
end

def pass_args(m)
	if m.maxargs==-1 then Out.print "argc,argv" end
	m.args.each_with_index{|arg,i|
		Out.print "," if i>0 or m.maxargs==-1
		Out.print "argc<#{i+1}?(#{arg.type})#{arg.default}:" if arg.default
		Out.print "TO(#{arg.type},argv[#{i}])"
	}
	Out.print ");DEF_OUT;}"
end

def handle_def(line,in_class_block=false)
	m = parse_methoddecl(line,"\\{?.*$")
	term = line[/\{.*/]
	qlass = $stack[-1]
	raise "missing \\class #{where}" if not qlass or not ClassDecl===qlass
	classname = qlass.name
	n = m
	if qlass.methods[m.selector2]
		m = qlass.methods[m.selector2]
		if !m===n then
			STDERR.puts "ERROR: def does not match decl:"
			STDERR.puts "#{m.where}: \\decl #{m.inspect}"
			STDERR.puts "#{n.where}: \\def #{n.inspect}"
			$exit = 1
		end
	else
		qlass.methods[m.selector2] = m
	end
	if in_class_block then Out.print "static void " else Out.print "void #{classname}::" end
	Out.print "#{m.selector2}_wrap(#{classname} *self, VA) {"
	if /^_(\d+)_(\w+)$/ =~ m.selector2 then context = "inlet #{$1} method #{$2}" else context = m.selector2 end
	Out.print "DEF_IN(\"method #{m.selector2}\");"
	Out.print "#{m.rettype} foo;" if m.rettype!="void"
	check_argc m
	Out.print "foo = " if m.rettype!="void"
	Out.print " self->#{m.selector2}("
	pass_args m
	Out.print m.rettype+" "
	Out.print classname+"::" unless in_class_block
	Out.print m.selector2+"("
	Out.print "VA" if m.maxargs<0
	Out.print unparse_args(n.args,false) if m.args.length>0
	Out.print ")"+term+" "
	qlass.methods[m.selector2].done=true
end

def handle_constructor(line)
	frame = $stack[-1]
	raise "missing \\class #{where}" if not frame or not ClassDecl===frame
	m = parse_methoddecl("void constructor"+line,"(.*)$")
	Out.print "#{frame.name}(BFObject *bself, MESSAGE) : #{frame.supername}(bself,MESSAGE2) {"
	Out.print "DEF_IN(\"constructor\");"
	check_argc m
	Out.print "#{m.selector}(sel"
	Out.print "," if m.maxargs!=0
	pass_args m
	Out.print "#{m.rettype} #{m.selector}(t_symbol *sel"
	Out.print ", VA" if m.maxargs<0
	Out.print ", #{unparse_args m.args}" if m.args.length>0
	Out.print ") "+line[/\{.*/]
end

def handle_classinfo(line)
	frame = $stack[-1]
	cl = frame.name
	line="{}" if /^\s*$/ =~ line
	Out.print "CLASSINFO(#{cl})"
	get="void ___get(t_symbol *s=0) {t_atom a[1];"
	frame.attrs.each {|name,attr|
		virtual = if attr.virtual then "()" else "" end
		get << "if (s==gensym(\"#{name}\")) set_atom(a,#{name}#{virtual}); else "
		next if frame.methods["_0_"+name].done
		type,name,default = attr.to_a
		handle_def "0 #{name} (#{type} #{name}) {this->#{name}=#{name}; changed(gensym(\"#{name}\"));}"
	}
	line.gsub!(/^\s*(\w+\s*)?\{/,"")
	get << "RAISE(\"unknown attr %s\",s->s_name); outlet_anything(out[noutlets-1],s,1,a);}"
	handle_def get if frame.attrs.size>0
	Out.print "void #{frame.name}_startup (FClass *fclass) {"
	frame.methods.each {|name,method|
	 Out.print "fclass->methods[insel(#{method.inlet},gensym(\"#{method.selector}\"))] = FMethod(#{frame.name}::#{method.selector2}_wrap);" }
	frame.attrs.each   {|name,attr|   Out.print "fclass->  attrs[gensym(\"#{name}\")] = new AttrDecl(gensym(\"#{name}\"),\"#{attr.type}\");" }
	Out.print line.chomp
end

def handle_grin(line)
	fields = line.split(/\s+/)
	i = fields[0].to_i
	Out.print "GRINDECL(#{i})"
	handle_decl "#{i} grid(GridOut *foo);"
	handle_decl "#{i} list(...);"
	handle_decl "#{i} float(float f);"
	$stack[-1].grins[i] = fields.dup
end

def handle_end(line)
	frame = $stack[-1]
	fields = line.split(/\s+/)
	n = fields.length
	if not ClassDecl===frame then raise "\\end: frame is not a \\class" end
	cl = frame.name
	if fields[0]!="class" or (n>1 and not /^\{/ =~ fields[1] and fields[1]!=cl) then raise "end not matching #{where}" end
	frame.grins.each {|i,v|
		Out.print "static GridHandler #{cl}_grid_#{i}_hand = GRIN_#{v[1]||'all'}(#{cl}::grinw_#{i});"
		check = "CHECK_GRIN(#{cl},#{i});"
		handle_def "#{i} grid(GridOut *foo) {#{check}in[#{i}]->begin(foo);}"
		handle_def "#{i} list(...)          {#{check}in[#{i}]->from_list(argc,argv);}" if not frame.methods["_#{i}_list" ].done
		handle_def "#{i} float(float f)     {#{check}in[#{i}]->from_float(f);}"        if not frame.methods["_#{i}_float"].done
	}
	if /^class\s*(\w+\s+)?\{(.*)/ =~ line then handle_classinfo("{"+$2) end
	$stack.pop
	Out.print "\n#undef THISCLASS\n\# #{$linenumber}\n"
end

def handle_startall(line)
	$classes.each {|q|
		Out.print "fclass_install(&ci#{q.name},"
		if q.supername then Out.print "&ci#{q.supername}" else Out.print "0" end
		Out.print ");"
	}
end

$linenumber=1
loop{
	x = In.gets
	break if not x
	if /^\s*\\(\w+)\s*(.*)$/.match x then
		begin
			send("handle_#{$1}",$2)
			Out.puts "//FCS"
		rescue StandardError => e
			STDERR.puts e.inspect, "at line #{$linenumber}", e.backtrace
			File.unlink ARGV[1]
			exit 1
		end
	else Out.puts x end
	$linenumber+=1
}

exit $exit
