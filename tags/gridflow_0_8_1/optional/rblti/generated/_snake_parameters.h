#ifndef _Rsnake_parameters_h
#define _Rsnake_parameters_h

namespace lti {
namespace _snake {
class Rsnake_parameters : public lti::_segmentation::Rsegmentation_parameters
{
public:
     Rsnake_parameters (  );

    virtual ~Rsnake_parameters (  );

    virtual functor::Rsnake_parameters * clone (  ) const;

    Rsnake_parameters & copy ( const Rsnake_parameters &  arg0 );

    virtual const char * getTypeName (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    rectangle patchPlace  ;
    int lfilter  ;
    frgbPixel average  ;
    frgbPixel variance  ;
    bool splitting  ;
};
}
typedef lti::_snake::Rsnake_parameters snake_parameters;
}
#endif

