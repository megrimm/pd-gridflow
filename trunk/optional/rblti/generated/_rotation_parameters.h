#ifndef _Rrotation_parameters_h
#define _Rrotation_parameters_h

namespace lti {
namespace _rotation {
class Rrotation_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
     Rrotation_parameters (  );

     Rrotation_parameters ( const Rrotation_parameters &  arg0 );

     ~Rrotation_parameters (  );

    const char * getTypeName (  ) const;

    Rrotation_parameters & copy ( const Rrotation_parameters &  arg0 );

    Rrotation_parameters & operator= ( const Rrotation_parameters &  arg0 );

    virtual functor::Rrotation_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    double angle  ;
};
}
typedef lti::_rotation::Rrotation_parameters rotation_parameters;
}
#endif

