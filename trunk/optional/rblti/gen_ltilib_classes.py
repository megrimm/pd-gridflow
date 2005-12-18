#******************************************************************************
#
#      project:  pylti
#    copyright:  (c) 2005 by Michael Neuroth
#
#  description:  python modul for ltilib (using SWIG)
#
#  $Source$
#  $Revision$
#  $Date$
# 
#  $Log$
#  Revision 1.1  2005/12/18 00:16:55  matju
#  .
#
#  Revision 1.9  2005/07/04 20:01:42  Michael
#  Added support to build pylti as a misc-project in the ltilib distribution
#
#  Revision 1.8  2005/06/28 21:39:56  Michael
#  write output files in a directory (generated).
#
#  Revision 1.7  2005/06/28 20:55:16  Michael
#  Added new functors: cannyEdges and classifier and updated to doygen version 1.4.3
#
#  Revision 1.6  2005/05/01 10:41:32  Michael
#  Added objectStruct. Tests for support for trees.
#
#  Revision 1.5  2005/02/22 23:08:41  Michael
#  Added more functors
#
#  Revision 1.4  2005/02/19 22:29:09  Michael
#  Added some more functors and functions.
#
#  Revision 1.3  2005/02/14 22:17:30  Michael
#  updates for the new project structure
#
#  Revision 1.2  2005/02/12 10:41:34  Michael
#  generate an aditional typedef declaration
#
#  Revision 1.1.1.1  2005/02/09 21:41:11  Michael
#  initial checkin
#
#
#******************************************************************************
#
# Tool to generate header files for SWIG to process the nested classes "parameters"
#
# 1) generate XML files for the ltilib with doxygen (tested with version 1.4.3) (use this switch in doc.cfg file: GENERATE_XML = YES)
# 2) generate the new header files for SWIG with this python script
#  

from xml.dom import minidom
import os
from distutils.text_file import TextFile

str_version = '0.32'

# handle the settings: WORKAREA has to be defined !
basedir = None

workarea = os.getenv('WORKAREA')

# is pylti not part of the ltilib ? Yes --> than we need a directory structure of WORKAREA_PATH/import/ltilib/xml
if workarea<>None:
    basedir = workarea+os.sep+'import'+os.sep+'ltilib'+os.sep+'xml'+os.sep

# check if we are in misc directory of the ltilib distribution (ltilib/misc/pylti)
# very simple way: navigate up and navigate down again
if workarea==None:    
    strCheckPath = '..'+os.sep+'..'+os.sep+'misc'+os.sep+'pylti'+'-'+str_version
    try:
        # try to read lti.i, this is a good indication of a pylti directory
        aTestFile = TextFile(filename=strCheckPath+os.sep+'lti.i')
        basedir = '..'+os.sep+'..'+os.sep+'xml'+os.sep        # move up ltipy and misc directory 
    except IOError :
        pass


output_dir = 'generated'

# list of tuples with ( xml-file-name, full-qualified-name of the base class )
lst = [   ('classlti_1_1functor_1_1parameters.xml',                 'lti::ioObject')
        , ('classlti_1_1ioFunctor_1_1parameters.xml',              'lti::_functor::_functor_parameters') 
        , ('classlti_1_1ioImage_1_1parameters.xml',                'lti::_ioFunctor::_ioFunctor_parameters') 
                
        , ('classlti_1_1usePalette_1_1parameters.xml',             'lti::_functor::_functor_parameters') 

        , ('classlti_1_1segmentation_1_1parameters.xml',            'lti::_functor::_functor_parameters')
        , ('classlti_1_1regionGrowing_1_1parameters.xml',          'lti::_segmentation::_segmentation_parameters')
        , ('classlti_1_1meanShiftSegmentation_1_1parameters.xml', 'lti::_segmentation::_segmentation_parameters')
        , ('classlti_1_1kMeansSegmentation_1_1parameters.xml',    'lti::_segmentation::_segmentation_parameters')
        , ('classlti_1_1whiteningSegmentation_1_1parameters.xml',  'lti::_segmentation::_segmentation_parameters')
        , ('classlti_1_1csPresegmentation_1_1parameters.xml',      'lti::_segmentation::_segmentation_parameters')

        , ('classlti_1_1colorQuantization_1_1parameters.xml',      'lti::_functor::_functor_parameters')
        , ('classlti_1_1kMColorQuantization_1_1parameters.xml',  'lti::_colorQuantization::_colorQuantization_parameters')

        , ('classlti_1_1ioBMP_1_1parameters.xml',                'lti::_ioFunctor::_ioFunctor_parameters')
#TODO: fuer JPEG parameters:        , ('',                'lti::_ioFunctor::_ioFunctor_parameters')
#TODO: fuer PNG parameters:        , ('',                'lti::_ioFunctor::_ioFunctor_parameters')
        , ('classlti_1_1viewerBase_1_1parameters.xml',             'lti::_functor::_functor_parameters')
        , ('classlti_1_1externViewer_1_1parameters.xml',           'lti::_viewerBase::_viewerBase_parameters')

        , ('classlti_1_1objectsFromMask_1_1parameters.xml',       'lti::_segmentation::_segmentation_parameters')
        , ('classlti_1_1objectsFromMask_1_1objectStruct.xml',    'lti::ioObject')

        , ('classlti_1_1tree_1_1node.xml',                          'lti::ioObject')

        , ('classlti_1_1featureExtractor_1_1parameters.xml',        'lti::_functor::_functor_parameters')
        , ('classlti_1_1globalFeatureExtractor_1_1parameters.xml', 'lti::_featureExtractor::_featureExtractor_parameters')
        , ('classlti_1_1localFeatureExtractor_1_1parameters.xml',  'lti::_featureExtractor::_featureExtractor_parameters')
        , ('classlti_1_1geometricFeatures_1_1parameters.xml',      'lti::_globalFeatureExtractor::_globalFeatureExtractor_parameters')
        , ('classlti_1_1localMoments_1_1parameters.xml',            'lti::_localFeatureExtractor::_localFeatureExtractor_parameters')
        , ('classlti_1_1chromaticityHistogram_1_1parameters.xml',  'lti::_globalFeatureExtractor::_globalFeatureExtractor_parameters')

        , ('classlti_1_1modifier_1_1parameters.xml',                'lti::_functor::_functor_parameters')
        , ('classlti_1_1polygonApproximation_1_1parameters.xml',  'lti::_modifier::_modifier_parameters')

        , ('classlti_1_1transform_1_1parameters.xml',               'lti::_functor::_functor_parameters')
        , ('classlti_1_1gradientFunctor_1_1parameters.xml',        'lti::_transform::_transform_parameters')
        , ('classlti_1_1skeleton_1_1parameters.xml',                'lti::_transform::_transform_parameters')

        , ('classlti_1_1colorContrastGradient_1_1parameters.xml', 'lti::_gradientFunctor::_gradientFunctor_parameters')

        , ('classlti_1_1edgeDetector_1_1parameters.xml',            'lti::_modifier::_modifier_parameters')
        , ('classlti_1_1classicEdgeDetector_1_1parameters.xml',    'lti::_edgeDetector::_edgeDetector_parameters')

        , ('classlti_1_1cannyEdges_1_1parameters.xml',              'lti::_edgeDetector::_edgeDetector_parameters')

        , ('classlti_1_1filter_1_1parameters.xml',                   'lti::_modifier::_modifier_parameters')
        , ('classlti_1_1convolution_1_1parameters.xml',             'lti::_filter::_filter_parameters')

        , ('classlti_1_1morphology_1_1parameters.xml',              'lti::_modifier::_modifier_parameters')
        , ('classlti_1_1dilation_1_1parameters.xml',                'lti::_morphology::_morphology_parameters')
        , ('classlti_1_1erosion_1_1parameters.xml',                 'lti::_morphology::_morphology_parameters')
        , ('classlti_1_1distanceTransform_1_1parameters.xml',      'lti::_morphology::_morphology_parameters')

        , ('classlti_1_1classifier_1_1parameters.xml',               'lti::ioObject')
        , ('classlti_1_1classifier_1_1outputTemplate.xml',          'lti::ioObject')
        , ('classlti_1_1classifier_1_1outputVector.xml',            'lti::ioObject')
        
        , ('classlti_1_1decisionTree_1_1parameters.xml',            'lti::_classifier::_classifier_parameters')
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
            s += '_'
            s += nameitems[i]
        
    thenewclassname = s #classname.replace('::','_')           # lti_object
    print thenewclassname
    
    outputfilename = thenewclassname
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

    
