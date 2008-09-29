// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "controlutil.h"
#include "controlid.h"
#include "controllabel.h"
#include "keysyms.h"
#include "keyconfig.h"
#include "mouseconfig.h"
#include "buttonconfig.h"

#include "cache.h"

#include <cstring>


CONTROL::KeyConfig* instance_keyconfig = NULL;
CONTROL::KeyConfig* instance_keyconfig_bkup = NULL;

CONTROL::KeyConfig* CONTROL::get_keyconfig()
{
    if( ! instance_keyconfig ) instance_keyconfig = new CONTROL::KeyConfig();

    return instance_keyconfig;
}


void CONTROL::delete_keyconfig()
{
    if( instance_keyconfig ) delete instance_keyconfig;
    instance_keyconfig = NULL;

    if( instance_keyconfig_bkup ) delete instance_keyconfig_bkup;
    instance_keyconfig_bkup = NULL;
}


void CONTROL::bkup_keyconfig()
{
    if( ! instance_keyconfig_bkup ) instance_keyconfig_bkup = new CONTROL::KeyConfig();
    *instance_keyconfig_bkup = * instance_keyconfig;
}

void CONTROL::restore_keyconfig()
{
    if( ! instance_keyconfig_bkup ) return;
    *instance_keyconfig = * instance_keyconfig_bkup;
}


void CONTROL::load_conf()
{
    CONTROL::get_keyconfig()->load_conf();
    CONTROL::get_mouseconfig()->load_conf();
    CONTROL::get_buttonconfig()->load_conf();
}


void CONTROL::save_conf()
{
    CONTROL::get_keyconfig()->save_conf( CACHE::path_keyconf() );
    CONTROL::get_mouseconfig()->save_conf( CACHE::path_mouseconf() );
    CONTROL::get_buttonconfig()->save_conf( CACHE::path_buttonconf() );
}


// keysymはアスキー文字か
const bool CONTROL::is_ascii( const guint keysym )
{
    if( keysym > 32 && keysym < 127 ) return true;

    return false;
}


// メニューにショートカットキーやマウスジェスチャを表示
void CONTROL::set_menu_motion( Gtk::Menu* menu )
{
    if( !menu ) return;

    Gtk::Menu_Helpers::MenuList& items = menu->items();
    Gtk::Menu_Helpers::MenuList::iterator it_item = items.begin();
    for( ; it_item != items.end(); ++it_item ){

        // menuitemの中の名前を読み込んで ID を取得し、CONTROL::Noneでなかったら
        // ラベルを置き換える
        Gtk::Label* label = dynamic_cast< Gtk::Label* >( (*it_item).get_child() );
        if( label ){
#ifdef _DEBUG
            std::cout << label->get_text() << std::endl;
#endif
            int id = CONTROL::get_id( label->get_text() );
            if( id != CONTROL::None ){

                std::string str_label = CONTROL::get_label_with_mnemonic( id );
                std::string str_motions = CONTROL::get_str_motions( id );

                ( *it_item ).remove();
                Gtk::Label *label = Gtk::manage( new Gtk::Label( str_label + ( str_motions.empty() ? "" : "  " ), true ) );
                Gtk::Label *label_motion = Gtk::manage( new Gtk::Label( str_motions ) );
                Gtk::HBox *box = Gtk::manage( new Gtk::HBox() );

                box->pack_start( *label, Gtk::PACK_SHRINK );
                box->pack_end( *label_motion, Gtk::PACK_SHRINK );
                (*it_item).add( *box );
                box->show_all();
            }
        }

        if( (*it_item).has_submenu() ) CONTROL::set_menu_motion( (*it_item).get_submenu() );
    }
}


// IDからモードを取得
// 例えば id == CONTROL::Up の時は CONTROL::COMMONMOTION を返す
const int CONTROL::get_mode( const int id )
{
    if( id < CONTROL::COMMONMOTION_END ) return CONTROL::MODE_COMMON;
    if( id < CONTROL::BBSLISTMOTION_END ) return CONTROL::MODE_BBSLIST;
    if( id < CONTROL::BOARDMOTION_END ) return CONTROL::MODE_BOARD;
    if( id < CONTROL::ARTICLEMOTION_END ) return CONTROL::MODE_ARTICLE;
    if( id < CONTROL::IMAGEICONMOTION_END ) return CONTROL::MODE_IMAGEICON;
    if( id < CONTROL::IMAGEVIEWMOTION_END ) return CONTROL::MODE_IMAGEVIEW;
    if( id < CONTROL::MESSAGEMOTION_END ) return CONTROL::MODE_MESSAGE;
    if( id < CONTROL::EDITMOTION_END ) return CONTROL::MODE_EDIT;

    return CONTROL::MODE_ERROR;
}


// キー名からkeysymを取得
// 例えば keyname == "Space" の時は GDK_space を返す
const guint CONTROL::get_keysym( const std::string& keyname )
{
#ifdef _DEBUG
    std::cout << "CONTROL::get_keysym name = " << keyname;
#endif

    if( keyname.empty() ) return 0;

    for( size_t i = 0; i < sizeof( CONTROL::keysyms ) / sizeof( KEYSYMS ); ++i ){

        if( CONTROL::keysyms[ i ].keyname == keyname ){
#ifdef _DEBUG
            std::cout << " found sym = " << CONTROL::keysyms[ i ].keysym << std::endl;
#endif
            return CONTROL::keysyms[ i ].keysym;
        }
    }

    // データベース内に見つからなかったらアスキー文字を返す

#ifdef _DEBUG
    std::cout << " not found sym = " << ( guint )keyname[ 0 ] << std::endl;
#endif

    return keyname[ 0 ];
}


// keysymからキー名を取得
// 例えば keysym == GDK_space の時は "Space"  を返す
const std::string CONTROL::get_keyname( const guint keysym )
{
#ifdef _DEBUG
    std::cout << "CONTROL::get_keyname sym = " << keysym;
#endif

    for( size_t i = 0; i < sizeof( CONTROL::keysyms ) / sizeof( KEYSYMS ); ++i ){

        if( CONTROL::keysyms[ i ].keysym == keysym ){
#ifdef _DEBUG
            std::cout << " found name = " << CONTROL::keysyms[ i ].keyname << std::endl;
#endif
            return CONTROL::keysyms[ i ].keyname;
        }
    }

#ifdef _DEBUG
    std::cout << " not found\n";
#endif

    // データベース内に見つからなかったらアスキー文字を返す
    if( CONTROL::is_ascii( keysym ) ){
        char c[ 2 ];
        c[ 0 ] = keysym;
        c[ 1 ] = '\0';
        return std::string( c );
    }

    return std::string();
}



// 操作名からID取得
// 例えば name == "Up" の時は CONTROL::Up を返す
const int CONTROL::get_id( const std::string& name )
{
    for( int id = CONTROL::COMMONMOTION; id < CONTROL::CONTROL_END; ++id ){
        if( name == CONTROL::control_label[ id ][0] ) return id;
    }

    return CONTROL::None;
}


// IDから操作名取得
// 例えば id == CONTROL::Up の時は "Up" を返す
const std::string CONTROL::get_name( const int id )
{
    if( id < CONTROL::COMMONMOTION || id >=  CONTROL::CONTROL_END ) return std::string();

    return CONTROL::control_label[ id ][0];
}


// IDからラベル取得
// 例えば id == CONTROL::Up の時は "上移動" を返す
const std::string CONTROL::get_label( const int id )
{
    if( id < CONTROL::COMMONMOTION || id >=  CONTROL::CONTROL_END ) return std::string();

    return CONTROL::control_label[ id ][ 1 ];
}


// IDからキーボード操作を取得
const std::string CONTROL::get_str_keymotions( const int id )
{
    return CONTROL::get_keyconfig()->get_str_motions( id );
}


// IDからデフォルトキーボード操作を取得
const std::string CONTROL::get_default_keymotions( const int id )
{
    return CONTROL::get_keyconfig()->get_default_motions( id );
}


// スペースで区切られた複数のキーボード操作をデータベースに登録
void CONTROL::set_keymotions( const std::string& name, const std::string& str_motions )
{
    CONTROL::get_keyconfig()->set_motions( name, str_motions );
}


// 指定したIDのキーボード操作を全て削除
const bool CONTROL::remove_keymotions( const int id )
{
    return CONTROL::get_keyconfig()->remove_motions( id );
}


// IDからマウス操作を取得
const std::string CONTROL::get_str_mousemotions( const int id )
{
    return CONTROL::get_mouseconfig()->get_str_motions( id );
}


// IDからキーボードとマウス操作の両方を取得
const std::string CONTROL::get_str_motions( const int id )
{
    std::string str_motion = get_str_keymotions( id );
    std::string mouse_motion = get_str_mousemotions( id );
    if( ! mouse_motion.empty() ){
        if( !str_motion.empty() ) str_motion += " ";
        str_motion += "( " + mouse_motion + " )";
    }

    return str_motion;
}



// IDからラベルと操作の両方を取得
const std::string CONTROL::get_label_motions( const int id )
{
    std::string motion = CONTROL::get_str_motions( id );
    return CONTROL::get_label( id ) + ( motion.empty() ? "" :  "  " ) + motion;
}



// IDからショートカットを付けたラベルを取得
// 例えば id == CONTROL::Save の時は "名前を付けて保存(_S)..." を返す
const std::string CONTROL::get_label_with_mnemonic( const int id )
{
    unsigned int pos;

    std::string label = CONTROL::get_label ( id );
    pos = label.find( "...", 0);
    if ( pos != std::string::npos )
    {
        switch ( id )
        {
            case CONTROL::Save:         //名前を付けて保存...
                label.replace( pos, strlen( "..." ), "(_S)..." );
                break;
	
            case CONTROL::Property:     //プロパティ...
                label.replace( pos, strlen( "..." ), "(_P)..." );
                break;
        }
    }
    else
    {
        switch ( id )
        {
            case CONTROL::PreBookMark:  //前のブックマークへ移動
                label += "(_R)";
                break;

            case CONTROL::NextBookMark: //次のブックマークへ移動
                label += "(_X)";
                break;

            case CONTROL::PrevView:     //前へ戻る
                label += "(_P)";
                break;

            case CONTROL::NextView:     //次へ進む
                label += "(_N)";
                break;

            case CONTROL::Home:         //先頭へ移動
                label += "(_H)";
                break;

            case CONTROL::End:          //最後へ移動
                label += "(_E)";
                break;

            case CONTROL::Quit:         //閉じる
                label += "(_C)";
                break;

            case CONTROL::Delete:       //削除
                label += "(_D)";
                break;

            case CONTROL::Reload:       //再読み込み
                label += "(_R)";
                break;

            case CONTROL::StopLoading:  //読み込み中止
                label += "(_T)";
                break;

            case CONTROL::Copy:         //コピー
                label += "(_C)";
                break;

            case CONTROL::AppendFavorite:   //お気に入りに追加
                label += "(_A)";
                break;

            case CONTROL::Search:       //検索
                label += "(_S)";
                break;

            case CONTROL::SearchNext:   //次検索
                label += "(_N)";
                break;

            case CONTROL::SearchPrev:   //前検索
                label += "(_P)";
                break;

            case CONTROL::GotoNew:      //新着へ移動
                label += "(_W)";
                break;

            case CONTROL::LiveStartStop:  // 実況
                label += "(_L)";
                break;

            case CONTROL::CancelMosaic: //モザイク解除
                label += "(_M)";
                break;

            case CONTROL::ZoomFitImage: //画面に画像サイズを合わせる
                label += "(_A)";
                break;

            case CONTROL::ZoomInImage:  //ズームイン
                label += "(_I)";
                break;

            case CONTROL::ZoomOutImage: //ズームアウト
                label += "(_Z)";
                break;

            case CONTROL::OrgSizeImage: //元の画像サイズ
                label += "(_N)";
                break;
        }
    }

    return label;
}


// キーボード操作が重複していないか
const int CONTROL::check_key_conflict( const int mode, const std::string& str_motion )
{
    return CONTROL::get_keyconfig()->check_conflict( mode, str_motion );
}

const int CONTROL::check_key_conflict( const int mode,
                                       const guint keysym, const bool ctrl, const bool shift, const bool alt )
{
    return CONTROL::get_keyconfig()->check_conflict( mode, keysym, ctrl, shift, alt, false, false );
}
