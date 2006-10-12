#ifndef _Rfilter_parameters_h
#define _Rfilter_parameters_h

namespace lti {
namespace _filter {
class Rfilter_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
    Rfilter_parameters & copy ( const Rfilter_parameters &  arg0 );

};
}
typedef lti::_filter::Rfilter_parameters filter_parameters;
}
#endif

