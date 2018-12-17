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
    : m_file( file ),
      m_broken( false )
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



bool ConfLoader::empty()
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

// 値を変更 (bool型)
void ConfLoader::update( const std::string& name, const bool value )
{
    std::string str_value = value ? "1" : "0";

    update( name, str_value );
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
#ifdef _WIN32
    // not support to 'l' flag, error occurred if using
    snprintf( str_value, buflng, "%f", value );
#else
    snprintf( str_value, buflng, "%lf", value );
#endif
    update( name, std::string( str_value ) );
}



//
// string 型
//
// dflt はデフォルト値, デフォルト引数 maxlength = 0
std::string ConfLoader::get_option_str( const std::string& name, const std::string& dflt, const size_t maxlength )
{
    if( name.empty() ) return std::string();

    std::list < ConfData >::iterator it = m_data.begin();
    for( ; it != m_data.end(); ++it )
    {
        if( (*it).name == name )
        {
            // maxlengthが設定されている場合は文字数制限をする
            if( maxlength > 0 && (*it).value.length() > maxlength )
            {
                m_broken = true;
#ifdef _DEBUG
    std::cout << "ConfLoader::get_option_str: " << name << "=" << (*it).value << std::endl;
#endif
                break;
            }

            return (*it).value;
        }
    }

    return dflt;
}

//
// bool型
//
bool ConfLoader::get_option_bool( const std::string& name, const bool dflt )
{
    std::string val_str = get_option_str( name, std::string() );

    if( val_str.empty() ) return dflt;

    if( val_str == "1" ) return true;
    if( val_str == "0" ) return false;

    val_str = MISC::toupper_str( val_str );

    if( val_str == "TRUE" || val_str == "T" ) return true;
    if( val_str == "FALSE" || val_str == "F" ) return false;

    m_broken = true;

#ifdef _DEBUG
    std::cout << "ConfLoader::get_option_bool: " << name << "=" << val_str << std::endl;
#endif

    return dflt;
}

//
// int 型
//
int ConfLoader::get_option_int( const std::string& name, const int dflt, const int min, const int max )
{
    std::string val_str = get_option_str( name, std::string() );

    if( val_str.empty() ) return dflt;

    int val_int = atoi( val_str.c_str() );

    if( val_int < min || val_int > max )
    {
        val_int = dflt;
        m_broken = true;
#ifdef _DEBUG
    std::cout << "ConfLoader::get_option_int: " << name << "=" << val_int << std::endl;
#endif
    }

    return val_int;
}


//
// double 型
//
double ConfLoader::get_option_double( const std::string& name, const double dflt, const double min, const double max )
{
    std::string val_str = get_option_str( name, std::string() );

    if( val_str.empty() ) return dflt;

    double val_double = atof( val_str.c_str() );

    if( val_double < min || val_double > max )
    {
        val_double = dflt;
        m_broken = true;
#ifdef _DEBUG
    std::cout << "ConfLoader::get_option_double: " << name << "=" << val_double << std::endl;
#endif
    }

    return val_double;
}
