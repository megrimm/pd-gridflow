#!/usr/bin/env python
#******************************************************************************
# rblti, Copyright 2005 by Mathieu Bouchard and Heri Andria
# pylti, Copyright 2005 by Michael Neuroth
# a wrapper for ltilib using SWIG
#******************************************************************************
#
# Tool to generate header files for SWIG to process the nested classes "parameters"
#
# 1) generate XML files for the ltilib with doxygen (tested with version 1.4.3) (use this switch in doc.cfg file: GENERATE_XML = YES)
# 2) generate the new header files for SWIG with this python script

from xml.dom import minidom
import os
from distutils.text_file import TextFile

str_version = '0.33.1'


# handle the settings: WORKAREA has to be defined !
basedir = None

workarea = os.getenv("WORKAREA")

# is pylti not part of the ltilib ? Yes --> than we need a directory structure of WORKAREA_PATH/import/ltilib/xml
if workarea<>None:
    basedir = workarea+os.sep+'import'+os.sep+'ltilib'+os.sep+'xml'+os.sep
    
# check if we are in misc directory of the ltilib distribution (ltilib/misc/pylti)
# very simple way: navigate up and navigate down again

if workarea==None:    
    strCheckPath = '..'+os.sep+'..'+os.sep+'misc'+os.sep+'pylti'+'-'+str_version
    try:
        # try to read lti.i, this is a good indication of a pylti directory
        aTestFile = TextFile(filename=strCheckPath+os.sep+'swig'+os.sep+'lti.i')
        basedir = '..'+os.sep+'..'+os.sep+'xml'+os.sep        # move up ltipy and misc directory 
    except IOError :
        pass


output_dir = 'generated'

def f(x): return 'classlti_1_1'+x+'.xml'
#def g(x): return 'lti::_'+x+'::_'+x+'_parameters'
def g(x): return 'lti::_'+x+'::R'+x+'_parameters'             #replaced leading underscore in parameters classname with R


# list of tuples with ( xml-file-name, full-qualified-name of the base class )
lst = [   (f('functor_1_1parameters'),                          'lti::ioObject')
        , (f('ioFunctor_1_1parameters'),                        g('functor') )
        , (f('ioImage_1_1parameters'),                          g('ioFunctor'))
        , (f('usePalette_1_1parameters'),                       g('functor') )
            
        # Statistics)
        , (f('chiSquareFunctor_1_1parameters'),                 g('functor'))
    
        # Segmentation and Localization    )
        , (f('segmentation_1_1parameters'),                     g('functor'))
        , (f('regionGrowing_1_1parameters'),                    g('segmentation'))
        , (f('meanShiftSegmentation_1_1parameters'),            g('segmentation'))
        , (f('kMeansSegmentation_1_1parameters'),               g('segmentation'))
        , (f('whiteningSegmentation_1_1parameters'),            g('segmentation'))
        , (f('csPresegmentation_1_1parameters'),                g('segmentation'))
        , (f('thresholding_1_1parameters'),                     g('modifier'))
        , (f('thresholdSegmentation_1_1parameters'),            g('segmentation'))
        , (f('watershedSegmentation_1_1parameters'),            g('segmentation'))
        , (f('snake_1_1parameters'),                            g('segmentation'))
        , (f('regionMerge_1_1parameters'),                      g('functor'))
        , (f('similarityMatrix_1_1parameters'),                 g('functor'))
        , (f('fastRelabeling_1_1parameters'),                   g('modifier'))
        , (f('colorQuantization_1_1parameters'),                g('functor'))
        , (f('kMColorQuantization_1_1parameters'),              g('colorQuantization'))
        , (f('lkmColorQuantization_1_1parameters'),             g('colorQuantization'))
	, (f('medianCut_1_1parameters'),                        g('colorQuantization'))
        , (f('computePalette_1_1parameters'),              	g('functor'))
        , (f('ioBMP_1_1parameters'),                		g('ioFunctor'))
        , (f('viewerBase_1_1parameters'),             		g('functor'))
        , (f('externViewer_1_1parameters'),           		g('viewerBase'))
        , (f('objectsFromMask_1_1parameters'),       		g('segmentation'))
        , (f('objectsFromMask_1_1objectStruct'),    		'lti::ioObject')
        , (f('tree_1_1node'),                          		'lti::ioObject')
        , (f('featureExtractor_1_1parameters'),        		g('functor'))
        , (f('globalFeatureExtractor_1_1parameters'), 		g('featureExtractor'))
        , (f('localFeatureExtractor_1_1parameters'),  		g('featureExtractor'))
        , (f('geometricFeatures_1_1parameters'),      		g('globalFeatureExtractor'))
        , (f('localMoments_1_1parameters'),            		g('localFeatureExtractor'))
        , (f('chromaticityHistogram_1_1parameters'),  		g('globalFeatureExtractor'))
        , (f('modifier_1_1parameters'),                		g('functor'))
        , (f('transform_1_1parameters'),               		g('functor'))
        , (f('realFFT_1_1parameters'),                		g('transform'))
        , (f('qmf_1_1parameters'),                     		g('filter'))
        , (f('orientedHLTransform_1_1parameters'),    		g('transform'))
        , (f('gHoughTransform_1_1parameters'),        		g('modifier'))
        , (f('orientationMap_1_1parameters'),        		g('transform'))
        , (f('medianFilter_1_1parameters'),           		g('filter'))
        , (f('kNearestNeighFilter_1_1parameters'),    		g('filter'))
        , (f('gradientFunctor_1_1parameters'),        		g('transform'))
        , (f('skeleton_1_1parameters'),                		g('transform'))
        , (f('colorContrastGradient_1_1parameters'), 		g('gradientFunctor'))
        , (f('cornerDetector_1_1parameters'),         		g('modifier'))
        , (f('harrisCorners_1_1parameters'),          		g('modifier'))
        , (f('edgeDetector_1_1parameters'),            		g('modifier'))
        , (f('classicEdgeDetector_1_1parameters'),    		g('edgeDetector'))
        , (f('cannyEdges_1_1parameters'),              		g('edgeDetector'))

        # Points, Contours and Shape Manipulation    )
        , (f('interpolator_1_1parameters'),            		g('functor'))
        , (f('variablySpacedSamplesInterpolator_1_1parameters'),g('interpolator'))
        , (f('cubicSpline_1_1parameters'),             		g('variablySpacedSampleInterpolator'))
        , (f('borderExtrema_1_1parameters'),           		g('modifier'))
        , (f('boundingBox_1_1parameters'),             		g('segmentation'))
        , (f('polygonApproximation_1_1parameters'),  		g('modifier'))
        , (f('convexHull_1_1parameters'),              		g('modifier'))
        , (f('filter_1_1parameters'),                   	g('modifier'))
        , (f('convolution_1_1parameters'),             		g('filter'))
        , (f('correlation_1_1parameters'),             		g('filter'))
        , (f('morphology_1_1parameters'),              		g('modifier'))
        , (f('dilation_1_1parameters'),                		g('morphology'))
        , (f('erosion_1_1parameters'),                 		g('morphology'))
        , (f('distanceTransform_1_1parameters'),      		g('morphology'))
        , (f('scaling_1_1parameters'),                 		g('modifier'))
        , (f('rotation_1_1parameters'),                		g('modifier'))
        , (f('flipImage_1_1parameters'),               		g('modifier'))
        , (f('geometricTransform_1_1parameters'),      		g('modifier'))
        , (f('classifier_1_1parameters'),               	'lti::ioObject')
        , (f('classifier_1_1outputTemplate'),          		'lti::ioObject')
        , (f('classifier_1_1outputVector'),            		'lti::ioObject')
        , (f('decisionTree_1_1parameters'),            		g('classifier'))
        , (f('serial_1_1parameters'),                  		'lti::ioObject')
	, (f('ioPNG_1_1parameters'),                 		g('ioFunctor'))
        , (f('ioJPEG_1_1parameters'),               		g('ioFunctor'))
	, (f('distanceTransform_1_1sedMask'),                   None)
    ]
    

# some constants
nl = '\n'

# Doku:
#-------
# 'memberdef'   ==> 'kind'=function
#  --> 'name'
#  --> 'type'               # Return Type
#  --> 'param'             # Argumente
#       --> 'type' 
#
#
#
#

def WriteFile(name,txt):
    f = open(name,'w')
    f.write(txt)

def GetAttribute(attribs,key):
    for k,v in attribs:
        if k==key:
            return v

def GetValue(elem):
    #print elem,elem.firstChild,elem.nodeType
    if elem.nodeType == elem.TEXT_NODE:
        return elem.data
    elif elem.nodeType == elem.ELEMENT_NODE:
        if elem.firstChild <> None:
            s = ''
            for e in elem.childNodes:
                s += GetValue(e) #+' '
            return s
        return ''
    elif elem.nodeType == elem.ATTRIBUTE_NODE:
        return 'xxxx'
    elif elem.firstChild.nodeType == elem.TEXT_NODE:
        return elem.firstChild.data
    return '???'

def GetValueForItem(node,itemname,bOnlyFirst=False):
    s = ''
    nodes = node.getElementsByTagName(itemname)
    #print '>>>>>>>',nodes,itemname,len(nodes)
    if bOnlyFirst:
        s += GetValue( nodes[0] )
    else:
        for node in nodes:
            s += GetValue( node )
            s += ' '
    return s

def ProcessFunction(member,classname,newclassname,bIsConst=False,bIsPureVirtual=False,bIsHeader=True):
    s = ''
    arg = 'arg'
    names = member.getElementsByTagName('name')
    # es sollte immer nur einen Namen geben !
    nameNode = names[0]
    name = GetValue(nameNode)

    s += GetValueForItem(member,'type',bOnlyFirst=True)
    s += ' '
    if not bIsHeader:
        s += classname+'::'
    s += GetValueForItem(member,'name')
    s += '( '

    # Behandlung des Constructors
    bIsConstructor = (name == classname)

    params = member.getElementsByTagName('param')
    for i in range(len(params)):
        if i>0:
            s += ', '
        s += GetValueForItem(params[i],'type')
        s += ' '+arg+str(i)

    s += ' )'
    if bIsConst:
        s += ' const'
    if bIsPureVirtual:
        s += ' = 0'

    if not bIsHeader:
        s += nl
        s += '{'+nl
        s += '    '+'pObj->'+name+'('
        for i in range(len(params)):
            if i>0:
                s += ', '
            s += arg+str(i)
        s += ');'+nl
        s += '}'
    
    s += ';'+nl

    # dies ist fuer das manuelle name mangling notwendig !
    s = s.replace(classname,newclassname)

    return s

def VerifyParameterType(sType):
    """ 
        Sonderbehandlung fuer den Typ, falls es ein parameters Typ ist,
        notwendig, da der parameters Typ mit einem define umbenannt wird...
        
        Beispiel: gradientFunctor::parameters --> gradientFunctor_parameters
    """
    s = sType
    elem = sType.split('::')
    if len(elem)>1 and elem[-1]=='parameters':
        s = 'lti_'+sType.replace('::','_')
        print "FOUND:",s
    return s
    
def ProcessAttribute(member):
    s = ''
    sType = GetValueForItem(member,'type',bOnlyFirst=True)    
    sType = VerifyParameterType(sType)        
    s += sType
    s += ' '
    s += GetValueForItem(member,'name')
    s += GetValueForItem(member,'argsstring')
    s += ';'
    return s

def ProcessEnum(member):
    s = ''
    s += 'enum '
    name = GetValueForItem(member,'name',bOnlyFirst=True)
    # Behandlung der unbenannten enums
    if name[0]=='@':
        name = ''
    s += name
    s += ' {'+nl
    items = member.getElementsByTagName('enumvalue')
    for i in range(len(items)):
        e = items[i]
        if i>0:
            s += ','
        s += GetValueForItem(e,'name')
        val = GetValueForItem(e,'initializer')
        if val<>None and len(val)>0:
            s += ' = ' + val
        s += nl 
    s += '};'+nl
    return s

def ProcessMember(member,classname,newclassname,bIsHeader):
    s = ''
    attribs = member.attributes.items()
    if GetAttribute(attribs,'prot')=='public':
        if GetAttribute(attribs,'kind')=='function':
            s = ProcessFunction(member, classname, newclassname, GetAttribute(attribs,'const')=='yes', GetAttribute(attribs,'virt')=='pure-virtual', bIsHeader ) #, GetAttribute(attribs,'virtual')=='virtual'
        elif GetAttribute(attribs,'kind')=='variable':
            s = ProcessAttribute(member)
        elif GetAttribute(attribs,'kind')=='enum':
            s = ProcessEnum(member)
        else:
            s = '/* not a function or attribute */'     
    return s+'\n'

def ProcessAllMembers(theclassname,thenewclassname,members,bIsHeader=True,indent=' '*4):
    s = ''
    for member in members:
        s += indent+ProcessMember(member,theclassname,thenewclassname,bIsHeader)
    return s

def ProcessHeaderFile(classname,theclassname,thenewclassname,members,baseclassname=None):
    s = ''
    s += '#ifndef _'+thenewclassname+'_h'+nl
    s += '#define _'+thenewclassname+'_h'+nl
    s += nl
    sClose = ''
    names = classname.split('::')
    sTypedef = 'typedef '
    for n in range(len(names)):
        if n<len(names)-1:
            s += 'namespace '
            sPre = ''
            if n>0:
                sPre += '_'
            s += sPre+names[n]+' {'+nl
            if n<len(names)-2:
                sClose += '}'+nl            
            sTypedef += sPre+names[n]+'::'            
    s += 'class '+thenewclassname
    if baseclassname<>None:
        s += ' : public '+baseclassname
    s += nl
    s += '{'+nl
    s += 'public:'+nl
    s += ProcessAllMembers(theclassname,thenewclassname,members)
    #s += 'private:'+nl
    #s += '    /*pObj*/'+nl
    s += '};'+nl
    s += sClose
    s += sTypedef+thenewclassname+' '+thenewclassname[1:]+';'+nl    # erstes _ vom Klassennamen entfernen
    s += '}'
    s += nl
    s += '#endif'+nl
    return s

def ProcessCppFile(classname,theclassname,thenewclassname,members):
    s = ''
    s += nl
    s += '#include "'+thenewclassname+'.h"'+nl
    s += nl
    s += ProcessAllMembers(theclassname,thenewclassname,members,bIsHeader=False,indent='')
    s += nl
    return s

def ProcessClass(classname,theclassname,thenewclassname,members,baseclassname=None):
    s = ''
    s += ProcessHeaderFile(classname,theclassname,thenewclassname,members,baseclassname)
    s += nl
    #s += ProcessCppFile(classname,theclassname,thenewclassname,members)
    return s

def ProcessFile(filename,baseclassname,add_to_output=None):
    dom = minidom.parse(filename)

    #elem = dom.getElementsByTagName('compoundname')
    members = dom.getElementsByTagName('memberdef')
    
    classitem = dom.getElementsByTagName('compoundname')
    classname = GetValue(classitem[0])                      # lti::object
    print "processing",classname,"...",

    nameitems = classname.split('::')
    
    theclassname = nameitems[-1]                            # object
    s = ''
    for i in range(len(nameitems)):
        # ignoriere das erste lti::
        if i>0:
	    if i > 1:           #avoid class name starting with an underscore
                s += '_'
            s += nameitems[i]
    
    outputfilename = '_'+s   #header file name still starts with underscore (to minimize changes in .i files)
    
    thenewclassname = 'R'+s  #replaced leading underscore in parameters classname with R
    print thenewclassname
    
    if add_to_output<>None:
        outputfilename += add_to_output
    outputfilename += '.h'

    s = ProcessClass(classname,theclassname,thenewclassname,members,baseclassname)
    
    WriteFile(output_dir+os.sep+outputfilename,s)

def ProcessAllFiles(lst):
    for e in lst:
        filename = basedir+e[0]
        ProcessFile(filename,e[1])

#-------------------------------------------------

ProcessAllFiles(lst)

