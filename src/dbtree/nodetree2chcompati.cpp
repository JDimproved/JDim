// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "nodetree2chcompati.h"
#include "interface.h"

#include "jdlib/jdiconv.h"
#include "jdlib/loaderdata.h"
#include "jdlib/miscmsg.h"

#include "config/globalconf.h"

using namespace DBTREE;


NodeTree2chCompati::NodeTree2chCompati( const std::string& url, const std::string& date_modified )
    : NodeTreeBase( url, date_modified )
    , m_iconv( nullptr )
{
#ifdef _DEBUG
    std::cout << "NodeTree2chCompati::NodeTree2chCompati url = " << url << " modified = " << date_modified << std::endl;
#endif
}




NodeTree2chCompati::~NodeTree2chCompati()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2chCompati::~NodeTree2chCompati : " << get_url() << std::endl;
#endif

    NodeTree2chCompati::clear();
}



//
// バッファなどのクリア
//
void NodeTree2chCompati::clear()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2chCompati::clear : " << get_url() << std::endl;
#endif

    NodeTreeBase::clear();

    // iconv 削除
    if( m_iconv ) delete m_iconv;
    m_iconv = nullptr;
}



//
// ロード実行前に呼ぶ初期化関数
//
void NodeTree2chCompati::init_loading()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2chCompati::init_loading : " << get_url() << std::endl;
#endif

    NodeTreeBase::init_loading();

    // iconv 初期化
    std::string charset = DBTREE::board_charset( get_url() );
    if( ! m_iconv ) m_iconv = new JDLIB::Iconv( charset, "UTF-8" );
}



//
// キャッシュに保存する前の前処理
//
// 先頭にrawモードのステータスが入っていたら取り除く
//
char* NodeTree2chCompati::process_raw_lines( char* rawlines )
{
    char* pos = rawlines;

    if( *pos == '+' || *pos == '-' || *pos == 'E' ){

        int status = 0;
        if( pos[ 1 ] == 'O' && pos[ 2 ] == 'K' ) status = 1;
        if( pos[ 1 ] == 'E' && pos[ 2 ] == 'R' && pos[ 3 ] == 'R' ) status = 2;        
        if( pos[ 1 ] == 'I' && pos[ 2 ] == 'N' && pos[ 3 ] == 'C' && pos[ 4 ] == 'R' ) status = 3;
        if( pos[ 0 ] == 'E' && pos[ 1 ] == 'R' && pos[ 2 ] == 'R' && pos[ 3 ] == 'O' && pos[ 4 ] == 'R' ) status = 4;

#ifdef _DEBUG
        std::cout << "NodeTree2chCompati::process_raw_lines : raw mode status = " << status << std::endl;
#endif

        if( status != 0 ){
            pos = skip_status_line( pos, status );
        }
    }

    return pos;
}


//
// ステータス行のスキップ処理
//   status == 1 : 正常ステータス
//   status != 1 : 異常ステータス
// 
char* NodeTree2chCompati::skip_status_line( char* pos, int status )
{
    // この行を飛ばす
    char* pos_msg = pos;
    while( *pos != '\n' && *pos != '\0' ) ++pos;

    // エラー
    if( status != 1 ){
        int byte;
        std::string ext_err = std::string( m_iconv->convert( pos_msg, pos - pos_msg, byte ) );
        set_ext_err( ext_err );
        MISC::ERRMSG( ext_err );
    }

    if( *pos == '\n' ) ++pos;

    return pos;
}


//
// raw データを dat 形式に変換
//
// 2ch型サーバの場合は文字コードを変換するだけ
//
const char* NodeTree2chCompati::raw2dat( char* rawlines, int& byte )
{
    assert( m_iconv != nullptr );

    // バッファ自体はiconvクラスの中で持っているのでポインタだけもらう
    return  m_iconv->convert( rawlines, strlen( rawlines ), byte );
}



//
// ロード用データ作成
//
void NodeTree2chCompati::create_loaderdata( JDLIB::LOADERDATA& data )
{
    data.url = get_url();
    data.agent = DBTREE::get_agent( get_url() );

    // 1byte前からレジュームして '\n' が返ってこなかったらあぼーんがあったってこと
    if( get_lng_dat() ) {
        data.byte_readfrom = get_lng_dat() -1;
        set_resume( true );
    }
    else set_resume( false );

    data.host_proxy = DBTREE::get_proxy_host( get_url() );
    data.port_proxy = DBTREE::get_proxy_port( get_url() );
    data.basicauth_proxy = DBTREE::get_proxy_basicauth( get_url() );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();
    data.basicauth = DBTREE::board_basicauth( get_url() );
    data.cookie_for_write = DBTREE::board_cookie_for_write( get_url() );

    if( ! get_date_modified().empty() ) data.modified = get_date_modified();
}
