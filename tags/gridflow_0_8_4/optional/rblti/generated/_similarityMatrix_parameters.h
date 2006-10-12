#ifndef _RsimilarityMatrix_parameters_h
#define _RsimilarityMatrix_parameters_h

namespace lti {
namespace _similarityMatrix {
class RsimilarityMatrix_parameters : public lti::_functor::Rfunctor_parameters
{
public:
    enum eModeType {
BorderBased 
,AreaBased 
};

     RsimilarityMatrix_parameters (  );

     RsimilarityMatrix_parameters ( const RsimilarityMatrix_parameters &  arg0 );

     ~RsimilarityMatrix_parameters (  );

    const char * getTypeName (  ) const;

    RsimilarityMatrix_parameters & copy ( const RsimilarityMatrix_parameters &  arg0 );

    RsimilarityMatrix_parameters & operator= ( const RsimilarityMatrix_parameters &  arg0 );

    virtual functor::RsimilarityMatrix_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    eModeType mode  ;
    double(* distFunction )(const rgbPixel &, const rgbPixel &) ;
};
}
typedef lti::_similarityMatrix::RsimilarityMatrix_parameters similarityMatrix_parameters;
}
#endif

