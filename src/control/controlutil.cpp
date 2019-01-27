// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "controlutil.h"
#include "get_config.h"
#include "controlid.h"
#include "controllabel.h"
#include "keysyms.h"

#include "keyconfig.h"
#include "mouseconfig.h"
#include "buttonconfig.h"

#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "skeleton/admin.h"

#include "cache.h"
#include "command.h"

#include <cstring>


CONTROL::KeyConfig* instance_keyconfig = NULL;
CONTROL::KeyConfig* instance_keyconfig_bkup = NULL;


CONTROL::MouseConfig* instance_mouseconfig = NULL;
CONTROL::MouseConfig* instance_mouseconfig_bkup = NULL;


CONTROL::ButtonConfig* instance_buttonconfig = NULL;
CONTROL::ButtonConfig* instance_buttonconfig_bkup = NULL;


//////////////////////////////////////////////////////////


CONTROL::KeyConfig* CONTROL::get_keyconfig()
{
    if( ! instance_keyconfig ){

        instance_keyconfig = new CONTROL::KeyConfig();

        // ラベルをセットしておく
        strncpy( control_label[ CONTROL::SearchWeb ][ 1 ], CONFIG::get_menu_search_web().c_str(), MAX_CONTROL_LABEL );
        strncpy( control_label[ CONTROL::SearchTitle ][ 1 ], CONFIG::get_menu_search_title().c_str(), MAX_CONTROL_LABEL );
    }

    return instance_keyconfig;
}


void CONTROL::delete_keyconfig()
{
    if( instance_keyconfig ) delete instance_keyconfig;
    instance_keyconfig = NULL;

    if( instance_keyconfig_bkup ) delete instance_keyconfig_bkup;
    instance_keyconfig_bkup = NULL;
}


CONTROL::MouseConfig* CONTROL::get_mouseconfig()
{
    if( ! instance_mouseconfig ) instance_mouseconfig = new CONTROL::MouseConfig();

    return instance_mouseconfig;
}


void CONTROL::delete_mouseconfig()
{
    if( instance_mouseconfig ) delete instance_mouseconfig;
    instance_mouseconfig = NULL;

    if( instance_mouseconfig_bkup ) delete instance_mouseconfig_bkup;
    instance_mouseconfig_bkup = NULL;
}


CONTROL::ButtonConfig* CONTROL::get_buttonconfig()
{
    if( ! instance_buttonconfig ) instance_buttonconfig = new CONTROL::ButtonConfig();

    return instance_buttonconfig;
}


void CONTROL::delete_buttonconfig()
{
    if( instance_buttonconfig ) delete instance_buttonconfig;
    instance_buttonconfig = NULL;

    if( instance_buttonconfig_bkup ) delete instance_buttonconfig_bkup;
    instance_buttonconfig_bkup = NULL;
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


void CONTROL::delete_conf()
{
    CONTROL::delete_keyconfig();
    CONTROL::delete_mouseconfig();
    CONTROL::delete_buttonconfig();
}


/////////////////////////////////////////////////////////


// keysymはアスキー文字か
bool CONTROL::is_ascii( const guint keysym )
{
    if( keysym > 32 && keysym < 127 ) return true;

    return false;
}


// メニューにショートカットキーやマウスジェスチャを表示
void CONTROL::set_menu_motion( Gtk::Menu* menu )
{
    if( !menu ) return;

    menu->foreach( []( Gtk::Widget& w ) {
        auto* const item = dynamic_cast< Gtk::MenuItem* >( &w );

        // menuitemの中の名前を読み込んで ID を取得し、CONTROL::Noneでなかったら
        // ラベルを置き換える
        auto* const label = dynamic_cast< Gtk::Label* >( item->get_child() );
        if( label ){
#ifdef _DEBUG
            std::cout << label->get_text() << std::endl;
#endif
            int id = CONTROL::get_id( label->get_text() );
            if( id != CONTROL::None ){

                std::string str_label = CONTROL::get_label_with_mnemonic( id );
                std::string str_motions = CONTROL::get_str_motions( id );

                item->remove();
                Gtk::Label *label = Gtk::manage( new Gtk::Label( str_label + ( str_motions.empty() ? "" : "  " ), true ) );
                Gtk::Label *label_motion = Gtk::manage( new Gtk::Label( str_motions ) );
                Gtk::HBox *box = Gtk::manage( new Gtk::HBox() );

                box->pack_start( *label, Gtk::PACK_SHRINK );
                box->pack_end( *label_motion, Gtk::PACK_SHRINK );
                item->add( *box );
                box->show_all();
            }
        }

        if( item->has_submenu() ) CONTROL::set_menu_motion( item->get_submenu() );
    } );
}


// IDからモードを取得
// 例えば id == CONTROL::Up の時は CONTROL::COMMONMOTION を返す
int CONTROL::get_mode( const int id )
{
    if( id < CONTROL::COMMONMOTION_END ) return CONTROL::MODE_COMMON;
    if( id < CONTROL::BBSLISTMOTION_END ) return CONTROL::MODE_BBSLIST;
    if( id < CONTROL::BOARDMOTION_END ) return CONTROL::MODE_BOARD;
    if( id < CONTROL::ARTICLEMOTION_END ) return CONTROL::MODE_ARTICLE;
    if( id < CONTROL::IMAGEICONMOTION_END ) return CONTROL::MODE_IMAGEICON;
    if( id < CONTROL::IMAGEVIEWMOTION_END ) return CONTROL::MODE_IMAGEVIEW;
    if( id < CONTROL::MESSAGEMOTION_END ) return CONTROL::MODE_MESSAGE;
    if( id < CONTROL::EDITMOTION_END ) return CONTROL::MODE_EDIT;
    if( id < CONTROL::JDGLOBALS_END ) return CONTROL::MODE_JDGLOBALS;

    return CONTROL::MODE_ERROR;
}


// 操作モードIDからモード名取得
// 例えば mode == CONTROL::MODE_COMMON の時は "共通" を返す
const std::string CONTROL::get_mode_label( const int mode )
{
    if( mode < CONTROL::MODE_START || mode > MODE_END ) return std::string();

    return CONTROL::mode_label[ mode ];
}


// キー名からkeysymを取得
// 例えば keyname == "Space" の時は GDK_space を返す
guint CONTROL::get_keysym( const std::string& keyname )
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
int CONTROL::get_id( const std::string& name )
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
            case CONTROL::Save:                 //名前を付けて保存...
            case CONTROL::SaveDat:              //datを保存...
                label.replace( pos, strlen( "..." ), "(_S)..." );
                break;
	
            case CONTROL::AppendFavorite:       //お気に入りに追加..
                label.replace( pos, strlen( "..." ), "(_F)..." );
                break;

            case CONTROL::OpenURL:              //URLを開く
                label.replace( pos, strlen( "..." ), "(_U)..." );
                break;

            case CONTROL::PreferenceView:       //プロパティ...
            case CONTROL::PreferenceArticle:    //スレのプロパティ...
                label.replace( pos, strlen( "..." ), "(_P)..." );
                break;

            case CONTROL::PreferenceBoard:      //板のプロパティ...
                label.replace( pos, strlen( "..." ), "(_O)..." );
                break;

            case CONTROL::PreferenceImage:      //画像のプロパティ...
                label.replace( pos, strlen( "..." ), "(_M)..." );
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

            case CONTROL::ShowMenuBar:  // メニューバー表示
                label += "(_M)";
                break;

            case CONTROL::ShowToolBarMain:  // メインツールバー表示
                label += "(_M)";
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

            case CONTROL::Search:       //検索
                label += "(_S)";
                break;

            case CONTROL::SearchNext:   //次検索
                label += "(_N)";
                break;

            case CONTROL::SearchPrev:   //前検索
                label += "(_P)";
                break;

            case CONTROL::OpenArticleTab: // タブでスレを開く
                label += "(_T)";
                break;

            case CONTROL::GotoNew:      //新着へ移動
                label += "(_W)";
                break;

            case CONTROL::LiveStartStop:  // 実況
                label += "(_L)";
                break;

            case CONTROL::SearchNextArticle: // 次スレ検索
                label += "(_N)";
                break;

            case CONTROL::SearchWeb: // web検索
                label += "(_W)";
                break;

            case CONTROL::SearchTitle: // スレタイ検索
                label += "(_T)";
                break;

            case CONTROL::SearchCacheLocal: // ログ検索(対象: 板)
                label += "(_L)";
                break;

            case CONTROL::SearchCacheAll: // ログ検索(対象: 全て)
                label += "(_A)";
                break;

            case CONTROL::ShowSelectImage: // 選択範囲の画像を開く
                label += "(_G)";
                break;

            case CONTROL::DeleteSelectImage: // 選択範囲の画像を削除（サブメニュー）
                label += "(_D)";
                break;

            case CONTROL::AboneSelectImage: // 選択範囲の画像をあぼ〜ん（サブメニュー）
                label += "(_A)";
                break;

            case CONTROL::AboneSelectionRes: // 選択範囲のレスをあぼ〜ん
                label += "(_A)";
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

            case CONTROL::FullScreen: // 全画面表示
                label += "(_F)";
                break;
        }
    }

    return label;
}


// IDからキーボードとマウスジェスチャの両方を取得
const std::string CONTROL::get_str_motions( const int id )
{
    int ctl = id;

    // メニューのラベルとコントロールのIDが異なる、共通コントロールIDの場合は、ここで変換する
    switch( ctl ){
        // 表示中のビューのプロパティを表示
    case CONTROL::PreferenceArticle:    //スレのプロパティ...
    case CONTROL::PreferenceBoard:      //板のプロパティ...
    case CONTROL::PreferenceImage:      //画像のプロパティ...
        ctl = CONTROL::PreferenceView;
        break;
        //名前を付けて保存...
    case CONTROL::SaveDat:              //datを保存...
        ctl = CONTROL::Save;
        break;
    }

    std::string str_motion = get_str_keymotions( ctl );
    std::string mouse_motion = get_str_mousemotions( ctl );
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


// 共通操作
bool CONTROL::operate_common( const int control, const std::string& url, SKELETON::Admin* admin )
{
    if( control == CONTROL::None ) return false;;

    switch( control ){
            
            // 全てのタブを閉じる
        case CONTROL::CloseAllTabs:
            if( admin ) admin->set_command( "close_view", "", "closealltabs" );
            break;

            // 他のタブを閉じる
        case CONTROL::CloseOtherTabs:
            if( admin ) admin->set_command( "close_view", url, "closeother" );
            break;

            // 最後に閉じたタブを復元
        case CONTROL::RestoreLastTab:
            if( admin ) admin->set_command( "restore_lasttab" );
            break;

            // 全ての更新されたタブを再読み込み
        case CONTROL::CheckUpdateTabs:
            if( admin ) admin->set_command( "check_update_reload_all_tabs" );
            break;

            // 右、左のタブに切り替え
        case CONTROL::TabLeft:
            if( admin ) admin->set_command( "tab_left" );
            break;

        case CONTROL::TabRight:
            if( admin ) admin->set_command( "tab_right" );
            break;

        case CONTROL::TabLeftUpdated:
            if( admin ) admin->set_command( "tab_left_updated" );
            break;

        case CONTROL::TabRightUpdated:
            if( admin ) admin->set_command( "tab_right_updated" );
            break;

        // タブ位置(1-9)で移動
        case CONTROL::TabNum1:
            if( admin ) admin->set_command( "tab_num", "", "1" );
            break;

        case CONTROL::TabNum2:
            if( admin ) admin->set_command( "tab_num", "", "2" );
            break;

        case CONTROL::TabNum3:
            if( admin ) admin->set_command( "tab_num", "", "3" );
            break;

        case CONTROL::TabNum4:
            if( admin ) admin->set_command( "tab_num", "", "4" );
            break;

        case CONTROL::TabNum5:
            if( admin ) admin->set_command( "tab_num", "", "5" );
            break;

        case CONTROL::TabNum6:
            if( admin ) admin->set_command( "tab_num", "", "6" );
            break;

        case CONTROL::TabNum7:
            if( admin ) admin->set_command( "tab_num", "", "7" );
            break;

        case CONTROL::TabNum8:
            if( admin ) admin->set_command( "tab_num", "", "8" );
            break;

        case CONTROL::TabNum9:
            if( admin ) admin->set_command( "tab_num", "", "9" );
            break;

            // サイドバー表示/非表示
        case CONTROL::ShowSideBar:
            CORE::core_set_command( "toggle_sidebar" );
            break;

            // メニューバー表示/非表示
        case CONTROL::ShowMenuBar:
            CORE::core_set_command( "toggle_menubar" );
            break;

            // メインツールバー表示/非表示
        case CONTROL::ShowToolBarMain:
            CORE::core_set_command( "toggle_toolbarmain" );
            break;

            // URLを開くダイアログを表示
        case CONTROL::OpenURL:
            CORE::core_set_command( "show_openurl_diag" );
            break;

            // JD終了
        case CONTROL::QuitJD:
            CORE::core_set_command( "quit_jd" );
            break;

            // 最大化 / 最大化解除
        case CONTROL::MaximizeMainWin:
            CORE::core_set_command( "maximize_mainwin" );
            break;

            // 最小化
        case CONTROL::IconifyMainWin:
            CORE::core_set_command( "iconify_mainwin" );
            break;

            // サイドバー更新チェック
        case CONTROL::CheckUpdateRoot:
            CORE::core_set_command( "check_update_root" );
            break;

        case CONTROL::CheckUpdateOpenRoot:
            CORE::core_set_command( "check_update_open_root" );
            break;

            // 全画面表示
        case CONTROL::FullScreen:
            CORE::core_set_command( "toggle_fullscreen" );
            break;

            // スレタイ検索
        case CONTROL::SearchTitle:
            CORE::core_set_command( "open_article_searchtitle", "", "", "noexec" );
            break;

        default:
            return false;
    }

    return true;
}


/////////////////////////////////////////////////////////



// キーボード設定の一時的なバックアップと復元
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
void CONTROL::set_keymotions( const int id, const std::string& str_motions )
{
    CONTROL::get_keyconfig()->set_motions( id, str_motions );
}


// 指定したIDのキーボード操作を全て削除
bool CONTROL::remove_keymotions( const int id )
{
    return CONTROL::get_keyconfig()->remove_motions( id );
}


// キーボード操作が重複していないか
const std::vector< int > CONTROL::check_key_conflict( const int mode, const std::string& str_motion )
{
    return CONTROL::get_keyconfig()->check_conflict( mode, str_motion );
}


// editviewの操作をemacs風にする
bool CONTROL::is_emacs_mode()
{
    return CONTROL::get_keyconfig()->is_emacs_mode();
}


void CONTROL::toggle_emacs_mode()
{
    CONTROL::get_keyconfig()->toggle_emacs_mode();
}


// 「タブで開く」キーを入れ替える
bool CONTROL::is_toggled_tab_key()
{
    return CONTROL::get_keyconfig()->is_toggled_tab_key();
}


void CONTROL::toggle_tab_key( const bool toggle )
{
    CONTROL::get_keyconfig()->toggle_tab_key( toggle );
}


// Gtk アクセラレーションキーを取得
Gtk::AccelKey CONTROL::get_accelkey( const int id )
{
    return CONTROL::get_keyconfig()->get_accelkey( id );
}


/////////////////////////////////////////////////////////


const std::string convert_mouse_motions( std::string motions )
{
    motions = MISC::replace_str( motions, "8", "↑" );
    motions = MISC::replace_str( motions, "6", "→" );
    motions = MISC::replace_str( motions, "4", "←" );
    motions = MISC::replace_str( motions, "2", "↓" );

    return motions;
}


const std::string convert_mouse_motions_reverse( std::string motions )
{
    motions = MISC::replace_str( motions, "↑", "8" );
    motions = MISC::replace_str( motions, "→", "6" );
    motions = MISC::replace_str( motions, "←", "4" );
    motions = MISC::replace_str( motions, "↓", "2" );

    return motions;
}


// マウスジェスチャ設定の一時的なバックアップと復元
void CONTROL::bkup_mouseconfig()
{
    if( ! instance_mouseconfig_bkup ) instance_mouseconfig_bkup = new CONTROL::MouseConfig();
    *instance_mouseconfig_bkup = * instance_mouseconfig;
}


void CONTROL::restore_mouseconfig()
{
    if( ! instance_mouseconfig_bkup ) return;
    *instance_mouseconfig = * instance_mouseconfig_bkup;
}


// IDからマウスジェスチャを取得
const std::string CONTROL::get_str_mousemotions( const int id )
{
    return convert_mouse_motions( CONTROL::get_mouseconfig()->get_str_motions( id ) );
}


// IDからデフォルトマウスジェスチャを取得
const std::string CONTROL::get_default_mousemotions( const int id )
{
    return convert_mouse_motions( CONTROL::get_mouseconfig()->get_default_motions( id ) );
}


// スペースで区切られた複数のマウスジェスチャをデータベースに登録
void CONTROL::set_mousemotions( const int id, const std::string& str_motions )
{
    const std::string motions = convert_mouse_motions_reverse( str_motions );    
    CONTROL::get_mouseconfig()->set_motions( id, motions );
}


// 指定したIDのマウスジェスチャを全て削除
bool CONTROL::remove_mousemotions( const int id )
{
    return CONTROL::get_mouseconfig()->remove_motions( id );
}


// マウスジェスチャが重複していないか
const std::vector< int > CONTROL::check_mouse_conflict( const int mode, const std::string& str_motion )
{
    const std::string motion = convert_mouse_motions_reverse( str_motion );
    return CONTROL::get_mouseconfig()->check_conflict( mode, motion );
}


/////////////////////////////////////////////////////////


// ボタン設定の一時的なバックアップと復元
void CONTROL::bkup_buttonconfig()
{
    if( ! instance_buttonconfig_bkup ) instance_buttonconfig_bkup = new CONTROL::ButtonConfig();
    *instance_buttonconfig_bkup = * instance_buttonconfig;
}


void CONTROL::restore_buttonconfig()
{
    if( ! instance_buttonconfig_bkup ) return;
    *instance_buttonconfig = * instance_buttonconfig_bkup;
}


// IDからボタン設定を取得
const std::string CONTROL::get_str_buttonmotions( const int id )
{
    return CONTROL::get_buttonconfig()->get_str_motions( id );
}


// IDからデフォルトボタン設定を取得
const std::string CONTROL::get_default_buttonmotions( const int id )
{
    return CONTROL::get_buttonconfig()->get_default_motions( id );
}


// スペースで区切られた複数のボタン設定をデータベースに登録
void CONTROL::set_buttonmotions( const int id, const std::string& str_motions )
{
    CONTROL::get_buttonconfig()->set_motions( id, str_motions );
}

// 指定したIDのボタン設定を全て削除
bool CONTROL::remove_buttonmotions( const int id )
{
    return CONTROL::get_buttonconfig()->remove_motions( id );
}


    // ボタンが重複していないか
const std::vector< int > CONTROL::check_button_conflict( const int mode, const std::string& str_motion )
{
    return CONTROL::get_buttonconfig()->check_conflict( mode, str_motion );
}


/////////////////////////////////////////////////////////


// タブで開くボタンを入れ替える
bool CONTROL::is_toggled_tab_button()
{
    return CONTROL::get_buttonconfig()->is_toggled_tab_button();
}


void CONTROL::toggle_tab_button( const bool toggle )
{
    CONTROL::get_buttonconfig()->toggle_tab_button( toggle );
}


// ポップアップ表示の時にクリックでワープ
bool CONTROL::is_popup_warpmode()
{
    return CONTROL::get_buttonconfig()->is_popup_warpmode();
}


void CONTROL::toggle_popup_warpmode()
{
    CONTROL::get_buttonconfig()->toggle_popup_warpmode();
}
