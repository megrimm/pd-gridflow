#ifndef _Rtransform_parameters_h
#define _Rtransform_parameters_h

namespace lti {
namespace _transform {
class Rtransform_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     Rtransform_parameters (  );

     Rtransform_parameters ( const Rtransform_parameters &  arg0 );

    Rtransform_parameters & copy ( const Rtransform_parameters &  arg0 );

    virtual functor::Rtransform_parameters * clone (  ) const;

    virtual const char * getTypeName (  ) const;

};
}
typedef lti::_transform::Rtransform_parameters transform_parameters;
}
#endif

