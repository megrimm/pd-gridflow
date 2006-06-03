#ifndef _Rclassifier_outputTemplate_h
#define _Rclassifier_outputTemplate_h

namespace lti {
namespace _classifier {
class Rclassifier_outputTemplate : public lti::ioObject
{
public:
    enum eMultipleMode {
Ignore  =  0 
,Max 
,Uniform 
,ObjProb 
};

     Rclassifier_outputTemplate (  );

     Rclassifier_outputTemplate ( const Rclassifier_outputTemplate &  arg0 );

     Rclassifier_outputTemplate ( const ivector &  arg0 );

     Rclassifier_outputTemplate ( const int &  arg0 );

    Rclassifier_outputTemplate & copy ( const Rclassifier_outputTemplate &  arg0 );

    Rclassifier_outputTemplate & operator= ( const Rclassifier_outputTemplate &  arg0 );

    Rclassifier_outputTemplate * clone (  ) const;

    void setMultipleMode ( const eMultipleMode &  arg0 );

    eMultipleMode getMultipleMode (  ) const;

    bool setIds ( const ivector &  arg0 );

    const ivector & getIds (  ) const;

    bool setProbs ( const int &  arg0, const ivector &  arg1, const dvector &  arg2 );

    bool setProbs ( const int &  arg0, const outputVector &  arg1 );

    const outputVector & getProbs ( const int &  arg0 ) const;

    bool setData ( const int &  arg0, const int &  arg1, const outputVector &  arg2 );

    int size (  ) const;

    bool apply ( const dvector &  arg0, outputVector &  arg1 ) const;

    bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    bool read ( ioHandler &  arg0, const bool  arg1 );

    
    
    
};
}
typedef lti::_classifier::Rclassifier_outputTemplate classifier_outputTemplate;
}
#endif

