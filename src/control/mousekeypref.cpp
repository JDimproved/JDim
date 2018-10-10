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

    get_vbox()->pack_start( m_label );

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
        std::cout << "InputDiag::on_key_press_event key = " << std::hex << key;
        if( ctrl ) std::cout << " ctrl";
        if( shift ) std::cout << " shift";
        if( alt ) std::cout << " alt";
        std::cout << "\n";
#endif

        m_str_motion = std::string();

        const std::string keyname = CONTROL::get_keyname( key );
        std::string control_label;
        if( ! keyname.empty() ){
            if( ctrl )  m_str_motion += "Ctrl+";
            if( shift ) m_str_motion += "Shift+";
            if( alt ) m_str_motion += "Alt+";
            m_str_motion += keyname;

            // ラベルに被っている操作名を表示
            control_label = get_key_label();
        }

        m_label.set_label( m_str_motion + "\n" + control_label );

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
        std::string control_label;

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
        else if( button == 9 ) buttonname = "Button6";

        if( ! buttonname.empty() ){
            if( ctrl )  m_str_motion += "Ctrl+";
            if( shift ) m_str_motion += "Shift+";
            if( alt ) m_str_motion += "Alt+";

            m_str_motion += buttonname;

            control_label = get_button_label();
        }

        m_label.set_label( m_str_motion + "\n" + control_label );
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


const std::string InputDiag::get_key_label()
{
    std::string label;

    for( int mode = CONTROL::MODE_START; mode <= CONTROL::MODE_END; ++mode ){

        const std::vector< int > vec_ids = CONTROL::check_key_conflict( mode, m_str_motion );
        std::vector< int >::const_iterator it = vec_ids.begin();
        for( ; it != vec_ids.end(); ++it ){
            const int id = *it;
            label += "\n" + CONTROL::get_label( id ) + " ( " + CONTROL::get_mode_label( mode ) + " )";
            if( mode == m_controlmode && id != m_id ) label += " ×";
        }
    }

    return label;
}



const std::string InputDiag::get_mouse_label()
{
    std::string label;

    for( int mode = CONTROL::MODE_START; mode <= CONTROL::MODE_END; ++mode ){

        const std::vector< int > vec_ids = CONTROL::check_mouse_conflict( mode, m_str_motion );
        std::vector< int >::const_iterator it = vec_ids.begin();
        for( ; it != vec_ids.end(); ++it ){
            const int id = *it;
            label += "\n" + CONTROL::get_label( id ) + " ( " + CONTROL::get_mode_label( mode ) + " )";
            if( mode == m_controlmode && id != m_id ) label += " ×";
        }
    }

    return label;
}


const std::string InputDiag::get_button_label()
{
    std::string label;

    for( int mode = CONTROL::MODE_START; mode <= CONTROL::MODE_END; ++mode ){

        const std::vector< int > vec_ids = CONTROL::check_button_conflict( mode, m_str_motion );
        std::vector< int >::const_iterator it = vec_ids.begin();
        for( ; it != vec_ids.end(); ++it ){
            const int id = *it;
            label += "\n" + CONTROL::get_label( id ) + " ( " + CONTROL::get_mode_label( mode ) + " )";
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
    : SKELETON::PrefDiag( parent, url ),
      m_id( id ),
      m_controlmode( CONTROL::get_mode( m_id ) ),
      m_single( false ),
      m_label( "編集したい" + target + "設定をダブルクリックして下さい。" ),
      m_button_delete( Gtk::Stock::DELETE ),
      m_button_add( Gtk::Stock::ADD ),
      m_button_reset( "デフォルト" )
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

    m_hbox.pack_start( m_scrollwin, Gtk::PACK_SHRINK );
    m_hbox.pack_start( m_vbuttonbox, Gtk::PACK_SHRINK );

    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_label );
    get_vbox()->pack_start( m_hbox );

    show_all_children();
    set_title( CONTROL::get_label( m_id ) + " ( " + CONTROL::get_mode_label( m_controlmode ) + " )" );

    // キー設定をスペース毎に区切って行を作成
    std::list< std::string > list_motions = MISC::StringTokenizer( str_motions, ' ' );
    if( list_motions.size() ){
        std::list< std::string >::iterator it = list_motions.begin();
        for( ; it != list_motions.end() ; ++it ) append_row( MISC::remove_space( *it ) );

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


const std::string MouseKeyDiag::get_str_motions()
{
    std::string str_motions;

    Gtk::TreeModel::Children children = m_liststore->children();
    Gtk::TreeModel::iterator it = children.begin();
    for( ; it != children.end(); ++it ){
        if( it != children.begin() ) str_motions += " ";
        str_motions += ( *it )[ m_columns.m_col_motion ];
    }

#ifdef _DEBUG
    std::cout << "MouseKeyDiag::get_str_motions motions = " << str_motions << std::endl;
#endif

    return str_motions;
}


//
// 入力ダイアログを表示
//
const std::string MouseKeyDiag::show_inputdiag( bool is_append )
{
    std::string str_motion;

    if( m_single ){
        // 1つだけしか設定できない場合
        const int count = get_count();
        if( count > 1 || ( count == 1 && is_append ) ){
            SKELETON::MsgDiag mdiag( NULL, "この項目には、1つだけ設定できます。" );
            mdiag.run();
            return std::string();
        }
    }

    InputDiag* diag = create_inputdiag();
    if( diag == NULL ) return std::string();

    while( diag->run() == Gtk::RESPONSE_OK ){

        // 設定が重複していないかチェック
        bool conflict = false;
        const std::vector< int > vec_ids = check_conflict( m_controlmode, diag->get_str_motion() );
        std::vector< int >::const_iterator it = vec_ids.begin();
        for( ; it != vec_ids.end(); ++it ){
            const int id = *it;
            if( id != CONTROL::None && id != m_id ){
                SKELETON::MsgDiag mdiag( NULL, diag->get_str_motion() + "\n\nは「" + CONTROL::get_label( id ) + "」で使用されています" );
                mdiag.run();
                conflict = true;
                break;
            }
        }

        if( ! conflict ){
            str_motion = diag->get_str_motion();
            break;
        }
    }

    delete diag;
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
    if( ! str_motion.empty() ) row[ m_columns.m_col_motion ] = str_motion;
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
    Gtk::TreeModel::iterator it = children.begin();
    for( ; it != children.end(); ++it ){
        const std::string motion_tmp = ( *it )[ m_columns.m_col_motion ];
        if( str_motion == motion_tmp ) return;
    }

    Gtk::TreeModel::iterator it_new = m_liststore->append();
    ( *it_new )[ m_columns.m_col_motion ] = str_motion;
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
    std::list< std::string >::iterator it = list_motions.begin();
    for( ; it != list_motions.end() ; ++it ){

        const std::string motion = MISC::remove_space( *it );

        bool conflict = false;
        const std::vector< int > vec_ids = check_conflict( m_controlmode, motion );
        std::vector< int >::const_iterator it = vec_ids.begin();
        for( ; it != vec_ids.end(); ++it ){
            const int id = *it;

            if( id != CONTROL::None && id != m_id ){
                SKELETON::MsgDiag mdiag( NULL, motion + "\n\nは「" + CONTROL::get_label( id ) + "」で使用されています" );
                mdiag.run();
                conflict = true;
                break;
            }
        }

        if( ! conflict ) list_defaults.push_back( motion );
    }

    // クリアして再登録
    m_liststore->clear();
    it = list_defaults.begin();
    for( ; it != list_defaults.end() ; ++it ) append_row( ( *it ) );
}


///////////////////////////////////


//
// マウスジェスチャ、キーボード設定ダイアログの基底クラス
//
MouseKeyPref::MouseKeyPref( Gtk::Window* parent, const std::string& url, const std::string& target  )
    : SKELETON::PrefDiag( parent, url ),
      m_button_reset( "全てデフォルト設定に戻す" ),
      m_label( "編集したい" + target + "設定をダブルクリックして下さい。" )
{
    m_liststore = Gtk::ListStore::create( m_columns );
    m_treeview.set_model( m_liststore );
    m_treeview.set_size_request( 640, 400 );
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
    m_scrollwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS );

    m_button_reset.signal_clicked().connect( sigc::mem_fun( *this, &MouseKeyPref::slot_reset ) );
    m_hbox.pack_start( m_button_reset, Gtk::PACK_SHRINK );

    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_label, Gtk::PACK_SHRINK );
    get_vbox()->pack_start( m_scrollwin );
    get_vbox()->pack_start( m_hbox, Gtk::PACK_SHRINK );

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
        if( motions != get_default_motions( id ) ) row[ get_colums().m_col_drawbg ] = true;
        else row[ get_colums().m_col_drawbg ] = false;
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
        row[ m_columns.m_col_id ] = CONTROL::None;
        row[ m_columns.m_col_drawbg ] = false;
    }
}


//
// デフォルト設定に戻す
//
void MouseKeyPref::slot_reset()
{
    const Gtk::TreeModel::Children children = get_liststore()->children();
    Gtk::TreeModel::iterator it = children.begin();
    for( ; it != children.end(); ++it ){
        Gtk::TreeModel::Row row = ( *it );
        if( row ){
            const int id = row[ get_colums().m_col_id ];
            if( id != CONTROL::None ){

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

    Gtk::TreeModel::Row row = *( get_liststore()->get_iter( path ) );
    if( ! row ) return;

    const int id = row[ get_colums().m_col_id ];
    if( id == CONTROL::None ) return;

    MouseKeyDiag* diag = create_setting_diag( id, row[ get_colums().m_col_motions ] );
    if( diag->run() == Gtk::RESPONSE_OK ){

        const std::string motions = diag->get_str_motions();

        row[ get_colums().m_col_motions ] = motions;
        if( motions != get_default_motions( id ) ) row[ get_colums().m_col_drawbg ] = true;
        else row[ get_colums().m_col_drawbg ] = false;

        remove_motions( id );
        set_motions( id, motions );
    }

    delete diag;
}


//
// 実際の描画の際に cellrendere のプロパティをセットするスロット関数
//
void MouseKeyPref::slot_cell_data( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it )
{
    Gtk::TreeModel::Row row = *it;

    if( row[ m_columns.m_col_drawbg ] ){
        cell->property_cell_background() = CONFIG::get_color( COLOR_BACK_HIGHLIGHT_TREE );
        cell->property_cell_background_set() = true;
    }
    else cell->property_cell_background_set() = false;
}
