/* グローバルな定数やstructなど */

#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <string>


// msec  内部クロックの周期
#define TIMER_TIMEOUT 50  

// 最大表示可能レス数
#define MAX_RESNUMBER 11000


// マウスジェスチャの最大ストローク
#define MAX_MG_LNG 5


// 画像アイコンの大きさ
#define ICON_SIZE 32 


//
// str をクリップボードにコピー
//
#define COPYCLIP( str ) do{  \
 Glib::RefPtr< Gtk::Clipboard > clip = Gtk::Clipboard::get(); \
 clip->set_text( str ); \
 clip = Gtk::Clipboard::get( GDK_SELECTION_PRIMARY ); \
 clip->set_text( str ); \
} while( false )


// スレッド状態( or を取る)
enum
{
    STATUS_UNKNOWN = 0,  // 不明
    STATUS_NORMAL = 1,   // 通常
    STATUS_OLD = 2,      // DAT落ち or 板が移転した
    STATUS_BROKEN = 4,   // あぼーんなどで壊れている
};


// CoreやAdminクラスで使うコマンド構造体
struct COMMAND_ARGS
{
    std::string command;
    std::string url;
    std::string arg1;
    std::string arg2;
    std::string arg3;
    std::string arg4;
    std::string arg5;
    std::string arg6;
};


// プロトコル
#define PROTO_ANCHORE "anc://"
#define PROTO_RES "res://"
#define PROTO_ID "ID://"
#define PROTO_BE "BE://"


// 仮想 URL

#define URL_LOGIN2CH "jdlogin://login2ch"

#define URL_BBSLISTADMIN "jdadmin://bbslist"
#define URL_BBSLISTVIEW "jdview://bbslist"
#define URL_FAVORITEVIEW "jdview://favorite"
#define URL_ETCVIEW "jdview://etc"

#define URL_BOARDADMIN "jdadmin://board"

#define URL_ARTICLEADMIN "jdadmin://article"

#define URL_IMAGEADMIN "jdadmin://image"

#define URL_MESSAGEADMIN "jdadmin://message"

// フォントID
enum
{
    FONT_MAIN = 0,
    FONT_POPUP,
    FONT_NUM
};


// タイプ
enum 
{
    // 板のタイプ
    TYPE_BOARD_2CH = 0,
    TYPE_BOARD_2CH_COMPATI,  // 2ch 互換
    TYPE_BOARD_JBBS,         // したらば
    TYPE_BOARD_MACHI,        // まち
    TYPE_BOARD_UNKNOWN,

    // その他一般的なデータタイプ
    TYPE_BOARD,
    TYPE_THREAD,
    TYPE_IMAGE,
    TYPE_DIR,
    TYPE_DIR_END, // お気に入りの追加の時にサブディレクトリの終了の意味で使う
    TYPE_COMMENT,
    TYPE_LINK,

    TYPE_UNKNOWN
};



#endif
