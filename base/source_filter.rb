#!/usr/bin/env ruby
=begin
	$Id$

	GridFlow
	Copyright (c) 2001-2006 by Mathieu Bouchard

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

$keywords = %w(class decl def end grdecl)
$stack = []
$classes = []

ClassDecl = Struct.new(:name,:supername,:methods,:grins,:attrs,:info)
MethodDecl = Struct.new(:rettype,:selector,:arglist,:minargs,:maxargs,:where,:static)
Arg = Struct.new(:type,:name,:default)

class MethodDecl
	def ==(o)
		return false unless rettype==o.rettype && static==o.static &&
			maxargs==o.maxargs
		arglist.each_index{|i| arglist[i] == o.arglist[i] or return false }
		return true
	end
end

class Arg; def ==(o) type==o.type && name==o.name end end

In = File.open ARGV[0], "r"
Out = File.open ARGV[1], "w"

def handle_class(line)
	raise "already in class #{where}" if $stack[-1] and ClassDecl===$stack[-1]
	#STDERR.puts "class: #{line}"
	/^(\w+)(?:\s*<\s*(\w+))?$/.match line or raise "syntax error #{where}"
	q=ClassDecl.new($1,$2,{},{},{},false)
	$stack << q
	$classes << q
	Out.puts ""
end

def parse_methoddecl(line,term)
	/^(static\s)?\s*(\w+)\s+(\w+)\s*\(([^\)]*)\)\s*#{term}/.match line or
		raise "syntax error #{where} #{line}"
	static,rettype,selector,arglist = $1,$2,$3,$4
	arglist,minargs,maxargs = parse_arglist arglist
	MethodDecl.new(rettype,selector,arglist,minargs,maxargs,where,static)
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
		x="#{arg.type} #{arg.name}"
		x<<'='<<arg.default if with_default and arg.default
		x
	}.join(", ")
end

def where; "[#{ARGV[0]}:#{$linenumber}]" end

def handle_attr(line)
	type = line.gsub(%r"//.*$","").gsub(%r"/\*.*\*/","").gsub(%r";?\s*$","")
	name = type.slice!(/\w+$/)
	raise "missing \\class #{where}" if
		not $stack[-1] or not ClassDecl===$stack[-1]
	$stack[-1].attrs[name]=Arg.new(type,name,nil)
	Out.print line.gsub(/\/\/.*$/,"") # hack!
	handle_decl "void _0_#{name}_m (#{type} #{name});"
#	Out.puts "# #{$linenumber}"
end

def handle_decl(line)
	raise "missing \\class #{where}" if
		not $stack[-1] or not ClassDecl===$stack[-1]
	classname = $stack[-1].name
	m = parse_methoddecl(line,";\s*$")
	$stack[-1].methods[m.selector] = m

	Out.print "static " if m.static
	Out.print "#{m.rettype} #{m.selector}(int argc, Ruby *argv"
	Out.print "," if m.arglist.length>0
	Out.print "#{unparse_arglist m.arglist});"
	Out.puts "static Ruby #{m.selector}_wrap"+
	"(int argc, Ruby *argv, Ruby rself);//FCS"
#	Out.puts "# #{$linenumber}"
end

def handle_def(line)
	m = parse_methoddecl(line,"\\{?.*$")
	term = line[/\{.*/]
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

	Out.print "Ruby #{classname}::#{m.selector}_wrap"+
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
			Out.print "else if (argc>#{i} && TYPE(argv[#{i}])!=T_STRING) "+
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
        if m.rettype=="R"
	  Out.print ").r;"
	else
	  Out.print ");"
	end
	Out.print "self->check_magic();"
	Out.print "return Qnil;" if m.rettype=="void"
	Out.print "return foo;" if m.rettype!="void" ###
	Out.print "} #{m.rettype} #{classname}::#{m.selector}(int argc, Ruby *argv"
	Out.print "," if m.arglist.length>0
	Out.puts "#{unparse_arglist m.arglist, false})#{term}//FCS"
end

def handle_classinfo(line)
	frame = $stack[-1]
	cl = frame.name
	line="{}" if /^\s*$/ =~ line
	Out.print "static void #{cl}_startup (Ruby rself);"
	Out.print "static void *#{cl}_allocator () {return new #{cl};}"
	Out.print "static MethodDecl #{cl}_methods[] = {"
	Out.print frame.methods.map {|foo,method|
		c,s = frame.name,method.selector
		"{ \"#{s}\",(RMethod)#{c}::#{s}_wrap }"
	}.join(",")
	Out.print "}; static FClass ci#{cl} = { #{cl}_allocator, #{cl}_startup,"
	Out.print "#{cl.inspect}, COUNT(#{cl}_methods), #{cl}_methods };"
	Out.puts "void #{frame.name}_startup (Ruby rself) "+line
end

def handle_grin(line)
	fields = line.split(/\s+/)
	i = fields[0].to_i
	c = $stack[-1].name
	Out.print "template <class T> void grin_#{i}(GridInlet *in, long n, T *data);"
	Out.print "template <class T> static void grinw_#{i} (GridInlet *in, long n, T *data);"
	Out.print "static GridHandler grid_#{i}_hand;"
	handle_decl "Ruby _#{i}_grid(...);"
	$stack[-1].grins[i] = fields.dup
end

def handle_end(line)
	frame = $stack.pop
	fields = line.split(/\s+/)
	n = fields.length
	if ClassDecl===frame then
		#handle_classinfo if not frame.info
		cl = frame.name
		if fields[0]!="class" or
		(n>1 and fields[1]!=cl)
		then raise "end not matching #{where}" end
		$stack.push frame
		frame.attrs.each {|name,attr|
			type,name,default = attr.to_a
			#STDERR.puts "type=#{type} name=#{name} default=#{default}"
			handle_def "void _0_#{name}_m (#{type} #{name}) { this->#{name}=#{name}; }"
		}
		frame.grins.each {|i,v|
			cli = "#{cl}::grinw_#{i}"
			k = case v[1]
			when     nil: [1,1,1,1,1,1,1]
			when 'int32': [0,0,1,0,0,0,0]
			when   'int': [1,1,1,1,0,0,0]
			when 'float': [0,0,0,0,1,1,0]
			else raise 'BORK BORK BORK' end
			ks = k.map{|ke| if ke==0 then 0 else cli end}.join(",")
			Out.print "static GridHandler #{cl}_grid_#{i}_hand = GRIN(#{ks});"
			handle_def "Ruby _#{i}_grid(...) {"+
				"if (in.size()<=#{i}) in.resize(#{i}+1);"+
				"if (!in[#{i}]) in[#{i}]=new GridInlet((GridObject *)this,&#{cl}_grid_#{i}_hand);"+
				"return in[#{i}]->begin(argc,argv);}"

		}
		$stack.pop
	end
	if :ruby==frame then
		if fields[0]!="ruby" then raise "expected \\end ruby" end
	end
	Out.puts ""
end

def handle_startall(line)
	$classes.each {|q|
		Out.print "rb_funcall(EVAL(\"GridFlow\"),SI(fclass_install),2,PTR2FIX(&ci#{q.name}),"
		if q.supername then
			Out.print "EVAL(\"GridFlow::#{q.supername}\"));"
		else
			Out.print "Qnil);"
		end
	}
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
		begin
			send("handle_#{$1}",$2)
		rescue StandardError => e
			STDERR.puts e.inspect, "at line #{$linenumber}", e.backtrace
			File.unlink ARGV[1]
			exit 1
		end
	else
		if $stack[-1]==:ruby then
			x.gsub!(/([\\\"])/) { "\\"+$1 }
			x="\"#{x.chomp}\\n\"\n"
		end
		Out.puts x
	end
	$linenumber+=1
}
