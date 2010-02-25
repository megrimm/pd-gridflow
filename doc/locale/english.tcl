# this file will not contain any aliases, just the canonical names

proc say2 {k v} {
	say [lindex $k 0] $v
	foreach kk [lrange $k 1 end] {say $k "old name of $k : $v"}
}

proc say3 {k v} {
	set kk [regsub ^# $k @]
	say2 [list k kk] $v
}

category "Stuff"
say #                "plain numeric operators on grids: + - * / etc"
say #border          "add padding on sides of a grid"
say #cast            "convert grid from one number type to another"
say #centroid        "find centroid (weighted average) of the coordinates of a grid"
say #convolve        "compute convolution product of a grid (blur and edge-detection)"
say #dim             "get the size (dimensions) of a grid"
say #downscale_by    "reduce the size of an image by a whole factor"
say #draw_image      "picture-in-picture"
say #draw_points     "(future use)"

category "Data Conversion"
say #to_literal       "convert grid to grid-literal (list with a # sign)"
say #to_float        "convert grid to sequence of floats"
say #to_list         "convert grid to list"
say #to_pix          "convert grid to pix (GEM)"
say #to_symbol       "convert grid of ASCII codes to symbol"

category "Stuff"
say #fft             "compute forward or inverse one-or-two-dimensional complex FFT of a grid"
say #finished        "bang when grid transmission has ended"
say #fold            "compute the sum of each row, product of each row, and other similar operations"
say #for             "make a grid from all numbers or coordinates in a certain range"
say #from_pix        "convert pix (GEM) to grid"
say #import_pix      "old name of #from_pix"
say #grade           "make an index of the sorting of a grid"
say #import          "convert float, list, or symbol ASCII codes, to a grid"
say #inner           "scalar (\"dot\") product, matrix product, tensor contraction, image channel remapping, etc."

category "Hardware and Files"
say #camera          "control panel for opening cameras"
say #io.aalib        "open an aalib window"
say #io.dc1394       "open a faiawaia \"DC\" camera (not DV) using a Linux-compatible OS"
say #io.grid         "read or write a .grid file (GridFlow storage format)"
say #io.jpeg         "read or write a .jpeg file"
say #io.mpeg         "read a .mpeg video file"
say #io.png          "read a .png image file"
say #io.ppm          "read or write a .pbm or .pgm or .ppm image file"
say #io.quicktime    "read a .mov video file (or perhaps .avi)"
say #io.sdl          "open a SDL window"
say #io.videodev     "open a V4L1 device (Linux interface for video cameras and video digitisers)"
say #io.x11          "open a X11 window"
say parallel_port    "send to and receive from a DB25 parallel port"
say plotter_control  "make HPGL commands"

category "Stuff"
say #labelling       "tag connected pixels with region numbers in a two-tone single-channel image"
say #layer           "layer two same-sized images"
say #moment          "find 1st or 2nd order moment (weighted average or variance) of the coordinates of a grid"
say #noise_gate_yuvs "replaces dark pixels by black pixels in signed YUV images"
say #outer           "apply numeric operator on all possible combinations of elements of one grid with elements of another"
say #pack            "combine floats on separate inlets to make a grid"
say #perspective     "divide each point by its depth"
say #print           "print to console"
say #redim           "change the size of a grid by restreaming contents into a new shape"
say #reverse         "mirror image of a grid along a dimension"
say #rotatificator   "make rotation matrix for any two dimensions chosen from a space of any number of dimensions"
say #scale_by        "reduce the size of an image by a whole factor"
say #scan            "compute the cumulative sums of each row, and other similar operations"
say #store           "store image in RAM, in-place picture-in-picture, and make lookups in various ways"
say #transpose       "swap two dimensions out of any, in a grid"
say #type            "get a symbol representing the number type of a grid"
say #unpack          "split a grid into floats on separate outlets"
say #join            "join two grids together along any dimension"

category "Lists"
say foreach          "convert a list to a sequence of atom messages"
say listfind         "find index of an element in a list"
say listread         "find element at an index in a list"
say listflatten      "merge all nested lists together, depth-first"
say listreverse      "mirror image of a list"

category "Stuff"
say args             "pick up the arguments of an abstraction instance, including nested lists and init-messages"
say ascii            "write integer as decimal in ascii codes"
say class_exists     "figure out whether a class has been loaded by pd"
say display          "\[display\]: print message or grid inside of the patch"
say gf.error         "emit error message from the perspective of the current abstraction instance in its parent patch"
say gf.suffix_lookup "find the objectclass corresponding "
say range            "multiple moses in cascade"
say receives         "multiple receives with common outlet and other outlet telling the name of intended destination"
say route2           "route messages but keep them intact (does not remove selector)"
say shunt            "demultiplexer: send input to separately specified outlet"
say systemtime       "time spent by process in kernel mode, as measured by the OS"
say tsctime          "high-resolution real time, as measured by the CPU"
say unix_time        "real time as measured by the OS, including date"
say usertime         "time spent by process in non-kernel mode, as measured by the OS"

category "Stuff"
say #apply_colormap_channelwise "apply color correction tables separately on each channel"
say #background_model "make mask from learning to distinguish background from foreground"
say #change           "send grid only if different from previous grid"
say #checkers         "make image of chequered background in two tones of grey"
say #clip             "min and max"
say #color            "GUI for selecting a colour"
say #contrast         "adjust contrast in two different ways"
say #fade_lin         "fade in piecewise-linear fashion"
say #fade             "fade in exponential-decay fashion (linear recurrence)"
say #fastblur         "speedy shortcut for rectangular blur"
say for               "make sequence of float messages for each number in a range with specified stepping"
say fps               "measure frames per second and make statistics"
say #gamma            "apply gamma correction"
say gf.io_generate    "for internal use by #in and #out"
say #hueshift         "apply hue shift by rotating the color wheel"

say #in               "open file or device for reading or download"
say #out              "open file or device for writing or upload"

say inv*              "swapped /"
say inv+              "swapped -"

say #moment_polar     "convert covariance matrix to the longest and shortest radius of an ellipse and a rotation angle"
say #motion_detection "frame difference with some options"
say #mouse            "converts mouse events to reports of clicks, drags, unclicks, motions, and separate buttons and wheel"
say gf.oneshot        "spigot that shuts itself down after each message"
say pingpong          "turns value of a counter into a zigzag between 0 and a given value"

category "Polygons"
say #polygon_area       "find area of a polygon in square pixels"
say #polygon_comparator "find similarity between two polygons independently of rotation, by radial maps and FFT"
say #polygon_each_edge  "convert a polygon to a sequence of overlapping 2-sided polygons representing edges"
say #polygon_moment     "find average of all points inside of a polygon"
say #polygon_perimetre  "find perimetre of a polygon in pixels (euclidian)"
say #polygon_radial_map "find radius of a polygon from a given origin, sampled at equally spaced angles"
say #line_to_polygon  "convert line (as point pair) to polygon (rotated rectangle)"
say #draw_rect        "draw a rectangle in an image"
say #draw_slider      "draw a slider in an image"
say #edit_polygon     "draw a polygon in an image and react to mouse events"
say #make_cross       "make cross-shaped polygon"
say #draw_polygon    "draw polygon in an image"

category "Stuff"
say #posterize        "quantise pixel values"
say #ravel            "do #redim so that a grid keeps same number of elements but just one dimension"
say #record           "wrapper around \[#in quicktime\]"
say #remap_image      "apply object chain on pixel positions to make new image from chosen pixels of the input image"
say #rotate           "rotate points through two axes (or rotate pixels as points in a colorspace)"
say #saturation       "multiply chroma by some value"
say #scale_to         "scale an image from one size to any other size by deleting or duplicating rows and columns"
say #seq_fold         "cascade the use of an object on all elements of an incoming grid message sequence"
say seq_fold          "cascade the use of an object on all elements of an incoming message sequence"
say #slice            "crop an image using a start point (top left) and end point (bottom right)"
say #solarize         "like pingpong but on all pixel values of a grid"
say #sort             "sort each row of a grid"
say #spread           "add noise to each vector (point or pixel)"
say #swap             "like \[swap\] for grids"
say #text_to_image    "use a fixed-width font grid to make a tiling of characters as specified by a text string"
say #t                "like \[t a a\] for grids"
say var.#             "like \[f\] for grids"

category "Colorspace Conversion"
say #rgb_to_yuv       "convert RGB to unsigned YUV"
say #yuv_to_rgb       "convert unsigned YUV to RGB"
say #rgb_to_rgba      "convert RGB to RGBA (but setting alpha to 0)"
say #rgba_to_rgb      "convert RGBA to RGB"
say #greyscale_to_rgb "convert greyscale to RGB"
say #rgb_to_greyscale "convert RGB to greyscale"

category "OpenCV"
say cv/#Add           "OpenCV addition"
say cv/#Div           "OpenCV division"
say cv/#Mul           "OpenCV multiplication"
say cv/#Sub           "OpenCV subtraction"
say cv/#And           "OpenCV bitwise AND"
say cv/#Or            "OpenCV bitwise OR"
say cv/#Xor           "OpenCV bitwise XOR"
say cv/#Invert        "OpenCV invert matrix"
say cv/#SVD           "OpenCV singular value decomposition (eigendecomposition)"
say cv/#Ellipse       "OpenCV draw ellipse"
say cv/#KMeans        "OpenCV K-Means clusteriser"
say cv/#HaarDetectObjects "OpenCV (future use)"
say cv/#Kalman        "OpenCV (future use)"
say cv/#ApproxPoly    ""
say cv/#CornerHarris  ""

category "Stuff"
say ascii_to_f       "converts a list of ascii codes to a float"
say plotter_parser   "interprets ascii codes as HPGL commands and output them as messages"
say list.==          "test two lists of floats and/or symbols for equality"
say expect           "currently does like list.== (will do more than that in the future)"
say #hello           "make 7 colour bars"
say #window          "a \[#out window\] that can be toggled to appear and disappear"

say #cluster_avg "computes several averages at once according to a table of indices"
say #draw_hpgl "plot lines given in HPGL format, using \[#draw_polygon\]"
say #extract_diagonal "extract the main diagonal from a matrix"
say #reinterval "change the scale of values from one interval to another"
say hpgl_find_bbox "find the bounding box of a sequence of hpgl commands"
say hpgl_font_render "include hpgl files from disk for drawing text"
say hpgl_op "use \[#\] on hpgl commands"

say hpgl_track_position "remember last mentioned hpgl coordinate"
say interval_overlap    "test whether two intervals of floats overlap"
say norecurse           "simple spigot for preventing control-recursion"
say #make_arrow         "make an arrow polygon from 2 points (for use with hpgl)"
say #many               "create and organise many identical GUI objects"
say #see                "\[#see\]: view video output within patch and collect mouse/key info"
say #to_iem             "convert grid(3) to IEMGUI colour code"
say qwerty_piano        "imitation of \[notein\] using \[key\] and \[keyup\]"
say doremi              "gui object for displaying midi notes"
say gf/mouse_spy        "tell mouse/key events of a single patch window in \[#out window\] format"

# never finished (2007)
if 0 {
  say cv/#CalcOpticalFlowBM
  say cv/#CalcOpticalFlowHS
  say cv/#CalcOpticalFlowLK
  say cv/#CalcOpticalFlowPyrLK
}

# doc: many of them can't even be listed normally in the doc index
if 0 {
say doc_add
say doc_also
say doc_below
say doc_bottom
say doc_c
say doc_cc
say doc_editmode
say doc_exist
say doc_f
say doc_h
say doc_i
say doc_ii
say doc_layout
say doc_m
say doc_make
say doc_o
say doc_oo
say doc_same
}

# experimental undocumented

if 0 {
say gf.display
say gf.nbxhsl
say gf.not_open
say gf.print
say gf/canvas_count
say gf/canvas_dollarzero
say gf/canvas_edit_mode
say gf/canvas_filename
say gf/canvas_getpos
say gf/canvas_hehehe
say gf/canvas_hohoho
say gf/canvas_is_selected
say gf/canvas_loadbang
say gf/canvas_setgop
say gf/canvas_setpos
say gf/canvas_xid
say gf/getpid
say gf/lol
say gf/string_<
say gf/string_replace
say memstat
say setargs
}
