#ifndef _RmaskImage_parameters_h
#define _RmaskImage_parameters_h

namespace lti {
namespace _maskImage {
class RmaskImage_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RmaskImage_parameters (  );

     RmaskImage_parameters ( const RmaskImage_parameters &  arg0 );

     ~RmaskImage_parameters (  );

    const char * getTypeName (  ) const;

    RmaskImage_parameters & copy ( const RmaskImage_parameters &  arg0 );

    RmaskImage_parameters & operator= ( const RmaskImage_parameters &  arg0 );

    virtual functor::RmaskImage_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    bool assumeLabeledMask  ;
    int backgroundLabel  ;
    palette colors  ;
    palette borderColors  ;
    bool enhanceRegionBorders  ;
};
}
typedef lti::_maskImage::RmaskImage_parameters maskImage_parameters;
}
#endif

