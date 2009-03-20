// ライセンス: GPL2

// #define _DEBUG
#include "jddebug.h"

#include "nodetreedummy.h"

using namespace DBTREE;


NodeTreeDummy::NodeTreeDummy( const std::string& url )
    : NodeTreeBase( url, std::string() )
{
#ifdef _DEBUG    
    std::cout << "NodeTreeDummy::NodeTreeDummy : " << get_url() << std::endl;
#endif
}


NodeTreeDummy::~NodeTreeDummy()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeDummy::~NodeTreeDummy : " << get_url() << std::endl;
#endif
}
