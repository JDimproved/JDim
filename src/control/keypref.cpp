// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "keypref.h"
#include "controlid.h"
#include "controlutil.h"

#include "skeleton/msgdiag.h"

#include "jdlib/miscutil.h"

#include "config/globalconf.h"

#include "colorid.h"

using namespace CONTROL;


//
// キーボード入力をラベルに表示するダイアログ
//
KeyInputDiag::KeyInputDiag( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url ),
      m_label( "ショートカットキーを入力して下さい" )
{
    resize( 200, 100 );

    get_vbox()->pack_start( m_label );

    show_all_children();
    set_title( "ショートカットキー入力" );
}


bool KeyInputDiag::on_key_press_event( GdkEventKey* event )
{
    m_key = event->keyval;
    m_ctrl = ( event->state ) & GDK_CONTROL_MASK;
    m_shift = ( event->state ) & GDK_SHIFT_MASK;
    m_alt = ( event->state ) & GDK_MOD1_MASK;

    // keyがアスキー文字の場合は shift を無視する
    // KeyConfig::set_one_motion()も参照せよ
    if( CONTROL::is_ascii( m_key ) ) m_shift = false;

#ifdef _DEBUG
    std::cout << "KeyInputDiag::on_key_press_event key = " << std::hex << m_key;
    if( m_ctrl ) std::cout << " ctrl";
    if( m_shift ) std::cout << " shift";
    if( m_alt ) std::cout << " alt";
    std::cout << "\n";
#endif

    const std::string keyname = CONTROL::get_keyname( m_key );
    if( ! keyname.empty() ){
        m_str_motion = "";
        if( m_ctrl )  m_str_motion += "Ctrl+";
        if( m_shift ) m_str_motion += "Shift+";
        if( m_alt ) m_str_motion += "Alt+";
        m_str_motion += keyname;
    }

    m_label.set_label( m_str_motion );

    return true;
}



///////////////////////////////


//
// 個別のショートカットキー設定ダイアログ
//
KeyDiag::KeyDiag( Gtk::Window* parent, const std::string& url,
                  const int id, const std::string& label, const std::string& str_motions )
    : SKELETON::PrefDiag( parent, url ),
      m_id( id ),
      m_label( "編集したいショートカットキー設定をダブルクリックして下さい。" ),
      m_button_delete( Gtk::Stock::DELETE ),
      m_button_add( Gtk::Stock::ADD ),
      m_button_reset( "リセット" )
{
    m_liststore = Gtk::ListStore::create( m_columns );
    m_treeview.set_model( m_liststore );
    m_treeview.set_size_request( 320, 200 );
    m_treeview.signal_row_activated().connect( sigc::mem_fun( *this, &KeyDiag::slot_row_activated ) );

    Gtk::TreeViewColumn* column = Gtk::manage( new Gtk::TreeViewColumn( "ショートカットキー", m_columns.m_col_motion ) );
    column->set_resizable( true );
    m_treeview.append_column( *column );

    m_scrollwin.add( m_treeview );
    m_scrollwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );

    m_button_delete.signal_clicked().connect( sigc::mem_fun( *this, &KeyDiag::slot_delete ) );
    m_button_add.signal_clicked().connect( sigc::mem_fun( *this, &KeyDiag::slot_add ) );
    m_button_reset.signal_clicked().connect( sigc::mem_fun( *this, &KeyDiag::slot_reset ) );

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
    set_title( label );

    // キー設定をスペース毎に区切って行を作成
    std::list< std::string > list_motions = MISC::StringTokenizer( str_motions, ' ' );
    std::list< std::string >::iterator it = list_motions.begin();
    for( ; it != list_motions.end() ; ++it ) append_row( MISC::remove_space( *it ) );
}


void KeyDiag::append_row( const std::string& motion )
{
    Gtk::TreeModel::Row row;
    row = *( m_liststore->append() );
    if( row ) row[ m_columns.m_col_motion ] = motion;
}


const std::string KeyDiag::get_str_motions()
{
    std::string str_motions;

    Gtk::TreeModel::Children children = m_liststore->children();
    Gtk::TreeModel::iterator it = children.begin();
    for( ; it != children.end(); ++it ){
        if( it != children.begin() ) str_motions += " ";
        str_motions += ( *it )[ m_columns.m_col_motion ];
    }

#ifdef _DEBUG
    std::cout << "KeyDiag::get_str_motions motions = " << str_motions << std::endl;
#endif

    return str_motions;
}


//
// キーボード入力ダイアログを表示
//
const std::string KeyDiag::show_inputdiag()
{
    std::string str_motion;

    KeyInputDiag diag( this,  "" );
    while( diag.run() == Gtk::RESPONSE_OK ){

        // キー設定が重複していないかチェック
        const int mode = CONTROL::get_mode( m_id );
        const guint key = diag.get_key(); 
        const bool ctrl = diag.get_ctrl();
        const bool shift = diag.get_shift(); 
        const bool alt = diag.get_alt();

        const int id = CONTROL::check_key_conflict( mode, key, ctrl, shift, alt );
        if( id != CONTROL::None && id != m_id ){
            SKELETON::MsgDiag mdiag( NULL, diag.get_str_motion() + "\n\nは「" + CONTROL::get_label( id ) + "」で使用されています" );
            mdiag.run();
        }
        else{
            str_motion = diag.get_str_motion();
            break;
        }
    }

    return str_motion;
}


// 行をダブルクリック
void KeyDiag::slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
#ifdef _DEBUG
    std::cout << "KeyDiag::slot_row_activated path = " << path.to_string() << std::endl;
#endif

    Gtk::TreeModel::Row row = *( m_liststore->get_iter( path ) );
    if( ! row ) return;

    std::string str_motion = show_inputdiag();
    if( ! str_motion.empty() ) row[ m_columns.m_col_motion ] = str_motion;
}


// 行削除
void KeyDiag::slot_delete()
{
    Gtk::TreeModel::iterator row;

    std::list< Gtk::TreeModel::Path > rows = m_treeview.get_selection()->get_selected_rows();
    if( ! rows.size() ) return;

    row = *( m_liststore->get_iter( *rows.begin() ) );
    if( ! row ) return;

    m_liststore->erase( row );
}


// 行追加
void KeyDiag::slot_add()
{
    std::string str_motion = show_inputdiag();
    if( ! str_motion.empty() ){
        Gtk::TreeModel::iterator it_new = m_liststore->append();
        ( *it_new )[ m_columns.m_col_motion ] = str_motion;
    }
}


// デフォルトに戻す
void KeyDiag::slot_reset()
{
    const std::string default_motions = CONTROL::get_default_keymotions( m_id );
    const int mode = CONTROL::get_mode( m_id );

    // デフォルト設定が既に使われていないか確認
    std::list< std::string > list_motions = MISC::StringTokenizer( default_motions, ' ' );
    std::list< std::string >::iterator it = list_motions.begin();
    for( ; it != list_motions.end() ; ++it ){

        const int id = CONTROL::check_key_conflict( mode, MISC::remove_space( *it ) );
        if( id != CONTROL::None && id != m_id ){
            SKELETON::MsgDiag mdiag( NULL, (*it) + "\n\nは「" + CONTROL::get_label( id ) + "」で使用されています" );
            mdiag.run();
            return;
        }
    }

    // クリアして再登録
    m_liststore->clear();
    it = list_motions.begin();
    for( ; it != list_motions.end() ; ++it ) append_row( MISC::remove_space( *it ) );
}


///////////////////////////////////////////////


//
// キーボード設定ダイアログ
//
KeyPref::KeyPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url ),
      m_button_reset( "全てリセット" ),
      m_label( "編集したいショートカットキー設定をダブルクリックして下さい。" )
{
    // キー設定のバックアップを取る
    // キャンセルを押したら戻す
    CONTROL::bkup_keyconfig();

    m_liststore = Gtk::ListStore::create( m_columns );
    m_treeview.set_model( m_liststore );
    m_treeview.set_size_request( 640, 400 );
    m_treeview.signal_row_activated().connect( sigc::mem_fun( *this, &KeyPref::slot_row_activated ) );

    Gtk::TreeViewColumn* column = Gtk::manage( new Gtk::TreeViewColumn( "コマンド", m_columns.m_col_label ) );
    column->set_fixed_width( 220 );
    column->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
    column->set_resizable( true );
    m_treeview.append_column( *column );

    column = Gtk::manage( new Gtk::TreeViewColumn( "ショートカットキー", m_columns.m_col_motions ) );
    column->set_resizable( true );
    m_treeview.append_column( *column );
    Gtk::CellRenderer *cell = column->get_first_cell_renderer();
    if( cell ) column->set_cell_data_func( *cell, sigc::mem_fun( *this, &KeyPref::slot_cell_data ) );

    m_scrollwin.add( m_treeview );
    m_scrollwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS );

    m_button_reset.signal_clicked().connect( sigc::mem_fun( *this, &KeyPref::slot_reset ) );
    m_hbox.pack_start( m_button_reset, Gtk::PACK_SHRINK );

    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_label );
    get_vbox()->pack_start( m_scrollwin );
    get_vbox()->pack_start( m_hbox );

    show_all_children();
    set_title( "ショートカットキー設定" );

    append_rows();
}


void KeyPref::append_rows()
{
    append_comment_row( "■ 共通" );
    append_row( CONTROL::Up );
    append_row( CONTROL::Down );

    append_row( CONTROL::Right );
    append_row( CONTROL::Left );

    append_row( CONTROL::TabRight );
    append_row( CONTROL::TabLeft );

    append_row( CONTROL::PreBookMark );
    append_row( CONTROL::NextBookMark );

    append_row( CONTROL::PrevView );
    append_row( CONTROL::NextView );

    append_row( CONTROL::ToggleArticle );

    append_row( CONTROL::ShowPopupMenu );

    append_row( CONTROL::ShowMenuBar );
    append_row( CONTROL::ShowSideBar );

    append_row( CONTROL::PageUp );
    append_row( CONTROL::PageDown );

    append_row( CONTROL::Home );
    append_row( CONTROL::End );

    append_row( CONTROL::Back );

    append_row( CONTROL::Quit );
    append_row( CONTROL::Save );
    append_row( CONTROL::Delete );
    append_row( CONTROL::Reload );
    append_row( CONTROL::StopLoading );
    append_row( CONTROL::Copy );
    append_row( CONTROL::SelectAll );

    append_row( CONTROL::Search );
    append_row( CONTROL::SearchInvert );
    append_row( CONTROL::SearchNext );
    append_row( CONTROL::SearchPrev );
    append_row( CONTROL::DrawOutAnd );

    append_comment_row( "" );
    append_comment_row( "■ 編集" );
    append_row( CONTROL::HomeEdit );
    append_row( CONTROL::EndEdit );

    append_row( CONTROL::UpEdit );
    append_row( CONTROL::DownEdit );
    append_row( CONTROL::RightEdit );
    append_row( CONTROL::LeftEdit );

    append_row( CONTROL::DeleteEdit );
    append_row( CONTROL::BackspEdit );
    append_row( CONTROL::UndoEdit );

    append_row( CONTROL::InputAA );

    append_comment_row( "" );
    append_comment_row( "■ 板一覧 / お気に入り" );
    append_row( CONTROL::OpenBoard );
    append_row( CONTROL::OpenBoardTab );

    append_comment_row( "" );
    append_comment_row( "■ スレ一覧" );
    append_row( CONTROL::OpenArticle );
    append_row( CONTROL::OpenArticleTab );
    append_row( CONTROL::NewArticle );
    append_row( CONTROL::SearchCache );

    append_row( CONTROL::ScrollRightBoard );
    append_row( CONTROL::ScrollLeftBoard );

    append_comment_row( "" );
    append_comment_row( "■ スレビュー" );
    append_row( CONTROL::UpMid );
    append_row( CONTROL::UpFast );

    append_row( CONTROL::DownMid );
    append_row( CONTROL::DownFast );

    append_row( CONTROL::PrevRes );
    append_row( CONTROL::NextRes );

    append_row( CONTROL::GotoNew );
    append_row( CONTROL::WriteMessage );

    append_row( CONTROL::LiveStartStop );

    append_comment_row( "" );
    append_comment_row( "■ 画像ビュー" );
    append_row( CONTROL::CancelMosaic );
    append_row( CONTROL::ZoomFitImage );
    append_row( CONTROL::ZoomInImage );
    append_row( CONTROL::ZoomOutImage );
    append_row( CONTROL::OrgSizeImage );

    append_row( CONTROL::ScrollUpImage );
    append_row( CONTROL::ScrollDownImage );
    append_row( CONTROL::ScrollLeftImage );
    append_row( CONTROL::ScrollRightImage );

    append_comment_row( "" );
    append_comment_row( "■ 書き込みビュー" );
    append_row( CONTROL::CancelWrite );
    append_row( CONTROL::ExecWrite );
    append_row( CONTROL::FocusWrite );
}


void KeyPref::append_row( const int id )
{
    Gtk::TreeModel::Row row;
    row = *( m_liststore->append() );
    if( row ){

        const std::string motions = CONTROL::get_str_keymotions( id );

        row[ m_columns.m_col_label ] = CONTROL::get_label( id );
        row[ m_columns.m_col_motions ] = motions;
        row[ m_columns.m_col_id ] = id;
        if( motions != CONTROL::get_default_keymotions( id ) ) row[ m_columns.m_col_drawbg ] = true;
        else row[ m_columns.m_col_drawbg ] = false;
    }
}


void KeyPref::append_comment_row( const std::string& comment )
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
// キャンセルボタンを押した
//
void KeyPref::slot_cancel_clicked()
{
#ifdef _DEBUG
    std::cout << "KeyPref::slot_cancel_clicked\n";
#endif

    // キー設定を戻す
    CONTROL::restore_keyconfig();
}



//
// 実際の描画の際に cellrendere のプロパティをセットするスロット関数
//
void KeyPref::slot_cell_data( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it )
{
    Gtk::TreeModel::Row row = *it;

    if( row[ m_columns.m_col_drawbg ] ){
        cell->property_cell_background() = CONFIG::get_color( COLOR_BACK_HIGHLIGHT_TREE );
        cell->property_cell_background_set() = true;
    }
    else cell->property_cell_background_set() = false;
}


void KeyPref::slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
#ifdef _DEBUG
    std::cout << "KeyPref::slot_row_activated path = " << path.to_string() << std::endl;
#endif

    Gtk::TreeModel::Row row = *( m_liststore->get_iter( path ) );
    if( ! row ) return;

    const int id = row[ m_columns.m_col_id ];
    if( id == CONTROL::None ) return;

    KeyDiag diag( this, "", id, row[ m_columns.m_col_label ], row[ m_columns.m_col_motions ] );
    if( diag.run() == Gtk::RESPONSE_OK ){

        const std::string motions = diag.get_str_motions();

        row[ m_columns.m_col_motions ] = motions;
        if( motions != CONTROL::get_default_keymotions( id ) ) row[ m_columns.m_col_drawbg ] = true;
        else row[ m_columns.m_col_drawbg ] = false;

        CONTROL::remove_keymotions( id );
        CONTROL::set_keymotions( CONTROL::get_name( id ), motions );
    }
}


// デフォルト設定に戻す
void KeyPref::slot_reset()
{
    const Gtk::TreeModel::Children children = m_liststore->children();
    Gtk::TreeModel::iterator it = children.begin();
    for( ; it != children.end(); ++it ){
        Gtk::TreeModel::Row row = ( *it );
        if( row ){
            const int id = row[ m_columns.m_col_id ];
            if( id != CONTROL::None ){

                const std::string motions = CONTROL::get_default_keymotions( id );

                row[ m_columns.m_col_motions ] = motions;
                row[ m_columns.m_col_drawbg ] = false;

                CONTROL::remove_keymotions( id );
                CONTROL::set_keymotions( CONTROL::get_name( id ), motions );
            }
        }
    }
}
