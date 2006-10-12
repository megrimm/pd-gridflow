#ifndef _Rclassifier_parameters_h
#define _Rclassifier_parameters_h

namespace lti {
namespace _classifier {
class Rclassifier_parameters : public lti::ioObject
{
public:
    enum eDistanceMeasure {
L1 
,L2 
};

     Rclassifier_parameters (  );

     Rclassifier_parameters ( const Rclassifier_parameters &  arg0 );

    virtual ~Rclassifier_parameters (  );

    const char * getTypeName (  ) const;

    Rclassifier_parameters & copy ( const Rclassifier_parameters &  arg0 );

    Rclassifier_parameters & operator= ( const Rclassifier_parameters &  arg0 );

    virtual Rclassifier_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    outputTemplate::eMultipleMode multipleMode  ;
};
}
typedef lti::_classifier::Rclassifier_parameters classifier_parameters;
}
#endif

