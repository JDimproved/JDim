// ライセンス: GPL2

//
// Thanks to 「テスト運用中」スレの18氏
//
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1149945056/18
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MIGEMO_H

//#define _DEBUG
#include "jddebug.h"

#include "jdmigemo.h"

#include <migemo.h>


static migemo* migemo_object{};


std::string jdmigemo::convert(const char* input)
{
    if(! migemo_is_enable(migemo_object)) return {};

    std::basic_string<unsigned char> query;
    while( *input != '\n' && *input != '\0' ) {
        query.push_back( static_cast<unsigned char>( *input ) );
        ++input;
    }

    unsigned char* result = migemo_query(migemo_object, query.c_str());
    std::string output = reinterpret_cast<const char*>( result );
    migemo_release(migemo_object, result);

#ifdef _DEBUG
    std::cout << "migemo converted: " << output << std::endl;
#endif
    return output;
}


bool jdmigemo::init(const std::string& filename)
{
    if(migemo_is_enable(migemo_object)) return true;

#ifdef _DEBUG
    std::cout << "migemo-dict: " << filename << std::endl;
#endif
    migemo_object = migemo_open(filename.c_str());
    if(migemo_is_enable(migemo_object)) {
        return true;
    }
    else {
        migemo_close(migemo_object);
        migemo_object = nullptr;
        return false;
    }
}


void jdmigemo::close()
{
    migemo_close(migemo_object);
    migemo_object = nullptr;
}

#endif // HAVE_MIGEMO_H
