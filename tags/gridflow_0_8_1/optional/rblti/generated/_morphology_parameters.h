#ifndef _Rmorphology_parameters_h
#define _Rmorphology_parameters_h

namespace lti {
namespace _morphology {
class Rmorphology_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
     Rmorphology_parameters (  );

     Rmorphology_parameters ( const Rmorphology_parameters &  arg0 );

     ~Rmorphology_parameters (  );

    const char * getTypeName (  ) const;

    Rmorphology_parameters & copy ( const Rmorphology_parameters &  arg0 );

    virtual functor::Rmorphology_parameters * clone (  ) const;

};
}
typedef lti::_morphology::Rmorphology_parameters morphology_parameters;
}
#endif

