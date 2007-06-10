// License: GPL2

#include "tools.h"

#include "icons/iconmanager.h"
#include "global.h"


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
            
        case TYPE_THREAD: // スレ
            name = "thread";
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

        case TYPE_AA: // AA
            name = "aa";
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
    else if( node_name == "subdir" )
    {
        type = TYPE_DIR;
    }
    else if( node_name == "thread" )
    {
        type = TYPE_THREAD;
    }
    else if( node_name == "aa" )
    {
        type = TYPE_AA;
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

        case TYPE_THREAD:
            icon = ICON::get_icon( ICON::THREAD );
            break;

        case TYPE_IMAGE:
            icon = ICON::get_icon( ICON::IMAGE );
            break;

        case TYPE_LINK:
            icon = ICON::get_icon( ICON::LINK );
            break;
    }

    return icon;
}
