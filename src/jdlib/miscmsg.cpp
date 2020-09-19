// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "miscmsg.h"

#include <iostream>
#include <ctime>


namespace MISC
{
    int id_err = 0;
    int id_msg = 0;
}


//
// エラー出力
//
void MISC::ERRMSG( const std::string& err )
{
   time_t current;
   time( &current );
   char buf[ 256 ];
   std::string str_date = ctime_r( &current, buf );
   str_date.resize( str_date.length() -1 );
   std::cerr << str_date << " (ER " << id_err << ") : " << err << std::endl;

   ++id_err;
}


//
// メッセージ
//
void MISC::MSG( const std::string& msg )
{
    std::cout << "(MSG " << id_msg << "): " << msg << std::endl;

    ++id_msg;
}
