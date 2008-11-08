// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "logmanager.h"
#include "logitem.h"
#include "messageadmin.h"

#include "config/globalconf.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "cache.h"
#include "session.h"

#include <sys/time.h>
#include <cstring>
#include <sys/types.h> // chmod
#include <sys/stat.h>


enum
{
    WRITE_TIMEOUT = 120  // 書き込んでからこの秒数を過ぎても書き込んだレスを判定できなかったらロストとする
};


MESSAGE::Log_Manager* instance_log_manager = NULL;

MESSAGE::Log_Manager* MESSAGE::get_log_manager()
{
    if( ! instance_log_manager ) instance_log_manager = new MESSAGE::Log_Manager();
    assert( instance_log_manager );

    return instance_log_manager;
}


void MESSAGE::delete_log_manager()
{
    if( instance_log_manager ) delete instance_log_manager;
    instance_log_manager = NULL;
}



///////////////////////////////////////////////

using namespace MESSAGE;


Log_Manager::Log_Manager()
{
#ifdef _DEBUG
    std::cout << "MESSAGE::Log_Manager\n";
#endif
}


Log_Manager::~Log_Manager()
{
#ifdef _DEBUG
    std::cout << "MESSAGE::~Log_Manager\n";
#endif

    std::list< LogItem* >::iterator it = m_logitems.begin();
    for( ; it != m_logitems.end(); ++it ){
#ifdef _DEBUG
        std::cout << "url = " << (*it)->url << std::endl;
        std::cout << "newthread = " << (*it)->newthread << std::endl;
        std::cout << "msg = " << (*it)->msg << std::endl;
#endif
        delete *it;
    }
}



const bool Log_Manager::has_items( const std::string& url, const bool newthread )
{
#ifdef _DEBUG
    std::cout << "Log_Manager::has_items url = " << url << " newthread " << newthread << std::endl;
#endif

    if( ! m_logitems.size() ) return false;

    std::list< LogItem* >::iterator it = m_logitems.begin();
    for( ; it != m_logitems.end(); ++it ){

#ifdef _DEBUG
        std::cout << (*it)->url << std::endl;
#endif
        if( newthread != (*it)->newthread ) continue;

        if( (*it)->url == url ) return true;
        if( (*it)->newthread && url.find( (*it)->url ) == 0 ) return true;
    }

#ifdef _DEBUG
        std::cout << "not found" << std::endl;
#endif

    return false;
}


void Log_Manager::remove_items( const std::string& url )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    if( ! m_logitems.size() ) return;

#ifdef _DEBUG
    std::cout << "Log_Manager::remove_items url = " << url << std::endl
              << "size = " << m_logitems.size() << std::endl;
#endif 

    std::list< LogItem* >::iterator it = m_logitems.begin();
    for( ; it != m_logitems.end(); ++it ){

        if( (*it)->url == url
            || ( (*it)->newthread && url.find( (*it)->url ) == 0 )
            ){

            const time_t elapsed = tv.tv_sec - (*it)->time_write;

#ifdef _DEBUG
            std::cout << "elapsed = " << elapsed << std::endl;
#endif 

            // removeフラグが立っているか、時間切れの場合は削除
            if( (*it)->remove || elapsed > WRITE_TIMEOUT ){

#ifdef _DEBUG
                std::cout << "removed url = " << (*it)->url << std::endl;
#endif
                delete (*it);
                m_logitems.erase( it );
                it = m_logitems.begin();
            }
        }
    }

#ifdef _DEBUG
    std::cout << "-> size = " << m_logitems.size() << std::endl;
#endif 
}


//
// messageが自分の書き込んだものかチェックする
//
// newthread == true の時は新スレの>>1のチェック
//
// headsize > 0 の時は先頭の headsize 文字だけを比較
//
const bool Log_Manager::check_write( const std::string& url, const bool newthread, const char* msg, const size_t headsize )
{
    if( ! m_logitems.size() ) return false;

#ifdef _DEBUG
    std::cout << "Log_Manager::check_write url = " << url << " newthread = " << newthread << " headsize = " << headsize << std::endl;
#endif

    std::list< LogItem* >::iterator it = m_logitems.begin();
    for( ; it != m_logitems.end(); ++it ){

        if( (*it)->newthread != newthread ) continue;

        if( ! (*it)->newthread && (*it)->url != url ) continue;
        if( (*it)->newthread && url.find( (*it)->url ) != 0 ) continue;

        // 先頭のheadsize文字だけチェック (空白除く)
        // ヒットしてもremove は true にしない
        if( headsize ){

            bool flag = true;
            size_t i = 0, i2 = 0;
            while( (*it)->head[ i ] != '\0' && i2 < headsize ){

                while( (*it)->head[ i ] == ' ' ) ++i;
                while( msg[ i2 ] == ' ' ) ++i2;
#ifdef _DEBUG
                std::cout << (int)( (*it)->head[ i ] ) << " - " << (int)( msg[ i2 ] ) << std::endl;
#endif
                if( (*it)->head[ i ] != msg[ i2 ] ){
                    flag = false;
                    break;
                }
                ++i;
                ++i2;
            }
            if( ! flag ) continue;

#ifdef _DEBUG
            std::cout << "!! hit (head) !!\n";
#endif
            return true;
        }


        // 全文でチェック
        
        // MISC::replace_str( ..., "\n", " \n" ) しているのは MISC::get_lines 実行時に
        // 改行のみの行を削除しないようにするため
        std::list< std::string > msg_lines = MISC::get_lines( MISC::replace_str( MISC::remove_spaces( msg ), "\n", " \n" ) );

#ifdef _DEBUG
        std::cout << "lines = " << msg_lines.size() << " : " << (*it)->msg_lines.size() << std::endl;
        std::cout << "newthread = " << newthread << " : " << (*it)->newthread << std::endl;
#endif

        if( msg_lines.size() != (*it)->msg_lines.size() ) continue;

        std::list< std::string >::iterator it_msg = msg_lines.begin();
        std::list< std::string >::iterator it_item = (*it)->msg_lines.begin();
        for( ; it_msg != msg_lines.end() ; ++it_msg, ++it_item ){
#ifdef _DEBUG
            std::cout << (*it_msg) << " | " << (*it_item) << std::endl;
#endif
            if( MISC::remove_spaces( (*it_msg) ) != MISC::remove_spaces( (*it_item ) ) ) break;
        }
        if( it_msg != msg_lines.end() ) continue;

#ifdef _DEBUG
        std::cout << "!! hit !!\n";
#endif

        (*it)->remove = true;

        return true;

    }

    return false;
}


//
// 自分の書き込みの判定用データの保存
//
void Log_Manager::push_logitem( const std::string& url, const bool newthread,  const std::string& msg )
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    std::string logurl = url;
    if( newthread && logurl.find( ID_OF_NEWTHREAD ) != std::string::npos ) logurl = logurl.substr( 0, logurl.find( ID_OF_NEWTHREAD ) );
    m_logitems.push_back( new LogItem( logurl, newthread, msg, tv.tv_sec ) );

#ifdef _DEBUG
    std::cout << "Log_Manager::push_logitem\n";
    std::cout << "url = " << logurl << std::endl;
    std::cout << "newthread = " << newthread << std::endl;
    std::cout << "msg = " << msg << std::endl;
#endif
}


//
// ログの保存
//
void Log_Manager::save( const std::string& url,
                        const std::string& subject,  const std::string& msg, const std::string& name, const std::string& mail )
{
#ifdef _DEBUG
    std::cout << "Log_Manager::save\n";
    std::cout << "url = " << url << std::endl;
    std::cout << "subject = " << subject << std::endl;
    std::cout << "msg = " << msg << std::endl;
#endif

    if( ! CONFIG::get_save_postlog() ) return;

    // 実況中の時は保存しない
    if( SESSION::is_live( url ) ) return;

    if( ! CACHE::mkdir_logroot() ) return;

    struct timeval tv;
    struct timezone tz;
    gettimeofday( &tv, &tz );

    // 保存メッセージ作成
    const std::string date = MISC::timettostr( tv.tv_sec );
    const bool include_url = false; // URLを除外して msg をエスケープ

    std::string html = "<hr><br>" + DBTREE::url_readcgi( url, 0, 0 ) + "<br>"
    + "[ " + MISC::html_escape( DBTREE::board_name( url ) ) + " ] "
    + MISC::html_escape( subject ) + "<br>"
    + "名前：" + MISC::html_escape( name ) + " [" + MISC::html_escape( mail ) + "]：" + date + "<br>"
    + MISC::html_escape( msg, include_url ) + "<br><br>";

    html = MISC::replace_str( html, "\n", "<br>" );
    html += "\n";

    // 保存先決定
    // もし保存先ファイルの容量が大きい場合は mv する
    const std::string path = CACHE::path_postlog();
    const size_t filesize = CACHE::get_filesize( path );

#ifdef _DEBUG
    std::cout << path << std::endl;
    std::cout << "size = " << filesize << std::endl;
#endif

    if( filesize > CONFIG::get_maxsize_postlog() ){

        const int maxno = get_max_num_of_log() + 1;
        const std::string newpath = path + "-" + MISC::itostr( maxno );
        CACHE::jdmv( path, newpath );
        chmod( newpath.c_str(), S_IWUSR | S_IRUSR );

#ifdef _DEBUG
        std::cout << "mv to " << newpath << std::endl;
#endif
    }


    // 保存
#ifdef _DEBUG
    std::cout << html << std::endl;
#endif 
    const bool append = true;
    CACHE::save_rawdata( path, html, append );
    chmod( path.c_str(), S_IWUSR | S_IRUSR );
}



//
//　書き込みログ取得
//
const std::string Log_Manager::get_post_log( const int num )
{
    std::string path = CACHE::path_postlog();

    if( num ) path += "-" + MISC::itostr( num );

    std::string html;
    CACHE::load_rawdata( path, html );
    return html;
}


//
// ログファイル( log/postlog-* ) の最大数
//
const int Log_Manager::get_max_num_of_log()
{
#ifdef _DEBUG
    std::cout << "Log_Manager::get_max_num_of_log\n";
#endif

    std::list< std::string > filelist = CACHE::get_filelist( CACHE::path_logroot() );
    if( ! filelist.size() ) return 0;

    const std::string path = CACHE::path_postlog() + "-";

    int maxno = 0;
    std::list< std::string >::iterator it = filelist.begin();
    for( ; it != filelist.end(); ++it ){

        const std::string target = CACHE::path_logroot() + (*it);
#ifdef _DEBUG
        std::cout << target << std::endl;
#endif
        if( target.find( path ) == 0 ){

            const int tmpno = atoi( target.substr( path.length() ).c_str() );
            if( tmpno > maxno ) maxno = tmpno;
#ifdef _DEBUG
            std::cout << "no = " << tmpno << " max = " << maxno << std::endl;
#endif
        }
    }

    return maxno;
}



//
// ログ削除
//
void Log_Manager::clear_post_log()
{
    const int maxno = get_max_num_of_log();

    for( int num = 0; num <= maxno; ++num ){

        std::string path = CACHE::path_postlog();
        if( num ) path += "-" + MISC::itostr( num );

        unlink( path.c_str() );
    }
}
