// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "xml.h"
#include "global.h"

#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"

// 行のタイプ判定
int XML::get_type( const std::string& line, std::string& url,  std::string& name )
{
    JDLIB::Regex regex;

    int type = TYPE_UNKNOWN;

    for(;;){

        if( regex.exec( "<thread( +url=\"(.*)\")? +name=\"(.*)\" */>", line, 0, true ) ){
            type = TYPE_THREAD;
            break;
        }
        if( regex.exec( "<board( +url=\"(.*)\")? +name=\"(.*)\" */>", line, 0, true ) ){
            type = TYPE_BOARD;
            break;
        }
        if( regex.exec( "<subdir( +url=\"(.*)\")? +name=\"(.*)\" *>", line, 0, true ) ){
            type = TYPE_DIR;
            break;
        }
        if( regex.exec( "<image( +url=\"(.*)\")? +name=\"(.*)\" */>", line, 0, true ) ){
            type = TYPE_IMAGE;
            break;
        }
        if( regex.exec( "<comment( +url=\"(.*)\")? +name=\"(.*)\" */>", line, 0, true ) ){
            type = TYPE_COMMENT;
            break;
        }
        if( regex.exec( "<link( +url=\"(.*)\")? +name=\"(.*)\" */>", line, 0, true ) ){
            type = TYPE_LINK;
            break;
        }

#ifdef _DEBUG
        std::cout << "XML::get_type failed : " << line << std::endl;
#endif
        return type;
    }

    url = regex.str( 2 );
    name =  MISC::recover_quot( regex.str( 3 ) );

#ifdef _DEBUG
    std::cout << url << " " << name << std::endl;
#endif

    return type;
}
