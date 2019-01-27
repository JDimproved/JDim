// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articlelocal.h"
#include "nodetreelocal.h"


using namespace DBTREE;


ArticleLocal::ArticleLocal( const std::string& datbase, const std::string& id )
    : Article2chCompati( datbase, id, true )
{
#ifdef _DEBUG
    std::cout << "ArticleLocal::ArticleLocal datbase = " << datbase
              << ", id = " << id
              << ", url = " << get_url() << std::endl;
#endif
}


ArticleLocal::~ArticleLocal()
{
#ifdef _DEBUG
    std::cout << "ArticleLocal::~ArticleLocal url = " << get_url() << std::endl;
#endif
}


// ID がこのスレのものかどうか
bool ArticleLocal::equal( const std::string& datbase, const std::string& id )
{
    return ( get_url() == datbase + id );
}


NodeTreeBase* ArticleLocal::create_nodetree()
{
    return new NodeTreeLocal( get_url() );
}
