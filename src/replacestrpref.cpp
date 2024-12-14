// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "replacestrpref.h"
#include "replacestrmanager.h"

#include "config/globalconf.h"
#include "control/controlid.h"
#include "jdlib/miscgtk.h"
#include "skeleton/msgdiag.h"
#include "xml/document.h"

#include "command.h"
#include "environment.h"

#include <algorithm>
#include <iterator>


using namespace CORE;


ReplaceStrDiag::ReplaceStrDiag( Gtk::Window* parent, ReplaceStrCondition condition,
                                const Glib::ustring& pattern, const Glib::ustring& replace )
    : SKELETON::PrefDiag( parent, "" )
    , m_button_copy( "この設定をクリップボードにコピー(_L)", true )
    , m_check_active( "有効(_E)", true )
    , m_check_icase( "大文字小文字(_I)", true )
    , m_check_regex( "正規表現(_R)", true )
    , m_check_wchar( "全角半角(_W)", true )
    , m_check_norm( "互換文字(_N)", true )
    , m_label_pattern( "置換パターン(_P)：", true )
    , m_label_replace( "置換文字列(_S)：", true )
{
    resize( 600, 1 );

    m_button_copy.signal_clicked().connect( sigc::mem_fun( *this, &ReplaceStrDiag::slot_copy ) );
    m_check_regex.signal_clicked().connect( sigc::mem_fun( *this, &ReplaceStrDiag::slot_sens ) );
    m_check_wchar.signal_clicked().connect( sigc::mem_fun( *this, &ReplaceStrDiag::slot_sens ) );
    m_check_norm.signal_clicked().connect( sigc::mem_fun( *this, &ReplaceStrDiag::slot_sens ) );

    m_check_active.set_active( condition.active );
    m_check_icase.set_active( condition.icase );
    m_check_regex.set_active( condition.regex );
    m_check_wchar.set_active( condition.wchar );
    m_check_wchar.set_sensitive( condition.regex && ! condition.norm );
    m_check_norm.set_active( condition.norm );
    m_check_norm.set_sensitive( condition.regex && condition.wchar );

    m_check_active.set_tooltip_text( "この条件の置換を有効にする" );
    m_check_icase.set_tooltip_text( "大文字小文字を区別しない" );
    m_check_regex.set_tooltip_text( "正規表現を使用する" );
    m_check_wchar.set_tooltip_text( "英数字とカナの文字幅(いわゆる全角半角)を区別しない" );
    m_check_norm.set_tooltip_text( "Unicodeの互換文字を区別しない" );

    m_hbox_regex.pack_start( m_check_regex, Gtk::PACK_SHRINK );
    m_hbox_regex.pack_start( m_check_icase, Gtk::PACK_SHRINK );
    m_hbox_regex.pack_start( m_check_wchar, Gtk::PACK_SHRINK );
    m_hbox_regex.pack_start( m_check_norm, Gtk::PACK_SHRINK );

    m_entry_pattern.set_text( pattern );
    m_entry_replace.set_text( replace );
    m_entry_pattern.set_hexpand( true );
    m_entry_replace.set_hexpand( true );
    m_label_pattern.set_mnemonic_widget( m_entry_pattern );
    m_label_replace.set_mnemonic_widget( m_entry_replace );
    m_label_pattern.set_xalign( 0 );
    m_label_replace.set_xalign( 0 );
    m_grid_entry.attach( m_label_pattern, 0, 0, 1, 1 );
    m_grid_entry.attach( m_entry_pattern, 1, 0, 1, 1 );
    m_grid_entry.attach( m_label_replace, 0, 1, 1, 1 );
    m_grid_entry.attach( m_entry_replace, 1, 1, 1, 1 );
    m_grid_entry.set_row_spacing( 8 );

    m_hbox_active.pack_start( m_check_active );
    m_hbox_active.pack_start( m_button_copy, Gtk::PACK_SHRINK );

    get_content_area()->set_spacing( 8 );
    get_content_area()->pack_start( m_hbox_active, Gtk::PACK_SHRINK );
    get_content_area()->pack_start( m_hbox_regex, Gtk::PACK_SHRINK );
    get_content_area()->pack_start( m_grid_entry, Gtk::PACK_SHRINK );

    set_title( "置換条件設定" );
    show_all_children();
}


//
// 条件フラグ取得
//
ReplaceStrCondition ReplaceStrDiag::get_condition() const
{
    ReplaceStrCondition condition{};

    condition.active = get_active();
    condition.icase = get_icase();
    condition.regex = get_regex();
    condition.wchar = get_wchar();
    condition.norm = get_norm();

    return condition;
}


//
// クリップボードにコピー
//
void ReplaceStrDiag::slot_copy()
{
    XML::Document document;
    const XML::Dom* node = ReplaceStr_Manager::dom_append( &document, get_condition(), get_pattern(), get_replace() );

    MISC::CopyClipboard( node->get_xml() );
}


//
// チェックボックスのsensitive切り替え
//
void ReplaceStrDiag::slot_sens()
{
    if( ! get_regex() ) {
        m_check_wchar.set_active( false );
        m_check_norm.set_active( false );
    }
    m_check_wchar.set_sensitive( get_regex() && ! get_norm() );
    m_check_norm.set_sensitive( get_regex() && get_wchar() );
}


///////////////////////////////////////////////


ReplaceStrPref::ReplaceStrPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url )
    , m_id_target( REPLACETARGET_MAX - 1 ) // 初期設定は本文
    , m_label_target( "置換対象(_R)：", true )
    , m_link_manual( ENVIRONMENT::get_jdhelpreplstr(), "オンラインマニュアル(_M)" )
    , m_check_chref( "対象の置換前に文字参照をデコード(_E)", true )
    , m_chref( REPLACETARGET_MAX )
    , m_store( REPLACETARGET_MAX )
    , m_button_top( g_dpgettext( GTK_DOMAIN, "Stock label, navigation\x04_Top", 24 ), true )
    , m_button_up( g_dpgettext( GTK_DOMAIN, "Stock label, navigation\x04_Up", 24 ), true )
    , m_button_down( g_dpgettext( GTK_DOMAIN, "Stock label, navigation\x04_Down", 24 ), true )
    , m_button_bottom( g_dpgettext( GTK_DOMAIN, "Stock label, navigation\x04_Bottom", 24 ), true )
    , m_button_delete( g_dpgettext( GTK_DOMAIN, "Stock label\x04_Delete", 12 ), true )
    , m_button_add( g_dpgettext( GTK_DOMAIN, "Stock label\x04_Add", 12 ), true )
    , m_vbuttonbox{ Gtk::ORIENTATION_VERTICAL }
{
    const bool use_symbolic = CONFIG::get_use_symbolic_icon();
    m_button_top.set_image_from_icon_name( use_symbolic ? "go-top-symbolic" : "go-top" );
    m_button_up.set_image_from_icon_name( use_symbolic ? "go-up-symbolic" : "go-up" );
    m_button_down.set_image_from_icon_name( use_symbolic ? "go-down-symbolic" : "go-down" );
    m_button_bottom.set_image_from_icon_name( use_symbolic ? "go-bottom-symbolic" : "go-bottom" );

    m_button_top.signal_clicked().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_top ) );
    m_button_up.signal_clicked().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_up ) );
    m_button_down.signal_clicked().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_down ) );
    m_button_bottom.signal_clicked().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_bottom ) );
    m_button_delete.signal_clicked().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_delete ) );
    m_button_add.signal_clicked().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_add ) );

    std::generate( m_store.begin(), m_store.end(), [this] { return Gtk::ListStore::create( m_columns ); } );
    m_current_store = m_store.back();
    m_treeview.set_model( m_current_store );
    m_treeview.set_size_request( 640, 400 );
    m_treeview.signal_row_activated().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_row_activated ) );
    m_treeview.sig_key_press().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_key_press ) );
    m_treeview.sig_key_release().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_key_release ) );

    std::vector<Gtk::TreeViewColumn*> columns( m_columns.size() );

    columns[0] = Gtk::manage( new Gtk::TreeViewColumn( "有効", m_columns.m_col_active ) );
    columns[0]->set_fixed_width( 35 );
    columns[0]->set_alignment( Gtk::ALIGN_CENTER );
    // 正規と大小は設定の関係性から元になったパッチから変更して列を入れ替えた
    columns[1] = Gtk::manage( new Gtk::TreeViewColumn( "正規", m_columns.m_col_regex ) );
    columns[1]->set_fixed_width( 35 );
    columns[1]->set_alignment( Gtk::ALIGN_CENTER );
    columns[2] = Gtk::manage( new Gtk::TreeViewColumn( "大小", m_columns.m_col_icase ) );
    columns[2]->set_fixed_width( 35 );
    columns[2]->set_alignment( Gtk::ALIGN_CENTER );
    columns[3] = Gtk::manage( new Gtk::TreeViewColumn( "全角", m_columns.m_col_wchar ) );
    columns[3]->set_fixed_width( 35 );
    columns[3]->set_alignment( Gtk::ALIGN_CENTER );
    columns[4] = Gtk::manage( new Gtk::TreeViewColumn( "互換", m_columns.m_col_norm ) );
    columns[4]->set_fixed_width( 35 );
    columns[4]->set_alignment( Gtk::ALIGN_CENTER );
    columns[5] = Gtk::manage( new Gtk::TreeViewColumn( "置換パターン", m_columns.m_col_pattern ) );
    columns[5]->set_fixed_width( 220 );
    columns[6] = Gtk::manage( new Gtk::TreeViewColumn( "置換文字列", m_columns.m_col_replace ) );
    columns[6]->set_fixed_width( 200 );

    for( Gtk::TreeViewColumn* col : columns ) {
        col->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
        col->set_resizable( true );
        m_treeview.append_column( *col );
    }

    m_scrollwin.add( m_treeview );
    m_scrollwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
    m_scrollwin.set_size_request( 640, 400 );

    m_vbuttonbox.pack_start( m_button_top, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_up, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_down, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_bottom, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_delete, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_add, Gtk::PACK_SHRINK );
    m_vbuttonbox.set_spacing( 4 );

    m_hbox.pack_start( m_scrollwin, Gtk::PACK_EXPAND_WIDGET );
    m_hbox.pack_start( m_vbuttonbox, Gtk::PACK_SHRINK );

    for( const char* target : kReplStrTargetLabels ) {
        m_menu_target.append( target );
    }
    m_menu_target.signal_changed().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_target_changed ) );
    m_menu_target.set_active_text( kReplStrTargetLabels.back() );
    m_label_target.set_mnemonic_widget( m_menu_target );

    m_grid_head.attach( m_label_target, 0, 0, 1, 1 );
    m_grid_head.attach( m_menu_target, 1, 0, 1, 1 );
    m_grid_head.attach( m_link_manual, 2, 0, 1, 1 );
    m_grid_head.attach( m_check_chref, 0, 1, 2, 1 );
    m_grid_head.set_hexpand( true );
    m_link_manual.set_halign( Gtk::ALIGN_END );
    m_link_manual.set_hexpand( true );
    m_link_manual.set_use_underline( true );

    m_check_chref.set_tooltip_markup( "ダブルクォーテション<b>\"</b>, アンパサント"
                                      "<b>&amp;</b>, 少なり記号<b>&lt;</b>, 大なり記号<b>&gt;</b>を<b>除く</b>"
                                      "文字参照をデコードしてから置換を行います。" );

    get_content_area()->set_spacing( 8 );
    get_content_area()->pack_start( m_grid_head, Gtk::PACK_SHRINK );
    get_content_area()->pack_start( m_hbox );

    show_all_children();
    m_treeview.grab_focus();
    set_title( "文字列置換設定" );

    append_rows();
}


void ReplaceStrPref::append_rows()
{
    const ReplaceStr_Manager* const mgr = CORE::get_replacestr_manager();

    for( int i = 0; i < REPLACETARGET_MAX; ++i ) {

        m_chref[i] = mgr->get_chref( i );

        for( const auto& item : mgr->get_list( i ) ) {
            append_row( m_store[i], item.condition, item.pattern, item.replace );
        }
    }

    const int id = m_menu_target.get_active_row_number();
    m_check_chref.set_active( m_chref[id] );
    select_row( get_top_row() );
}


void ReplaceStrPref::append_row( const Glib::RefPtr<Gtk::ListStore>& store, ReplaceStrCondition condition,
                                 const std::string& pattern, const std::string& replace )
{
    Gtk::TreeModel::Row row = *store->append();

    if( row ) {

        row[ m_columns.m_col_active ] = condition.active;
        row[ m_columns.m_col_icase ] = condition.icase;
        row[ m_columns.m_col_regex ] = condition.regex;
        row[ m_columns.m_col_wchar ] = condition.wchar;
        row[ m_columns.m_col_norm ] = condition.norm;
        row[ m_columns.m_col_pattern ] = pattern;
        row[ m_columns.m_col_replace ] = replace;

        select_row( row );
    }
}


Gtk::TreeModel::const_iterator ReplaceStrPref::get_selected_row() const
{
    std::vector<Gtk::TreeModel::Path> paths = m_treeview.get_selection()->get_selected_rows();

    if( paths.empty() ) return Gtk::TreeModel::const_iterator();
    return *( m_current_store->get_iter( paths.front() ) );
}


Gtk::TreeModel::const_iterator ReplaceStrPref::get_top_row() const
{
    Gtk::TreeModel::Children children = m_current_store->children();

    if( children.empty() ) return Gtk::TreeModel::const_iterator();
    return children.begin();
}


Gtk::TreeModel::const_iterator ReplaceStrPref::get_bottom_row() const
{
    Gtk::TreeModel::Children children = m_current_store->children();

    if( children.empty() ) return Gtk::TreeModel::const_iterator();
    return std::prev( children.end() );
}


void ReplaceStrPref::select_row( const Gtk::TreeModel::const_iterator& it )
{
    if( ! it ) return;

    const Gtk::TreePath path( it );
    m_treeview.get_selection()->select( path );
}


//
// OK ボタンを押した
//
void ReplaceStrPref::slot_ok_clicked()
{
    ReplaceStr_Manager* const mgr = CORE::get_replacestr_manager();

    m_chref[ m_id_target ] = m_check_chref.get_active();

    for( int i = 0; i < REPLACETARGET_MAX; ++i ) {

        mgr->list_clear( i );

        mgr->set_chref( i, m_chref[i] );

        for( const Gtk::TreeModel::Row& row : m_store[i]->children() ) {

            const bool active = row[ m_columns.m_col_active ];
            const bool icase = row[ m_columns.m_col_icase ];
            const bool regex = row[ m_columns.m_col_regex ];
            const bool wchar = row[ m_columns.m_col_wchar ];
            const bool norm = row[ m_columns.m_col_norm ];

            const ReplaceStrCondition condition{ active, icase, regex, wchar, norm };
            const Glib::ustring& pattern = row[ m_columns.m_col_pattern ];
            const Glib::ustring& replace = row[ m_columns.m_col_replace ];

            mgr->list_append( i, condition, pattern.raw(), replace.raw() );
        }
    }

    mgr->save_xml();

    //DBTREE::update_abone_thread();
    CORE::core_set_command( "relayout_all_board" );
    CORE::core_set_command( "relayout_all_article", "", "completely" );
}


void ReplaceStrPref::slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* )
{
#ifdef _DEBUG
    std::cout << "ReplaceStrPref::slot_row_activated path = " << path.to_string() << std::endl;
#endif

    Gtk::TreeModel::Row row = *( m_current_store->get_iter( path ) );
    if( ! row ) return;

    const bool active = row[ m_columns.m_col_active ];
    const bool icase = row[ m_columns.m_col_icase ];
    const bool regex = row[ m_columns.m_col_regex ];
    const bool wchar = row[ m_columns.m_col_wchar ];
    const bool norm = row[ m_columns.m_col_norm ];
    const ReplaceStrCondition condition{ active, icase, regex, wchar, norm };

    ReplaceStrDiag dlg( this, condition, row[ m_columns.m_col_pattern ], row[ m_columns.m_col_replace ] );
    if( dlg.run() == Gtk::RESPONSE_OK ) {
        row.set_value( m_columns.m_col_active, dlg.get_active() );
        row.set_value( m_columns.m_col_icase, dlg.get_icase() );
        row.set_value( m_columns.m_col_regex, dlg.get_regex() );
        row.set_value( m_columns.m_col_wchar, dlg.get_wchar() );
        row.set_value( m_columns.m_col_norm, dlg.get_norm() );
        row.set_value( m_columns.m_col_pattern, dlg.get_pattern() );
        row.set_value( m_columns.m_col_replace, dlg.get_replace() );

        if( dlg.get_regex() ) {
            JDLIB::RegexPattern ptn;
            constexpr bool newline = true;
            constexpr bool migemo = false;

            if( ! ptn.set( dlg.get_pattern(), dlg.get_icase(), newline, migemo, dlg.get_wchar(), dlg.get_norm() ) ) {
                const std::string msg = ptn.errstr() + "\n\n" + dlg.get_pattern();

                SKELETON::MsgDiag mdlg( *this, msg, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK );
                mdlg.run();
            }
        }
    }
}


bool ReplaceStrPref::slot_key_press( GdkEventKey* event )
{
    const int id = m_control.key_press( event );

#ifdef _DEBUG
    std::cout << "ReplaceStrPref::slot_key_press id = " << id << std::endl;
#endif

    if( id == CONTROL::Up ) {
        if( auto it = get_selected_row() ) {
            if( --it ) {
                m_treeview.get_selection()->select( it );
                return true;
            }
        }
    }
    else if( id == CONTROL::Down ) {
        if( auto it = get_selected_row() ) {
            if( ++it ) {
                m_treeview.get_selection()->select( it );
                return true;
            }
        }
    }
    else if( event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_space ) {
        if( auto it = get_selected_row() ) {
            slot_row_activated( Gtk::TreePath( it ), nullptr );
        }
    }

    return false;
}


bool ReplaceStrPref::slot_key_release( GdkEventKey* event )
{
    const int id = m_control.key_press( event );

#ifdef _DEBUG
    std::cout << "ReplaceStrPref::slot_key_release id = " << id << std::endl;
#endif

    if( id == CONTROL::Delete ) slot_delete();

    return true;
}


//
// 一番上へ移動
//
void ReplaceStrPref::slot_top()
{
    Gtk::TreeModel::iterator it = get_selected_row();
    Gtk::TreeModel::iterator it_top = get_top_row();

    if( it && it != it_top ) m_current_store->move( it, it_top );
}


//
// 上へ移動
//
void ReplaceStrPref::slot_up()
{
    if( auto it = get_selected_row() ) {

        if( auto it_dest = std::prev( it ) ) {
            m_current_store->iter_swap( it, it_dest );
        }
    }
}


//
// 下へ移動
//
void ReplaceStrPref::slot_down()
{
    if( auto it = get_selected_row() ) {

        if( auto it_dest = std::next( it ) ) {
            m_current_store->iter_swap( it, it_dest );
        }
    }
}



//
// 一番下へ移動
//
void ReplaceStrPref::slot_bottom()
{
    Gtk::TreeModel::iterator it = get_selected_row();
    Gtk::TreeModel::iterator it_bottom = get_bottom_row();

    if( it && it != it_bottom ) {
        m_current_store->move( it, m_current_store->children().end() );
    }
}


//
// 削除ボタン
//
void ReplaceStrPref::slot_delete()
{
    Gtk::TreeModel::iterator it = get_selected_row();
    if( ! it ) return;

    SKELETON::MsgDiag mdiag( *this, "削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    auto it_next = std::next( it );

    m_current_store->erase( it );

    if( it_next ) select_row( it_next );
    else {
        Gtk::TreeModel::iterator it_bottom = get_bottom_row();
        if( it_bottom ) select_row( it_bottom );
    }
}


//
// 追加ボタン
//
void ReplaceStrPref::slot_add()
{
    constexpr ReplaceStrCondition condition{ true, false, false, false, false };
    ReplaceStrDiag dlg( this, condition, "", "" );
    if( dlg.run() == Gtk::RESPONSE_OK ) {
        append_row( m_current_store, dlg.get_condition(),
                    dlg.get_pattern(), dlg.get_replace() );
    }
}


//
// 置換対象変更
//
void ReplaceStrPref::slot_target_changed()
{
    const int id = m_menu_target.get_active_row_number();
#ifdef _DEBUG
    std::cout << "ReplaceStrPref::slot_target_changed target=" << id << std::endl;
#endif

    m_chref[ m_id_target ] = m_check_chref.get_active();
    m_check_chref.set_active( m_chref[ id ] );
    m_current_store = m_store[ id ];
    m_treeview.set_model( m_current_store );
    select_row( get_top_row() );
    m_id_target = id;
}
