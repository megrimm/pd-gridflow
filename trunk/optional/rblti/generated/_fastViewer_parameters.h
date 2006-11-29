#ifndef _RfastViewer_parameters_h
#define _RfastViewer_parameters_h

namespace lti {
namespace _fastViewer {
class RfastViewer_parameters : public lti::_viewerBase::RviewerBase_parameters
{
public:
     RfastViewer_parameters (  );

     RfastViewer_parameters ( const RfastViewer_parameters &  arg0 );

    virtual ~RfastViewer_parameters (  );

    RfastViewer_parameters & copy ( const RfastViewer_parameters &  arg0 );

    RfastViewer_parameters & operator= ( const RfastViewer_parameters &  arg0 );

    functor::RfastViewer_parameters * clone (  ) const;

    const char * getTypeName (  ) const;

    bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    bool read ( ioHandler &  arg0, const bool  arg1 );

    point topleft  ;
    bool noBorder  ;
};
}
typedef lti::_fastViewer::RfastViewer_parameters fastViewer_parameters;
}
#endif

