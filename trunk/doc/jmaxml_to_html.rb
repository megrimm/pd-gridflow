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

class JmaxmlParser < XMLParser
	def initialize(args)
		super
		@indent_out = 0
	end
	def startElement(tag,attrs)
		print "\n", "  "*@indent_out, "(#{tag} #{attrs.inspect}"
		@indent_out += 1
	end	
	def endElement(tag)
		@indent_out -= 1
		print ")"
	end
	def character(text)
		text = text.gsub /\s+/, " "
		text.strip!
		return if text==""
		print "\n", "  "*@indent_out, "\"", text[0...40], "\""
	end
end


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
<img src="images/jmax_titre.jpg" width="253" height="23">
</td>
</tr>
<tr> 
<td>&nbsp;</td>
</tr>
EOF
black_ruler
puts <<EOF
<tr> 
<td colspan="4" height="16"> 
<h4>Video4jmax 0.2.2 - reference index</h4>
</td>
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

#header
#black_ruler
#footer

parser = JmaxmlParser.new "ISO-8859-1"
#STDIN.each {|line|
#	parser.parse line
#}
#parser.parse "", true
begin
	parser.parse(STDIN.readlines.join("\n"), true)
rescue XMLParserError => e
	puts ""
	puts ""
	STDERR.puts e.inspect

	# strange that line numbers are doubled.
	STDERR.puts "  line: #{parser.line/2 + 1}"
	STDERR.puts "column: #{parser.column}"
	STDERR.puts "  byte: #{parser.byteIndex}"
end
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

