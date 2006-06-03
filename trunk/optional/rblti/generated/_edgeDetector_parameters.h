#ifndef _RedgeDetector_parameters_h
#define _RedgeDetector_parameters_h

namespace lti {
namespace _edgeDetector {
class RedgeDetector_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
     RedgeDetector_parameters (  );

     RedgeDetector_parameters ( const RedgeDetector_parameters &  arg0 );

     ~RedgeDetector_parameters (  );

    const char * getTypeName (  ) const;

    RedgeDetector_parameters & copy ( const RedgeDetector_parameters &  arg0 );

    RedgeDetector_parameters & operator= ( const RedgeDetector_parameters &  arg0 );

    virtual functor::RedgeDetector_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    ubyte noEdgeValue  ;
    ubyte edgeValue  ;
};
}
typedef lti::_edgeDetector::RedgeDetector_parameters edgeDetector_parameters;
}
#endif

