// ライセンス: 最新のGPL

//
// XML関係のマクロとか
//

#ifndef _XML_H
#define _XML_H

#include <string>

#define XML_MAKE_DIR(name) do{ \
        xml << "<subdir name=\"" << MISC::replace_quot( name ) << "\" >\n"; }while(0)

#define XML_MAKE_BOARD(url,name) do{ \
        xml << "<board url=\"" << url << "\" name=\"" << MISC::replace_quot( name ) <<  "\" />\n"; }while(0)

#define XML_MAKE_THREAD(url,name) do{ \
        xml << "<thread url=\"" << url  << "\" name=\"" << MISC::replace_quot( name ) <<  "\" />\n"; }while(0)

#define XML_MAKE_IMAGE(url,name) do{ \
        xml << "<image url=\"" << url << "\" name=\"" << MISC::replace_quot( name ) <<  "\" />\n"; }while(0)

#define XML_MAKE_COMMENT(name) do{ \
        xml << "<comment name=\"" << MISC::replace_quot( name ) <<  "\" />\n"; }while(0)

#define XML_MAKE_LINK(url,name) do{ \
        xml << "<link url=\"" << url << "\" name=\"" << MISC::replace_quot( name ) <<  "\" />\n"; }while(0)

namespace XML
{
    // 行のパースとタイプ判定
    // url, name に値が入る。戻り値はタイプ
    int get_type( const std::string& line, std::string& url,  std::string& name );
}


#endif
