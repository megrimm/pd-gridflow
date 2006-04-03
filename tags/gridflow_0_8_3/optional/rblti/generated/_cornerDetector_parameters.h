#ifndef _RcornerDetector_parameters_h
#define _RcornerDetector_parameters_h

namespace lti {
namespace _cornerDetector {
class RcornerDetector_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
     RcornerDetector_parameters (  );

     RcornerDetector_parameters ( const RcornerDetector_parameters &  arg0 );

     ~RcornerDetector_parameters (  );

    const char * getTypeName (  ) const;

    RcornerDetector_parameters & copy ( const RcornerDetector_parameters &  arg0 );

    RcornerDetector_parameters & operator= ( const RcornerDetector_parameters &  arg0 );

    virtual functor::RcornerDetector_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    ubyte cornerValue  ;
    ubyte noCornerValue  ;
};
}
typedef lti::_cornerDetector::RcornerDetector_parameters cornerDetector_parameters;
}
#endif

