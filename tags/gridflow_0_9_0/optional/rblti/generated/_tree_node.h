#ifndef _Rtree_node_h
#define _Rtree_node_h

namespace lti {
namespace _tree {
class Rtree_node : public lti::ioObject
{
public:
    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

     Rtree_node ( const Rtree_node &  arg0 );

    virtual ~Rtree_node (  );

    bool isChildValid ( const int &  arg0 ) const;

    const Rtree_node & child ( const int &  arg0 ) const;

    Rtree_node & child ( const int &  arg0 );

    void setDegree ( const int &  arg0, bool  arg1 );

    Rtree_node & appendChild ( const T &  arg0 );

    Rtree_node & insertChild ( const T &  arg0 );

    Rtree_node & insertChild ( const int &  arg0, const T &  arg1 );

    Rtree_node & moveChild ( const int &  arg0, const int &  arg1 );

    Rtree_node & insertSibling ( const T &  arg0 );

    Rtree_node & deleteChild ( const int &  arg0 );

    bool isRoot (  ) const;

    Rtree_node & parent (  );

    const Rtree_node & parent (  ) const;

    int degree (  ) const;

    int numberOfChildren (  ) const;

    T & getData (  );

    const T & getData (  ) const;

    void setData ( const T &  arg0 );

    Rtree_node & copy ( const Rtree_node &  arg0 );

    int height (  ) const;

    int subtreeSize (  ) const;

    const int & level (  ) const;

    int position (  ) const;

    bool equals ( const Rtree_node &  arg0 );

    bool isSameNode ( const Rtree_node &  arg0 ) const;

    bool isParentOf ( const Rtree_node &  arg0 ) const;

    bool isSiblingOf ( const Rtree_node &  arg0 ) const;

    bool equalsTopology ( const Rtree_node &  arg0 );

    
    
    
    
    
    
    
    
    
    
    
    
    
};
}
typedef lti::_tree::Rtree_node tree_node;
}
#endif

