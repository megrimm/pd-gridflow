#ifndef _RbackgroundModel_parameters_h
#define _RbackgroundModel_parameters_h

namespace lti {
namespace _backgroundModel {
class RbackgroundModel_parameters : public lti::_segmentation::Rsegmentation_parameters
{
public:
     RbackgroundModel_parameters (  );

     RbackgroundModel_parameters ( const RbackgroundModel_parameters &  arg0 );

     ~RbackgroundModel_parameters (  );

    const char * getTypeName (  ) const;

    RbackgroundModel_parameters & copy ( const RbackgroundModel_parameters &  arg0 );

    RbackgroundModel_parameters & operator= ( const RbackgroundModel_parameters &  arg0 );

    virtual functor::RbackgroundModel_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    float outputThreshold  ;
    bool binaryOutput  ;
    float adaptationThreshold  ;
    bool adaptModel  ;
    float alpha  ;
};
}
typedef lti::_backgroundModel::RbackgroundModel_parameters backgroundModel_parameters;
}
#endif

