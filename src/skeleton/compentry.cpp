// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "compentry.h"

#include "control/controlid.h"

#include "compmanager.h"

#include <algorithm>


using namespace SKELETON;


namespace ce {
constexpr int kPopupSize = 5;
}


CompletionEntry::CompletionEntry( const int mode )
    : m_mode( mode )
    , m_enable_changed( true )
    , m_popup_win( SKELETON::POPUPWIN_DRAWFRAME )
{
    m_entry.signal_key_press().connect( sigc::mem_fun( *this, &CompletionEntry::slot_entry_key_press ) );
    m_entry.signal_button_press().connect( sigc::mem_fun(*this, &CompletionEntry::slot_entry_button_press ) );
    m_entry.signal_operate().connect( sigc::mem_fun( *this, &CompletionEntry::slot_entry_operate ) );
    m_entry.signal_activate().connect( sigc::mem_fun( *this, &CompletionEntry::slot_entry_acivate ) );
    m_entry.signal_changed().connect( sigc::mem_fun( *this, &CompletionEntry::slot_entry_changed ) );
    m_entry.signal_focus_out_event().connect( sigc::mem_fun(*this, &CompletionEntry::slot_entry_focus_out ) );
    m_entry.set_max_width_chars( 1 );
    m_entry.set_width_chars( 1 );
    m_entry.set_hexpand( true );
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

    // ウインドウを移動させる gdk_window_move_to_rect() は Gdk::Window が作成されてないと失敗する
    // そのため、初回のポップアップ配置は Gdk::Window が関連付けられたときに行う
    m_popup_win.signal_realize().connect( sigc::mem_fun( *this, &CompletionEntry::place_popup ) );

    set_size_request( 0 );
}


CompletionEntry::~CompletionEntry() noexcept = default;


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


/**
 * @brief 入力補完のポップアップを配置する
 */
void CompletionEntry::place_popup()
{
    // ポップアップの表示位置を計算する
    int x, y;
    translate_coordinates( *get_toplevel(), 0, 0, x, y );
    const GdkRectangle rect_dest = { x, y, get_allocated_width(), get_allocated_height() };
#ifdef _DEBUG
    std::cout << "CompletionEntry::place_popup: coords(x, y) = " << x << ", " << y
              << "; (width, height) = " << rect_dest.width << ", " << rect_dest.height
              << std::endl;
#endif

    // ウインドウが表示されていると、位置の変更が適応されないウインドウマネージャーがあるので非表示にする
    hide_popup();

    // Entryの左下角と、ポップアップの左上角を合わせる
    constexpr GdkGravity rect_anchor = GDK_GRAVITY_SOUTH_WEST;
    constexpr GdkGravity window_anchor = GDK_GRAVITY_NORTH_WEST;

    constexpr GdkAnchorHints anchor_hints{};
    constexpr int offset = 0;
    gdk_window_move_to_rect( m_popup_win.get_window()->gobj(), &rect_dest,
                             rect_anchor, window_anchor, anchor_hints, offset, offset );
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

    for( const std::string& comp : complist ) {
        if( comp != m_entry.get_text() ) {
            row = *( m_liststore->append() );
            row[ m_column ] = comp;
        }
    }

    const int size = m_liststore->children().size();
    if( ! size ){
        hide_popup();
        return;
    }

    // 座標と大きさを計算してポップアップ表示

    // gdk_window_move_to_rect() を使うためにポップアップの一時的な親ウインドウを設定する
    Gtk::Widget* toplevel = get_toplevel();
    if( auto win = dynamic_cast<Gtk::Window*>( toplevel ); win ) {
        m_popup_win.set_transient_for( *win );
    }

    // Entryの幅に合わせてポップアップの幅を調整する
    const int cell_h = m_treeview.get_row_height() + mrg;
    m_popup_win.resize( get_width(), cell_h * (std::min)( ce::kPopupSize, size ) + mrg );

    // Gdk::Window が関連付けられているならここでポップアップを配置する
    if( m_popup_win.get_realized() ) place_popup();

    m_popup_win.show_all();
    m_show_popup = true;

    m_treeview.unset_cursor();
    m_treeview.scroll_to_point( -1, 0 );
    Gdk::RGBA rgba;
    if( !get_style_context()->lookup_color( "theme_bg_color", rgba ) ) {
#ifdef _DEBUG
        std::cout << "ERROR:CompletionEntry::show_popup() "
                  << "lookup theme_bg_color faild." << std::endl;
#endif
    }
    m_treeview.get_column_cell_renderer( 0 )->property_cell_background_rgba() = rgba;
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
