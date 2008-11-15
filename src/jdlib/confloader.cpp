// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "confloader.h"
#include "miscutil.h"
#include "cache.h"

#include <cstdio>
#include <cstdlib>

using namespace JDLIB;

//
// file : 設定ファイル
// str_conf : 設定文字列
//
// もしstr_confがemptyの時はfileから読み込む
//
ConfLoader::ConfLoader( const std::string& file, std::string str_conf )
    : m_file( file )
{
    if( str_conf.empty() ) CACHE::load_rawdata( m_file, str_conf );

#ifdef _DEBUG
    std::cout << "ConfLoader::ConfLoader " << m_file << std::endl;
    std::cout << str_conf << std::endl;
#endif

    // 行ごとに分割してConfDataに登録
    if( ! str_conf.empty() ){

        std::list< std::string > lines = MISC::get_lines( str_conf );
        if( lines.size() == 0 ) return;

        std::list < std::string >::iterator it = lines.begin();
        for( ; it != lines.end(); ++it ){

            std::string line = MISC::remove_space( ( *it ) );

            size_t i = line.find( "=" );
            if( i != std::string::npos ){

                ConfData data;
                data.name = MISC::remove_space( line.substr( 0, i ) );
                data.value = MISC::remove_space( line.substr( i +1 ) );
                m_data.push_back( data );
#ifdef _DEBUG
                std::cout << data.name << " = " << data.value << std::endl;
#endif 
            }
        }
    }
}



const bool ConfLoader::empty()
{
    return !( m_data.size() );
}



// 保存
void ConfLoader::save()
{
    if( m_file.empty() ) return;

    std::string str_conf;

    std::list < ConfData >::iterator it = m_data.begin();
    for( ; it != m_data.end(); ++it ) str_conf += (*it).name + " = " + (*it).value + "\n";

#ifdef _DEBUG
    std::cout << "ConfLoader::save " << m_file << std::endl;
    std::cout << str_conf << std::endl;    
#endif

    if( !str_conf.empty() ) CACHE::save_rawdata( m_file, str_conf );
}



// 値を変更 (string型)
// name が無い場合は綱目を追加
void ConfLoader::update( const std::string& name, const std::string& value )
{
    if( name.empty() ) return;

    std::list < ConfData >::iterator it = m_data.begin();
    for( ; it != m_data.end(); ++it ){
        if( (*it).name == name ){
            (*it).value = value;
            return;
        }
    }

    // 追加
    ConfData data;
    data.name = name;
    data.value = value;
    m_data.push_back( data );
}

// 値を変更 (int型)
void ConfLoader::update( const std::string& name, const int value )
{
    const int buflng = 256;
    char str_value[ buflng ];
    snprintf( str_value, buflng, "%d", value );
    update( name, std::string( str_value ) );
}


// 値を変更 (double型)
void ConfLoader::update( const std::string& name, const double value )
{
    const int buflng = 256;
    char str_value[ buflng ];
    snprintf( str_value, buflng, "%lf", value );
    update( name, std::string( str_value ) );
}



//
// string 型
//
// dflt はデフォルト値
//
std::string ConfLoader::get_option( const std::string& name, std::string dflt )
{
    if( name.empty() ) return std::string();

    std::list < ConfData >::iterator it = m_data.begin();
    for( ; it != m_data.end(); ++it ){
        if( (*it).name == name ) return (*it).value;
    }

    return dflt;
}



//
// int 型
//
int ConfLoader::get_option( const std::string& name, int dflt )
{
    std::string val = get_option( name, std::string() );

    if( val.empty() ) return dflt;

    val = MISC::toupper_str( val );
    
    if( val == "TRUE" || val == "T" || val == "true" || val == "t" ) return 1;
    if( val == "FALSE" || val == "F" ||  val == "false"|| val == "f" ) return 0;
    
    return atoi( val.c_str() );
}


//
// double 型
//
double ConfLoader::get_option( const std::string& name, double dflt )
{
    std::string val = get_option( name, std::string() );

    if( val.empty() ) return dflt;

    return atof( val.c_str() );
}
