// ライセンス: GPL2

//#define _DEBUG
#include "gtkmmversion.h"
#include "jddebug.h"

#include "selectitempref.h"

#include "config/globalconf.h"
#include "jdlib/miscutil.h"

#include "global.h"
#include "session.h"

#if GTKMM_CHECK_VERSION(3,0,0)
#include <gdk/gdkkeysyms-compat.h>
#endif

using namespace SKELETON;

#define ROW_COLOR "WhiteSmoke"

SelectItemPref::SelectItemPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url, true, true ),
      m_button_top( Gtk::Stock::GOTO_TOP ),
      m_button_up( Gtk::Stock::GO_UP ),
      m_button_down( Gtk::Stock::GO_DOWN ),
      m_button_bottom( Gtk::Stock::GOTO_BOTTOM ),
      m_button_delete( Gtk::Stock::GO_FORWARD, std::string(), Gtk::ICON_SIZE_BUTTON ),
      m_button_add( Gtk::Stock::GO_BACK, std::string(), Gtk::ICON_SIZE_BUTTON ),
      m_button_default( Gtk::Stock::REVERT_TO_SAVED )
{
    m_list_default_data.clear();

    pack_widgets();
}


//
// widgetのパック
//
void SelectItemPref::pack_widgets()
{
    // 同一カラムにアイコンと項目名の両方を表示するので、手動で列を作成する

    // 表示項目リスト
    m_tree_shown.set_rules_hint( CONFIG::get_use_tree_gtkrc() );
    m_tree_shown.get_selection()->set_mode( Gtk::SELECTION_MULTIPLE );
    m_tree_shown.signal_focus_in_event().connect( sigc::mem_fun( *this, &SelectItemPref::slot_focus_in_shown ) );
    m_store_shown = Gtk::ListStore::create( m_columns_shown );
    m_tree_shown.set_model( m_store_shown );
    // "表示"の列を作成
    Gtk::TreeViewColumn* view_column_shown = Gtk::manage( new Gtk::TreeViewColumn( "表示" ) );
    m_tree_shown.append_column( *view_column_shown );
    // アイコン
    Gtk::CellRendererPixbuf* render_pixbuf_shown = Gtk::manage( new Gtk::CellRendererPixbuf() );
    view_column_shown->pack_start( *render_pixbuf_shown, false );
    view_column_shown->add_attribute( *render_pixbuf_shown, "pixbuf", 0 );
    // 項目名
    Gtk::CellRendererText* render_text_shown = Gtk::manage( new Gtk::CellRendererText() );
    view_column_shown->pack_start( *render_text_shown, true );
    view_column_shown->add_attribute( *render_text_shown, "text", 1 );


    // ボタン(縦移動)
    m_vbuttonbox_v.pack_start( m_button_top, Gtk::PACK_SHRINK );
    m_vbuttonbox_v.pack_start( m_button_up, Gtk::PACK_SHRINK );
    m_vbuttonbox_v.pack_start( m_button_down, Gtk::PACK_SHRINK );
    m_vbuttonbox_v.pack_start( m_button_bottom, Gtk::PACK_SHRINK );
    // ボタン(横移動)
    m_vbuttonbox_h.pack_start( m_button_delete, Gtk::PACK_SHRINK );
    m_vbuttonbox_h.pack_start( m_button_add, Gtk::PACK_SHRINK );
    // ボタン(アクション)
    m_vbuttonbox_action.pack_start( m_button_default, Gtk::PACK_SHRINK );

    // ボタン(スロット関数)
    m_button_top.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_top ) );
    m_button_up.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_up ) );
    m_button_down.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_down ) );
    m_button_bottom.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_bottom ) );
    m_button_delete.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_delete ) );
    m_button_add.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_add ) );
    m_button_default.signal_clicked().connect( sigc::mem_fun( *this, &SelectItemPref::slot_default ) );


    // 非表示項目リスト
    m_tree_hidden.set_rules_hint( CONFIG::get_use_tree_gtkrc() );
    m_tree_hidden.get_selection()->set_mode( Gtk::SELECTION_MULTIPLE );
    m_tree_hidden.signal_focus_in_event().connect( sigc::mem_fun( *this, &SelectItemPref::slot_focus_in_hidden ) );
    m_store_hidden = Gtk::ListStore::create( m_columns_hidden );
    m_tree_hidden.set_model( m_store_hidden );
    // "非表示"の列を作成
    Gtk::TreeViewColumn* view_column_hidden = Gtk::manage( new Gtk::TreeViewColumn( "非表示" ) );
    m_tree_hidden.append_column( *view_column_hidden );
    // アイコン
    Gtk::CellRendererPixbuf* render_pixbuf_hidden = Gtk::manage( new Gtk::CellRendererPixbuf() );
    view_column_hidden->pack_start( *render_pixbuf_hidden, false );
    view_column_hidden->add_attribute( *render_pixbuf_hidden, "pixbuf", 0 );
    // 項目名
    Gtk::CellRendererText* render_text_hidden = Gtk::manage( new Gtk::CellRendererText() );
    view_column_hidden->pack_start( *render_text_hidden, true );
    view_column_hidden->add_attribute( *render_text_hidden, "text", 1 );


    // 全体のパッキング
    m_scroll_shown.add( m_tree_shown );
    m_scroll_shown.set_size_request( 250, 300 );
    m_scroll_shown.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS );

    m_hbox.pack_start( m_scroll_shown, Gtk::PACK_EXPAND_WIDGET );

    m_vbuttonbox_v.set_layout( Gtk::BUTTONBOX_START );
    m_vbuttonbox_v.set_spacing( 4 );
    m_vbox.pack_start( m_vbuttonbox_v, Gtk::PACK_EXPAND_WIDGET );

    m_vbuttonbox_h.set_layout( Gtk::BUTTONBOX_EDGE );
    m_vbuttonbox_h.set_spacing( 4 );
    m_vbox.pack_start( m_vbuttonbox_h, Gtk::PACK_SHRINK );

    m_vbuttonbox_action.set_layout( Gtk::BUTTONBOX_END );
    m_vbuttonbox_action.set_spacing( 4 );
    m_vbox.pack_start( m_vbuttonbox_action, Gtk::PACK_EXPAND_WIDGET );

    m_hbox.pack_start( m_vbox, Gtk::PACK_SHRINK, 4 );

    m_scroll_hidden.add( m_tree_hidden );
    m_scroll_hidden.set_size_request( 250, 300 );
    m_scroll_hidden.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS );

    m_hbox.pack_start( m_scroll_hidden, Gtk::PACK_EXPAND_WIDGET );

    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_hbox );

    show_all_children();
}


//
// フォーカスが外れたTreeViewから項目の選択をなくす
//
bool SelectItemPref::slot_focus_in_shown( GdkEventFocus* event )
{
    m_tree_hidden.get_selection()->unselect_all();

    return true;
}

bool SelectItemPref::slot_focus_in_hidden( GdkEventFocus* event )
{
    m_tree_shown.get_selection()->unselect_all();

    return true;
}


//
// 項目名でデフォルトデータからアイコンを取得
//
Glib::RefPtr< Gdk::Pixbuf > SelectItemPref::get_icon( const Glib::ustring& name )
{
    Glib::RefPtr< Gdk::Pixbuf > icon;

    std::list< DEFAULT_DATA >::iterator it = m_list_default_data.begin();
    while( it != m_list_default_data.end() )
    {
        if( (*it).name == name )
        {
            icon = (*it).icon;
            break;
        }

        ++it;
    }

    return icon;
}


//
// 表示項目のクリア
//
void SelectItemPref::clear()
{
    m_store_shown->clear();
    m_store_hidden->clear();
}


//
// デフォルトデータを追加( 項目名、アイコンID )
//
void SelectItemPref::append_default_pair( const Glib::ustring& name,
                                          const Glib::RefPtr< Gdk::Pixbuf > icon )

{
    if( name.empty() ) return;

    DEFAULT_DATA default_data;
    default_data.name = name;
    default_data.icon = icon;

    m_list_default_data.push_back( default_data );
}


//
// 文字列を元に行を作成
//
void SelectItemPref::append_rows( const std::string& str )
{
    clear();

    bool found_separator = false;

    // "非表示"にデフォルトデータを全て追加
    std::list< DEFAULT_DATA >::iterator def_it = m_list_default_data.begin();
    while( def_it != m_list_default_data.end() )
    {
        // 2つ以上セパレータを追加しない
        if( (*def_it).name == ITEM_NAME_SEPARATOR )
        {
            if( ! found_separator ) append_hidden( (*def_it).name, false );
            found_separator = true;
        }
        else append_hidden( (*def_it).name, false );

        ++def_it;
    }

    if( str.empty() ) return;

    // "表示"に追加と不要な"非表示"を削除
    std::list< std::string > items = MISC::split_line( str );
    std::list< std::string >::iterator it = items.begin();
    while( it != items.end() )
    {
        if( append_shown( *it, false ) ) erase_hidden( *it );

        ++it;
    }
}


//
// 全ての有効な項目を文字列で取得
//
std::string SelectItemPref::get_items()
{
    std::string items;

    const Gtk::TreeModel::Children children = m_store_shown->children();
    Gtk::TreeModel::iterator it = children.begin();
    while( it != children.end() )
    {
        Gtk::TreeModel::Row row = *it;
        items.append( row[ m_columns_shown.m_column_text ] + " " );
        ++it;
    }

    return items;
}


//
// 表示項目に指定した項目を追加
//
Gtk::TreeRow SelectItemPref::append_shown( const std::string& name, const bool set_cursor )
{
    Gtk::TreeModel::Row row;

    if( SESSION::parse_item( name ) == ITEM_END ) return row;

    row = *( m_store_shown->append() );

    Glib::RefPtr< Gdk::Pixbuf > icon = get_icon( name );

    if( icon ) row[ m_columns_shown.m_column_icon ] = icon;
    row[ m_columns_shown.m_column_text ] = name;

    if( set_cursor ){
        m_tree_shown.get_selection()->select( row );
        m_tree_shown.set_cursor( m_store_shown->get_path( row ) );
    }

    return row;
}


//
// 非表示項目に指定した項目を追加
//
Gtk::TreeRow SelectItemPref::append_hidden( const std::string& name, const bool set_cursor )
{
    Gtk::TreeModel::Row row = *( m_store_hidden->append() );

    Glib::RefPtr< Gdk::Pixbuf > icon = get_icon( name );

    if( icon ) row[ m_columns_hidden.m_column_icon ] = icon;
    row[ m_columns_hidden.m_column_text ] = name;

    if( set_cursor ){
        m_tree_hidden.get_selection()->select( row );
        m_tree_hidden.set_cursor( m_store_hidden->get_path( row ) );
    }

    return row;
}


//
// 非表示項目から指定した項目を削除
//
void SelectItemPref::erase_hidden( const std::string& name )
{
    if( name == ITEM_NAME_SEPARATOR ) return;

    const Gtk::TreeModel::Children children = m_store_hidden->children();
    Gtk::TreeModel::iterator it = children.begin();
    while( it != children.end() )
    {
        Gtk::TreeModel::Row row = *it;
        if( row[ m_columns_hidden.m_column_text ] == name )
        {
            m_store_hidden->erase( *it );
            break;
        }

        ++it;
    }
}


//
// KeyPressのフック
//
bool SelectItemPref::on_key_press_event( GdkEventKey* event )
{
    bool hook = false;

    // "Gtk::Dialog"は"Esc"を押すと閉じられてしまうので
    // "GDK_Escape"をキャンセルする
    switch( event->keyval )
    {
        case GDK_Escape :
            hook = true;
            break;
    }

    m_sig_key_press.emit( event );
    if( hook ) return true;

    return Gtk::Dialog::on_key_press_event( event );
}


//
// KeyReleaseのフック
//
bool SelectItemPref::on_key_release_event( GdkEventKey* event )
{
    bool hook = false;

    switch( event->keyval )
    {
        case GDK_Escape :
            hook = true;
            m_tree_shown.get_selection()->unselect_all();
            m_tree_hidden.get_selection()->unselect_all();
            break;

        case GDK_Right :
            slot_delete();
            break;

        case GDK_Left :
            slot_add();
            break;
    }

    m_sig_key_release.emit( event );
    if( hook ) return true;

    return Gtk::Dialog::on_key_release_event( event );
}


//
// 上端へ移動
//
void SelectItemPref::slot_top()
{
    Gtk::TreeModel::Children children = m_store_shown->children();
    if( children.empty() ) return;

    // 移動先のイテレータ
    Gtk::TreeIter upper_it = children.begin();

    std::vector< Gtk::TreePath > selection_path = m_tree_shown.get_selection()->get_selected_rows();
    std::vector< Gtk::TreePath >::iterator it = selection_path.begin();
    while( it != selection_path.end() )
    {
        Gtk::TreeIter src_it = m_store_shown->get_iter( *it );
        Gtk::TreeIter dst_it = upper_it;

        // 元と先が同じでない
        if( src_it != dst_it )
        {
            // 参照渡しなので書き換えられてしまう
            m_store_shown->move( src_it, dst_it );
            upper_it = src_it;
        }

        // 移動先の位置を下げる
        if( upper_it != children.end() ) ++upper_it;

        ++it;
    }

    // フォーカスを移す
    set_focus( m_tree_shown );
}


//
// 上へ移動
//
void SelectItemPref::slot_up()
{
    Gtk::TreeModel::Children children = m_store_shown->children();
    if( children.empty() ) return;

    // 上限のイテレータ
    Gtk::TreeIter upper_it = children.begin();

    std::vector< Gtk::TreePath > selection_path = m_tree_shown.get_selection()->get_selected_rows();
    std::vector< Gtk::TreePath >::iterator it = selection_path.begin();
    while( it != selection_path.end() )
    {
        Gtk::TreePath src = *it;
        Gtk::TreeIter src_it = m_store_shown->get_iter( src );

        // 限度位置に達していなければ入れ替え
        if( src_it != upper_it )
        {
            Gtk::TreePath dst( src );

            if( dst.prev() )
            {
                Gtk::TreeIter dst_it = m_store_shown->get_iter( dst );

                m_store_shown->iter_swap( src_it, dst_it );
            }
        }
        // 上限に達していたら次の限度位置を下げる
        else if( upper_it != children.end() ) ++upper_it;

        ++it;
    }

    // フォーカスを移す
    set_focus( m_tree_shown );
}


//
// 下へ移動
//
void SelectItemPref::slot_down()
{
    Gtk::TreeModel::Children children = m_store_shown->children();
    if( children.empty() ) return;

    // 下限のイテレータ
    Gtk::TreeIter bottom_it = --children.end();

    std::vector< Gtk::TreePath > selection_path = m_tree_shown.get_selection()->get_selected_rows();
    std::vector< Gtk::TreePath >::reverse_iterator it = selection_path.rbegin();
    while( it != selection_path.rend() )
    {
        Gtk::TreePath src = *it;
        Gtk::TreeIter src_it = m_store_shown->get_iter( src );

        // 限度位置に達していなければ入れ替え
        if( src_it != bottom_it )
        {
            Gtk::TreePath dst( src );

            dst.next();

            Gtk::TreeIter dst_it = m_store_shown->get_iter( dst );

            if( dst_it )
            {
                m_store_shown->iter_swap( src_it, dst_it );
            }
        }
        // 下限に達していたら次の限度位置を上げる
        else if( bottom_it != children.begin() ) --bottom_it;

        ++it;
    }

    // フォーカスを移す
    set_focus( m_tree_shown );
}


//
// 下端へ移動
//
void SelectItemPref::slot_bottom()
{
    Gtk::TreeModel::Children children = m_store_shown->children();
    if( children.empty() ) return;

    // 移動先のイテレータ
    Gtk::TreeIter bottom_it = children.end();

    std::vector< Gtk::TreePath > selection_path = m_tree_shown.get_selection()->get_selected_rows();
    std::vector< Gtk::TreePath >::reverse_iterator it = selection_path.rbegin();
    while( it != selection_path.rend() )
    {
        Gtk::TreeIter src_it = m_store_shown->get_iter( *it );
        Gtk::TreeIter dst_it = bottom_it;

        // 元と先が同じでない
        if( src_it != dst_it )
        {
            // 参照渡しなので書き換えられてしまう
            m_store_shown->move( src_it, dst_it );
            bottom_it = dst_it;
        }

        // 移動先の位置を上げる
        if( bottom_it != children.begin() ) --bottom_it;

        ++it;
    }

    // フォーカスを移す
    set_focus( m_tree_shown );
}


//
// 削除ボタン
//
void SelectItemPref::slot_delete()
{
    std::vector< Gtk::TreePath > selection_path = m_tree_shown.get_selection()->get_selected_rows();

    // 選択したアイテムが無い場合はフォーカスだけ移して出る
    if( selection_path.empty() )
    {
        set_focus( m_tree_hidden );
        return;
    }

    std::list< Gtk::TreeRow > erase_rows;
    bool set_cursor = true;  // 一番上の選択項目にカーソルをセット

    std::vector< Gtk::TreePath >::iterator it = selection_path.begin();
    while( it != selection_path.end() )
    {
        Gtk::TreePath path = *it;
        Gtk::TreeRow row = *m_store_shown->get_iter( path );

        if( row )
        {
            Glib::ustring name = row[ m_columns_shown.m_column_text ];

            // 非表示項目に追加
            if( name != ITEM_NAME_SEPARATOR )
            {
                append_hidden( name, set_cursor );
                set_cursor = false;
            }

            // 表示項目から削除するリストに追加
            erase_rows.push_back( row );
        }

        ++it;
    }

    // 追加と削除を同時にすると滅茶苦茶になるので、分けて削除する
    std::list< Gtk::TreeRow >::iterator erase_it = erase_rows.begin();
    while( erase_it != erase_rows.end() )
    {
        m_store_shown->erase( *erase_it );

        ++erase_it;
    }

    // フォーカスを移す
    set_focus( m_tree_hidden );
}


//
// 追加ボタン
//
void SelectItemPref::slot_add()
{
    std::vector< Gtk::TreePath > selection_path = m_tree_hidden.get_selection()->get_selected_rows();

    // 選択したアイテムが無い場合はフォーカスだけ移して出る
    if( selection_path.empty() )
    {
        set_focus( m_tree_shown );
        return;
    }

    std::list< Gtk::TreeRow > erase_rows;
    bool set_cursor = true; // 一番上の選択項目にカーソルをセット

    // 選択したアイテムを追加
    std::vector< Gtk::TreePath >::iterator it = selection_path.begin();
    while( it != selection_path.end() )
    {
        Gtk::TreeRow row = *m_store_hidden->get_iter( *it );

        if( row )
        {
            Glib::ustring name = row[ m_columns_hidden.m_column_text ];

            // 表示項目に追加
            append_shown( name, set_cursor );
            set_cursor = false;

            // 非表示項目から削除するリストに追加
            if( name != ITEM_NAME_SEPARATOR ) erase_rows.push_back( row );
        }

        ++it;
    }

    // 追加と削除を同時にすると滅茶苦茶になるので、分けて削除する
    std::list< Gtk::TreeRow >::iterator erase_it = erase_rows.begin();
    while( erase_it != erase_rows.end() )
    {
        m_store_hidden->erase( *erase_it );

        ++erase_it;
    }

    // フォーカスを移す
    set_focus( m_tree_shown );
}
