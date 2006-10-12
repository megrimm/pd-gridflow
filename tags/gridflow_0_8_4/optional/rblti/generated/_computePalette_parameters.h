#ifndef _RcomputePalette_parameters_h
#define _RcomputePalette_parameters_h

namespace lti {
namespace _computePalette {
class RcomputePalette_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RcomputePalette_parameters (  );

     RcomputePalette_parameters ( const RcomputePalette_parameters &  arg0 );

     ~RcomputePalette_parameters (  );

    const char * getTypeName (  ) const;

    RcomputePalette_parameters & copy ( const RcomputePalette_parameters &  arg0 );

    RcomputePalette_parameters & operator= ( const RcomputePalette_parameters &  arg0 );

    virtual functor::RcomputePalette_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

};
}
typedef lti::_computePalette::RcomputePalette_parameters computePalette_parameters;
}
#endif

