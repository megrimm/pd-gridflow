#ifndef _RmedianFilter_parameters_h
#define _RmedianFilter_parameters_h

namespace lti {
namespace _medianFilter {
class RmedianFilter_parameters : public lti::_filter::Rfilter_parameters
{
public:
     RmedianFilter_parameters (  );

     RmedianFilter_parameters ( const RmedianFilter_parameters &  arg0 );

     ~RmedianFilter_parameters (  );

    const char * getTypeName (  ) const;

    RmedianFilter_parameters & copy ( const RmedianFilter_parameters &  arg0 );

    RmedianFilter_parameters & operator= ( const RmedianFilter_parameters &  arg0 );

    virtual functor::RmedianFilter_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    int kernelSize  ;
};
}
typedef lti::_medianFilter::RmedianFilter_parameters medianFilter_parameters;
}
#endif

