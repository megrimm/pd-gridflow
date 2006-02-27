#ifndef _RflipImage_parameters_h
#define _RflipImage_parameters_h

namespace lti {
namespace _flipImage {
class RflipImage_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
    enum eFlipDirection {
None  =  0 
,Horizontal 
,Vertical 
,Both 
};

     RflipImage_parameters (  );

     RflipImage_parameters ( const RflipImage_parameters &  arg0 );

     ~RflipImage_parameters (  );

    const char * getTypeName (  ) const;

    RflipImage_parameters & copy ( const RflipImage_parameters &  arg0 );

    RflipImage_parameters & operator= ( const RflipImage_parameters &  arg0 );

    virtual functor::RflipImage_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    eFlipDirection direction  ;
};
}
typedef lti::_flipImage::RflipImage_parameters flipImage_parameters;
}
#endif

