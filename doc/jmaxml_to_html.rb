=begin
	$Id$
	convert jMax-XML to HTML with our formatting.

	GridFlow
	Copyright (c) 2001 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
=end

require "xmlparser"

=begin todo

	[ ] make it use the mk() function as much as possible.
	[ ] make it validate
	[ ] make it find the size of the pictures (and insert width/height attrs)
	[ ] tune the output

=end

if nil
	alias real_print print
	alias real_puts  puts
	def print(*x); real_print "[#{caller[0]}]"; real_print *x; end
	def puts (*x); real_print "[#{caller[0]}]"; real_puts  *x; end
end

def mk(tag,*values,&block)
	raise "value-list's length must be even" if values.length % 2 != 0
	print "<#{tag}"
	i=0
	while i<values.length
		print " #{values[i]}=\"#{values[i+1]}\""
		i+=2
	end
	print ">"
	(block[]; mke tag) if block
end
def mke(tag)
	print "</#{tag}>"
end

def mkimg(icon,alt=nil)
	icon.tag == 'icon' or raise "need icon"
	url = icon.att["image"]
	url = "images/" + url if url !~ /^images\//
	url += ".png" if url !~ /\.(png|jpe?g|gif)$/
	raise "#{url} not found" if not File.exist? url
	mk(:img, :src, url,
		:alt, alt || icon.att["text"],
		:border, 0)
end

class DTDParser < XMLParser
	def elementDecl(*args); print "elementDecl: "; p args; end
	def attlistDecl(*args); print "attlistDecl: "; p args; end
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
		"help" => 1,
		"arg" => 1,
		"rest" => 1,
		"method" => 1,
		"dim" => 1,
		"grid" => 1,
		"inlet" => 1,
		"outlet" => 1,
		"jmax_class" => 1,
		"operator_1" => 1,
		"operator_2" => 1,
		"jmax_doc" => 1,
		"format" => 1,
		"prose" => 1,
		"part" => 1,
	}
	def initialize tag, att, *contents
		@tag,@att,@contents =
		 tag, att, contents
	end
	attr_reader :tag, :att, :contents
	def [] i; contents[i] end

	def print_index
		e=nil
		case tag
		when "section"
			mk(:h4) { mk(:a,:href,"#"+att["name"].gsub(/ /,'_')) {
				print att["name"] }}
			print "<ul>\n"
			e="</ul>\n"
		when "jmax_class"
			icon = contents.find {|x| XNode===x && x.tag == "icon" }
			mk(:li) { mk(:a,:href,"\#"+att["name"]) {
				if icon
					icon.att['image'] ||= att['name']
					mkimg(icon,att["cname"])
				else
					print att["name"]
				end
			}}
			puts
		end

		contents.each {|x|
			next unless XNode===x
			x.print_index
		}

		print e if e
	end

	$counters=[]
	$sections=nil
	def print_contents
		e=nil
		case tag
		when "jmax_doc"; #nothing
		when "section"
			black_ruler
			mk(:tr) { mk(:td,:colspan,4) {
				mk(:a,:name,att["name"].gsub(/ /,'_')) {}
				mk(:h4) { print att["name"] }}}
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
			print "<tr>"
			mk(:td) {}
			mk(:td) {}
			print "<td>"
			e="</td></tr>"
		when "jmax_class"
			name = att['name'] or raise
			mk(:tr) {
			  mk(:td,:colspan,4,:bgcolor,"#ffb080") {
			    mk(:b) { print "&nbsp;"*2, "class " }
			    mk(:a,:name,name) { print name }
			  }
			}
			print "<tr>"
			mk(:td) {}
			print "<td valign='top'><br>\n"
			icon = contents.find {|x| XNode===x and x.tag == 'icon' }
			help = contents.find {|x| XNode===x and x.tag == 'help' }
			mkimg(icon) if icon
			mk(:br,:clear,"left")
			mk(:br)
			mk(:br)
			if help
				big = help.att['image']
				if big[0]==?@ then big="images/help_#{big}.png" end
				raise "#{big} not found" if not File.exist?(big)
				#small = big.gsub(/png$/, 'jpg').gsub(/\//, '/ic_')
				mk(:a,:href,big) {
					#mk(:img,:src,small,:border,0)
					mk(:img,:src,"images/see_screenshot.png",:border,0)
				}
			end
			mk(:br,:clear,"left")
			mk(:br)
			print "</td><td><br>\n"
			e="<br></td>"
		when "method"
			print "<br>"
			print "<b>", $portnum.join(" "), "</b> " if $portnum
			print "<b>method</b> #{att['name']} <b>(</b>"
			print contents.map {|x|
				next unless XNode===x
				case x.tag
				when "arg"
					x.att["name"]
				when "rest"
					x.att["name"] + "..."
				end
			}.compact.join "<b>,</b>"
			print "<b>)</b>"
			e="<br>\n"
		when "grid"
			print "<br>"
			print "<b>", $portnum.join(" "), "</b> " if $portnum
			print "<b>grid</b> "
			e="<br>\n"
		when "dim"
			print "<b>dim(</b>"
			e="<b>)</b>"
		when "inlet", "outlet"
			# hidden
			$portnum = [tag,att['id']]
		when "icon", "help", "arg", "rest"
			# hidden
			return
		when "grid", "dim"
			print "[#{tag}]"; e="[/#{tag}]"
		when "part"
			$section[:table] ||= (
				print "<tr>"
				2.times { mk(:td) {}}
				print "<td>"
				print "<table border='1'>"
			;0)
			print "<tr>"
			mk(:td) { print att["name"]  }
			mk(:td) { print att["who"]   }
			mk(:td) { print att["files"] }
			print "<td>"
			e="</td></tr>"
		
		when "operator_1", "operator_2"
			$section[:table] ||= (
				print "<tr>"
				2.times { mk(:td) {}}
				print "<td>"
				print "<table border='1'>"
			;0)
			print "<tr>"
			mk(:td) {
				icon = contents.find {|x| XNode===x && x.tag == "icon" }
				if icon then
					mkimg icon
				else
					mk(:img,:src,"images/op/#{att['cname']}.jpg",
						:border,0,:alt,att["cname"])
				end
				# print att["name"]
			}
			print "<td>"
			e="</td></tr>"
		else
			raise "crap in #{tag}"
		end
		contents.each {|x|
			case x
			when String
				x.gsub!(/[\r\n\t ]+$/," ")
				print x
			when XNode
				x.print_contents
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

	def produce x; print x; end

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
<!-- #{"$"}Id#{"$"} -->
<title>GridFlow 0.4.1 - reference</title>
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
<img src="images/titre_gridflow.png" width="253" height="23"></td>
</tr>
<tr> 
<td>&nbsp;</td>
</tr>
EOF
black_ruler
puts <<EOF
<tr><td colspan="4" height="16"> 
    <h4>GridFlow 0.4.1 - reference index</h4>
</td></tr>
<tr> 
  <td width="5%" rowspan="2">&nbsp;</td>
  <td width="25%" height="23">&nbsp;</td>
  <td width="66%" height="23">&nbsp;</td>
  <td width="5%"  height="23">&nbsp;</td>
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
<td colspan="4" bgcolor="black"><img src="images/black.png" width="1" height="2"></td>
</tr>
<tr> 
<td colspan="4"> 
<p><font size="-1">GridFlow 0.4.1 Documentation<br>
by Mathieu Bouchard <a href="mailto:matju@sympatico.ca">matju@sympatico.ca</a> 
and<br>
Alexandre Castonguay <a href="mailto:acastonguay@artengine.ca">acastonguay@artengine.ca</a></font></p>
</td>
</tr>
</table>
</body>
</html>
EOF
end

#----------------------------------------------------------------#

header

parser = JmaxmlParser.new "ISO-8859-1"

begin
	STDERR.puts "reading standard input..."
	parser.parse(STDIN.readlines.join("\n"), true)
	nodes = parser.instance_eval{@stack}[0][0]
	mk(:tr) { mk(:td,:colspan,2) {
		nodes.print_index
		puts "<br><br>"
	}}
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

