// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "historysubmenu.h"

#include "command.h"
#include "prefdiagfactory.h"
#include "session.h"

#include "jdlib/miscutil.h"

#include "skeleton/msgdiag.h"

#include "dbtree/interface.h"
#include "dbimg/imginterface.h"

#include "config/globalconf.h"

#include "control/controlid.h"
#include "control/controlutil.h"

#include "xml/tools.h"

using namespace HISTORY;

#define HIST_NONAME "--------------"

enum
{
    SPACING_MENU = 3, // アイコンと項目名の間のスペース
    HIST_MAX_LNG = 50 // 履歴に表示する文字数(半角)
};


HistorySubMenu::HistorySubMenu( const std::string& url_history )
    : Gtk::Menu(),
      m_url_history( url_history )
{
    Gtk::MenuItem* item;

    // メニュー項目作成

    // 履歴クリア
    Gtk::Menu* menu = Gtk::manage( new Gtk::Menu() );
    item = Gtk::manage( new Gtk::MenuItem( "クリアする(_C)", true ) );
    menu->append( *item );
    item->signal_activate().connect( sigc::mem_fun( *this, &HistorySubMenu::slot_clear ) ); 

    item = Gtk::manage( new Gtk::MenuItem( "履歴クリア(_C)", true ) );
    item->set_submenu( *menu );
    append( *item );

    item = Gtk::manage( new Gtk::MenuItem( "サイドバーに全て表示(_S)", true ) );
    append( *item );
    item->signal_activate().connect( sigc::mem_fun( *this, &HistorySubMenu::slot_switch_sideber ) );

    // セパレータ
    item = Gtk::manage( new Gtk::SeparatorMenuItem() );
    append( *item );

    // 履歴項目
    for( unsigned int i = 0, end = static_cast<unsigned int>(CONFIG::get_history_size()); i < end; ++i ) {

        Gtk::Image* image = Gtk::manage( new Gtk::Image() );
        m_vec_images.push_back( image );

        Gtk::Label* label = Gtk::manage( new Gtk::Label( HIST_NONAME ) );
        m_vec_label.push_back( label );

        Gtk::Label *label_motion = Gtk::manage( new Gtk::Label() );
        if( i == 0 ) label_motion->set_text( CONTROL::get_str_motions( CONTROL::RestoreLastTab ) );

        Gtk::HBox* hbox = Gtk::manage( new Gtk::HBox() );
        hbox->set_spacing( SPACING_MENU );
        hbox->pack_start( *image, Gtk::PACK_SHRINK );
        hbox->pack_start( *label, Gtk::PACK_SHRINK );
        hbox->pack_end( *label_motion, Gtk::PACK_SHRINK );

        item = Gtk::manage( new Gtk::MenuItem( *hbox ) );
        append( *item );
        item->signal_activate().connect( sigc::bind( sigc::mem_fun( *this, &HistorySubMenu::slot_active ), i ) );
        item->signal_button_press_event().connect( sigc::bind( sigc::mem_fun( *this, &HistorySubMenu::slot_button_press ), i ) );
    }

    // ポップアップメニュー作成
    m_popupmenu.signal_deactivate().connect( sigc::mem_fun( *this, &HistorySubMenu::deactivate ) );

    item = Gtk::manage( new Gtk::MenuItem( "タブで開く" ) );
    item->signal_activate().connect( sigc::mem_fun( *this, &HistorySubMenu::slot_open_history ) );
    m_popupmenu.append( *item );

    item = Gtk::manage( new Gtk::SeparatorMenuItem() );
    m_popupmenu.append( *item );

    item = Gtk::manage( new Gtk::MenuItem( "履歴から削除" ) );
    item->signal_activate().connect( sigc::mem_fun( *this, &HistorySubMenu::slot_remove_history ) );
    m_popupmenu.append( *item );

    item = Gtk::manage( new Gtk::SeparatorMenuItem() );
    m_popupmenu.append( *item );

    item = Gtk::manage( new Gtk::MenuItem( "プロパティ" ) );
    item->signal_activate().connect( sigc::mem_fun( *this, &HistorySubMenu::slot_show_property ) );
    m_popupmenu.append( *item );

    m_popupmenu.show_all_children();
}


HistorySubMenu::~HistorySubMenu()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::~HistorySubMenu\n";
#endif
}


// 履歴の先頭を復元
void HistorySubMenu::restore_history()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::restore_history " << m_url_history << std::endl;
#endif

    if( open_history( 0 ) ) CORE::core_set_command( "remove_headhistory", m_url_history );
}


// 履歴を開く
bool HistorySubMenu::open_history( const unsigned int i )
{
    bool ret = false;
    CORE::DATA_INFO_LIST info_list;
    SESSION::get_history( m_url_history, info_list );
    if( info_list.size() <= i ) return ret;

    if( ! info_list[ i ].url.empty() ){

        const std::string& url = info_list[i].url;
#ifdef _DEBUG
        std::cout << "open " << url << std::endl;
#endif
        const std::string tab = "newtab";
        const std::string mode = "";

        bool open_image = false;
        std::string open_mode;

        switch( info_list[ i ].type ){

            case TYPE_THREAD: 
            case TYPE_THREAD_UPDATE:
            case TYPE_THREAD_OLD:

                CORE::core_set_command( "open_article" , DBTREE::url_dat( url ), tab, mode );
                ret = true;
                break;

            case TYPE_BOARD:
                
                CORE::core_set_command( "open_board", DBTREE::url_boardbase( url ), tab, mode );
                ret = true;
                break;

            case TYPE_VBOARD:

                CORE::core_set_command( "open_sidebar_board", url, tab, mode, "", "set_history" );
                ret = true;
                break;

            case TYPE_IMAGE:

                if( DBIMG::get_abone( url )){
                    SKELETON::MsgDiag mdiag( nullptr, "あぼ〜んされています", false,
                                             Gtk::MESSAGE_INFO, Gtk::BUTTONS_YES_NO );
                    std::string abone_reason = DBIMG::get_img_abone_reason( url );
                    if( ! abone_reason.empty() ) {
                        abone_reason = MISC::replace_str( abone_reason, "<br>", "\n" );
                        abone_reason.append( "\n\n" );
                    }
                    abone_reason.append( DBIMG::kTemporaryMosaicQuestion );
                    mdiag.set_secondary_text( abone_reason );

                    const int response = mdiag.run();
                    if( response == Gtk::RESPONSE_YES ) {
                        open_image = true;
                        open_mode = "force_mosaic";
                    }
                }
                else {
                    open_image = true;
                }

                if( open_image ) {
                    CORE::core_set_command( "open_image", url, open_mode );
                    CORE::core_set_command( "switch_image" );
                    ret = true;
                }
                break;
        }
    }

    return ret;
}


// メニューアイテムがactiveになった
void HistorySubMenu::slot_active( const unsigned int i )
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::slot_key_press key = " << "no = " << i << std::endl;
#endif

    m_number_menuitem = i;

    open_history( i );
}


// マウスボタンをクリックした
bool HistorySubMenu::slot_button_press( GdkEventButton* event, unsigned int i )
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::slot_button_press button = " << event->button << " no = " << i << std::endl;
#endif

    m_number_menuitem = i;

    // ポップアップメニュー表示
    if( event->button == 3 )
    {
        m_popupmenu.popup_at_pointer( reinterpret_cast<GdkEvent*>( event ) );
    }

    return true;
}


// アクティブ時にラベルをセットする
void HistorySubMenu::set_menulabel()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::set_menulabel\n";
#endif

    CORE::DATA_INFO_LIST info_list;
    SESSION::get_history( m_url_history, info_list );
    for( size_t i = 0; i < m_vec_label.size(); ++i ){

        std::string name;
        int type = TYPE_UNKNOWN;
        if( i <  info_list.size() ){
            name = info_list[ i ].name;
            type = info_list[ i ].type;
        }
        if( name.empty() ) name = HIST_NONAME;

        m_vec_images[ i ]->set( Glib::RefPtr<const Gio::Icon>::cast_const( XML::get_icon( type ) ),
                                Gtk::ICON_SIZE_MENU );
        m_vec_label[ i ]->set_text( MISC::cut_str( name, HIST_MAX_LNG ) );
    }
}


// 履歴クリア
void HistorySubMenu::slot_clear()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::slot_clear " << m_url_history << std::endl;
#endif

    CORE::core_set_command( "remove_allhistories", m_url_history );
}


// サイドバー切り替え
void HistorySubMenu::slot_switch_sideber()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::slot_switch_sideber " << m_url_history << std::endl;
#endif

    CORE::core_set_command( "switch_sidebar", m_url_history );
}


// 指定した履歴を開く
// これを呼ぶ前に m_number_menuitem に番号をセットしておく
void HistorySubMenu::slot_open_history()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::slot_open_history no = " << m_number_menuitem << std::endl;
#endif

    open_history( m_number_menuitem );
}


// 指定した履歴を削除
// これを呼ぶ前に m_number_menuitem に番号をセットしておく
void HistorySubMenu::slot_remove_history()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::slot_remove_history no = " << m_number_menuitem << std::endl;
#endif 

    const unsigned int i = m_number_menuitem;

    CORE::DATA_INFO_LIST info_list;
    SESSION::get_history( m_url_history, info_list );
    if( info_list.size() <= i ) return;

    CORE::core_set_command( "remove_history", m_url_history, info_list[ i ].url );
}


// プロパティ表示
// これを呼ぶ前に m_number_menuitem に番号をセットしておく
void HistorySubMenu::slot_show_property()
{
#ifdef _DEBUG
    std::cout << "HistorySubMenu::slot_show_property no = " << m_number_menuitem << std::endl;
#endif

    const unsigned int i = m_number_menuitem;

    CORE::DATA_INFO_LIST info_list;
    SESSION::get_history( m_url_history, info_list );
    if( info_list.size() <= i ) return;

    if( ! info_list[ i ].url.empty() ){

#ifdef _DEBUG
        std::cout << "url " << info_list[ i ].url << std::endl;
#endif

        std::unique_ptr<SKELETON::PrefDiag> pref;
        switch( info_list[ i ].type ){

            case TYPE_THREAD: 
            case TYPE_THREAD_UPDATE:
            case TYPE_THREAD_OLD:

                pref= CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_ARTICLE, info_list[ i ].url );
                break;

            case TYPE_BOARD:
            case TYPE_VBOARD:

                pref= CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_BOARD, info_list[ i ].url );
                break;

            case TYPE_IMAGE:

                pref= CORE::PrefDiagFactory( nullptr, CORE::PREFDIAG_IMAGE, info_list[ i ].url );
                break;
        }

        if( pref ){
            pref->run();
        }
    }
}
