// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "bbslistview.h"
#include "bbslistadmin.h"

#include "skeleton/msgdiag.h"

#include "dbtree/bbsmenu.h"
#include "dbtree/interface.h"

#include "config/globalconf.h"

#include "jdlib/misctime.h"

#include "cache.h"
#include "type.h"
#include "command.h"

using namespace BBSLIST;

// ルート要素名( boards.xml )
#define ROOT_NODE_NAME "boardlist"

// メインビュー

BBSListViewMain::BBSListViewMain( const std::string& url,
                                  const std::string& arg1, const std::string& arg2 )
    : BBSListViewBase( url, arg1, arg2 )
{
    set_label( "板一覧" );

    set_open_only_onedir( CONFIG::get_open_one_category() );
}



BBSListViewMain::~BBSListViewMain()
{
#ifdef _DEBUG
    std::cout << "BBSListViewMain::~BBSListViewMain : " << get_url() << std::endl;
#endif
}


// xml保存
void BBSListViewMain::save_xml()
{
    const std::string file = CACHE::path_xml_listmain();
    constexpr std::string_view remove_dirs[] = { SUBDIR_ETCLIST, SUBDIR_BBSMENU };
    save_xml_impl( file, ROOT_NODE_NAME, remove_dirs );
}


//
// 表示
//
void BBSListViewMain::show_view()
{
#ifdef _DEBUG
    std::cout << "BBSListViewMain::show_view : " << get_url() << std::endl;
#endif

    // BBSListViewBase::m_document に Root::m_xml_document を代入
    set_document( DBTREE::get_xml_document() );

    if( get_document().hasChildNodes() ) update_view();

    // 板一覧のDomノードが空ならサーバからbbsmenuを取得
    // 取得が終わったらBBSListViewMain::update_view()が呼び出される
    else DBTREE::download_bbsmenu();
}


//
// 表示更新
//
void BBSListViewMain::update_view()
{
    XML::Document document;
    document = DBTREE::get_xml_document();

    // 空なら更新しない
    if( ! document.hasChildNodes() ) return;

    // BBSListViewBase::m_document に Root::m_document を代入
    set_document( document );

    // ルート要素を取得
    XML::Dom* root = get_document().get_root_element( std::string( ROOT_NODE_NAME ) );

    //----------------------------------
    // 外部板追加

    // <subdir>を挿入
    // ルート要素の有無で処理を分ける( 旧様式=無, 新様式=有 )
    XML::Dom* subdir = nullptr;
    if( root ) subdir = root->emplace_front( XML::NODE_TYPE_ELEMENT, "subdir" );
    else subdir = get_document().emplace_front( XML::NODE_TYPE_ELEMENT, "subdir" );
    subdir->setAttribute( "name", std::string( SUBDIR_ETCLIST ) );

    // 子要素( <board> )を追加
    if( const auto list_etc = DBTREE::get_etcboards(); // 外部板情報( dbtree/etcboardinfo.h )
        !list_etc.empty() ) {
        for( const DBTREE::ETCBOARDINFO& info : list_etc ) {
            XML::Dom* board = subdir->appendChild( XML::NODE_TYPE_ELEMENT, "board" );
            board->setAttribute( "name", info.name );
            board->setAttribute( "url", info.url );
        }
    }

    // 外部板追加ここまで
    //----------------------------------
    // 外部BBSMENU追加

    // <subdir>を挿入
    // ルート要素の有無で処理を分ける( 旧様式=無, 新様式=有 )
    if( root ) subdir = root->emplace_front( XML::NODE_TYPE_ELEMENT, "subdir" );
    else subdir = get_document().emplace_front( XML::NODE_TYPE_ELEMENT, "subdir" );
    subdir->setAttribute( "name", SUBDIR_BBSMENU );

    // 子要素( <bbsmenu> )を追加
    if( const auto& vec_bbsmenu = DBTREE::get_bbsmenus(); // 外部BBSMENU情報( dbtree/bbsmenu.h )
        ! vec_bbsmenu.empty() ) {
        for( const DBTREE::BBSMenu& info : vec_bbsmenu ) {
            const std::string& info_name = info.get_name();
            const std::string& info_url = info.get_url();

            // 外部BBSMENUに追加
            XML::Dom* bbsmenu = subdir->appendChild( XML::NODE_TYPE_ELEMENT, "bbsmenu" );
            bbsmenu->setAttribute( "name", info_name );
            bbsmenu->setAttribute( "url", info_url );

            // <subdir>を板一覧の末尾に追加
            // ルート要素の有無で処理を分ける( 旧様式=無, 新様式=有 )
            XML::Dom* bbsmenu_subdir = root ? root->appendChild( XML::NODE_TYPE_ELEMENT, "subdir" )
                                            : get_document().appendChild( XML::NODE_TYPE_ELEMENT, "subdir" );
            bbsmenu_subdir->setAttribute( "name", info_name );
            bbsmenu_subdir->setAttribute( "url", info_url );
            bbsmenu_subdir->setAttribute( "data", "nosave" ); // 外部BBSMENUの板は個別に保存する

            std::list<XML::Dom*> boardlist = info.xml_document().getElementsByTagName( "boardlist" );
            if( boardlist.empty() ) continue;
            const XML::Dom* categories = boardlist.front();

            // カテゴリのサブディレクトリを作成してその中に板を追加していく
            for( const XML::Dom* category_in : *categories ) {
                const std::string category_name = category_in->getAttribute( "name" );
                if( category_name.empty() ) continue;

                XML::Dom* category_subdir = bbsmenu_subdir->appendChild( XML::NODE_TYPE_ELEMENT, "subdir" );
                category_subdir->setAttribute( "name", category_name );
                category_subdir->setAttribute( "url", category_in->getAttribute( "url" ) );
                category_subdir->setAttribute( "dirid", category_in->getAttribute( "dirid" ) );

                for( const XML::Dom* board_in : *category_in ) {
                    const std::string board_name = board_in->getAttribute( "name" );
                    if( board_name.empty() ) continue;

                    XML::Dom* node_board = category_subdir->appendChild( XML::NODE_TYPE_ELEMENT, "board" );
                    node_board->setAttribute( "name", board_name );
                    node_board->setAttribute( "url", board_in->getAttribute( "url" ) );
                }
            }
        }
    }

    // 外部BBSMENU追加ここまで
    //----------------------------------

    // BBSListViewBase::xml2tree() m_document -> tree
    xml2tree( std::string( ROOT_NODE_NAME ) );
    set_status( std::string() );
    BBSLIST::get_admin()->set_command( "set_status", get_url(), get_status() );
}


//
// 削除
//
// BBSListViewMainの場合は外部板の削除
//
void BBSListViewMain::delete_view()
{
    // 選択範囲に通常の板が含まれていないか確認
    std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
    if( std::any_of( list_it.cbegin(), list_it.cend(),
                     [this]( const Gtk::TreeIter& iter ) { return ! is_etcboard( iter ); } ) ) {
        SKELETON::MsgDiag mdiag( get_parent_win(), "通常の板は削除出来ません", false, Gtk::MESSAGE_ERROR );
        mdiag.run();
        return;
    }

    SKELETON::MsgDiag mdiag( get_parent_win(), "削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    delete_view_impl();
}

// BBSListViewMainの場合は外部板の削除
void BBSListViewMain::delete_view_impl()
{
#ifdef _DEBUG
    std::cout << "BBSListViewMain::delete_view_impl\n";
#endif

    constexpr const int kDeleteTargetBoard = 0b01;
    constexpr const int kDeleteTargetBBSMenu = 0b10;
    int delete_target = 0;

    std::vector<Gtk::TreeIter> delete_bbsmenu_iterators;

    // データベースから外部板または外部BBSMENUを削除
    std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
    for( Gtk::TreeModel::iterator& iter : list_it ) {
        if( is_etcboard( iter ) ) {
            Gtk::TreePath path = get_treestore()->get_path( iter );
            std::string url = path2rawurl( path );
            std::string name = path2name( path );

#ifdef _DEBUG
            std::cout << "path = " << path.to_string() << std::endl
                      << "url = " << url << std::endl
                      << "name = " << name << std::endl;
#endif
            DBTREE::remove_etc( url , name );
            delete_target |= kDeleteTargetBoard;
        }
        else if( is_bbsmenu( iter ) ) {
            const std::string url = row2url( *iter );
            const std::string name = row2name( *iter );

            DBTREE::remove_bbsmenu( url, name );
            delete_target |= kDeleteTargetBBSMenu;

            // 板一覧の下部に追加した外部BBSMENUの板一覧を削除するためにイテレーターを記憶する
            auto it = get_treestore()->get_iter( "0" );
            while( it ) {
                auto row_url = row2url( *it );
                auto row_name = row2name( *it );
                auto row_type = row2type( *it );
                if( row_type == TYPE_DIR && row_url == url && row_name == name ) {
                    delete_bbsmenu_iterators.push_back( std::move( *it ) );
                    break;
                }
                ++it;
            }
        }
    }

    const bool force = true; // 強制的に削除
    get_treeview().delete_selected_rows( force );

    // 板一覧の下部に追加した外部BBSMENUの板一覧を削除する
    if( ! delete_bbsmenu_iterators.empty() ) {
        // 取得したTreeStoreのイテレーターが壊れないように後ろから削除していく
        auto rend = delete_bbsmenu_iterators.rend();
        for( auto rit = delete_bbsmenu_iterators.rbegin(); rit != rend; ++rit ) {
            get_treestore()->erase( *rit );
        }
    }

    if( delete_target & kDeleteTargetBoard ) {
        // etc.txt保存
        DBTREE::save_etc();
    }
    if( delete_target & kDeleteTargetBBSMenu ) {
        // bbsmenu.txt保存
        DBTREE::save_bbsmenu();
    }
}


void BBSListViewMain::show_preference()
{
    std::string modified = "最終更新日時 ：";

    if( DBTREE::get_date_modified().empty() ) modified +=  "未取得";
    else modified += 
             MISC::timettostr( DBTREE::get_time_modified(), MISC::TIME_WEEK )
             + " ( " + MISC::timettostr( DBTREE::get_time_modified(), MISC::TIME_PASSED ) + " )";

    SKELETON::MsgDiag mdiag( get_parent_win(), modified, false );
    mdiag.set_title( "板一覧のプロパティ" );
    mdiag.run();
}



//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* BBSListViewMain::get_popupmenu( const std::string& url )
{
    Gtk::Menu* popupmenu = nullptr;

    if( url.empty() ) return popupmenu;

#ifdef _DEBUG
    std::cout << "BBSListViewMain::get_popupmenu\n";
#endif

    std::list< Gtk::TreeModel::iterator > list_it = get_treeview().get_selected_iterators();
    if( list_it.size() == 1 ){

        Gtk::TreePath path = *( get_treeview().get_selection()->get_selected_rows().begin() );
        int type = path2type( path );

#ifdef _DEBUG
        std::cout << "path = " << path.to_string() << " type = " << type << std::endl;
#endif

        if( type == TYPE_DIR ){
            if( is_etcdir( path ) ) popupmenu = id2popupmenu(  "popup_menu_etcdir" );
            else if( is_bbsmenudir( path ) ) popupmenu = id2popupmenu( "popup_menu_bbsmenudir" );
            else popupmenu = id2popupmenu(  "popup_menu_dir" );
        }
        else if( type == TYPE_BOARD || type == TYPE_BOARD_UPDATE ){
            if( is_etcboard( path ) ) popupmenu = id2popupmenu(  "popup_menu_etc" );
            else popupmenu = id2popupmenu(  "popup_menu" );
        }
        else if( type == TYPE_BBSMENU ) {
            if( is_bbsmenu( path ) ) popupmenu = id2popupmenu( "popup_menu_bbsmenu" );
            else popupmenu = id2popupmenu( "popup_menu" );
        }
    }
    else{
        bool have_bbsmenu = true;
        bool have_etc = true;
        for( const Gtk::TreeIter& iter : list_it ) {
            if( ! is_bbsmenu( iter ) ) have_bbsmenu = false;
            if( ! is_etcboard( iter ) ) have_etc = false;

            if( ! have_bbsmenu && ! have_etc ) break;
        }
        if( have_bbsmenu ) popupmenu = id2popupmenu( "popup_menu_mul_bbsmenu" );
        else if( have_etc ) popupmenu = id2popupmenu( "popup_menu_mul_etc" );
        else popupmenu = id2popupmenu( "popup_menu_mul" );
    }

    return popupmenu;
}
