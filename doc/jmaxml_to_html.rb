# $Id$
# convert jMax-XML to HTML with our formatting.

require "xmlparser"

=begin todo

	[ ] make it use an XML library
	[ ] make it handle all relevant tags
	[ ] make it validate
	[ ] make it find the size of the pictures (and insert width/height attrs)
	[ ] tune the output

=end

class DTDParser < XMLParser
	def elementDecl(*args)
		print "elementDecl: "; p args
	end
	def attlistDecl(*args)
		print "attlistDecl: "; p args
	end
end

class XNode
	class<<self; attr_accessor :valid_tags; end
	self.valid_tags = {
		"p" => 1,
		"b" => 1,
		"u" => 1,
		"i" => 1,
		"k" => 1,
		"sup" => 1,
		"section" => 1,
		"li" => 1,
		"list" => 1,
		"icon" => 1,
		"arg" => 1,
		"method" => 1,
		"dim" => 1,
		"grid" => 1,
		"inlet" => 1,
		"outlet" => 1,
		"sample" => 1,
		"jmax_class" => 1,
		"operator_1" => 1,
		"operator_2" => 1,
		"jmax_doc" => 1,
		"format" => 1,
		"prose" => 1,
	}
	def initialize tag, att, *contents
		@tag,@att,@contents =
		 tag, att, contents
	end
	attr_reader :tag, :att, :contents
	def [] i; contents[i] end
	def print_index
		puts "<tr><td colspan='4'><i>index goes here.</i></td></tr>"
	end

	$counters=[]
	$sections=nil
	def print_contents
		e=nil
		case tag
		when "jmax_doc"; #nothing
		when "section"
			black_ruler
			print "<tr><td colspan='4'><h4>#{att['name']}</h4></td></tr>"
			e="<tr><td>&nbsp;</td></tr>"
			$section={}
		when "p", "i", "u", "b", "sup", "k"
			t = if tag=="k" then "kbd" else tag end
			print "<#{t}>"
			e="</#{t}>"
		when "list"
			$counters << (att.fetch("start"){"1"}).to_i
			print "<ul>"
			e="</ul>"
		when "li"
			print "<li><b>#{$counters.last}</b> : "
			$counters[-1] += 1
			e="</li>"
		when "prose"
			print "<tr><td></td><td></td><td>"
			e="</td></tr>"
		when "jmax_class"
			name = att['name'] or raise
			print "<tr><td colspan='4'>"
			print "<h5><a name='#{name}'></a>#{name}</h5>"
			print "</td></tr>"
			print "<tr><td></td><td valign='top'><br>"
			icon   = contents.find {|x| XNode===x and x.tag == 'icon'   }
			sample = contents.find {|x| XNode===x and x.tag == 'sample' }
			if icon
				print "<img src='#{icon.att['image']}'>"
			end
			print "<br clear='left'><br><br>"
			if sample
				big = sample.att['image']
				small = big.gsub(/png$/, 'jpg').gsub(/\//, '/ic_')
				print "<a href='#{big}'>"
				print "<img src='#{small}' border='0'>"
				print "</a>"
			end
			print "</td><td>"
		when "method"
			print "<br>"
			print "<b>", $portnum.join(" "), "</b> " if $portnum
			print "<b>method</b> #{att['name']} <b>(</b>"
			args = contents.find_all {|x| XNode===x and x.tag == 'arg' }
			print args.map {|x| x.att['name'] }.join "<b>,</b>"
			print "<b>)</b>"
			e="<br><br>"
		when "grid"
			print "<br><b>grid</b> "
			e="<br><br>"
		when "dim"
			print "<b>dim(</b>"
			e="<b>)</b>"
		when "inlet", "outlet"
			# hidden
			$portnum = [tag,att['id']]
		when "icon", "sample", "arg"
			# hidden
			return
		when "grid", "dim"
			print "[#{tag}]"; e="[/#{tag}]"
		when "operator_1", "operator_2", "format"
			$section[:table] ||= (
				print "<tr><td></td><td></td><td>"
				print "<table border='1'>"
			;0)
			print "<tr><td>#{att['name']}</td><td>"
			e="</td></tr>"
		else
			raise "crap in #{tag}"
		end
		contents.each {|x|
			case x
			when String; print x
			when XNode; x.print_contents
			else raise "crap"
			end
		}
		print e if e
		case tag
		when "list"
			$counters.pop
		when "section"
			print "</table>" if $section[:table]
			$section=nil
		when "inlet", "outlet"
			$portnum = nil
		end
	end
end

class JmaxmlParser < XMLParser
	def initialize(*a)
		super
		@jmax_lists = []
		@stack = [[]]
	end

	def startElement(tag,attrs)
		tag=tag.downcase.gsub(/-/,'_')
		if not XNode.valid_tags[tag] then
		 	raise "unknown tag #{tag}"
		end
		@stack<<[tag,attrs]
	end

	def endElement(tag)
		tag=tag.downcase.gsub(/-/,'_')
		node = XNode.new(*@stack.pop)
		@stack.last << node
	end

	def character(text)
		if String===@stack.last.last then
			@stack.last.last << text
		else
			@stack.last << text
		end
	end

	def produce x; puts x; end

	def begin_p   t,a; produce  "<p>"  end; def end_p t;   produce  "</p>"  end
	def begin_b   t,a; produce  "<b>"  end; def end_b t;   produce  "</b>"  end
	def begin_i   t,a; produce  "<i>"  end; def end_i t;   produce  "</i>"  end
	def begin_u   t,a; produce  "<u>"  end; def end_u t;   produce  "</u>"  end
	def begin_k   t,a; produce "<kbd>" end; def end_k t;   produce "</kbd>" end
	def begin_sup t,a; produce "<sup>" end; def end_sup t; produce "</sup>" end

	def begin_list t,a
		raise "start is missing" if not attrs["start"]
		@jmax_lists << attrs["start"]
	end
	def end_list t
		@jmax_lists.pop
	end

	def begin_li t,a
		raise "no list" if not @jmax_lists.last
		produce "#{@jmax_lists.last}. "
	end
	def end_li t
	end

	def begin_section t,a
		raise "can't nest sections" if @jmax_section
		raise "name is missing" if not attrs["name"]
		@jmax_section = attrs["name"]
	end
	def end_section t
		@jmax_section = nil
	end

	def begin_method t,a
	end
	def end_method t
	end

	def begin_arg t,a
	end
	def end_arg t
	end

	def begin_icon t,a
	end
	def end_icon t
	end

end

class DocumentTraceParser < XMLParser
	def initialize(args)
		@indent_out = 0
		super
	end
	def startElement(tag,attrs)
		print "\n", "  "*@indent_out, "(#{tag} #{attrs.inspect}"
		@indent_out += 1
		#super
	end	
	def endElement(tag)
		@indent_out -= 1
		print ")"
		#super
	end
	def character(text)
		text = text.gsub /\s+/, " "
		text.strip!
		return if text==""
		print "\n", "  "*@indent_out, "\"", text[0...40], "\""
		#super
	end
end

#----------------------------------------------------------------#

def header
puts <<EOF
<html>
<head>
<title>video4jmax - reference</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="jmax.css" type="text/css">
</head>
<body
  bgcolor="#FFFFFF"
  leftmargin="0"
  topmargin="0"
  marginwidth="0"
  marginheight="0">
<table
  width="100%"
  bgcolor="white"
  border="0"
  cellspacing="2">
<tr> 
<td colspan="4" bgcolor="#082069">
<img src="images/jmax_titre.jpg" width="253" height="23"></td>
</tr>
<tr> 
<td>&nbsp;</td>
</tr>
EOF
black_ruler
puts <<EOF
<tr><td colspan="4" height="16"> 
    <h4>Video4jmax 0.3.0 - reference index</h4>
</td></tr>
<tr> 
  <td rowspan="2" width="12%">&nbsp;</td>
  <td width="30%" height="23">&nbsp;</td>
  <td width="50%" height="23">&nbsp;</td>
  <td width="10%" height="23">&nbsp;</td>
</tr>
EOF
end

def black_ruler
puts <<EOF
<tr> 
<td colspan="4" bgcolor="black"><img src="images/black.png" width="1" height="2"></td>
</tr>
EOF
end

def section_header(name)
puts %{<tr><td>&nbsp;</<td></tr>}
black_ruler
puts %{<tr><td colspan="4" height="16"> 
	<h4><a name="grid"></a>#{name}</h4>
</td></tr>}
end

def footer
puts <<EOF
</table>
</body>
</html>
EOF
end

#----------------------------------------------------------------#

header
black_ruler

parser = JmaxmlParser.new "ISO-8859-1"

begin
	STDERR.puts "reading standard input..."
	parser.parse(STDIN.readlines.join("\n"), true)
	nodes = parser.instance_eval{@stack}[0][0]
	nodes.print_index
	nodes.print_contents
rescue XMLParserError => e
	puts ""
	puts ""
	STDERR.puts e.inspect

	# strange that line numbers are doubled.
	STDERR.puts "  line: #{parser.line/2 + 1}"
	STDERR.puts "column: #{parser.column}"
	STDERR.puts "  byte: #{parser.byteIndex}"
end

footer
puts ""
puts ""

__END__

<td colspan="4" bgcolor="black"><img src="images/black.png" width="1" height="2"></td>
</tr>
<tr> 
<td colspan="4"> 
<p><font size="-1">Video4jmax 0.2.2 Documentation<br>
by Mathieu Bouchard <a href="mailto:matju@sympatico.ca">matju@sympatico.ca</a> 
and<br>
Alexandre Castonguay <a href="mailto:acastonguay@artengine.ca">acastonguay@artengine.ca</a></font></p>
</td>
</tr>
</table>
</body>
</html>

