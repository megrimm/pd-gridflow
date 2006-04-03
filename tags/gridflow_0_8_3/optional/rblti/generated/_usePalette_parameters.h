#ifndef _RusePalette_parameters_h
#define _RusePalette_parameters_h

namespace lti {
namespace _usePalette {
class RusePalette_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RusePalette_parameters (  );

     RusePalette_parameters ( const RusePalette_parameters &  arg0 );

     ~RusePalette_parameters (  );

    const char * getTypeName (  ) const;

    RusePalette_parameters & copy ( const RusePalette_parameters &  arg0 );

    RusePalette_parameters & operator= ( const RusePalette_parameters &  arg0 );

    virtual functor::RusePalette_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    palette colors  ;
    bool linearSearch  ;
    bool kdTreeOnDemand  ;
    int bucketSize  ;
};
}
typedef lti::_usePalette::RusePalette_parameters usePalette_parameters;
}
#endif

