#ifndef _RexternViewer_parameters_h
#define _RexternViewer_parameters_h

namespace lti {
namespace _externViewer {
class RexternViewer_parameters : public lti::_viewerBase::RviewerBase_parameters
{
public:
     RexternViewer_parameters (  );

     RexternViewer_parameters ( const RexternViewer_parameters &  arg0 );

    virtual ~RexternViewer_parameters (  );

    RexternViewer_parameters & copy ( const RexternViewer_parameters &  arg0 );

    RexternViewer_parameters & operator= ( const RexternViewer_parameters &  arg0 );

    virtual functor::RexternViewer_parameters * clone (  ) const;

    const char * getTypeName (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    std::string tmpDirectory  ;
    std::string externViewerApp  ;
};
}
typedef lti::_externViewer::RexternViewer_parameters externViewer_parameters;
}
#endif

