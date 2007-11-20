#ifndef _Rclassifier_outputVector_h
#define _Rclassifier_outputVector_h

namespace lti {
namespace _classifier {
class Rclassifier_outputVector : public lti::ioObject
{
public:
     Rclassifier_outputVector (  );

     Rclassifier_outputVector ( const Rclassifier_outputVector &  arg0 );

     Rclassifier_outputVector ( const int &  arg0 );

     Rclassifier_outputVector ( const ivector &  arg0, const dvector &  arg1 );

    Rclassifier_outputVector & copy ( const Rclassifier_outputVector &  arg0 );

    Rclassifier_outputVector & operator= ( const Rclassifier_outputVector &  arg0 );

    Rclassifier_outputVector * clone (  ) const;

    int size (  ) const;

    bool find ( const int &  arg0, int &  arg1 ) const;

    void setValues ( const dvector &  arg0 );

    bool setValueByPosition ( const int &  arg0, const double &  arg1 );

    bool setValueById ( const int &  arg0, const double &  arg1 );

    const dvector & getValues (  ) const;

    void setIds ( const ivector &  arg0 );

    void setIdsAndValues ( const ivector &  arg0, const dvector &  arg1 );

    const ivector & getIds (  ) const;

    bool setPair ( const int &  arg0, const int &  arg1, const double &  arg2 );

    bool getId ( const int &  arg0, int &  arg1 ) const;

    bool getValueByPosition ( const int &  arg0, double &  arg1 ) const;

    bool getValueById ( const int &  arg0, double &  arg1 ) const;

    bool getPair ( const int &  arg0, int &  arg1, double &  arg2 ) const;

    bool setWinnerUnit ( const int &  arg0 );

    int setWinnerAtMax (  );

    int getWinnerUnit (  ) const;

    void setReject ( const bool &  arg0 );

    bool isRejected (  ) const;

    void setConfidenceValue ( const double &  arg0 );

    double getConfidenceValue (  ) const;

    void sortAscending (  );

    void sortDescending (  );

    void makeProbDistribution (  );

    bool compatible ( const Rclassifier_outputVector &  arg0 ) const;

    bool noMultipleIds (  ) const;

    void idMaximize (  );

    void idSum (  );

    double maxValue (  ) const;

    int maxPosition (  ) const;

    int maxId (  ) const;

    void maxPair ( int &  arg0, double &  arg1 ) const;

    double minValue (  ) const;

    int minPosition (  ) const;

    int minId (  ) const;

    void minPair ( int &  arg0, double &  arg1 ) const;

    Rclassifier_outputVector & add ( const Rclassifier_outputVector &  arg0 );

    Rclassifier_outputVector & add ( const Rclassifier_outputVector &  arg0, const Rclassifier_outputVector &  arg1 );

    Rclassifier_outputVector & add ( const double &  arg0 );

    Rclassifier_outputVector & addScaled ( const Rclassifier_outputVector &  arg0, const double &  arg1 );

    Rclassifier_outputVector & mul ( const Rclassifier_outputVector &  arg0 );

    Rclassifier_outputVector & mul ( const Rclassifier_outputVector &  arg0, const Rclassifier_outputVector &  arg1 );

    Rclassifier_outputVector & mul ( const double &  arg0 );

    Rclassifier_outputVector & divide ( const double &  arg0 );

    Rclassifier_outputVector & max ( const Rclassifier_outputVector &  arg0 );

    Rclassifier_outputVector & max ( const Rclassifier_outputVector &  arg0, const Rclassifier_outputVector &  arg1 );

    Rclassifier_outputVector & min ( const Rclassifier_outputVector &  arg0 );

    Rclassifier_outputVector & min ( const Rclassifier_outputVector &  arg0, const Rclassifier_outputVector &  arg1 );

    bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    bool read ( ioHandler &  arg0, const bool  arg1 );

    
    
    
    
    
    
    
    
};
}
typedef lti::_classifier::Rclassifier_outputVector classifier_outputVector;
}
#endif

