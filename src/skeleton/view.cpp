// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "view.h"

using namespace SKELETON;


View::View( const std::string& url, const std::string& arg1 ,const std::string& arg2 )
    : m_url( url ), m_status( std::string() ), m_enable_mg( 0 )
{}
