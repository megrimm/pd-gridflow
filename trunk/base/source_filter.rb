$keywords = %w(class decl def end grdecl)
$stack = []
$classes = []

ClassDecl = Struct.new(:name,:supername,:methods)
MethodDecl = Struct.new(:rettype,:selector,:arglist,:minargs,:maxargs,:where)
Arg = Struct.new(:type,:name,:default)

class MethodDecl
	def ==(o)
		return false unless rettype==o.rettype &&
		maxargs==o.maxargs # && minargs==o.minargs 
		arglist.each_index{|i| arglist[i] == o.arglist[i] or return false }
		return true
	end
end

class Arg
	def ==(o)
		type==o.type && name==o.name # && default==o.default
	end
end

In = File.open ARGV[0], "r"
Out = File.open ARGV[1], "w"

def handle_class(line)
	raise "already in class #{where}" if $stack[-1] and ClassDecl===$stack[-1]
	#STDERR.puts "class: #{line}"
	/^(\w+)\s+<\s+(\w+)$/.match line or raise "syntax error #{where}"
	q=ClassDecl.new($1,$2,{})
	$stack << q
	$classes << q
	Out.puts ""
end

def parse_methoddecl(line,term)
	/^(\w+)\s+(\w+)\s*\((.*)\)\s*#{term}\s*$/.match line or
		raise "syntax error #{where}"
	rettype,selector,arglist = $1,$2,$3
	arglist,minargs,maxargs = parse_arglist arglist
	MethodDecl.new(rettype,selector,arglist,minargs,maxargs,where)
end

def parse_arglist(arglist)
	arglist = arglist.split(/,/)
	maxargs = arglist.length
	args = arglist.map {|arg|
		if /^\s*\.\.\.\s*$/.match arg then maxargs=-1; next end
		/^\s*([\w\s\*<>]+)\s*\b(\w+)\s*(?:\=(.*))?/.match arg or
			raise "syntax error in \"#{arg}\" #{where}"
		type,name,default=$1,$2,$3
		Arg.new(type.sub(/\s+$/,""),name,default)
	}.compact
	minargs = args.length
	minargs-=1 while minargs>0 and args[minargs-1].default
	[args,minargs,maxargs]
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

def handle_attr(line)
	raise "missing \\class #{where}" if
		not $stack[-1] or not ClassDecl===$stack[-1]
	#$stack[-1].attrs[]
	Out.puts line
end

def handle_decl(line)
	#STDERR.puts "decl: #{line}"
	raise "missing \\class #{where}" if
		not $stack[-1] or not ClassDecl===$stack[-1]
	classname = $stack[-1].name
	m = parse_methoddecl(line,";")
	$stack[-1].methods[m.selector] = m

	Out.print "#{m.rettype} #{m.selector}(int argc, Ruby *argv"
	Out.print "," if m.arglist.length>0
	Out.puts "#{unparse_arglist m.arglist});//FCS"
end

def handle_def(line)
	#STDERR.puts "def: #{line}"
	m = parse_methoddecl(line,"\{?")
	qlass = $stack[-1]
	raise "missing \\class #{where}" if not qlass or not ClassDecl===qlass
	classname = qlass.name
	if qlass.methods[m.selector]
		n = m; m = qlass.methods[m.selector]
		if m!=n then
			STDERR.puts "warning: def does not match decl:"
			STDERR.puts "#{m.where}: \\decl #{m.inspect}"
			STDERR.puts "#{n.where}: \\def #{n.inspect}"
		end
	else
		qlass.methods[m.selector] = m
	end

	Out.print "static Ruby #{classname}_#{m.selector}_wrap"+
	"(int argc, Ruby *argv, Ruby rself) {"+
	"static const char *methodspec = "+
	"\"#{qlass.name}::#{m.selector}(#{unparse_arglist m.arglist,false})\";"+
	"DGS(#{classname});"

	Out.print "if (argc<#{m.minargs}"
	Out.print "||argc>#{m.maxargs}" if m.maxargs!=-1
	Out.print ") RAISE(\"got %d args instead of %d..%d in %s\""+
		",argc,#{m.minargs},#{m.maxargs},methodspec);"

	error = proc {|x,y|
		"RAISE(\"got %s instead of #{x} in %s\","+
		"rb_str_ptr(rb_inspect(rb_obj_class(#{y}))),methodspec)"
	}

	m.arglist.each_with_index{|arg,i|
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

#	Out.print "return " if m.rettype!="void"
	Out.print "VALUE foo = " if m.rettype!="void" ###

	Out.print " self->#{m.selector}(argc,argv"
	m.arglist.each_with_index{|arg,i|
		if arg.default then
			Out.print ",argc<#{i+1}?#{arg.default}:convert(argv[#{i}],(#{arg.type}*)0)"
		else
			Out.print ",convert(argv[#{i}],(#{arg.type}*)0)"
		end
	}
	Out.print ");"
	Out.print "self->check_magic();"
	Out.print "return Qnil;" if m.rettype=="void"
	Out.print "return foo;" if m.rettype!="void" ###
	Out.print "} #{m.rettype} #{classname}::#{m.selector}(int argc, Ruby *argv"
	Out.print "," if m.arglist.length>0
	Out.puts "#{unparse_arglist m.arglist, false}){//FCS"
end

def handle_grdecl(line)
	qlass = $stack[-1]
	Out.puts qlass.methods.map {|foo,method|
		c,s = qlass.name,method.selector
		"{ \"#{s}\",(RMethod)#{c}_#{s}_wrap }"
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
	if :ruby==frame then
		if fields[0]!="ruby" then raise "expected \\end ruby" end
	end
	Out.puts ""
end

def handle_startup(line)
	$classes.each {|q| Out.print "fclass_install(&ci#{q.name});" }
	Out.puts ""
end

def handle_ruby(line)
	Out.puts ""
	$stack.push :ruby
end

$rubymode=false
$linenumber=1
loop{
	x = In.gets
	break if not x
	if /^\s*\\(\w+)\s*(.*)$/.match x then
		send("handle_#{$1}",$2)
	else
		if $stack[-1]==:ruby then
			x.gsub!(/\\""/,'\\\1')
			x="\"#{x.chomp}\"\n"
		end
		Out.puts x
	end
	$linenumber+=1
}
