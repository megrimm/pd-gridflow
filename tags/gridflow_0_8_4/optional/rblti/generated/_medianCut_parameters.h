#ifndef _RmedianCut_parameters_h
#define _RmedianCut_parameters_h

namespace lti {
namespace _medianCut {
class RmedianCut_parameters : public lti::_colorQuantization::RcolorQuantization_parameters
{
public:
     RmedianCut_parameters (  );

     RmedianCut_parameters ( const RmedianCut_parameters &  arg0 );

     ~RmedianCut_parameters (  );

    const char * getTypeName (  ) const;

    RmedianCut_parameters & copy ( const RmedianCut_parameters &  arg0 );

    RmedianCut_parameters & operator= ( const RmedianCut_parameters &  arg0 );

    virtual functor::RmedianCut_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    int preQuant  ;
};
}
typedef lti::_medianCut::RmedianCut_parameters medianCut_parameters;
}
#endif

