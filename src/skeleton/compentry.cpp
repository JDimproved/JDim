// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "compentry.h"

#include "control/controlid.h"

#include "compmanager.h"

using namespace SKELETON;

enum
{
    POPUP_SIZE = 5
};


CompletionEntry::CompletionEntry( const int mode )
    : m_mode( mode ),
      m_enable_changed( true ),
      m_focused( false ),
      m_show_popup( false ),
      m_popup_win( true )
{
    m_entry.signal_key_press().connect( sigc::mem_fun( *this, &CompletionEntry::slot_entry_key_press ) );
    m_entry.signal_button_press().connect( sigc::mem_fun(*this, &CompletionEntry::slot_entry_button_press ) );
    m_entry.signal_operate().connect( sigc::mem_fun( *this, &CompletionEntry::slot_entry_operate ) );
    m_entry.signal_activate().connect( sigc::mem_fun( *this, &CompletionEntry::slot_entry_acivate ) );
    m_entry.signal_changed().connect( sigc::mem_fun( *this, &CompletionEntry::slot_entry_changed ) );
    m_entry.signal_focus_out_event().connect( sigc::mem_fun(*this, &CompletionEntry::slot_entry_focus_out ) );
    pack_start( m_entry );

    // ポップアップ
    m_column_record.add( m_column );
    m_liststore = Gtk::ListStore::create( m_column_record );
    m_treeview.set_model( m_liststore );
    m_treeview.append_column( "", m_column );
    m_treeview.set_headers_visible( false );
    m_treeview.sig_motion_notify().connect( sigc::mem_fun(*this, &CompletionEntry::slot_treeview_motion ) );
    m_treeview.sig_button_release().connect( sigc::mem_fun(*this, &CompletionEntry::slot_treeview_button_release ) );

    m_scr_win.add( m_treeview );
    m_scr_win.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC );
    m_scr_win.set_size_request( 1, 1 );

    m_popup_win.add( m_scr_win );

    set_size_request( 0 );
}


CompletionEntry::~CompletionEntry()
{}


// 補完実行
bool CompletionEntry::completion()
{
    bool ret = false;

    if( m_show_popup ){
        Gtk::TreeModel::Row row = m_treeview.get_current_row();
        if( row ){
            set_text( row[ m_column ] );
            ret = true;
        }
    }
    hide_popup();

    return ret;
}


Glib::ustring CompletionEntry::get_text()
{
    return m_entry.get_text();
}


void CompletionEntry::set_text( const Glib::ustring& text )
{
    m_enable_changed = false;
    m_entry.set_text( text );
    m_entry.set_position( text.length() );
    m_enable_changed = true;
}


void CompletionEntry::grab_focus()
{
#ifdef _DEBUG
    std::cout << "CompletionEntry::grab_focus\n";
#endif

    m_focused = true;
    m_entry.grab_focus();
}


//
// ポップアップ表示
//
// show_all == true なら候補を全て表示する
//
void CompletionEntry::show_popup( const bool show_all )
{
    const int mrg = 2;

    std::string query = m_entry.get_text();
    if( ! show_all && query.empty() ){
        hide_popup();
        return;
    }

    if( show_all ) query = std::string();

    CORE::COMPLIST complist = CORE::get_completion_manager()->get_list( m_mode, query );
    if( ! complist.size() ) hide_popup();

    m_liststore->clear();
    Gtk::TreeModel::Row row;

    CORE::COMPLIST_ITERATOR it = complist.begin();
    for( ; it != complist.end(); ++it ){
        if( *it != m_entry.get_text() ){
            row = *( m_liststore->append() );
            row[ m_column ] = *it;
        }
    }

    const int size = m_liststore->children().size();
    if( ! size ){
        hide_popup();
        return;
    }

    // 座標と大きさを計算してポップアップ表示

    const int cell_h = m_treeview.get_row_height() + mrg;

    int x, y;
    get_window()->get_origin( x, y );

    Gdk::Rectangle rect = get_allocation();
    m_popup_win.move( x + rect.get_x(), y + rect.get_y() + rect.get_height() );
    m_popup_win.resize( get_width(), cell_h * MIN( POPUP_SIZE, size ) + mrg );
    m_popup_win.show_all();
    m_show_popup = true;

    m_treeview.unset_cursor();
    m_treeview.scroll_to_point( -1, 0 );
    m_treeview.get_column_cell_renderer( 0 )->property_cell_background_gdk() = get_style()->get_bg( Gtk::STATE_NORMAL );
}


// ポップアップ閉じる
void CompletionEntry::hide_popup()
{
    if( m_show_popup ){
        m_popup_win.hide();
        m_show_popup = false;
    }
}


// entryでキーが押された
void CompletionEntry::slot_entry_key_press( int keyval )
{
#ifdef _DEBUG    
    std::cout << "CompletionEntry::slot_entry_key_press keyval = " << keyval << std::endl;
#endif
}


// entryでボタンを押した
void CompletionEntry::slot_entry_button_press( GdkEventButton* event )
{
    if( event->type != GDK_BUTTON_PRESS ) return;

#ifdef _DEBUG    
    std::cout << "CompletionEntry::slot_entry_button_press button = " << event->button
              << " focused = " << m_focused << std::endl;
#endif

    if( m_show_popup ) hide_popup();
    // 右クリックならコンテキストメニューを優先して補完候補は表示しない
    else if( m_focused && event->button != 3 ) {
        show_popup( m_entry.get_text().empty() );
    }

    m_focused = true;
}


// entry操作
void CompletionEntry::slot_entry_operate( int controlid )
{
#ifdef _DEBUG    
    std::cout << "CompletionEntry::slot_entry_operate id = " << controlid << std::endl;
#endif

    switch( controlid ){

        case CONTROL::Up:
            if( ! m_treeview.row_up() ) m_treeview.goto_bottom();
            break;

        case CONTROL::Down:
            if( ! m_treeview.row_down() ) m_treeview.goto_top();
            break;

        case CONTROL::Cancel:
            if( m_show_popup ) hide_popup();
            else m_sig_operate.emit( controlid );
            break;

        default:
            m_sig_operate.emit( controlid );
    }

}


// entry からsignal_activateを受け取った
void CompletionEntry::slot_entry_acivate()
{
#ifdef _DEBUG    
    std::cout << "CompletionEntry::slot_entry_acivate\n";
#endif

    if( ! completion() ) m_sig_activate.emit();
}


// entry からsignal_changedを受け取った
void CompletionEntry::slot_entry_changed()
{
    if( m_enable_changed ){

        if( m_entry.get_text().empty() ) hide_popup();
        else show_popup( false );
    }

    m_sig_changed.emit();
}


// entryのフォーカスが外れた
bool CompletionEntry::slot_entry_focus_out( GdkEventFocus* )
{
#ifdef _DEBUG    
    std::cout << "CompletionEntry::slot_entry_focus_out\n";
#endif

    hide_popup();
    m_focused = false;

    return true;
}


// ポップアップ内をマウスを動かした
bool CompletionEntry::slot_treeview_motion( GdkEventMotion* )
{
    Gtk::TreeModel::Path path = m_treeview.get_path_under_mouse();
    if( path.size() > 0 ) m_treeview.set_cursor( path );

    return true;
}


// ポップアップクリック
bool CompletionEntry::slot_treeview_button_release( GdkEventButton* )
{
#ifdef _DEBUG    
    std::cout << "CompletionEntry::slot_treeview_button_release\n";
#endif

    hide_popup();
    Gtk::TreeModel::Row row = m_treeview.get_current_row();
    if( row ){
        set_text( row[ m_column ] );
        hide_popup();
    }

    return true;
}
