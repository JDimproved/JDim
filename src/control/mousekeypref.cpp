// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "mousekeypref.h"
#include "controlutil.h"
#include "controlid.h"

#include "config/globalconf.h"

#include "skeleton/msgdiag.h"

#include "jdlib/miscutil.h"

#include "colorid.h"

#include <glib/gi18n.h>


using namespace CONTROL;


//
// キーやマウス入力ダイアログの基底クラス
//
InputDiag::InputDiag( Gtk::Window* parent, const std::string& url,
                      const int id, const std::string& target, const int mode )
    : SKELETON::PrefDiag( parent, url ),
      m_id( id ),
      m_mode( mode ),
      m_controlmode( CONTROL::get_mode( m_id ) ),
      m_label( target + "を入力して下さい。" )
{
    add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK );

    m_control.add_mode( m_controlmode );
    m_control.set_send_mg_info( false );

    set_title( CONTROL::get_label( m_id ) + " ( " + CONTROL::get_mode_label( m_controlmode ) + " )" );
    resize( 400, 400 );

    get_content_area()->pack_start( m_label );

    show_all_children();
}


bool InputDiag::on_key_press_event( GdkEventKey* event )
{
    guint key = event->keyval;
    const bool ctrl = ( event->state ) & GDK_CONTROL_MASK;
    bool shift = ( event->state ) & GDK_SHIFT_MASK;
    const bool alt = ( event->state ) & GDK_MOD1_MASK;
    const bool caps = ( event->state ) & GDK_LOCK_MASK; // caps lock

    if( m_mode & INPUTDIAG_MODE_KEY ){

        // caps lockされている場合は、アスキー文字を大文字小文字入れ替えて、capsを無視する
        // Control::key_press()も参照のこと
        if( caps ){
            if( key >= 'A' && key <= 'Z' ){
                key += 'a' - 'A';
            } else if( key >= 'a' && key <= 'z' ){
                key += 'A' - 'a';
            }
        }

        // keyがアスキー文字の場合は shift を無視する
        // KeyConfig::set_one_motion()も参照せよ
        if( CONTROL::is_ascii( key ) ) shift = false;

#ifdef _DEBUG
        std::cout << "InputDiag::on_key_press_event key = " << std::hex << key << std::dec;
        if( ctrl ) std::cout << " ctrl";
        if( shift ) std::cout << " shift";
        if( alt ) std::cout << " alt";
        std::cout << "\n";
#endif

        m_str_motion = std::string();

        const std::string keyname = CONTROL::get_keyname( key );
        std::string ctrl_label;
        if( ! keyname.empty() ){
            if( ctrl )  m_str_motion += "Ctrl+";
            if( shift ) m_str_motion += "Shift+";
            if( alt ) m_str_motion += "Alt+";
            m_str_motion += keyname;

            // ラベルに被っている操作名を表示
            ctrl_label = get_key_label();
        }

        m_label.set_label( m_str_motion + "\n" + ctrl_label );

        // エンターやスペースキーを押すとキャンセルボタンが押されてしまうのでキャンセルする
        if( keyname == "Space" || keyname == "Enter" ) return true;
    }

    return SKELETON::PrefDiag::on_key_press_event( event );
}


bool InputDiag::on_button_press_event( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "InputDiag::on_button_press_event\n";
#endif

    const bool ret = SKELETON::PrefDiag::on_button_press_event( event );


    if( m_mode & INPUTDIAG_MODE_MOUSE ){

        m_control.MG_start( event );

        m_str_motion = std::string();
        m_label.set_label( "" );
    }

    else if( m_mode & INPUTDIAG_MODE_BUTTON ){

        const guint button = event->button;
        const bool ctrl = ( event->state ) & GDK_CONTROL_MASK;
        const bool shift = ( event->state ) & GDK_SHIFT_MASK;
        const bool alt = ( event->state ) & GDK_MOD1_MASK;
        const bool dblclick = ( event->type == GDK_2BUTTON_PRESS );
        const bool trpclick = ( event->type == GDK_3BUTTON_PRESS );

        m_str_motion = std::string();
        std::string buttonname;
        std::string ctrl_label;

        if( button == 1 ){
            if( trpclick ) buttonname = "TrpLeft";
            else if( dblclick ) buttonname = "DblLeft";
            else buttonname = "Left";
        }
        else if( button == 2 ){
            if( trpclick ) buttonname = "TrpMid";
            else if( dblclick ) buttonname = "DblMid";
            else buttonname = "Mid";
        }
        else if( button == 3 ){
            if( trpclick ) buttonname = "TrpRight";
            else if( dblclick ) buttonname = "DblRight";
            else buttonname = "Right";
        }
        else if( button == 6 ) buttonname = "Tilt_Left";
        else if( button == 7 ) buttonname = "Tilt_Right";
        else if( button == 8 ) buttonname = "Button4";
        else if( button == 9 ) buttonname = "Button5";

        if( ! buttonname.empty() ){
            if( ctrl )  m_str_motion += "Ctrl+";
            if( shift ) m_str_motion += "Shift+";
            if( alt ) m_str_motion += "Alt+";

            m_str_motion += buttonname;

            ctrl_label = get_button_label();
        }

        m_label.set_label( m_str_motion + "\n" + ctrl_label );
    }
    
    return ret;
}


bool InputDiag::on_button_release_event( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "InputDiag::on_button_release_event\n";
#endif

    const bool ret = SKELETON::PrefDiag::on_button_release_event( event );

    if( m_mode & INPUTDIAG_MODE_MOUSE ){
        m_control.MG_end( event );
    }

    return ret;
}


bool InputDiag::on_motion_notify_event( GdkEventMotion* event )
{
#ifdef _DEBUG
//    std::cout << "InputDiag::on_motion_notify_event\n";
#endif

    const bool ret = SKELETON::PrefDiag::on_motion_notify_event( event );

    if( ( m_mode & INPUTDIAG_MODE_MOUSE ) && m_control.is_mg_mode() ){

        m_control.MG_motion( event );
        if( m_str_motion != m_control.get_mg_direction() ){

            m_str_motion = m_control.get_mg_direction();
            m_label.set_label( m_str_motion + "\n" + get_mouse_label() );
        }
    }

    return ret;
}


std::string InputDiag::get_key_label() const
{
    std::string label;

    for( int mode = CONTROL::MODE_START; mode <= CONTROL::MODE_END; ++mode ){

        const std::vector< int > vec_ids = CONTROL::check_key_conflict( mode, m_str_motion );
        for( const int id : vec_ids ) {
            label += "\n" + CONTROL::get_label( id ) + " ( " + CONTROL::get_mode_label( mode ) + " )";
            if( mode == m_controlmode && id != m_id ) label += " ×";
        }
    }

    return label;
}



std::string InputDiag::get_mouse_label() const
{
    std::string label;

    for( int mode = CONTROL::MODE_START; mode <= CONTROL::MODE_END; ++mode ){

        const std::vector< int > vec_ids = CONTROL::check_mouse_conflict( mode, m_str_motion );
        for( const int id : vec_ids ) {
            label += "\n" + CONTROL::get_label( id ) + " ( " + CONTROL::get_mode_label( mode ) + " )";
            if( mode == m_controlmode && id != m_id ) label += " ×";
        }
    }

    return label;
}


std::string InputDiag::get_button_label() const
{
    std::string label;

    for( int mode = CONTROL::MODE_START; mode <= CONTROL::MODE_END; ++mode ){

        const std::vector< int > vec_ids = CONTROL::check_button_conflict( mode, m_str_motion );
        for( const int id : vec_ids ) {
            label.append( "\n" + CONTROL::get_label( id ) + " ( " + CONTROL::get_mode_label( mode ) + " )" );
        }
    }

    return label;
}


///////////////////////////////////


//
// 個別のショートカットキー設定ダイアログ
//
MouseKeyDiag::MouseKeyDiag( Gtk::Window* parent, const std::string& url,
                            const int id, const std::string& target, const std::string& str_motions )
    : SKELETON::PrefDiag( parent, url )
    , m_id( id )
    , m_controlmode( CONTROL::get_mode( m_id ) )
    , m_label( "編集したい" + target + "設定をダブルクリックして下さい。" )
    , m_button_delete( g_dpgettext( GTK_DOMAIN, "Stock label\x04_Delete", 12 ), true )
    , m_button_add( g_dpgettext( GTK_DOMAIN, "Stock label\x04_Add", 12 ), true )
    , m_button_reset( "デフォルト" )
{
    m_liststore = Gtk::ListStore::create( m_columns );
    m_treeview.set_model( m_liststore );
    m_treeview.set_size_request( 320, 200 );
    m_treeview.signal_row_activated().connect( sigc::mem_fun( *this, &MouseKeyDiag::slot_row_activated ) );

    Gtk::TreeViewColumn* column = Gtk::manage( new Gtk::TreeViewColumn( target, m_columns.m_col_motion ) );
    column->set_resizable( true );
    m_treeview.append_column( *column );

    m_scrollwin.add( m_treeview );
    m_scrollwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );

    m_button_delete.signal_clicked().connect( sigc::mem_fun( *this, &MouseKeyDiag::slot_delete ) );
    m_button_add.signal_clicked().connect( sigc::mem_fun( *this, &MouseKeyDiag::slot_add ) );
    m_button_reset.signal_clicked().connect( sigc::mem_fun( *this, &MouseKeyDiag::slot_reset ) );

    m_vbuttonbox.pack_start( m_button_delete, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_add, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_reset, Gtk::PACK_SHRINK );
    m_vbuttonbox.set_layout( Gtk::BUTTONBOX_START );
    m_vbuttonbox.set_spacing( 4 );

    m_hbox.pack_start( m_scrollwin, Gtk::PACK_EXPAND_WIDGET );
    m_hbox.pack_start( m_vbuttonbox, Gtk::PACK_SHRINK );

    get_content_area()->set_spacing( 8 );
    get_content_area()->pack_start( m_label, Gtk::PACK_SHRINK );
    get_content_area()->pack_start( m_hbox );

    show_all_children();
    set_title( CONTROL::get_label( m_id ) + " ( " + CONTROL::get_mode_label( m_controlmode ) + " )" );

    // キー設定をスペース毎に区切って行を作成
    std::list< std::string > list_motions = MISC::StringTokenizer( str_motions, ' ' );
    if( list_motions.size() ){
        for( const std::string& motion : list_motions ) append_row( MISC::utf8_trim( motion ) );

        // 先頭にカーソルセット
        Gtk::TreeModel::Children children = m_liststore->children();
        Gtk::TreeModel::iterator it_row = children.begin();
        if( *it_row ) m_treeview.set_cursor( m_liststore->get_path( *it_row ) );
    }
}


Gtk::TreeModel::Row MouseKeyDiag::append_row( const std::string& motion )
{
    Gtk::TreeModel::Row row;
    row = *( m_liststore->append() );
    if( row ) row[ m_columns.m_col_motion ] = motion;

    return row;
}


std::string MouseKeyDiag::get_str_motions()
{
    std::string str_motions;

    Gtk::TreeModel::Children children = m_liststore->children();
    for( const Gtk::TreeRow& row : children ) {
        if( ! str_motions.empty() ) str_motions.push_back( ' ' );
        str_motions.append( row[ m_columns.m_col_motion ] );
    }

#ifdef _DEBUG
    std::cout << "MouseKeyDiag::get_str_motions motions = " << str_motions << std::endl;
#endif

    return str_motions;
}


//
// 入力ダイアログを表示
//
std::string MouseKeyDiag::show_inputdiag( bool is_append )
{
    std::string str_motion;

    if( m_single ){
        // 1つだけしか設定できない場合
        const int count = get_count();
        if( count > 1 || ( count == 1 && is_append ) ){
            SKELETON::MsgDiag mdiag( nullptr, "この項目には、1つだけ設定できます。" );
            mdiag.run();
            return std::string();
        }
    }

    std::unique_ptr<InputDiag> diag = create_inputdiag();
    if( diag == nullptr ) return std::string();

    while( diag->run() == Gtk::RESPONSE_OK ){

        // 設定が重複していないかチェック
        bool conflict = false;
        const std::vector< int > vec_ids = check_conflict( m_controlmode, diag->get_str_motion() );
        auto it = std::find_if( vec_ids.cbegin(), vec_ids.cend(),
                                [this]( int id ) { return id != CONTROL::NoOperation && id != m_id; } );
        if( it != vec_ids.cend() ) {
            const std::string label = CONTROL::get_label( *it );
            SKELETON::MsgDiag mdiag( nullptr, diag->get_str_motion() + "\n\nは「" + label + "」で使用されています" );
            mdiag.run();
            conflict = true;
        }

        if( ! conflict ){
            str_motion = diag->get_str_motion();
            break;
        }
    }

    return str_motion;
}


// 行をダブルクリック
void MouseKeyDiag::slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
#ifdef _DEBUG
    std::cout << "MouseKeyDiag::slot_row_activated path = " << path.to_string() << std::endl;
#endif

    Gtk::TreeModel::Row row = *( m_liststore->get_iter( path ) );
    if( ! row ) return;

    std::string str_motion = show_inputdiag( false );
    if( ! str_motion.empty() ) {
        row[ m_columns.m_col_motion ] = str_motion;
        static_cast<void>( row ); // cppcheck: unreadVariable
    }
}


// 行削除
void MouseKeyDiag::slot_delete()
{
    std::vector< Gtk::TreeModel::Path > rows = m_treeview.get_selection()->get_selected_rows();
    if( ! rows.size() ) return;

    Gtk::TreeModel::Path path = *rows.begin();
    Gtk::TreeModel::iterator row = *( m_liststore->get_iter( path ) );
    if( ! row ) return;

    // 削除する行の次の行にカーソルを移動
    Gtk::TreeModel::Path path_next = path;
    path_next.next();
    Gtk::TreeModel::iterator row_next = *( m_liststore->get_iter( path_next ) );
    if( row_next ) m_treeview.set_cursor( path_next );
    else{

        // 次が無ければ前の行にカーソル移動
        Gtk::TreeModel::Path path_prev = path;
        path_prev.prev();
        Gtk::TreeModel::iterator row_prev = *( m_liststore->get_iter( path_prev ) );
        if( row_prev ) m_treeview.set_cursor( path_prev );
    }

    m_liststore->erase( row );
}


// 行追加
void MouseKeyDiag::slot_add()
{
    std::string str_motion = show_inputdiag( true );
    if(  str_motion.empty() ) return;

    // 既に登録済みか調べる
    Gtk::TreeModel::Children children = m_liststore->children();
    for( const Gtk::TreeRow& row : children ) {
        const std::string& motion_tmp = row[ m_columns.m_col_motion ];
        if( str_motion == motion_tmp ) return;
    }

    Gtk::TreeModel::iterator it_new = m_liststore->append();
    ( *it_new )[ m_columns.m_col_motion ] = str_motion;
    static_cast<void>( *it_new ); // cppcheck: unreadVariable
}


// デフォルトに戻す
void MouseKeyDiag::slot_reset()
{
    const std::string default_motions = get_default_motions( m_id );

#ifdef _DEBUG
    std::cout << "MouseKeyDiag::slot_reset default = " << default_motions << std::endl;
#endif

    // デフォルト設定が既に使われていないか確認
    std::list< std::string > list_motions = MISC::StringTokenizer( default_motions, ' ' );
    std::list< std::string > list_defaults;
    for( const std::string& raw_motion : list_motions ) {

        std::string motion = MISC::utf8_trim( raw_motion );

        bool conflict = false;
        const std::vector< int > vec_ids = check_conflict( m_controlmode, motion );
        auto it = std::find_if( vec_ids.cbegin(), vec_ids.cend(),
                                [this]( int id ) { return id != CONTROL::NoOperation && id != m_id; } );
        if( it != vec_ids.cend() ) {
            const std::string label = CONTROL::get_label( *it );
            SKELETON::MsgDiag mdiag( nullptr, motion + "\n\nは「" + label + "」で使用されています" );
            mdiag.run();
            conflict = true;
        }

        if( ! conflict ) list_defaults.push_back( std::move( motion ) );
    }

    // クリアして再登録
    m_liststore->clear();
    for( const std::string& motion : list_defaults ) append_row( motion );
}


///////////////////////////////////


//
// マウスジェスチャ、キーボード設定ダイアログの基底クラス
//
MouseKeyPref::MouseKeyPref( Gtk::Window* parent, const std::string& url, const std::string& target  )
    : SKELETON::PrefDiag( parent, url )
    , m_hbox_search{ Gtk::ORIENTATION_HORIZONTAL, 0 }
    , m_button_reset{ "全てデフォルト設定に戻す" }
    , m_label{ "編集したい" + target + "設定をダブルクリックして下さい。" }
{
    signal_key_press_event().connect( sigc::mem_fun( *this, &MouseKeyPref::slot_key_press_event ) );

    m_hbox_search.pack_start( m_label, Gtk::PACK_SHRINK );
    m_hbox_search.pack_end( m_toggle_search, Gtk::PACK_SHRINK );

    m_label.set_hexpand( true );
    m_toggle_search.set_image_from_icon_name( "edit-find-symbolic" );
    // m_toggle_search 以外にも m_search_bar を閉じる操作があるため双方向にバインドする
    m_binding_search = Glib::Binding::bind_property( m_toggle_search.property_active(),
                                                     m_search_bar.property_search_mode_enabled(),
                                                     Glib::BINDING_BIDIRECTIONAL );

    m_search_bar.add( m_search_entry );
    m_search_bar.connect_entry( m_search_entry );
    m_search_bar.set_hexpand( true );
    m_search_bar.set_show_close_button( true );

    m_search_entry.set_hexpand( true );
    m_search_entry.set_width_chars( 50 );
    m_search_entry.signal_changed().connect( sigc::mem_fun( *this, &MouseKeyPref::slot_entry_changed ) );

    m_liststore = Gtk::ListStore::create( m_columns );
    m_model_filter = Gtk::TreeModelFilter::create( m_liststore );
    m_model_filter->set_visible_func( sigc::mem_fun( *this, &MouseKeyPref::slot_visible_func ) );

    m_treeview.set_model( m_model_filter );
    m_treeview.set_search_entry( m_search_entry );
    m_treeview.set_size_request( 640, 400 );
    m_scrollwin.set_margin_bottom( 8 );
    m_scrollwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
    m_scrollwin.set_propagate_natural_height( true );
    m_scrollwin.set_propagate_natural_width( true );
    m_treeview.signal_row_activated().connect( sigc::mem_fun( *this, &MouseKeyPref::slot_row_activated ) );

    Gtk::TreeViewColumn* column = Gtk::manage( new Gtk::TreeViewColumn( "コマンド", m_columns.m_col_label ) );
    column->set_fixed_width( 220 );
    column->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
    column->set_resizable( true );
    m_treeview.append_column( *column );

    column = Gtk::manage( new Gtk::TreeViewColumn( target, m_columns.m_col_motions ) );
    column->set_resizable( true );
    m_treeview.append_column( *column );
    Gtk::CellRenderer *cell = column->get_first_cell();
    if( cell ) column->set_cell_data_func( *cell, sigc::mem_fun( *this, &MouseKeyPref::slot_cell_data ) );

    m_scrollwin.add( m_treeview );
    m_scrollwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
    m_scrollwin.set_propagate_natural_height( true );
    m_scrollwin.set_propagate_natural_width( true );

    m_button_reset.signal_clicked().connect( sigc::mem_fun( *this, &MouseKeyPref::slot_reset ) );
    m_hbox.pack_start( m_button_reset, Gtk::PACK_SHRINK );

    get_content_area()->pack_start( m_hbox_search, Gtk::PACK_SHRINK );
    get_content_area()->pack_start( m_search_bar, Gtk::PACK_SHRINK );
    get_content_area()->pack_start( m_scrollwin );
    get_content_area()->pack_start( m_hbox, Gtk::PACK_SHRINK );

    show_all_children();
    set_title( target + "設定" );
}


// 行追加
void MouseKeyPref::append_row( const int id, const std::string& label )
{
    Gtk::TreeModel::Row row;
    row = *( get_liststore()->append() );
    if( row ){

        const std::string motions = get_str_motions( id );

        if( label.empty() ){
            row[ get_colums().m_col_label ] = CONTROL::get_label( id );
        }else{
            row[ get_colums().m_col_label ] = label;
        }
        row[ get_colums().m_col_motions ] = motions;
        row[ get_colums().m_col_id ] = id;
        row.set_value( get_colums().m_col_drawbg, motions != get_default_motions( id ) );
    }
}


//
// コメント行追加
//
void MouseKeyPref::append_comment_row( const std::string& comment )
{
    Gtk::TreeModel::Row row;
    row = *( m_liststore->append() );
    if( row ){
        row[ m_columns.m_col_label ] = comment;
        row[ m_columns.m_col_motions ] = std::string();
        row[ m_columns.m_col_id ] = CONTROL::NoOperation;
        row[ m_columns.m_col_drawbg ] = false;
        static_cast<void>( row ); // cppcheck: unreadVariable
    }
}


//
// デフォルト設定に戻す
//
void MouseKeyPref::slot_reset()
{
    const Gtk::TreeModel::Children children = get_liststore()->children();
    for( Gtk::TreeRow row : children ) {
        if( row ){
            const int id = row[ get_colums().m_col_id ];
            if( id != CONTROL::NoOperation ){

                const std::string str_motions = get_default_motions( id );

                row[ get_colums().m_col_motions ] = str_motions;
                row[ get_colums().m_col_drawbg ] = false;

                remove_motions( id );
                set_motions( id, str_motions );
            }
        }
    }
}


//
// 行をダブルクリック
//
void MouseKeyPref::slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
#ifdef _DEBUG
    std::cout << "MouseKeyPref::slot_row_activated path = " << path.to_string() << std::endl;
#endif

    Gtk::TreeModel::Row row = *( m_model_filter->get_iter( path ) );
    if( ! row ) return;

    const int id = row[ get_colums().m_col_id ];
    if( id == CONTROL::NoOperation ) return;

    std::unique_ptr<MouseKeyDiag> diag = create_setting_diag( id, row[ get_colums().m_col_motions ] );
    if( diag->run() == Gtk::RESPONSE_OK ){

        const std::string motions = diag->get_str_motions();

        row[ get_colums().m_col_motions ] = motions;
        row.set_value( get_colums().m_col_drawbg, motions != get_default_motions( id ) );

        remove_motions( id );
        set_motions( id, motions );
    }
}


//
// 実際の描画の際に cellrendere のプロパティをセットするスロット関数
//
void MouseKeyPref::slot_cell_data( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it )
{
    Gtk::TreeModel::Row row = *it;
    Gtk::CellRendererText* rentext = dynamic_cast<Gtk::CellRendererText*>( cell );

    if( row[ m_columns.m_col_drawbg ] ){
        rentext->property_foreground() = CONFIG::get_color( COLOR_CHAR_HIGHLIGHT_TREE );
        rentext->property_foreground_set() = true;
        rentext->property_cell_background() = CONFIG::get_color( COLOR_BACK_HIGHLIGHT_TREE );
        rentext->property_cell_background_set() = true;
    }
    else {
        rentext->property_foreground_set() = false;
        rentext->property_cell_background_set() = false;
    }
}


/** @brief ダイアログ内で Ctrl+F が押されたときは検索ボックスを表示する
 *
 * @param[in] event キーイベントの状態
 * @return 検索ボックスが非表示のときは m_search_bar の処理から返す、それ以外は false (GDK_EVENT_PROPAGATE)
 */
bool MouseKeyPref::slot_key_press_event( GdkEventKey* event )
{
    if( ( event->state & GDK_CONTROL_MASK ) && event->keyval == 'f' ) {
        if( m_search_bar.get_search_mode() ) {
            m_search_entry.grab_focus();
        }
        else {
            m_search_bar.set_search_mode( true );
            return m_search_bar.handle_event( event );
        }
    }
    return false;
}


/**
 * @brief 検索ボックスに入力されたテキストで設定項目のフィルタリングを実行する
 */
void MouseKeyPref::slot_entry_changed()
{
    m_model_filter->refilter();
}


/** @brief 検索ボックスに入力されたテキストが設定の名称に含まれるかチェックする
 *
 * @details 入力は1つのキーワードとして検索する。
 * 空白でテキストを区切ってもAND検索やOR検索は行わない。
 * @param[in] iter 設定項目の行
 * @return チェックした行を表示するなら true
 * - 設定の名称に入力テキストが含まれるなら表示する
 * - 検索ボックスが非表示のときや入力テキストが空のときはすべての行を表示する
 */
bool MouseKeyPref::slot_visible_func( const Gtk::TreeModel::const_iterator& iter )
{
    if( ! m_search_bar.get_search_mode() ) return true;

    Glib::ustring query = m_search_entry.get_text();
    if( query.empty() ) return true;

    const Gtk::TreeModel::Row& row = *iter;
    // 検索中は設定カテゴリの名前や空行を取り除く
    if( row[ m_columns.m_col_id ] == CONTROL::NoOperation ) return false;

    // 検索クエリの前後にある空白文字は無視する
    constexpr auto is_not_space = []( gunichar uc ) { return uc != U' ' && uc != U'\u3000'; };
    if( auto it = std::find_if( query.begin(), query.end(), is_not_space ); it != query.begin() ) {
        query.erase( query.begin(), it );
    }
    if( auto rit = std::find_if( query.rbegin(), query.rend(), is_not_space ); rit != query.rbegin() ) {
        query.erase( rit.base(), query.end() );
    }

    const std::string& name = row[ m_columns.m_col_label ];
    return name.find( query.raw() ) != std::string::npos;
}
