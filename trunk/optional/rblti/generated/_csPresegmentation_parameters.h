#ifndef _RcsPresegmentation_parameters_h
#define _RcsPresegmentation_parameters_h

namespace lti {
namespace _csPresegmentation {
class RcsPresegmentation_parameters : public lti::_segmentation::Rsegmentation_parameters
{
public:
    enum eFilterType {
Nothing  =  0 
,Median  =  1 
,KNearest  =  2 
};

     RcsPresegmentation_parameters (  );

     RcsPresegmentation_parameters ( const RcsPresegmentation_parameters &  arg0 );

     ~RcsPresegmentation_parameters (  );

    const char * getTypeName (  ) const;

    RcsPresegmentation_parameters & copy ( const RcsPresegmentation_parameters &  arg0 );

    RcsPresegmentation_parameters & operator= ( const RcsPresegmentation_parameters &  arg0 );

    virtual functor::RcsPresegmentation_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    int smoothingKernelSize  ;
    lti_kMColorQuantization_parameters quantParameters  ;
    bool useAlwaysNewPalette  ;
    int borderSize  ;
    int borderParts  ;
    bool forceBorderToBackground  ;
    bool labelObjects  ;
    float backgroundTolerance  ;
    eFilterType filterType  ;
    const int All  ;
    const int Top  ;
    const int Bottom  ;
    const int Left  ;
    const int Right  ;
};
}
typedef lti::_csPresegmentation::RcsPresegmentation_parameters csPresegmentation_parameters;
}
#endif

