// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "nodetreelocal.h"

using namespace DBTREE;


NodeTreeLocal::NodeTreeLocal( const std::string& url )
    : NodeTree2chCompati( url, std::string() )
{
#ifdef _DEBUG
    std::cout << "NodeTreeLocal::NodeTreeLocal url = " << get_url() << std::endl;
#endif
}


NodeTreeLocal::~NodeTreeLocal()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeLocal::~NodeTreeLocal : " << get_url() << std::endl;
#endif
}
