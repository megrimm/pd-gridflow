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

GF_VERSION = "0.7.3"

#$use_rexml = true
$use_rexml = false

if $use_rexml
	# this is a pure ruby xml-parser
	#require "gridflow"
	require "rexml/sax2parser"
	include REXML
else
	# this uses libexpat.so
	require "xmlparser"
end

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

class XString < String
	def show
		print escape_html(gsub(/[\r\n\t ]+$/," "))
	end
end

module HasOwnLayout; end

class XNode
	# subclass interface:
	#  #show_index : print as html in index
	#  #show : print as html in main part of the doc

	@valid_tags = {}
	class<<self
		attr_reader :valid_tags
		def register(*args,&b)
			qlass = (if b then Class.new self else self end)
			qlass.class_eval(&b) if b
			for k in args do XNode.valid_tags[k]=qlass end
		end
	end

	def initialize tag, att, *contents
		@tag,@att,@contents =
		 tag, att, contents
		contents.each {|c| c.parent = self if XNode===c }
	end

	attr_reader :tag, :att, :contents
	attr_accessor :parent
	def [] i; contents[i] end

	def show_index
		contents.each {|x| next unless XNode===x; x.show_index }
	end

	# this method segfaults in ruby 1.8
	# because of method lookup on Qundef or whatever.
	def show
		#STDERR.puts GridFlow.get_id(contents)
		#STDERR.puts self
		contents.each {|x|
			# STDERR.puts GridFlow.get_id(x)
			x.show
		}
	end
	def inspect; "#<XNode #{tag}>"; end
	def to_s; inspect; end
end

XNode.register("documentation") {}

XNode.register(*%w( icon help arg rest )) {
	def show; end
}

XNode.register("section") {
	def show
		write_black_ruler
		mk(:tr) { mk(:td,:colspan,4) {
			mk(:a,:name,att["name"].gsub(/ /,'_')) {}
			mk(:h4) { print att["name"] }}}
		
		contents.each {|x|
			if HasOwnLayout===x then
				x.show
			else
				mk(:tr) { mk(:td) {}; mk(:td) {}; mk(:td) { x.show }}
				puts ""
			end
		}

		mk(:tr) { mk(:td) { print "&nbsp;" }}
		puts ""
	end
	def show_index
		mk(:h4) {
			mk(:a,:href,"#"+att["name"].gsub(/ /,'_')) {
				print att["name"] }}
		print "<ul>\n"
		super
		print "</ul>\n"
	end
}


# basic text formatting nodes.
XNode.register(*%w( p i u b k sup )) {
	def show
		t = if tag=="k" then "kbd" else tag end
		print "<#{t}>"
		super
		print "</#{t}>"
	end
}

# explicit hyperlink on the web.
XNode.register("link") {
	def show
		print "<a href='#{att[:to]}'>"
		super
		print att[:to] if contents.length==0
		print "</a>"
	end
}

XNode.register("list") {
	attr_accessor :counter
	def show
		self.counter = att.fetch("start"){"1"}.to_i
		mk(:ul) {
			super # method call on Qundef ???
		}
	end
}

XNode.register("li") {
	def show
		mk(:li) {
			print "<b>#{parent.counter}</b>", " : "
			parent.counter += 1
			super
		}
	end
}

# and "macro", "enum", "type", "use"
XNode.register("class") {
	include HasOwnLayout
	def show
		tag = self.tag
		name = att['name'] or raise
		mk(:tr) {
		  mk(:td,:colspan,4,:bgcolor,"#ffb080") {
		    mk(:b) { print "&nbsp;"*2, "#{tag} " }
		    mk(:a,:name,name) { print name }
		  }
		}
		mk(:tr) {
		mk(:td) {}
		mk(:td,:valign,:top) {
		print "<br>\n"
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
		}#/td
		mk(:td) {
			print "<br>\n"
			super
			print "<br>"
		}#/td
		}#/tr
	end
	def show_index
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
		super
	end
}

def nice_table
	mk(:table,:border,0,:bgcolor,:black,:cellspacing,1) {
		mk(:tr) {
			mk(:td,:valign,:top,:align,:left) {
				mk(:table,:bgcolor,:white,:border,0,
				:cellpadding,4,:cellspacing,1) {
					yield }}}}
end

XNode.register("method") {
	def show
		print "<br>"
		if parent.tag == "inlet" or parent.tag == "outlet"
			mk(:b) {
				print "#{parent.tag}&nbsp;#{parent.att['id']} "
				
			}
		end
		print "<b>method</b>&nbsp;"
#=begin
		print "#{escape_html att['name']} <b>(</b>"
		print contents.map {|x|
			next unless XNode===x
			case x.tag
			when "arg"
				s=escape_html(x.att["name"])
				s="<i>#{x.att['type']}</i> #{s}" if x.att['type']
				s
			when "rest"
				x.att["name"] + "..."
			end
		}.compact.join("<b>, </b>")
		print "<b>)</b> "
#=end
=begin
		nice_table { mk(:tr) {
			mk(:td,:width,1) { print escape_html(att['name']) }
			contents.each {|x|
				next unless XNode===x
				case x.tag
				when "arg"
					mk(:td,:bgcolor,:pink) {
						s = ""
						if x.att["type"]
							s << "<i>" << escape_html(x.att["type"]) << "</i>"
						end
						if x.att["name"]
							s << " " << escape_html(x.att["name"])
						end
						s<<"&nbsp;" if s.length==0
						mk(:b) { puts s }
					}
				when "rest"
					mk(:td,:bgcolor,:pink) {
						mk(:b) { print escape_html(x.att["name"]), "..."}
					}
				end
			}
		}}
		print "<br>"
=end
		super
		print "<br>\n"
	end
}

XNode.register("table") {
	def show
		mk(:tr) {
		2.times { mk(:td) {} }
		mk(:td) {
		nice_table {
			mk(:tr) {
				columns = contents.find_all {|x| XNode===x && x.tag=="column" }
				columns.each {|x| mk(:td) { mk(:b) {
					x.contents.each {|y| y.show }}}}
			}
			super
		}}}
	end
}

XNode.register("column") {
	def show; end
}

XNode.register("row") {
	def show
		columns = parent.contents.find_all {|x| XNode===x && x.tag=="column" }
		mk(:tr) { columns.each {|x| mk(:td) {
			id = x.att["id"]
			if id==""
				then contents.each {|x| x.show }
				else print att[id] end
		}}}
	end
}

XNode.register("operator-1", "operator-2") {
	def show
		mk(:tr) {
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
		mk(:td) { super }
		mk(:td) { print "#{(escape_html att['color'])||'&nbsp;'}" }
		mk(:td) { print "#{(escape_html att['space'])||'&nbsp;'}" }
		}#/tr
		puts ""
	end
}

XNode.register("inlet","outlet") {}

#----------------------------------------------------------------#

if $use_rexml
	class GFDocParser
		def initialize(file)
			@sax = SAX2Parser.new(File.open(file))
			@xml_lists = []
			@stack = [[]]
			@sax.listen(:start_element) {|a,b,c,d| startElement(b,d) }
			@sax.listen(  :end_element) {|a,b,c|   endElement(b) }
			@sax.listen(   :characters) {|a|       characters(a) }
		end
		def do_it; @sax.parse; end
	end
else
	class GFDocParser
		def initialize(file)
			@xml = XMLParser.new("ISO-8859-1")
			foo=self; @xml.instance_eval { @gfdoc=foo }
			def @xml.startElement(tag,attrs) @gfdoc.startElement(tag,attrs) end
			def @xml.endElement(tag) @gfdoc.endElement(tag) end
			def @xml.character(text) @gfdoc.character(text) end
			@file = File.open file
			@xml_lists = []
			@stack = [[]]
		end
		def do_it; @xml.parse(@file.readlines.join("\n"), true) end
	end
end

class GFDocParser
	attr_reader :stack
	def startElement(tag,attrs)
		if not XNode.valid_tags[tag] then
			raise XMLParserError, "unknown tag #{tag}"
		end
		@stack<<[tag,attrs]
	end
	def endElement(tag)
		node = XNode.valid_tags[tag].new(*@stack.pop)
		@stack.last << node
	end
	def character(text)
		if not String===@stack.last.last then
			@stack.last << XString.new("")
		end
		@stack.last.last << text
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
  <td width="15%" height="23">&nbsp;</td>
  <td width="80%" height="23">&nbsp;</td>
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
XMLParserError = Exception if $use_rexml

def read_one_page file
	begin
		STDERR.puts "reading #{file}"
		parser = GFDocParser.new(file)
		parser.do_it
		$nodes[file] = parser.stack[0][0]
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
	tree = $nodes[file]
	mk(:tr) { mk(:td,:colspan,2) { mk(:div,:cols,tree.att["indexcols"]||1) {
		tree.show_index
		puts "<br><br>"
	}}}
	tree.show
	write_footer
	puts ""
	puts ""
end

$files = %w(
	install.xml project_policy.xml
	reference.xml format.xml internals.xml architecture.xml)

$files.each {|input_name| read_one_page input_name }
$files.each {|input_name| write_one_page input_name }
