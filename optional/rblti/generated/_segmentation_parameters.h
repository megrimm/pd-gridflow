#ifndef _Rsegmentation_parameters_h
#define _Rsegmentation_parameters_h

namespace lti {
namespace _segmentation {
class Rsegmentation_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     Rsegmentation_parameters (  );

     Rsegmentation_parameters ( const Rsegmentation_parameters &  arg0 );

    virtual ~Rsegmentation_parameters (  );

    Rsegmentation_parameters & copy ( const Rsegmentation_parameters &  arg0 );

    virtual functor::Rsegmentation_parameters * clone (  ) const;

    virtual const char * getTypeName (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

};
}
typedef lti::_segmentation::Rsegmentation_parameters segmentation_parameters;
}
#endif

