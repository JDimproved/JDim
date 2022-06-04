// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "tabswitchmenu.h"
#include "dragnote.h"

#include "icons/iconmanager.h"
#include "jdlib/miscutil.h"


using namespace SKELETON;


Glib::RefPtr<TabSwitchMenu> TabSwitchMenu::create( DragableNoteBook* notebook )
{
    return Glib::RefPtr<TabSwitchMenu>( new TabSwitchMenu( notebook ) );
}


TabSwitchMenu::TabSwitchMenu( DragableNoteBook* notebook )
    : Gio::Menu()
    , m_parentnote( notebook )
    , m_deactivated{ true }
{
}


/**
 * @brief メニュー項目を作り直してラベルとアイコンを更新する
 */
void TabSwitchMenu::update_labels_and_icons()
{
    // Gio::Menu に追加した Gio::MenuItem の属性は変更できないため
    // メニューを更新するときは item を全て削除して追加し直す
    remove_all();

    alloc_items();

    const int n_pages = m_parentnote->get_n_pages();
    for( int i = 0; i < n_pages; ++i ) {

        // ラベルの更新
        std::string label = m_parentnote->get_tab_fulltext( i );
        if( label.empty() ) label = "\?\?\?";
        const unsigned int maxsize = 50;
        m_items[i]->set_label( MISC::cut_str( label, maxsize ) );

        // アイコンの更新
        const int icon = m_parentnote->get_tabicon( i );
        if( icon != ICON::NONE && icon != ICON::NUM_ICONS ){
            m_items[i]->set_icon( ICON::get_icon( icon ) );
        }

        append_item( m_items[i] );
    }
    m_deactivated = false;
}


/**
 * @brief メニュー項目を作り直してアイコンを更新する
 */
void TabSwitchMenu::update_icons()
{
    // メニューが表示されてないときはアイコンの更新を行わない
    if( m_deactivated ) return;

    // Gio::Menu に追加した Gio::MenuItem の属性は変更できないため
    // メニューを更新するときは item を全て削除して追加し直す
    remove_all();

    alloc_items();

    const int n_pages = m_parentnote->get_n_pages();
    for( int i = 0; i < n_pages; ++i ) {

        // アイコンの更新
        const int icon = m_parentnote->get_tabicon( i );
        if( icon != ICON::NONE && icon != ICON::NUM_ICONS ){
            m_items[i]->set_icon( ICON::get_icon( icon ) );
        }

        append_item( m_items[i] );
    }
}


/**
 * @brief メニューがスクリーンから消されるときに呼び出す
 */
void TabSwitchMenu::deactivate()
{
    m_deactivated = true;
}


/**
 * @brief メニュー項目を必要な分だけ確保しておき使い回す
 */
void TabSwitchMenu::alloc_items()
{
    const Glib::ustring empty_label;
    const int n_pages = m_parentnote->get_n_pages();
    for( int i = m_items.size(); i < n_pages; ++i ) {
        m_items.push_back( Gio::MenuItem::create( empty_label, Glib::ustring::compose( "admin.tab-switch(%1)", i ) ) );
    }
}
