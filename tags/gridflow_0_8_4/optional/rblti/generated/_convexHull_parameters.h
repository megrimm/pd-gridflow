#ifndef _RconvexHull_parameters_h
#define _RconvexHull_parameters_h

namespace lti {
namespace _convexHull {
class RconvexHull_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
     RconvexHull_parameters (  );

     RconvexHull_parameters ( const RconvexHull_parameters &  arg0 );

     ~RconvexHull_parameters (  );

    const char * getTypeName (  ) const;

    RconvexHull_parameters & copy ( const RconvexHull_parameters &  arg0 );

    RconvexHull_parameters & operator= ( const RconvexHull_parameters &  arg0 );

    virtual functor::RconvexHull_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

};
}
typedef lti::_convexHull::RconvexHull_parameters convexHull_parameters;
}
#endif

