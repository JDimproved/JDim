// AA 管理クラス

//#define _DEBUG
#include "jddebug.h"
#include "aamanager.h"

#include "jdlib/miscutil.h"

#include "cache.h"

CORE::AAManager* instance_aamanager = NULL;

CORE::AAManager* CORE::get_aamanager()
{
    if( ! instance_aamanager ) instance_aamanager = new CORE::AAManager();
    return instance_aamanager;
}


void CORE::delete_aamanager()
{
    if( instance_aamanager ) delete instance_aamanager;
    instance_aamanager = NULL;
}


////////////////////////////////////


#define AA_LIMIT 4096

using namespace CORE;

AAManager::AAManager()
    : m_save( false )
{
#ifdef _DEBUG
    std::cout << "AAManager::AAManager\n";
#endif

    // ラベル読み込み
    std::string aa_lines;
    if( CACHE::load_rawdata( CACHE::path_aalist(), aa_lines ) ){

        m_list_label = MISC::get_lines( aa_lines );
        m_list_label = MISC::remove_nullline_from_list( m_list_label );

        std::list< std::string >::iterator it = m_list_label.begin();
        for( ; it != m_list_label.end() ; ++it )
        {
            std::string asciiart = *it;

#ifdef _DEBUG
            std::cout << "label = " << asciiart << std::endl;
#endif

            // 先頭に"**"がある場合は、"*"を一つ取り除いて一行AAとして扱う
            if( asciiart.find( "**", 0 ) == 0 )
            {
                // **example -> *example
                asciiart.erase( 0, 1 );
            }
            // "*"が一つだけある場合は、"*"を取り除いてファイル名とみなす
            else if( asciiart.find( "*", 0 ) == 0 )
            {
                // *example -> example
                asciiart.erase( 0, 1 );

                // example -> .jd/aa/example
                std::string aafile_path = CACHE::path_aadir().append( asciiart );

#ifdef _DEBUG
                std::cout << "load : " << aafile_path << std::endl;
#endif

                // ファイルが存在しなければ".txt"を追加( .jd/aa/example -> .jd/aa/example.txt )
                if( CACHE::file_exists( aafile_path ) != CACHE::EXIST_FILE ) aafile_path.append( ".txt" );
                if( CACHE::file_exists( aafile_path ) != CACHE::EXIST_FILE ) asciiart = aafile_path + " が存在しません";
                else if( CACHE::get_filesize( aafile_path ) > AA_LIMIT ) asciiart = "ファイルサイズが大きすぎます";
                else CACHE::load_rawdata( aafile_path, asciiart );
            }

#ifdef _DEBUG
            std::cout << "AA : " << asciiart << std::endl;
#endif
            m_list_aa.push_back( asciiart );
        }
    }
}


AAManager::~AAManager()
{
#ifdef _DEBUG
    std::cout << "AAManager::~AAManager\n";
#endif

    // ラベル保存
    if( m_save ){

#ifdef _DEBUG
        std::cout << "save\n";
#endif

    }
}


// num 行目を先頭に移動
void AAManager::move_to_top( int num )
{
    if( num > 0 && num < ( int ) m_list_aa.size() ){

        std::list< std::string >::iterator it = m_list_label.begin();
        std::list< std::string >::iterator itaa = m_list_aa.begin();
        for( int i = 0; i < num && it != m_list_aa.end() ; ++i, ++it, ++itaa );

        std::string tmpstr = *it;
        m_list_label.remove( tmpstr );
        m_list_label.push_front( tmpstr );

        tmpstr = *itaa;
        m_list_aa.remove( tmpstr );
        m_list_aa.push_front( tmpstr );

        m_save = true;
    }
}


const std::string& AAManager::get_aa( int num )
{
    std::list< std::string >::iterator it = m_list_aa.begin();
    for( int i = 0; i < num && it != m_list_aa.end() ; ++i, ++it );
    return *it;
}
