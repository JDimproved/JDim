// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "view.h"

#include "global.h"

using namespace SKELETON;


View::View( const std::string& url, const std::string& arg1 ,const std::string& arg2 )
    : m_url( url ), m_status( std::string() ), m_enable_mg( 0 ), m_autoreload_mode( AUTORELOAD_NOT )
{}


void View::clock_in_always()
{
    // オートリロード
    if( inc_autoreload_counter() ) reload();
}


// オートリロードのカウンタをインクリメント
// 指定秒数を越えたら true を返す
bool View::inc_autoreload_counter()
{
    if( m_autoreload_mode == AUTORELOAD_NOT ) return false;

    ++m_autoreload_counter;

    if( m_autoreload_counter > m_autoreload_sec * 1000/TIMER_TIMEOUT ){
        reset_autoreload_counter();
        return true;
    }

    return false;
}


// オートリロードのモードを設定
void View::set_autoreload_mode( int mode, int sec )
{
    m_autoreload_mode = mode;
    m_autoreload_sec = MAX( AUTORELOAD_MINSEC, sec );
    m_autoreload_counter = 0;
}


// オートリロードのカウンタをリセット
void View::reset_autoreload_counter()
{
    m_autoreload_counter = 0;
    if( m_autoreload_mode == AUTORELOAD_ONCE ) m_autoreload_mode = AUTORELOAD_NOT;
}

