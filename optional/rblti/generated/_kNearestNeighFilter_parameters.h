#ifndef _RkNearestNeighFilter_parameters_h
#define _RkNearestNeighFilter_parameters_h

namespace lti {
namespace _kNearestNeighFilter {
class RkNearestNeighFilter_parameters : public lti::_filter::Rfilter_parameters
{
public:
     RkNearestNeighFilter_parameters (  );

     RkNearestNeighFilter_parameters ( const RkNearestNeighFilter_parameters &  arg0 );

     ~RkNearestNeighFilter_parameters (  );

    const char * getTypeName (  ) const;

    RkNearestNeighFilter_parameters & copy ( const RkNearestNeighFilter_parameters &  arg0 );

    RkNearestNeighFilter_parameters & operator= ( const RkNearestNeighFilter_parameters &  arg0 );

    virtual functor::RkNearestNeighFilter_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    int kernelSize  ;
};
}
typedef lti::_kNearestNeighFilter::RkNearestNeighFilter_parameters kNearestNeighFilter_parameters;
}
#endif

