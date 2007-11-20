#ifndef _RmaximumFilter_parameters_h
#define _RmaximumFilter_parameters_h

namespace lti {
namespace _maximumFilter {
class RmaximumFilter_parameters : public lti::_filter::Rfilter_parameters
{
public:
     RmaximumFilter_parameters ( const int  arg0 );

     RmaximumFilter_parameters ( const RmaximumFilter_parameters &  arg0 );

     ~RmaximumFilter_parameters (  );

    const char * getTypeName (  ) const;

    RmaximumFilter_parameters & copy ( const RmaximumFilter_parameters &  arg0 );

    virtual functor::RmaximumFilter_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    void initSquare ( const int  arg0 );

    rectangle kernelSize  ;
};
}
typedef lti::_maximumFilter::RmaximumFilter_parameters maximumFilter_parameters;
}
#endif

