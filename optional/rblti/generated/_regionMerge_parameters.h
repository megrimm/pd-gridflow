#ifndef _RregionMerge_parameters_h
#define _RregionMerge_parameters_h

namespace lti {
namespace _regionMerge {
class RregionMerge_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RregionMerge_parameters (  );

     RregionMerge_parameters ( const RregionMerge_parameters &  arg0 );

     ~RregionMerge_parameters (  );

    const char * getTypeName (  ) const;

    RregionMerge_parameters & copy ( const RregionMerge_parameters &  arg0 );

    RregionMerge_parameters & operator= ( const RregionMerge_parameters &  arg0 );

    virtual functor::RregionMerge_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    double threshold  ;
};
}
typedef lti::_regionMerge::RregionMerge_parameters regionMerge_parameters;
}
#endif

