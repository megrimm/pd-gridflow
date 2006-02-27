#ifndef _RwatershedSegmentation_parameters_h
#define _RwatershedSegmentation_parameters_h

namespace lti {
namespace _watershedSegmentation {
class RwatershedSegmentation_parameters : public lti::_segmentation::Rsegmentation_parameters
{
public:
     RwatershedSegmentation_parameters (  );

     RwatershedSegmentation_parameters ( const RwatershedSegmentation_parameters &  arg0 );

     ~RwatershedSegmentation_parameters (  );

    const char * getTypeName (  ) const;

    RwatershedSegmentation_parameters & copy ( const RwatershedSegmentation_parameters &  arg0 );

    RwatershedSegmentation_parameters & operator= ( const RwatershedSegmentation_parameters &  arg0 );

    virtual functor::RwatershedSegmentation_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    bool neighborhood8  ;
    int watershedValue  ;
    int basinValue  ;
    bool rainfall  ;
    ubyte threshold  ;
};
}
typedef lti::_watershedSegmentation::RwatershedSegmentation_parameters watershedSegmentation_parameters;
}
#endif

