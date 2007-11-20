#ifndef _RdecisionTree_parameters_h
#define _RdecisionTree_parameters_h

namespace lti {
namespace _decisionTree {
class RdecisionTree_parameters : public lti::_classifier::Rclassifier_parameters
{
public:
     RdecisionTree_parameters (  );

     RdecisionTree_parameters ( const RdecisionTree_parameters &  arg0 );

    virtual ~RdecisionTree_parameters (  );

    const char * getTypeName (  ) const;

    RdecisionTree_parameters & copy ( const RdecisionTree_parameters &  arg0 );

    RdecisionTree_parameters & operator= ( const RdecisionTree_parameters &  arg0 );

    virtual classifier::RdecisionTree_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

};
}
typedef lti::_decisionTree::RdecisionTree_parameters decisionTree_parameters;
}
#endif

