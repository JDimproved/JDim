// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "treeviewbase.h"

using namespace SKELETON;


JDTreeViewBase::JDTreeViewBase()
    : m_column_for_height( 0 )
{
    add_events( Gdk::KEY_PRESS_MASK );
    add_events( Gdk::KEY_RELEASE_MASK );
    add_events( Gdk::SCROLL_MASK );
    add_events( Gdk::BUTTON_PRESS_MASK );
    add_events( Gdk::POINTER_MOTION_MASK );
}


JDTreeViewBase::~JDTreeViewBase()
{}


//
// 行数
//
const int JDTreeViewBase::get_row_size()
{
    if( ! get_model() ) return 0;

    return get_model()->children().size();
}


//
// 現在フォーカスしてる行の最初のパスを取得
//
Gtk::TreeModel::Path JDTreeViewBase::get_current_path()
{
    Gtk::TreeModel::Path path;

    std::vector< Gtk::TreeModel::Path > paths = get_selection()->get_selected_rows();
    if( paths.size() ){

        std::vector< Gtk::TreeModel::Path >::iterator it = paths.begin();
        path = ( *it );
    }

    return path;
}


// 現在フォーカスしてる行の最初のrowを取得
Gtk::TreeModel::Row JDTreeViewBase::get_current_row()
{
    Gtk::TreePath path = get_current_path();
    return get_row( path );
}


//
// x, y 座標の下のパスを取得
//
Gtk::TreeModel::Path JDTreeViewBase::get_path_under_xy( int x, int y )
{
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* column;
    int cell_x, cell_y;
    if( !get_path_at_pos( x, y, path, column, cell_x, cell_y ) ) return Gtk::TreeModel::Path();

    return path;
}


//
// 現在のマウスポインタの下のパスを取得
//
Gtk::TreeModel::Path JDTreeViewBase::get_path_under_mouse()
{
    int x, y;
    get_pointer( x, y );
    return get_path_under_xy( x, y );
}


//
// 現在のマウスポインタの下のセルの幅高さとセル内での座標を取得
//
void JDTreeViewBase::get_cell_xy_wh( int& cell_x, int& cell_y, int& cell_w, int& cell_h )
{
    cell_x = cell_y = cell_w = cell_h = -1;

    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* column;
    int x, y, o_x, o_y;
    Gdk::Rectangle rect;

    get_pointer( x, y );
    get_path_at_pos( x, y, path, column, cell_x, cell_y );
    if( column ) column->cell_get_size( rect, o_x, o_y, cell_w, cell_h );
}


//
// 選択中の Gtk::TreeModel::iterator のリストを取得
//
// 削除などを実行してから get_model()->get_iter() するとパスが変わってエラーが出るので
// 先に iterator だけ取得しておく
//
std::list< Gtk::TreeModel::iterator > JDTreeViewBase::get_selected_iterators()
{
    std::list< Gtk::TreeModel::iterator > list_it;
    
    if( get_model() ){

        std::vector< Gtk::TreeModel::Path > paths = get_selection()->get_selected_rows();
        std::vector< Gtk::TreeModel::Path >::iterator it = paths.begin();
        for( ; it != paths.end(); ++it ) list_it.push_back( get_model()->get_iter( ( *it ) ) );
    }

    return list_it;
}


//
// 選択行の削除
//
void JDTreeViewBase::delete_selected_rows( const bool force )
{
    std::vector< Gtk::TreeModel::Path > list_path = get_selection()->get_selected_rows();

    if( ! list_path.size() ) return;

    Glib::RefPtr< Gtk::ListStore > liststore;
    Glib::RefPtr< Gtk::TreeStore > treestore = Glib::RefPtr< Gtk::TreeStore >::cast_dynamic( get_model() );
    if( ! treestore ){

        liststore = Glib::RefPtr< Gtk::ListStore >::cast_dynamic( get_model() );
        if( ! liststore ) return;
    }

    // カーソルを選択範囲の最後の行の次の行に移動
    const Gtk::TreePath next = next_path( list_path.back(), true );
    const bool gotobottom = ( ! get_row( next ) );
    if( ! gotobottom ) set_cursor( next );

    std::vector< Gtk::TreePath >::reverse_iterator it = list_path.rbegin();
    for( ; it != list_path.rend(); ++it ){
        Gtk::TreeRow row = get_row( *it );

        if( treestore ) treestore->erase( row );
        else liststore->erase( row );
    }

    if( gotobottom ) goto_bottom();
}



//
// 先頭へ
//
void JDTreeViewBase::goto_top()
{
    if( ! get_row_size() ) return;

    Gtk::TreePath path = get_model()->get_path(  *( get_model()->children().begin() ) );
    scroll_to_row( path, 0 );
    set_cursor( path );
}


//
// 一番最後へ
//
void JDTreeViewBase::goto_bottom()
{
    if( ! get_row_size() ) return;

    Gtk::TreePath path = get_model()->get_path( *( get_model()->children().rbegin() ) );

    // ディレクトリを開いている時、一番下の行に移動
    Gtk::TreePath path_prev = path;
    while( ! path.empty() ){
        Gtk::TreePath path_tmp = next_path( path );
        if( path_tmp == path ) break; // 変化が無くなったらbreak
        path_prev = path;
        path = path_tmp;
    }

    scroll_to_row( path_prev, 0 );
    set_cursor( path_prev );
}


//
// 選択行を上へ移動
//
const bool JDTreeViewBase::row_up()
{
    Gtk::TreePath path = get_current_path();
    if( !get_row( path ) ) return false;

    Gtk::TreePath new_path = prev_path( path );

    if( path != new_path ) set_cursor( new_path );
    else return false;

    return true;
}    


//
// 選択行を下へ移動
//
const bool JDTreeViewBase::row_down()
{
    Gtk::TreePath path = get_current_path();
    if( !get_row( path ) ) return false;

    Gtk::TreePath new_path = next_path( path );
    if( new_path.size() && get_row( new_path ) ) set_cursor( new_path );
    else return false;

    return true;
}


//
// page up
//
void JDTreeViewBase::page_up()
{
    bool set_top = false;

    // スクロール
    auto adj = get_vadjustment();
    double val = adj->get_value();
    if( val > adj->get_page_size()/2 ) set_top = true;
    val = MAX( 0, val - adj->get_page_size() );
    adj->set_value( val );

    // 選択行移動
    Gtk::TreePath path;
    if( set_top ) path = get_path_under_xy( 0, (int)adj->get_page_size() - 4 );
    else path = get_path_under_xy( 0, 0 );
    if( path.size() && get_row( path ) )set_cursor( path );
}


//
// page down
//
void JDTreeViewBase::page_down()
{
    bool set_bottom = false;

    // スクロール
    auto adj = get_vadjustment();
    double val = adj->get_value();
    if( val < adj->get_upper() - adj->get_page_size() - adj->get_page_size()/2 ) set_bottom = true;
    val = MIN( adj->get_upper() - adj->get_page_size(), val + adj->get_page_size() );
    adj->set_value( val );

    // 選択行移動
    Gtk::TreePath path;
    if( set_bottom ) path = get_path_under_xy( 0, 0 );
    else path = get_path_under_xy( 0, (int)adj->get_page_size() - 4 );
    if( path.size() && get_row( path ) ) set_cursor( path );
}


//
// path の前の path を取得
//
// check_expand = true なら行が開いてるかチェックして開いて無い時はdown()しない
//
Gtk::TreePath JDTreeViewBase::prev_path( const Gtk::TreePath& path, bool check_expand )
{
    Gtk::TreePath path_out( path );

    // 前に移動
    if( path_out.prev() && ( row_expanded( path_out ) || ! check_expand ) ){

        Gtk::TreePath path_tmp = path_out;
        while( get_row( path_out ) && ( path_out = next_path( path_out, check_expand ) ) != path ) path_tmp = path_out;
        if( get_row( path_tmp ) ) return path_tmp;
    }

    // 一番上まで到達したらup
    path_out = path;
    if( ! path_out.prev() && path_out.size() >= 2 ) path_out.up();

    return path_out;;
}


//
// path の次の path を取得
//
// check_expand = true なら行が開いてるかチェックして開いて無い時はdown()しない
//
Gtk::TreePath JDTreeViewBase::next_path( const Gtk::TreePath& path, bool check_expand )
{
    if( !get_row( path ) ) return path;
    Gtk::TreePath path_out( path );

    if( row_expanded( path_out ) || ! check_expand ){
        path_out.down();
        if( get_row( path_out ) ) return path_out;
    }

    // next()してレベルの一番下まで到達したら上のレベルに移動
    path_out = path;
    while( path_out.next(), ( ! get_row( path_out ) && path_out.size() >=2 ) ) path_out.up();

    return path_out;
}


//
// path->row 変換
//
Gtk::TreeModel::Row JDTreeViewBase::get_row( const Gtk::TreePath& path )
{
    if( path.empty() || ! get_model() ) return Gtk::TreeModel::Row();

    Gtk::TreeModel::Row row = *( get_model()->get_iter( path ) );
    if( !row ) return Gtk::TreeModel::Row();
    if( path != get_model()->get_path( row ) ) return Gtk::TreeModel::Row();

    return row;
}


//
// pathの親を再起的に開く
//
void JDTreeViewBase::expand_parents( const Gtk::TreePath& path )
{
    if( ! get_model() ) return;

    for( Gtk::TreePath::size_type level = 1; level < path.size(); ++level ){
                    
        Gtk::TreeModel::Row row_tmp = get_row( path );
        if( ! row_tmp ) return;

        for( Gtk::TreePath::size_type i = 0; i < path.size() - level; ++i ){
            if( row_tmp.parent() ) row_tmp = *( row_tmp.parent() );
        }
        Gtk::TreePath path_tmp  = get_model()->get_path( row_tmp );
        expand_row( path_tmp, false );
    }
}


//
// pathが開かれているか
//
const bool JDTreeViewBase::is_expand( const Gtk::TreePath& path )
{
    Gtk::TreePath parent( path );

    if( path.size() < 2 ) return true;
    if( parent.up() && row_expanded( parent ) ) return true;
    return false;
}

//
// 行のセルの高さ
//
int JDTreeViewBase::get_row_height()
{
    Gtk::TreeViewColumn* column = get_column( m_column_for_height );
    if( !column ) return 0;

    int x,y,w,h;
    Gdk::Rectangle rect;
    column->cell_get_size( rect, x, y, w, h );

    return h;
}


//
// キーボードのキーを押した
//
bool JDTreeViewBase::on_key_press_event( GdkEventKey* event )
{
    return m_sig_key_press.emit( event );
}


//
// キーボードのキーを離した
//
bool JDTreeViewBase::on_key_release_event( GdkEventKey* event )
{
    m_sig_key_release.emit( event );
    return true;
}


// マウスのwheelを回した
bool JDTreeViewBase::on_scroll_event( GdkEventScroll* event )
{
    m_sig_scroll_event.emit( event );

    return Gtk::TreeView::on_scroll_event( event );
}


//
// マウスボタンを押した
//
bool JDTreeViewBase::on_button_press_event( GdkEventButton* event )
{
    m_sig_button_press.emit( event );
    return Gtk::TreeView::on_button_press_event( event );
}


//
// マウスボタンを離した
//
bool JDTreeViewBase::on_button_release_event( GdkEventButton* event )
{
    m_sig_button_release.emit( event );
    return Gtk::TreeView::on_button_release_event( event );
}


//
// マウスを動かした
//
bool JDTreeViewBase::on_motion_notify_event( GdkEventMotion* event )
{
    m_sig_motion_notify.emit( event );
    return Gtk::TreeView::on_motion_notify_event( event );
}
