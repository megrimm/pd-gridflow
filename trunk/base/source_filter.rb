$keywords = %w(class decl def end grdecl)
$stack = []
$classes = []

ClassDecl = Struct.new(:name,:supername,:methods)
MethodDecl = Struct.new(:rettype,:selector,:arglist)
Arg = Struct.new(:type,:name,:default)

In = File.open ARGV[0], "r"
Out = File.open ARGV[1], "w"

def handle_class(line)
	raise "already in class #{where}" if $stack[-1] and ClassDecl===$stack[-1]
	STDERR.puts "class: #{line}"
	/^(\w+)\s+<\s+(\w+)$/.match line or raise "syntax error #{where}"
	q=ClassDecl.new($1,$2,{})
	$stack << q
	$classes << q
	Out.puts ""
end

def parse_arglist(arglist)
	arglist.split(/,/).map {|arg|
		/^\s*([\w\s\*<>]+)\s*\b(\w+)\s*(?:\=(.*))?/.match arg or
			raise "syntax error in \"#{arg}\" #{where}"
		type,name,default=$1,$2,$3
		Arg.new(type.sub(/\s+$/,""),name,default)
	}
end

def unparse_arglist(arglist,with_default=true)
	arglist.map {|arg|
		x="#{arg.type} #{arg.name} "
		x<<'='<<arg.default if with_default and arg.default
		x
	}.join(", ")
end

def where
  "[#{ARGV[0]}:#{$linenumber}]"
end

def handle_decl(line)
	STDERR.puts "decl: #{line}"
	/^(\w+)\s+(\w+)\s*\((.*)\)\s*;\s*$/.match line or
		raise "syntax error #{where}"
	rettype,selector,arglist = $1,$2,$3
	arglist = parse_arglist arglist
	raise "missing \\class #{where}" if
		not $stack[-1] or not ClassDecl===$stack[-1]
	classname = $stack[-1].name
	$stack[-1].methods[selector] = MethodDecl.new(rettype,selector,arglist)

	Out.print "#{rettype} #{selector}(int argc, Ruby *argv"
	Out.print "," if arglist.length>0
	Out.puts "#{unparse_arglist arglist});//FCS"
end

def handle_def(line)
	STDERR.puts "def: #{line}"
	/^(\w+)\s+(\w+)\s*\((.*)\)(\s*{?\s*)$/.match line or
		raise "syntax error #{where}"
	rettype,selector,arglist,brace = $1,$2,$3,$4
	arglist = parse_arglist arglist
	qlass = $stack[-1]
	raise "missing \\class #{where}" if not qlass or not ClassDecl===qlass
	classname = qlass.name
if qlass.methods[selector] #!@#$ HACK
	method = qlass.methods[selector] or raise "undeclared method #{where}"
	arglist = method.arglist
else
	qlass.methods[selector]=method=MethodDecl.new(rettype,selector,arglist)
end

	minargs = maxargs = method.arglist.length
	minargs-=1 while minargs>0 and method.arglist[minargs-1].default

	Out.print "static Ruby #{classname}_#{method.selector}_wrap"+
	"(int argc, Ruby *argv, Ruby rself) {"+
	"static const char *methodspec = "+
	"\"#{qlass.name}::#{method.selector}(#{unparse_arglist arglist,false})\";"+
	"DGS(#{classname});"

	Out.print "if (argc<#{minargs}||argc>#{maxargs}) "+
		"RAISE(\"got %d args instead of %d..%d in %s\""+
		",argc,#{minargs},#{maxargs},methodspec);"

	error = proc {|x,y|
		"RAISE(\"got %s instead of #{x} in %s\","+
		"rb_str_ptr(rb_inspect(rb_obj_class(#{y}))),methodspec)"
	}

	method.arglist.each_with_index{|arg,i|
		case arg.type
		when "Symbol"
			Out.print "if (argc>#{i} && TYPE(argv[#{i}])!=T_SYMBOL) "+
			error[arg.type,"argv[#{i}]"]+";"
		when "Array"
			Out.print "if (argc>#{i} && TYPE(argv[#{i}])!=T_ARRAY) "+
			error[arg.type,"argv[#{i}]"]+";"
		when "String"
			Out.print "if (argc>#{i} && TYPE(argv[#{i}])==T_SYMBOL) "+
			"argv[#{i}]=rb_funcall(argv[#{i}],SI(to_s),0);"
			Out.print "if (argc>#{i} && TYPE(argv[#{i}])!=T_STRING) "+
			error[arg.type,"argv[#{i}]"]+";"
		end
	}

	Out.print "return " if rettype!="void"
	Out.print " self->#{method.selector}(argc,argv"
	arglist.each_with_index{|arg,i|
		if arg.default then
			Out.print ",argc<#{i+1}?#{arg.default}:convert(argv[#{i}],(#{arg.type}*)0)"
		else
			Out.print ",convert(argv[#{i}],(#{arg.type}*)0)"
		end
	}
	Out.print ");"
	Out.print "return Qnil;" if rettype=="void"
	Out.print "} #{rettype} #{classname}::#{method.selector}(int argc, Ruby *argv"
	Out.print "," if arglist.length>0
	Out.puts "#{unparse_arglist arglist, false})#{brace}//FCS"
end

def handle_grdecl(line)
	qlass = $stack[-1]
	Out.puts qlass.methods.map {|foo,method|
		"DECL(#{qlass.name},#{method.selector})"
	}.join(",")
end

def handle_end(line)
	frame = $stack.pop
	fields = line.split(/\s+/)
	n = fields.length
	if ClassDecl===frame then
		if fields[0]!="class" or
		(n>1 and fields[1]!=frame.name)
		then raise "end not matching #{where}" end
	end
	Out.puts ""
end

def handle_startup(line)
	$classes.each {|q| Out.print "fclass_install(&ci#{q.name});" }
	Out.puts ""
end

$linenumber=1
loop{
	x = In.gets
	break if not x
	if /^\s*\\(\w+)\s*(.*)$/.match x then
		send("handle_#{$1}",$2)
	else
		Out.puts x
	end
	$linenumber+=1
}
