=begin
	$Id$
	convert GridFlow Documentation XML to HTML with special formatting.

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

GF_VERSION = "0.7.2"

require "xmlparser"

=begin todo

	[ ] make it use the mk() function as much as possible.
	[ ] make it validate
	[ ] make it find the size of the pictures (and insert width/height attrs)
	[ ] tune the output
	[ ] fix the header of the page

=end

if nil
	alias real_print print
	alias real_puts  puts
	def print(*x); real_print "[#{caller[0]}]"; real_print *x; end
	def puts (*x); real_print "[#{caller[0]}]"; real_puts  *x; end
end

def warn(text)
	STDERR.print "\e[1;031mWARNING:\e[0m "
	STDERR.puts text
end

$escape_map={
	"<" => "&lt;",
	">" => "&gt;",
	"&" => "&amp;",
}

def escape_html(text)
	return nil if not text
	text.gsub(/[<>&]/) {|x| $escape_map[x] }
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
	url = icon.att["image"] || icon.parent.att["name"]
	url = url.sub(/,.*$/,"")
	raise icon.att.to_s if not url
	url = "images/" + url if url !~ /^images\//
	url += ".png" if url !~ /\.(png|jpe?g|gif)$/
	raise "#{url} not found" if not File.exist? url
	mk(:img, :src, url,
		:alt, alt || icon.att["text"],
		:border, 0)
end

class XNode
	# subclass interface:
	#  #prc : called by #print_contents of base class
	#  #prx : called by #print_index of base class
	#  #print_contents : private, don't touch
	#  #print_index : private, don't touch

	class<<self; attr_accessor :valid_tags; end
	self.valid_tags = {}
	def self.register(*args)
		for k in args do XNode.valid_tags[k]=self end
	end

	def initialize tag, att, *contents
		@tag,@att,@contents =
		 tag, att, contents
		contents.each {|c| c.parent = self if XNode===c }
	end
	attr_reader :tag, :att, :contents
	attr_accessor :parent
	def [] i; contents[i] end

	$counters=[]
	$sections=nil
	def print_index
		pr = begin prx; rescue NameError=>x; end
		contents.each {|x| next unless XNode===x; x.print_index }
		pr[] if Proc===pr
	end

	def print_contents
		pr = send("prc")
		contents.each {|x|
			case x
			when String; x.gsub!(/[\r\n\t ]+$/," "); print escape_html(x)
			when XNode; x.print_contents
			else raise "crap"
			end
		}
		pr[] if Proc===pr
	end
end

class XHidden < XNode
	def prc; end
	register *%w( icon help arg rest documentation )
end

class XSection < XNode
	def prc
		write_black_ruler
		mk(:tr) { mk(:td,:colspan,4) {
			mk(:a,:name,att["name"].gsub(/ /,'_')) {}
			mk(:h4) { print att["name"] }}}
		$section={}
		proc {
			print "<tr><td>&nbsp;</td></tr>\n"
			print "</table>" if $section[:table]
			$section=nil
		}
	end
	def prx
		mk(:h4) {
			mk(:a,:href,"#"+att["name"].gsub(/ /,'_')) {
				print att["name"] }}
		print "<ul>\n"
		proc { print "</ul>\n" }
	end

	register "section"
end

class XProse < XNode
	def prc
		print "<tr>"
		mk(:td) {}
		mk(:td) {}
		print "<td>"
		proc { print "</td></tr>\n" }
	end
	register "prose"
end

# basic text formatting nodes.
class XTNode < XNode
	def prc
		t = if tag=="k" then "kbd" else tag end
		print "<#{t}>"
		proc { print "</#{t}>" }
	end
	register *%w( p i u b k sup )
end

# explicit hyperlink on the web.
class XLink < XNode
	def prc
		print "<a href='#{att[:to]}'>"
		proc {
			print att[:to] if contents.length==0
			print "</a>"
		}
	end
	register "link"
end

class XList < XNode
	def prc
		$counters << (att.fetch("start"){"1"}).to_i
		print "<ul>"
		proc { print "</ul>"; $counters.pop }
	end
	register "list"
end

class XLI < XNode
	def prc
		print "<li><b>#{$counters.last}</b> : "
		$counters[-1] += 1
		proc { print "</li>" }
	end
	register "li"
end

class XClass < XNode
	def prc
		tag = self.tag
		name = att['name'] or raise
		mk(:tr) {
		  mk(:td,:colspan,4,:bgcolor,"#ffb080") {
		    mk(:b) { print "&nbsp;"*2, "#{tag} " }
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
		2.times { mk(:br) }
		if help
			big = help.att['image'] || att['name']
			if big[0]==?@ then big="images/help_#{big}.png" end
			warn "#{big} not found" if not File.exist?(big)
			#small = big.gsub(/png$/, 'jpg').gsub(/\//, '/ic_')
			mk(:a,:href,big) {
				#mk(:img,:src,small,:border,0)
				mk(:img,:src,"images/see_screenshot.png",:border,0)
			}
		end
		mk(:br,:clear,"left")
		mk(:br)
		print "</td><td><br>\n"
		proc { print "<br></td>" }
	end
	def prx
		icon = contents.find {|x| XNode===x && x.tag == "icon" }
		if not att["name"] then
			raise "name tag missing?"
		end
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
	register "class" #, "macro", "enum", "type", "use"
end		

class XMethod < XNode
	def prc
		print "<br>"
		print "<b>", $portnum.join(" "), "</b> " if $portnum
		print "<b>method</b> #{att['name']} <b>(</b>"
		print contents.map {|x|
			next unless XNode===x
			case x.tag
			when "arg"
				s=x.att["name"]
				s="<i>#{x.att['type']}</i> #{s}" if x.att['type']
				s
			when "rest"
				x.att["name"] + "..."
			end
		}.compact.join("<b>, </b>")
		print "<b>)</b> "
		proc { print "<br>\n" }
	end
	register "method"
end

class XOperator < XNode
	def prc
		$section[:table] ||= (
			print "<tr>"
			2.times { mk(:td) {}}
			print "<td>"
			print "<table border='1'>"
			mk(:th) {
				print(["name","description",
					"meaning in pixel context (pictures, palettes)",
					"meaning in spatial context (indexmaps, polygons)"]\
				.map{|x|
					"<td>#{x}</td>" })
			}
		;0)
		print "<tr>"
		mk(:td) {
			icon = contents.find {|x| XNode===x && x.tag == "icon" }
			if icon then
				mkimg icon
			else
				x = "images/op/#{att['cname']}.jpg"
				x.sub! /jpg$/, "png" if not File.exist? x
				if not File.exist? x
					STDERR.print "warning: no icon for #{att['name']} (#{x})\n"
				end
				mk(:img,:src,x,:border,0,:alt,att["name"])
			end
			# print att["name"]
		}
		print "<td>"
		proc {
			print "</td>"
			print "<td>#{(escape_html att['color'])||'&nbsp;'}</td>"
			print "<td>#{(escape_html att['space'])||'&nbsp;'}</td>"
			print "</tr>\n"
		}
	end
	register "operator-1", "operator-2"
end

class XPort < XNode
	def prc
		# hidden
		$portnum = [tag,att['id']]
		proc { $portnum = nil }
	end
	register "inlet", "outlet"
end

#----------------------------------------------------------------#

class GFDocParser < XMLParser
	attr_reader :stack

	def initialize(*a)
		super
		@xml_lists = []
		@stack = [[]]
	end

	def startElement(tag,attrs)
		if not XNode.valid_tags[tag] then
		 	raise "unknown tag #{tag}"
		end
		@stack<<[tag,attrs]
	end

	def endElement(tag)
		node = XNode.valid_tags[tag].new(*@stack.pop)
		@stack.last << node
	end

	def character(text)
		if String===@stack.last.last then
			@stack.last.last << text
		else
			@stack.last << text
		end
	end
end

#----------------------------------------------------------------#

def write_header
puts <<EOF
<html><head>
<!-- #{"$"}Id#{"$"} -->
<title>GridFlow #{GF_VERSION} - reference</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="jmax.css" type="text/css">
</head>
<body bgcolor="#FFFFFF"
  leftmargin="0" topmargin="0"
  marginwidth="0" marginheight="0">
<table width="100%" bgcolor="white" border="0" cellspacing="2">
<tr><td colspan="4" bgcolor="#082069">
<img src="images/titre_gridflow.png" width="253" height="23">
</td></tr><tr><td>&nbsp;</td></tr>
EOF
write_black_ruler
puts <<EOF
<tr><td colspan="4" height="16"> 
    <h4>GridFlow #{GF_VERSION} - index of this page:</h4>
</td></tr>
<tr> 
  <td width="5%"  rowspan="2">&nbsp;</td>
  <td width="25%" height="23">&nbsp;</td>
  <td width="66%" height="23">&nbsp;</td>
  <td width="5%"  height="23">&nbsp;</td>
</tr>
EOF
end

def write_black_ruler
puts <<EOF
<tr><td colspan="4" bgcolor="black">
<img src="images/black.png" width="1" height="2"></td></tr>
EOF
end

def write_footer
puts <<EOF
<td colspan="4" bgcolor="black">
<img src="images/black.png" width="1" height="2"></td></tr>
<tr><td colspan="4"> 
<p><font size="-1">
GridFlow #{GF_VERSION} Documentation<br>
by Mathieu Bouchard
<a href="mailto:matju@sympatico.ca">matju@artengine.ca</a>
and<br>
Alexandre Castonguay
<a href="mailto:acastonguay@artengine.ca">acastonguay@artengine.ca</a>
</font></p>
</td></tr></table></body></html>
EOF
end

#----------------------------------------------------------------#

$nodes = {}

def read_one_page file
	begin
		STDERR.puts "reading #{file}"
		data = File.open(file) {|f| f.readlines }.join("\n")
		parser = GFDocParser.new "ISO-8859-1"
		parser.parse(data, true)
		$nodes[file] = parser.instance_eval{@stack}[0][0]
	rescue XMLParserError => e
		puts ""
		puts ""
		STDERR.puts e.inspect

		i = parser.stack.length-1
		(STDERR.puts "\tinside <#{parser.stack[i][0]}>"; i-=1) until i<1

		# strange that line numbers are doubled.
		# also the byte count is offset by the line count !?!?!?
		STDERR.puts "\tinside #{file}:#{parser.line/2 + 1}" +
			" (column #{parser.column}," +
			" byte #{parser.byteIndex - parser.line/2})"

		raise "why don't you fix the documentation"
	end
end

def write_one_page file
	output_name = file.sub(/\.xml/,".html")
	STDERR.puts "writing #{output_name}"
	STDOUT.reopen output_name, "w"
	write_header
	mk(:tr) { mk(:td,:colspan,2) {
		$nodes[file].print_index
		puts "<br><br>"
	}}
	$nodes[file].print_contents
	write_footer
	puts ""
	puts ""
end

$files = %w(
	install.xml
	project_policy.xml
	reference.xml
	format.xml
	internals.xml
	architecture.xml
)

$files.each {|input_name| read_one_page input_name }
$files.each {|input_name| write_one_page input_name }
