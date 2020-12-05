// License: GPL2

#include "tools.h"

#include "icons/iconmanager.h"
#include "type.h"


//
// TYPE_ID から要素名を取得( TYPE_ID は global.h を参照 )
//
std::string XML::get_name( const int type_id )
{
    std::string name;

    switch( type_id )
    {
        case TYPE_DIR: // サブディレクトリ
            name = "subdir";
            break;

        case TYPE_BOARD: // 板
            name = "board";
            break;
            
        case TYPE_BOARD_UPDATE: // 更新可能板
            name = "board_update";
            break;

        case TYPE_THREAD: // スレ
            name = "thread";
            break;

        case TYPE_THREAD_UPDATE: // 更新可能スレ
            name = "thread_update";
            break;

        case TYPE_THREAD_OLD: // dat落ちスレ
            name = "thread_old";
            break;

        case TYPE_IMAGE: // 画像
            name = "image";
            break;

        case TYPE_COMMENT: // コメント
            name = "comment";
            break;

        case TYPE_LINK: // リンク
            name = "link";
            break;

        case TYPE_VBOARD: // 仮想板
            name = "vboard";
            break;

        case TYPE_AA: // AA
            name = "aa";
            break;

        case TYPE_HISTITEM: // HISTORY::ViewHistoryItem
            name = "histitem";
            break;

        case TYPE_USRCMD:
            name = "usrcmd";
            break;

        case TYPE_LINKFILTER:
            name = "linkfilter";
            break;

        case TYPE_REPLACESTR:
            name = "replacestr";
            break;

        case TYPE_SEPARATOR:
            name = "separator";
            break;
    }

    return name;
}


//
// 要素名から TYPE_ID を取得
//
int XML::get_type( const std::string& node_name )
{
    int type = TYPE_UNKNOWN;

    if( node_name == "board" )
    {
        type = TYPE_BOARD;
    }
    else if( node_name == "board_update" )
    {
        type = TYPE_BOARD_UPDATE;
    }
    else if( node_name == "comment" )
    {
        type = TYPE_COMMENT;
    }
    else if( node_name == "image" )
    {
        type = TYPE_IMAGE;
    }
    else if( node_name == "link" )
    {
        type = TYPE_LINK;
    }
    else if( node_name == "vboard" )
    {
        type = TYPE_VBOARD;
    }
    else if( node_name == "subdir" )
    {
        type = TYPE_DIR;
    }
    else if( node_name == "thread" )
    {
        type = TYPE_THREAD;
    }
    else if( node_name == "thread_update" )
    {
        type = TYPE_THREAD_UPDATE;
    }
    else if( node_name == "thread_old" )
    {
        type = TYPE_THREAD_OLD;
    }
    else if( node_name == "aa" )
    {
        type = TYPE_AA;
    }
    else if( node_name == "histitem" )
    {
        type = TYPE_HISTITEM;
    }
    else if( node_name == "usrcmd" )
    {
        type = TYPE_USRCMD;
    }
    else if( node_name == "linkfilter" )
    {
        type = TYPE_LINKFILTER;
    }
    else if( node_name == "replacestr" )
    {
        type = TYPE_REPLACESTR;
    }
    else if( node_name == "separator" )
    {
        type = TYPE_SEPARATOR;
    }

    return type;
}


//
// TYPE_ID からアイコンを取得 ( アイコンは icons/iconmanager.h を参照 )
//
Glib::RefPtr< Gdk::Pixbuf > XML::get_icon( const int type_id )
{
	Glib::RefPtr< Gdk::Pixbuf > icon = ICON::get_icon( ICON::TRANSPARENT );

    switch( type_id )
    {
        case TYPE_DIR:
            icon = ICON::get_icon( ICON::DIR );
            break;

        case TYPE_BOARD:
            icon = ICON::get_icon( ICON::BOARD );
            break;

        case TYPE_BOARD_UPDATE:
            icon = ICON::get_icon( ICON::BOARD_UPDATE );
            break;

        case TYPE_THREAD:
            icon = ICON::get_icon( ICON::THREAD );
            break;

        case TYPE_THREAD_UPDATE:
            icon = ICON::get_icon( ICON::THREAD_UPDATE );
            break;

        case TYPE_THREAD_OLD:
            icon = ICON::get_icon( ICON::THREAD_OLD );
            break;

        case TYPE_IMAGE:
            icon = ICON::get_icon( ICON::IMAGE );
            break;

        case TYPE_LINK:
            icon = ICON::get_icon( ICON::LINK );
            break;

        case TYPE_VBOARD:
            icon = ICON::get_icon( ICON::BOARD );
            break;

        case TYPE_USRCMD:
            icon = ICON::get_icon( ICON::THREAD );
            break;
    }

    return icon;
}
