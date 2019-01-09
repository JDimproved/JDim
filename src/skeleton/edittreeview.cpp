// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "gtkmmversion.h"

#include "edittreeview.h"
#include "editcolumns.h"
#include "msgdiag.h"
#include "undobuffer.h"

#include "dbtree/interface.h"

#include "config/globalconf.h"

#include "xml/document.h"

#include "sharedbuffer.h"
#include "global.h"


#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


enum
{
    DNDSCROLLSPEED = 250  // D&D 時のスクロール速度
};



//
// 行の最新アドレスを取得
//
const std::string get_uptodate_url( const std::string& url_org, const int type )
{
    std::string url = url_org;

    if( type == TYPE_BOARD || type == TYPE_BOARD_UPDATE ){

        url = DBTREE::url_boardbase( url );
    }
    else if( type == TYPE_THREAD || type == TYPE_THREAD_UPDATE || type == TYPE_THREAD_OLD ){

        url = DBTREE::url_dat( url );
    }

    return url;
}


//
// 行の最新状態を取得
//
int get_uptodate_type( const std::string& url, const int type_org )
{
    int type = type_org;

    if( type == TYPE_BOARD || type == TYPE_BOARD_UPDATE ){

        if( DBTREE::board_status( url ) & STATUS_UPDATE ) type = TYPE_BOARD_UPDATE;
        else if( type != TYPE_BOARD ) type = TYPE_BOARD;
    }
    else if( type == TYPE_THREAD || type == TYPE_THREAD_UPDATE || type == TYPE_THREAD_OLD ){

        if( DBTREE::article_status( url ) & STATUS_OLD ) type = TYPE_THREAD_OLD;
        else if( DBTREE::article_status( url ) & STATUS_UPDATE ) type = TYPE_THREAD_UPDATE;
        else if( type != TYPE_THREAD ) type = TYPE_THREAD;
    }

    return type;
}


//
// ソート用比較クラス
//
// EditTreeView::sort()で使用
//
class compare_path
{
    SKELETON::JDTreeViewBase& m_treeview;
    SKELETON::EditColumns& m_columns;
    int m_mode;

    int type_to_order( const int type )
    {
        int order[]={

            TYPE_DIR,

            TYPE_VBOARD,

            TYPE_BOARD_UPDATE,
            TYPE_BOARD,

            TYPE_THREAD_UPDATE,
            TYPE_THREAD,

            TYPE_IMAGE,

            TYPE_COMMENT,

            TYPE_THREAD_OLD,

            TYPE_UNKNOWN
        };

        int i = 0;
        for( ; order[ i ] != TYPE_UNKNOWN; ++i ){

            if( type == order[ i ] ) return i;
        }

        return i;
    }

public:

    compare_path( SKELETON::JDTreeViewBase& treeview, SKELETON::EditColumns& columns, const int mode )
        : m_treeview( treeview ), m_columns( columns ), m_mode( mode ){}

    // path_a が上ならtrue　path_b　が上ならfalse
    bool operator () ( const Gtk::TreePath& path_a, const Gtk::TreePath& path_b )
    {
        const Gtk::TreeRow row_a = m_treeview.get_row( path_a );
        const Gtk::TreeRow row_b = m_treeview.get_row( path_b );

        if( ! row_a || ! row_b ) return false;

        // 名前でソート
        if( m_mode == SKELETON::SORT_BY_NAME ){

            const Glib::ustring name_a = row_a[ m_columns.m_name ];
            const Glib::ustring name_b = row_b[ m_columns.m_name ];

            if( name_a > name_b ) return false;
            else if( name_a < name_b ) return true;
        }

        // タイプでソート
        const Glib::ustring url_a = row_a[ m_columns.m_url ];
        const Glib::ustring url_b = row_b[ m_columns.m_url ];

        const int type_a = get_uptodate_type( url_a.raw(), row_a[ m_columns.m_type ] );
        const int type_b = get_uptodate_type( url_b.raw(), row_b[ m_columns.m_type ] );

        const int order_a = type_to_order( type_a );
        const int order_b = type_to_order( type_b );

        if( order_a == order_b ) return false;
        return ( order_a < order_b );
    }
};


/////////////////////////////////////////////////


using namespace SKELETON;


EditTreeView::EditTreeView( const std::string& url, const std::string& dndtarget, EditColumns& columns,
                            const bool use_usr_fontcolor, const std::string& fontname, const int colorid_text, const int colorid_bg, const int colorid_bg_even )
    : DragTreeView( url, dndtarget, use_usr_fontcolor, fontname, colorid_text, colorid_bg, colorid_bg_even ),
      m_url( url ),
      m_columns( columns )
{
    setup();
}


EditTreeView::EditTreeView( const std::string& url, const std::string& dndtarget, EditColumns& columns )
    : DragTreeView( url, dndtarget, false, "", 0, 0, 0 ),
      m_url( url ),
      m_columns( columns )
{
    setup();
}


void EditTreeView::setup()
{
#ifdef _DEBUG
    std::cout << "EditTreeView::Setup url = " << m_url << std::endl;
#endif

    m_parent_win = NULL;

    m_updated = false;
    m_dragging_on_tree = false;
    m_editable = false;
    m_undo_buffer = NULL;
    m_dnd_counter = 0;
    m_row_dest = Gtk::TreeRow();

    m_pre_adjust_upper = 0;
    m_jump_path = Gtk::TreePath();
}



EditTreeView::~EditTreeView()
{
#ifdef _DEBUG
    std::cout << "EditTreeView::~EditTreeView url = " << m_url << std::endl;
#endif
}


//
// 編集可能にする
//
void EditTreeView::set_editable_view( const bool editable )
{
    m_editable = editable;

    if( m_editable ){

        // D&D のドロップを可能にする
        std::vector< Gtk::TargetEntry > targets;
        targets.push_back( Gtk::TargetEntry( get_dndtarget(), Gtk::TARGET_SAME_APP, 0 ) );
        drag_dest_set( targets );
    }
}


//
// クロック入力
//
void EditTreeView::clock_in()
{
    DragTreeView::clock_in();

    // D&D 中にカーソルが画面の上か下の方にある場合はスクロールさせる
    if( m_editable && m_dragging_on_tree ){

        ++m_dnd_counter;
        if( m_dnd_counter >= DNDSCROLLSPEED / TIMER_TIMEOUT ){

            m_dnd_counter = 0;

            Gtk::TreePath path = get_path_under_mouse();
            auto adjust = get_vadjustment();

            if( get_row( path ) && adjust ){

                const int height = get_height();
                const int step = (int)adjust->get_step_increment() / 2;
                int val = -1;
                int x,y;
                get_pointer( x, y );

                if( y < step * 2 ){
                    val = MAX( 0, (int)adjust->get_value() - step );
                }
                else if( y > height - step * 2 ){
                    val = MIN( (int)adjust->get_value() + step, (int)( adjust->get_upper() - adjust->get_page_size() ) );
                }

                if( val != -1 ){
                    adjust->set_value( val );
                    path = get_path_under_mouse();
                    draw_underline_while_dragging( path );
                }
            }
        }
    }

    // 行を追加した直後に scroll_to_row() を呼んでもまだツリーが更新されてなくて
    // 正しくスクロールしないので更新されてからスクロールする
    else if( m_pre_adjust_upper && ! m_jump_path.empty() ){

        const int upper = ( int )( get_vadjustment()->get_upper() );

#ifdef _DEBUG
        std::cout << "adjust_upper : " << m_pre_adjust_upper << " -> " << upper << " count = " << m_jump_count << std::endl;
#endif

        // 時間切れ
        const int max_jump_count = 200 / TIMER_TIMEOUT;
        ++m_jump_count;

        if( m_pre_adjust_upper != upper || m_jump_count >= max_jump_count ){

#ifdef _DEBUG
            std::cout << "scroll to " << m_jump_path.to_string() << std::endl;
#endif

            Gtk::TreeRow row = get_row( Gtk::TreePath( m_jump_path ) );
            if( row ) scroll_to_row( m_jump_path, 0.5 );

            m_pre_adjust_upper = 0;
            m_jump_path = Gtk::TreePath();
            m_jump_count = 0;
        }
    }

}


//
// path にスクロール
//
// 遅延させてツリー構造が変わってからスクロールする
// clock_in()を参照
//
void EditTreeView::set_scroll( const Gtk::TreePath& path )
{
    m_pre_adjust_upper = ( int )( get_vadjustment()->get_upper() );
    m_jump_path = path;
    m_jump_count = 0;
}


//
// treestoreのセット
//
void EditTreeView::set_treestore( const Glib::RefPtr< Gtk::TreeStore >& treestore )
{
    set_model( treestore );
    set_headers_visible( false );
}


//
// xml -> tree 展開して treestore をセットする
//
void EditTreeView::xml2tree( XML::Document& document, Glib::RefPtr< Gtk::TreeStore >& treestore, const std::string& root_name )
{
#ifdef _DEBUG
    std::cout << "EditTreeView::xml2tree ";
    std::cout << " root = " << root_name;
    std::cout << " size = " << document.childNodes().size() << std::endl;
#endif

    treestore->clear();

#if GTKMM_CHECK_VERSION(2,8,0)
    unset_model();
#endif

    // 開いてるツリーの格納用
    std::list< Gtk::TreePath > list_path_expand;

    // Domノードから Gtk::TreeStore をセット
    document.set_treestore( treestore, m_columns, root_name, list_path_expand );

#if GTKMM_CHECK_VERSION(2,8,0)
    set_treestore( treestore );
#endif

    // ディレクトリIDのセット(まだセットされていない場合)
    get_max_dirid();
    set_dirid();

    // ディレクトリオープン
    std::list< Gtk::TreePath >::iterator it_path = list_path_expand.begin();
    while( it_path != list_path_expand.end() )
    {
        expand_parents( *it_path );
        expand_row( *it_path, false );
        ++it_path;
    }
}


//
// tree -> XML 変換
//
void EditTreeView::tree2xml( XML::Document& document, const std::string& root_name )
{
    Glib::RefPtr< Gtk::TreeStore > treestore = Glib::RefPtr< Gtk::TreeStore >::cast_dynamic( get_model() );
    if( ! treestore || treestore->children().empty() ){
        document.clear();
        return;
    }

    // 全てのツリーに row[ m_columns.expand ] の値をセットする
    set_expanded_row( treestore, treestore->children() );

    // m_treestore からノードツリーを作成
    document.init( treestore, m_columns, root_name );

#ifdef _DEBUG
    std::cout << "EditTreeView::tree2xml ";
    std::cout << " root = " << root_name;
    std::cout << " size = " << document.childNodes().size() << std::endl;
#endif
}


// ディレクトリIDの最大値を取得
void EditTreeView::get_max_dirid()
{
    m_max_dirid = 0;

    SKELETON::EditTreeViewIterator it( *this, m_columns, Gtk::TreePath() );
    for( ; ! it.end(); ++it ){

        Gtk::TreeModel::Row row = *it;

        if( row[ m_columns.m_type ] == TYPE_DIR ){

            const size_t dirid = row[ m_columns.m_dirid ];
            if( dirid > m_max_dirid ) m_max_dirid = dirid;
        }
    }

    ++m_max_dirid;

#ifdef _DEBUG
    std::cout << "EditTreeView::get_max_dirid id = " << m_max_dirid << std::endl;
#endif
}


// IDがついていないディレクトリにIDをセットする
void EditTreeView::set_dirid()
{
#ifdef _DEBUG
    std::cout << "EditTreeView::set_dirid\n";
#endif

    SKELETON::EditTreeViewIterator it( *this, m_columns, Gtk::TreePath() );
    for( ; ! it.end(); ++it ){

        Gtk::TreeModel::Row row = *it;

        if( row[ m_columns.m_type ] == TYPE_DIR ){

            const size_t dirid = row[ m_columns.m_dirid ];
            if( ! dirid ){

                row[ m_columns.m_dirid ] = m_max_dirid++;

#ifdef _DEBUG
                std::cout << row[ m_columns.m_name ] << " id = " << row[ m_columns.m_dirid ] << std::endl;
#endif
            }
        }
    }
}



//
// 全てのツリーに m_columns.m_expand の値をセットする( tree2xml()で使用 )
//
void EditTreeView::set_expanded_row( Glib::RefPtr< Gtk::TreeStore >& treestore, const Gtk::TreeModel::Children& children )
{
    Gtk::TreeModel::iterator it = children.begin();
    while( it != children.end() )
    {
        const Gtk::TreePath path = treestore->get_path( *it );

        // ツリーが開いているか
        if( row_expanded( path ) ) (*it)[ m_columns.m_expand ] = true;
        else (*it)[ m_columns.m_expand ] = false;

        // 再帰
        if( ! (*it)->children().empty() ) set_expanded_row( treestore, (*it)->children() );

        ++it;
    }
}


//
// 列の作成
//
// ypad : 行間スペース
//
Gtk::TreeViewColumn* EditTreeView::create_column( const int ypad )
{
    // Gtk::mange　してるのでdeleteしなくてもよい
    Gtk::TreeViewColumn* col = Gtk::manage( new Gtk::TreeViewColumn( "name" ) );
    col->pack_start( m_columns.m_image, Gtk::PACK_SHRINK );

    m_ren_text = Gtk::manage( new Gtk::CellRendererText() );
    m_ren_text->signal_edited().connect( sigc::mem_fun( *this, &EditTreeView::slot_ren_text_on_edited ) );
    m_ren_text->signal_editing_canceled().connect( sigc::mem_fun( *this, &EditTreeView::slot_ren_text_on_canceled ) );
    m_ren_text->property_underline() = Pango::UNDERLINE_SINGLE;

    // 行間スペース
    if( ypad >= 0 ) m_ren_text->property_ypad() = ypad;

    col->pack_start( *m_ren_text, true );
    col->add_attribute( *m_ren_text, "text", EDITCOL_NAME );
    col->add_attribute( *m_ren_text, "underline", EDITCOL_UNDERLINE );
    col->add_attribute( *m_ren_text, "foreground_gdk", EDITCOL_FGCOLOR );
    col->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );

    // 実際の描画時に偶数行に色を塗る
    col->set_cell_data_func( *col->get_first_cell(), sigc::mem_fun( *this, &DragTreeView::slot_cell_data ) );
    col->set_cell_data_func( *m_ren_text, sigc::mem_fun( *this, &DragTreeView::slot_cell_data ) );

    append_column( *col );

    return col;
}


//
// 新規ディレクトリ作成
//
const Gtk::TreePath EditTreeView::create_newdir( const Gtk::TreePath& path )
{
    CORE::DATA_INFO_LIST list_info;
    CORE::DATA_INFO info;
    info.type = TYPE_DIR;
    info.name = "新規ディレクトリ";
    info.path = path.to_string();

    while( ! dirid_to_path( m_max_dirid ).empty() ) ++m_max_dirid;
    info.dirid = m_max_dirid;

    list_info.push_back( info );

    const bool before = false;
    const bool scroll = false;
    const bool force = false;
    //  append_info 内でundoのコミットをしないで名前を変更してからslot_ren_text_on_canceled()でコミットする
    const bool cancel_undo_commit = true;
    const int check_dup = 0; // 項目の重複チェックをしない
    append_info( list_info, path, before, scroll, force, cancel_undo_commit, check_dup );

    Gtk::TreePath path_new = get_current_path();
    set_cursor( path_new );
    rename_row( path_new );

    return path_new;
}


// ディレクトリIDとパスを相互変換
const Gtk::TreePath EditTreeView::dirid_to_path( const size_t dirid )
{
    if( ! dirid ) return Gtk::TreePath();

    SKELETON::EditTreeViewIterator it( *this, m_columns, Gtk::TreePath() );
    for( ; ! it.end(); ++it ){

        Gtk::TreeModel::Row row = *it;

        if( row[ m_columns.m_type ] == TYPE_DIR ){

            if( row[ m_columns.m_dirid ] == dirid ) return get_model()->get_path( row );
        }
    }

    return Gtk::TreePath();
}


size_t EditTreeView::path_to_dirid( const Gtk::TreePath path )
{
    Gtk::TreeModel::Row row = get_row( Gtk::TreePath( path ) );
    if( row ) return row[ m_columns.m_dirid ];

    return 0;
}


//
// コメント挿入
//
const Gtk::TreePath EditTreeView::create_newcomment( const Gtk::TreePath& path )
{
    CORE::DATA_INFO_LIST list_info;
    CORE::DATA_INFO info;
    info.type = TYPE_COMMENT;
    info.name = "コメント";
    info.path = path.to_string();
    list_info.push_back( info );

    const bool before = false;
    const bool scroll = false;
    const bool force = false;
    //  append_info 内でundoのコミットをしないで名前を変更してからslot_ren_text_on_canceled()でコミットする
    const bool cancel_undo_commit = true;
    const int check_dup = 0; // 項目の重複チェックをしない
    append_info( list_info, path, before, scroll, force, cancel_undo_commit, check_dup );

    Gtk::TreePath path_new = get_current_path();
    set_cursor( path_new );
    rename_row( path_new );

    return path_new;
}


//
// pathで指定した行の名前変更
//
void EditTreeView::rename_row( const Gtk::TreePath& path )
{
    if( path.empty() ) return;

    // edit可 slot_ren_text_on_edited() と slot_ren_text_on_canceled で false にする
    m_ren_text->property_editable() = true;
    set_cursor( path, *get_column( 0 ), true );
}


//
// 行の名前を変更したときにCellRendererTextから呼び出される
//
void EditTreeView::slot_ren_text_on_edited( const Glib::ustring& path, const Glib::ustring& text )
{
#ifdef _DEBUG
    std::cout << "EditTreeView::slot_ren_text_on_edited\n"
              << "path = " << path << std::endl
              << "text = " << text << std::endl;
#endif

    Gtk::TreeRow row = get_row( Gtk::TreePath( path ) );
    if( row ){

        const Glib::ustring text_before = row[ m_columns.m_name ];
        row[ m_columns.m_name ] = text;

        if( m_editable && m_undo_buffer ) m_undo_buffer->set_name( Gtk::TreePath( path ), text, text_before );
    }

    slot_ren_text_on_canceled();
}


//
// 行の名前変更をキャンセルしたときにCellRendererTextから呼び出される
//
void EditTreeView::slot_ren_text_on_canceled()
{
#ifdef _DEBUG
    std::cout << "EditTreeView::slot_ren_text_on_canceld\n";
#endif

    m_ren_text->property_editable() = false;

    if( m_editable && m_undo_buffer ) m_undo_buffer->commit();
}


//
// D&D中の受信側上でマウスを動かした
//
// 編集可能の時は下線を引く
// 他のwidgetがソースの時も呼び出される。ドラッグ中は on_motion_notify_event() は呼び出されない
//
bool EditTreeView::on_drag_motion( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time )
{
    const Gtk::TreePath path = get_path_under_mouse();

#ifdef _DEBUG
    if( ! m_dragging_on_tree ) std::cout << "EditTreeView::on_drag_enter";
    else std::cout << "EditTreeView::on_drag_motion";

    std::cout << " x = " << x << " y = " << y;
    if( ! path.empty() ) std::cout << " path = " << path.to_string();
    std::cout << std::endl;
#endif

    const bool ret = DragTreeView::on_drag_motion( context, x, y, time );

    m_dragging_on_tree = true;
    draw_underline_while_dragging( path );

    return ret;
}


//
// D&D中に受信側からマウスが出た
//
void EditTreeView::on_drag_leave( const Glib::RefPtr<Gdk::DragContext>& context, guint time )
{
#ifdef _DEBUG
    std::cout << "EditTreeView::on_drag_leave\n";
#endif

    DragTreeView::on_drag_leave( context, time );

    m_dragging_on_tree = false;
    draw_underline( m_drag_path_uline, false );
}


//
// D&Dで受信側がデータ送信を要求してきた
//
void EditTreeView::on_drag_data_get( const Glib::RefPtr<Gdk::DragContext>& context,
                                     Gtk::SelectionData& selection_data, guint info, guint time )
{
#ifdef _DEBUG
    std::cout << "EditTreeView::on_drag_data_get target = " << selection_data.get_target() << std::endl;
#endif

    DragTreeView::on_drag_data_get( context, selection_data, info, time );

    // 範囲選択行を共有バッファに入れる
    if( selection_data.get_target() == get_dndtarget() ){

        CORE::DATA_INFO_LIST list_info;
        const bool dir = true;
        get_info_in_selection( list_info, dir );

        if( list_info.size() ){

            CORE::SBUF_set_list( list_info );

            // 受信側の on_drag_data_received() を呼び出す
            selection_data.set( get_dndtarget(), m_url );
        }
    }
}


//
// D&Dの受信側がデータを取得
//
void EditTreeView::on_drag_data_received( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y,
                                          const Gtk::SelectionData& selection_data, guint info, guint time )
{
    const std::string url_from = selection_data.get_data_as_string();

#ifdef _DEBUG
    std::cout << "EditTreeView::on_drag_data_received target = " << selection_data.get_target()
              << " url = " << m_url << " url_from = " << url_from << std::endl;
#endif

    DragTreeView::on_drag_data_received( context, x, y, selection_data, info, time );

    draw_underline( m_drag_path_uline, false );
    m_exec_drop = false;
    m_row_dest = Gtk::TreeRow();
    m_row_dest_before = false;
    m_dropped_from_other = false;

    // 挿入先のrowを保存
    if( m_editable && selection_data.get_target() == get_dndtarget() ){

        CORE::DATA_INFO_LIST list_info = CORE::SBUF_list_info();
        if( ! list_info.size() ) return;

        const Gtk::TreePath path_dest = get_path_under_mouse();
        m_row_dest = get_row( path_dest );

        // セル内の座標を見て真ん中より上だったら上に挿入
        if( m_row_dest ){

            int cell_x, cell_y, cell_w, cell_h;
            get_cell_xy_wh( cell_x, cell_y, cell_w, cell_h );

            if( cell_y < cell_h / 2 ) m_row_dest_before = true;
        }

        if( url_from != m_url ) m_dropped_from_other = true;

#ifdef _DEBUG
        std::cout << "x = " << x << " y = " << y
                  << " path_dest = " << path_dest.to_string()
                  << " before = " << m_row_dest_before
                  << " other = " << m_dropped_from_other
                  << std::endl;
#endif

        // 同じ widget からドロップされた場合は上書きになっていないかチェックする
        if( m_row_dest && ! m_dropped_from_other ){

            CORE::DATA_INFO_LIST::iterator it = list_info.begin();
            for( ; it != list_info.end(); ++it ){

#ifdef _DEBUG
                std::cout << ( *it ).name << " path = " << ( *it ).path << std::endl;
#endif

                if( ( *it ).path.empty() ) continue;

                // 移動先と送り側が同じならキャンセル
                if( ( *it ).path == path_dest.to_string() ) return;

                // 移動先がサブディレクトリに含まれないかチェック
                if( Gtk::TreePath( ( *it ).path ).is_ancestor( path_dest ) ){

                    SKELETON::MsgDiag mdiag( get_parent_win(), "移動先は送り側のディレクトリのサブディレクトリです", false, Gtk::MESSAGE_ERROR );
                    mdiag.run();
                    return;
                }
            }
        }

        m_exec_drop = true;

        // 送信側の on_drag_data_delete()を呼び出す
        context->drag_finish( true, true, time );
    }
}


//
// D&Dの送信側がデータを削除
//
void EditTreeView::on_drag_data_delete( const Glib::RefPtr<Gdk::DragContext>& context )
{
#ifdef _DEBUG
    std::cout << "EditTreeView::on_drag_data_delete\n";
#endif

    DragTreeView::on_drag_data_delete( context );

    // 選択行の削除
    if( m_editable ){

        CORE::DATA_INFO_LIST list_info = CORE::SBUF_list_info();
        delete_rows( list_info, Gtk::TreePath() );

        if( m_editable && m_undo_buffer ){
            m_undo_buffer->set_list_info_selected( list_info );  // undo したときに選択する列
            m_undo_buffer->set_list_info( CORE::DATA_INFO_LIST(), list_info );
        }
    }
}


//
// D&Dで受信側にデータがドロップされた
//
// 他のwidgetがソースの時も呼ばれるのに注意
//
bool EditTreeView::on_drag_drop( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time )
{
#ifdef _DEBUG
    std::cout << "EditTreeView::on_drag_drop\n";
#endif

    const bool ret = DragTreeView::on_drag_drop( context, x, y, time );
    if( ! get_model() ) return ret;
    if( ! m_editable ) return ret;
    if( ! m_exec_drop ) return ret;

    Gtk::TreePath path_dest = Gtk::TreePath();
    bool before = false;

    if( m_row_dest ){
        path_dest = get_model()->get_path( m_row_dest );
        before = m_row_dest_before;
    }

#ifdef _DEBUG
    std::cout << "path_dest = " << path_dest.to_string()
              << " before = " << before
              << std::endl;
#endif

    // 共有バッファ内の行を追加
    const bool scroll = false;
    const bool force = false;
    const bool cancel_undo_commit = false;
    const int check_dup = m_dropped_from_other ? CONFIG::get_check_favorite_dup() : 0;

    const CORE::DATA_INFO_LIST list_info = append_info( CORE::SBUF_list_info(), path_dest, before, scroll, force, cancel_undo_commit, check_dup );
    CORE::SBUF_clear_info();
    if( m_dropped_from_other ) m_sig_dropped_from_other.emit( list_info );

    return ret;
}


//
// D&Dで受信側に終了を知らせる
//
// この widget がソースでない時は呼び出されない
//
void EditTreeView::on_drag_end( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG
    std::cout << "EditTreeView::on_drag_end\n";
#endif

    draw_underline( m_drag_path_uline, false );

    DragTreeView::on_drag_end( context );
}


//
// ドラッグ中にマウスカーソルの下に下線を引く
//
void EditTreeView::draw_underline_while_dragging( Gtk::TreePath path )
{
    if( ! m_editable ) return;
    if( path.empty() ) return;

    bool draw = true;

    int cell_x, cell_y, cell_w, cell_h;
    get_cell_xy_wh( cell_x, cell_y, cell_w, cell_h );

    // 真ん中より上の場合
    if( cell_y < cell_h / 2 ){

        path.prev();
        if( is_dir( path )
            || path == Gtk::TreePath( "0" ) // 先頭行
            ) draw = false;
    }

    draw_underline( m_drag_path_uline, false );
    if( draw ) draw_underline( path, true );
    m_drag_path_uline = path;
}


//
// draw == true なら pathに下線を引く
//
void EditTreeView::draw_underline( const Gtk::TreePath& path, const bool draw )
{
    if( ! m_editable ) return;

    Gtk::TreeRow row = get_row( path );
    if( ! row ) return;

    row[ m_columns.m_underline ] = draw;
}


//
// path は ディレクトリか
//
bool EditTreeView::is_dir( Gtk::TreeModel::iterator& it )
{
    const Gtk::TreeRow row = ( *it );
    if( ! row ) return false;

    if( row[ m_columns.m_type ] == TYPE_DIR ) return true;

    return false;
}

bool EditTreeView::is_dir( const Gtk::TreePath& path )
{
    if( path.size() <= 0 ) return false;
    Gtk::TreeModel::iterator it = get_model()->get_iter( path );
    return is_dir( it );
}


// 前のディレクトリに移動
void EditTreeView::prev_dir()
{
    Gtk::TreePath path = get_current_path();
    for(;;){
        Gtk::TreePath new_path = prev_path( path );
        if( path == new_path ){
            goto_top();
            return;
        }
        path = new_path;
        if( is_dir( path ) ) break;
    }

    set_cursor( path );
}

// 次のディレクトリに移動
void EditTreeView::next_dir()
{
    Gtk::TreePath path = get_current_path();
    for(;;){
        path = next_path( path );
        if( ! path.size() || ! get_row( path ) ){
            goto_bottom();
            return;
        }
        if( is_dir( path ) ) break;
    }

    set_cursor( path );
}


//
// 指定したアドレスの行が含まれているか
//
bool EditTreeView::exist_row( const std::string& url, const int type )
{
    if( url.empty() ) return false;

    const std::string url_target = get_uptodate_url( url, type );

    SKELETON::EditTreeViewIterator it( *this, m_columns, Gtk::TreePath() );
    for( ; ! it.end(); ++it ){

        Gtk::TreeModel::Row row = *it;

        const Glib::ustring url_row = row[ m_columns.m_url ];
        if( ! url_row.empty() ){

            if( url_target == get_uptodate_url( url_row.raw(), row[ m_columns.m_type ] ) ) return true;
        }
    }

    return false;
}


//
// ディレクトリ内を全選択
//
void EditTreeView::select_all_dir( Gtk::TreePath path_dir )
{
    if( ! is_dir( path_dir ) ) return;

    get_selection()->select( path_dir );
    path_dir.down();

    while( get_row( path_dir ) ){

        get_selection()->select( path_dir );
        select_all_dir( path_dir );
        path_dir.next();
    }
}


//
// list_info を path_dest 以下に追加
//
// list_info の各path にあらかじめ値をセットしておくこと
//
// scroll = true なら追加した行にスクロールする
//
// force = true なら m_editable が false でも追加
//
// cancel_undo_commit = true なら undo バッファをコミットしない
//
// check_dup == 0 ならチェックせず追加 1 なら重複チェックをして重複してたらダイアログ表示、2なら重複チェックして重複してたら追加しない
//
// (1) path_dest が empty なら一番最後
//
// (2) before = true なら path_dest の前
//
// (3) path_destがディレクトリなら path_dest の下
//
// (4) そうでなければ path_dest の後
//
CORE::DATA_INFO_LIST EditTreeView::append_info( const CORE::DATA_INFO_LIST& list_info,
                                                const Gtk::TreePath& path_dest, const bool before, const bool scroll,
                                                const bool force, const bool cancel_undo_commit, int check_dup
    )
{
    CORE::DATA_INFO_LIST list_info_src;

    if( ! force && ! m_editable ) return list_info_src;
    if( ! list_info.size() ) return list_info_src;

#ifdef _DEBUG
    std::cout << "EditTreeView::append_info"
              << " path_dest = " << path_dest.to_string()
              << " before = " << before
              << " check_dup = " << check_dup
              << std::endl;
#endif

    if( ! check_dup ) list_info_src = list_info;

    // 重複がないかチェック
    else{

#ifdef _DEBUG
        std::cout << "checking duplicatiion\n";
#endif

        CORE::DATA_INFO_LIST::const_iterator it_info = list_info.begin();
        for( ; it_info != list_info.end(); ++it_info ){

            const CORE::DATA_INFO& info = ( *it_info );

            if( exist_row( info.url, info.type ) ){

                if( check_dup == 2 ) continue;

                SKELETON::MsgCheckDiag mdiag( get_parent_win(),
                                              info.name + "\n\nは既に含まれています。追加しますか？",
                                              "今後表示しない(常に追加しない) (_D)",
                                              Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, Gtk::RESPONSE_NO );
                mdiag.set_title( "お気に入り追加確認" );

                const int ret = mdiag.run();
                if( mdiag.get_chkbutton().get_active() ){
                    CONFIG::set_check_favorite_dup( 2 );
                    check_dup = 2;
                }
                if( ret != Gtk::RESPONSE_YES ) continue;
            }

            list_info_src.push_back( info );
        }
    }

    if( ! list_info_src.size() ) return list_info_src;

    if( m_editable && m_undo_buffer ){
        CORE::DATA_INFO_LIST list_info_selected;
        const bool dir = false;
        get_info_in_selection( list_info_selected, dir );
        m_undo_buffer->set_list_info_selected( list_info_selected );   // undo したときに選択する列
    }

    replace_infopath( list_info_src, path_dest, before );
    expand_parents( path_dest );
    append_rows( list_info_src );
    select_info( list_info_src );

    if( m_editable && m_undo_buffer ){
        m_undo_buffer->set_list_info( list_info_src, CORE::DATA_INFO_LIST() );
        m_undo_buffer->set_list_info_selected( list_info_src );  // redo したときに選択する列
        if( ! cancel_undo_commit ) m_undo_buffer->commit();
    }

    // 遅延させてツリー構造が変わってからスクロールする
    // clock_in()を参照
    if( scroll ) set_scroll( Gtk::TreePath( list_info_src.front().path ) );

    return list_info_src;
}


//
// pathをまとめて削除
//
// force = true なら m_editable が false でも削除
//
void EditTreeView::delete_path( std::list< Gtk::TreePath >& list_path, const bool force )
{
    if( ! list_path.size() ) return;
    if( ! force && ! m_editable ) return;

#ifdef _DEBUG
    std::cout << "EditTreeView::delete_path\n";
#endif

    // 削除範囲に現在のカーソルがある時はカーソルの位置を変更する
    bool selected = false;
    const Gtk::TreePath path_selected = get_current_path();

    CORE::DATA_INFO_LIST list_info;
    std::list< Gtk::TreePath >::iterator it = list_path.begin();
    for( ; it != list_path.end(); ++it ){

        if( path_selected == ( *it ) ) selected = true;

#ifdef _DEBUG
        std::cout << "path = " << ( *it ).to_string() << std::endl;
#endif

        CORE::DATA_INFO info;
        path2info( info, *it );
        list_info.push_back( info );
    }

    // カーソルを最後の行の次の行に移動するため、あらかじめ削除範囲の最後の行に移動しておく
    Gtk::TreePath next = path_selected;
    if( selected ) next = next_path( Gtk::TreePath( ( list_info.back() ).path ), true );

    delete_rows( list_info, next );

    if( m_editable && m_undo_buffer ){

        m_undo_buffer->set_list_info_selected( list_info );  // undo したときに選択する列

        m_undo_buffer->set_list_info( CORE::DATA_INFO_LIST(), list_info );

        CORE::DATA_INFO_LIST list_info_selected;
        const bool dir = false;
        get_info_in_selection( list_info_selected, dir );
        m_undo_buffer->set_list_info_selected( list_info_selected );  // redo したときに選択する列

        m_undo_buffer->commit();
    }
}


//
// 選択した行をまとめて削除
//
// force = true なら m_editable が false でも削除
//
void EditTreeView::delete_selected_rows( const bool force )
{
    if( ! force && ! m_editable ) return;

    std::list< Gtk::TreeModel::iterator > list_selected = get_selected_iterators();
    if( ! list_selected.size() ) return;

    // ディレクトリが含まれていないか無いか確認
    std::list< Gtk::TreeModel::iterator >::iterator it_selected = list_selected.begin();
    for( ; it_selected != list_selected.end(); ++it_selected ){

        if( is_dir( ( *it_selected ) ) ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "ディレクトリを削除するとディレクトリ内の行も全て削除されます。削除しますか？",
                                      false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            if( mdiag.run() != Gtk::RESPONSE_YES ) return;
            break;
        }
    }

#ifdef _DEBUG
    std::cout << "EditTreeView::delete_selected_rows"
              << " path = " << get_model()->get_path( *list_selected.begin() ).to_string() << std::endl;
#endif

    // 削除する行を取得
    CORE::DATA_INFO_LIST list_info;
    const bool dir = true;
    get_info_in_selection( list_info, dir );

    // カーソルを最後の行の次の行に移動するため、あらかじめ削除範囲の最後の行に移動しておく
    const Gtk::TreePath next = next_path( Gtk::TreePath( ( list_info.back() ).path ), true );

    delete_rows( list_info, next );

    if( m_editable && m_undo_buffer ){

        m_undo_buffer->set_list_info_selected( list_info );  // undo したときに選択する列

        m_undo_buffer->set_list_info( CORE::DATA_INFO_LIST(), list_info );

        CORE::DATA_INFO_LIST list_info_selected;
        const bool dir = false;
        get_info_in_selection( list_info_selected, dir );
        m_undo_buffer->set_list_info_selected( list_info_selected );  // redo したときに選択する列

        m_undo_buffer->commit();
    }
}


//
// UNDO
//
void EditTreeView::undo()
{
    if( ! m_undo_buffer ) return;
    if( ! m_undo_buffer->get_enable_undo() ) return;

    m_undo_buffer->undo();

    const UNDO_DATA& data = m_undo_buffer->get_undo_data();
    const int size = data.size();

#ifdef _DEBUG
    std::cout << "EditTreeView::undo size = " << size << std::endl;
#endif

    for( int i = size-1; i >=0; --i ){

        if( ! data[ i ].list_info_selected.empty() ){

            set_scroll( Gtk::TreePath( data[ i ].list_info_selected.front().path ) );
            select_info( data[ i ].list_info_selected );
        }

        // 追加キャンセル
        if( ! data[ i ].list_info_append.empty() ) delete_rows( data[ i ].list_info_append, Gtk::TreePath() );

        // 削除キャンセル
        if( ! data[ i ].list_info_delete.empty() ){
            expand_rows( data[ i ].list_info_delete );
            append_rows( data[ i ].list_info_delete );
        }

        // 名前変更キャンセル
        Gtk::TreeRow row = get_row( data[ i ].path_renamed );
        if( row ){

            scroll_to_row( data[ i ].path_renamed, 0.5 );  // ツリー構造に変化は無いのですぐにスクロールする
            set_cursor( data[ i ].path_renamed );
            if( ! data[ i ].name_before.empty() ) row[ m_columns.m_name ] = data[ i ].name_before;
        }
    }
}


//
// REDO
//
void EditTreeView::redo()
{
#ifdef _DEBUG
    std::cout << "EditTreeView::redo\n";
#endif

    if( ! m_undo_buffer ) return;
    if( ! m_undo_buffer->get_enable_redo() ) return;

    const UNDO_DATA& data = m_undo_buffer->get_undo_data();
    const int size = data.size();

#ifdef _DEBUG
    std::cout << "EditTreeView::redo size = " << size << std::endl;
#endif

    for( int i = 0; i < size; ++i ){

        if( ! data[ i ].list_info_selected.empty() ){

            set_scroll( Gtk::TreePath( data[ i ].list_info_selected.front().path ) );
            select_info( data[ i ].list_info_selected );
        }

        // 削除
        if( ! data[ i ].list_info_delete.empty() ) delete_rows( data[ i ].list_info_delete, Gtk::TreePath() );

        // 追加
        if( ! data[ i ].list_info_append.empty() ){
            expand_rows( data[ i ].list_info_append );
            append_rows( data[ i ].list_info_append );
        }

        // 名前変更
        Gtk::TreeRow row = get_row( data[ i ].path_renamed );
        if( row ){

            scroll_to_row( data[ i ].path_renamed, 0.5 );  // ツリー構造に変化は無いのですぐにスクロールする
            set_cursor( data[ i ].path_renamed );
            if( ! data[ i ].name_new.empty() ) row[ m_columns.m_name ] = data[ i ].name_new;
        }
    }

    m_undo_buffer->redo();
}


//
// list_info の各要素の path を path_dest 以下に変更
//
// list_info の各要素の path にあらかじめ値をセットしておくこと
//
// (1) path_dest が empty なら一番最後
//
// (2) before = true なら path_dest の前
//
// (3) path_destがディレクトリなら path_dest の下
//
// (4) そうでなければ path_dest の後
//
void EditTreeView::replace_infopath( CORE::DATA_INFO_LIST& list_info,
                                     const Gtk::TreePath& path_dest, const bool before )
{
    if( ! list_info.size() ) return;
    if( ! get_model() ) return;

    Gtk::TreePath path = path_dest;

    // path の初期値を求める
    if( path.empty() ){

        Gtk::TreeModel::Children children = get_model()->children();

        if( children.empty() ) path = Gtk::TreePath( "0" );
        else{
            path = get_model()->get_path( *( std::prev( children.end() ) ) );
            path.next();
        }
    }

    else if( before );

    else if( is_dir( path ) ) path.down();

    else path.next();

    // list_infoの先頭に path をセット
    CORE::DATA_INFO_LIST::iterator it = list_info.begin();
    Gtk::TreePath path_prev( ( *it ).path );
    ( *it ).path = path.to_string();

    const size_t max_pathsize_org = path_prev.size();
    const size_t max_pathsize = path.size();

#ifdef _DEBUG
    std::cout << "EditTreeView::replace_infopath\n"
              << "max_pathsize_org = " << max_pathsize_org
              << " max_pathsize = " << max_pathsize << std::endl
              << "path_prev = " << path_prev.to_string()
              << " -> " << ( *it ).path
              << " type = " << ( *it ).type
              << " name = " << ( *it ).name << std::endl;
#endif

    for( ++it; it != list_info.end() ; ++it ){

        CORE::DATA_INFO& info = ( *it );

        // list_infoの構造と合わせて path を更新していく
        do{

            // 次
            Gtk::TreePath path_tmp = path_prev;
            path_tmp.next();
            if( path_tmp.to_string() == info.path ){
                path.next();
                break;
            }

            // ディレクトリ下がる
            path_tmp = path_prev;
            path_tmp.down();
            if( path_tmp.to_string() == info.path ){
                path.down();
                break;
            }

            // ディレクトリ上がる
            bool reset = true;
            int count_up = 0;
            path_tmp = path_prev;
            while( path_tmp.size() != max_pathsize_org ){

                ++count_up;
                path_tmp.up();
                path_tmp.next();
                if( path_tmp.to_string() == info.path ){

                    while( count_up-- ) path.up();
                    path.next();
                    reset = false;
                    break;
                }
            }

            // どれにも該当しない場合には一番上のレベルまで戻る
            if( reset ){
                while( path.size() != max_pathsize ) path.up();
                path.next();
            }

        } while(0);

        path_prev = Gtk::TreePath( info.path );
        info.path = path.to_string();

#ifdef _DEBUG
        std::cout << "path_prev = " << path_prev.to_string()
                  << " -> " << info.path
                  << " type = " << info.type
                  << " name = " << info.name
                  << " data = " << info.data
                  << std::endl;
#endif
    }
}


//
// 選択行をlist_infoにセット
//
// dir : trueの時はディレクトリが選択されているときはディレクトリ内の行もlist_infoに再帰的にセットする
//
void EditTreeView::get_info_in_selection( CORE::DATA_INFO_LIST& list_info, const bool dir )
{
    list_info.clear();

    if( ! get_model() ) return;

    std::list< Gtk::TreeModel::iterator > list_selected = get_selected_iterators();
    if( ! list_selected.size() ) return;

    std::list< Gtk::TreeModel::iterator >::iterator it = list_selected.begin();
    for( ; it != list_selected.end(); ++it ){

        Gtk::TreePath path = get_model()->get_path( *it );
        CORE::DATA_INFO info;
        path2info( info, path );

        // 既に path が list_info に登録されていたら登録しない
        bool cancel = false;
        CORE::DATA_INFO_LIST::iterator it_info = list_info.begin();
        for( ; it_info != list_info.end(); ++it_info ){
            if( ( *it_info ).path == info.path ){
                cancel = true;
                break;
            }
        }
        if( cancel ) continue;

        list_info.push_back( info );
        if( dir ) get_info_in_dir( list_info, path );
    }
}


//
// ディレクトリ(path_dir)内の行を全てlist_infoにセットする
//
// path_dir が empty() ならルートの行を全てセット
//
void EditTreeView::get_info_in_dir( CORE::DATA_INFO_LIST& list_info, const Gtk::TreePath& path_dir )
{
    if( ! path_dir.empty() && ! is_dir( path_dir ) ) return;

    CORE::DATA_INFO info;
    Gtk::TreePath path = path_dir;
    if( path_dir.empty() ) path = Gtk::TreePath( "0" );
    else path.down();

    while( get_row( path ) ){

        path2info( info, path );
        list_info.push_back( info );

        get_info_in_dir( list_info, path );
        path.next();
    }
}


//
// path から info を取得
//
void EditTreeView::path2info( CORE::DATA_INFO& info, const Gtk::TreePath& path )
{
    Glib::ustring tmp_str;
    Gtk::TreeRow row = get_row( path );

    info.type = row[ m_columns.m_type ];

    info.parent = NULL;

    tmp_str = row[ m_columns.m_url ];
    info.url = tmp_str.raw();

    tmp_str = row[ m_columns.m_name ];
    info.name = tmp_str.raw();

    tmp_str = row[ m_columns.m_data ];
    info.data = tmp_str.raw();

    info.path = path.to_string();

    info.dirid = row[ m_columns.m_dirid ];

    info.expanded = row_expanded( path );
}


//
// 一行追加
//
// 戻り値は追加した行のpath
//
// (1) path_dest が empty なら一番最後に作る
//
// (2) before = true なら前に作る
//
// (3) path_dest がディレクトリかつ sudir == true なら path_dest の下に追加。
//
// (4) そうでなければ path_dest の後に追加
//
const Gtk::TreePath EditTreeView::append_one_row( const std::string& url, const std::string& name, const int type, const size_t dirid, const std::string& data,
                                                  const Gtk::TreePath& path_dest, const bool before, const bool subdir )
{
    Glib::RefPtr< Gtk::TreeStore > treestore = Glib::RefPtr< Gtk::TreeStore >::cast_dynamic( get_model() );
    if( ! treestore ) return Gtk::TreePath();

#ifdef _DEBUG
    std::cout << "EditTreeView::append_one_row : " << name << " path = " << path_dest.to_string()
              << " before = " << before
              << " subdir = " << subdir << std::endl;
#endif

    Gtk::TreeRow row_dest = get_row( path_dest );
    Gtk::TreeRow row_new;

    // 一番下に追加
    if( ! row_dest ) row_new = *( treestore->append() );

    // 前に追加
    else if( before ) row_new = *( treestore->insert( row_dest ) );

    // ディレクトリの下に追加
    else if( subdir && row_dest[ m_columns.m_type ] == TYPE_DIR ){
        row_new = *( treestore->prepend( row_dest.children() ) );
    }

    // 後ろに追加
    else row_new = *( treestore->insert_after( row_dest ) );

    // 行のアドレスや状態を最新にする
    const std::string url_new = get_uptodate_url( url, type );
    const int type_new = get_uptodate_type( url_new, type );

    m_columns.setup_row( row_new, url_new, name, data, type_new, dirid );

    return treestore->get_path( row_new );
}


//
// list_info に示した行の親を再起的にexpandする
// list_info の各要素の path にあらかじめ値をセットしておくこと
//
void EditTreeView::expand_rows( const CORE::DATA_INFO_LIST& list_info )
{
#ifdef _DEBUG
    std::cout << "EditTreeView::expand_rows\n";
#endif

    if( ! list_info.size() ) return;

    CORE::DATA_INFO_LIST::const_iterator it = list_info.begin();
    for( ; it != list_info.end(); ++it ){

        const CORE::DATA_INFO& info = ( *it );
#ifdef _DEBUG
        std::cout << "path = " << info.path << std::endl;
#endif
        expand_parents( Gtk::TreePath( info.path ) );
    }
}


//
// list_info に示した行を追加
//
// list_info の各要素の path にあらかじめ値をセットしておくこと
//
void EditTreeView::append_rows( const CORE::DATA_INFO_LIST& list_info )
{
#ifdef _DEBUG
    std::cout << "EditTreeView::append_rows\n";
#endif

    if( ! list_info.size() ) return;

    Gtk::TreePath path_parent;
    bool expand_parent = false;
    bool head_info = true;

    CORE::DATA_INFO_LIST::const_iterator it = list_info.begin();
    for( ; it != list_info.end(); ++it ){

        const CORE::DATA_INFO& info = ( *it );
        Gtk::TreePath path( info.path );

#ifdef _DEBUG
        std::cout << "path = " << path.to_string()
                  << " type = " << info.type
                  << " name = " << info.name
                  << " data = " << info.data
                  << std::endl;
#endif

        bool before = false;
        bool subdir = false;
        const bool prev = path.prev();

        if( ! prev ){

            // 先頭行
            if( path == Gtk::TreePath( "0" ) ) before = true;

            // ディレクトリの先頭
            else {

                path.up();
                subdir = true;

                // 最初の info なら親のディレクトリを開く
                if( head_info ){
#ifdef _DEBUG
                    std::cout << "expand parent dir\n";
#endif
                    path_parent = path;
                    expand_parent = true;
                }
            }
        }

        append_one_row( info.url, info.name, info.type, info.dirid, info.data, path, before, subdir );

        head_info = false;
    }

    // ディレクトリを開く
    if( expand_parent ) expand_row( path_parent, false );

    it = list_info.begin();
    for( ; it != list_info.end(); ++it ){
        const CORE::DATA_INFO& info = ( *it );
        if( info.expanded ){
            const Gtk::TreePath path( info.path );
            expand_row( path, false );
        }
    }

    m_updated = true;
}



//
// list_info に示した行を削除
//
// list_info の各要素の path にあらかじめ値をセットしておくこと
//
// 削除した後、path_select にカーソルを移動する(emptyの場合は移動しない)
//
void EditTreeView::delete_rows( const CORE::DATA_INFO_LIST& list_info, const Gtk::TreePath& path_select )
{
#ifdef _DEBUG
    std::cout << "EditTreeView::delete_rows";
    if( ! path_select.empty() ) std::cout << " path_select = " << path_select.to_string();
    std::cout << std::endl;
#endif

    if( ! list_info.size() ) return;

    Glib::RefPtr< Gtk::TreeStore > treestore = Glib::RefPtr< Gtk::TreeStore >::cast_dynamic( get_model() );
    if( ! treestore ) return;

    // あらかじめ path_select にカーソルを移動しておく
    // もし path_select が存在しなかったら削除してから一番下に移動
    bool gotobottom = false;
    if( ! path_select.empty() ){
        gotobottom = ( ! get_row( path_select ) );
        if( ! gotobottom ) set_cursor( path_select );
    }

    CORE::DATA_INFO_LIST::const_iterator it = list_info.end();
    do{

        --it;

        const CORE::DATA_INFO& info = ( *it );
        Gtk::TreePath path( info.path );

#ifdef _DEBUG
        std::cout << path.to_string()
                  << " type = " << info.type
                  << " name = " << info.name
                  << " url = " << info.url
                  << " data = " << info.data
                  << std::endl;
#endif

        Gtk::TreeRow row = get_row( path );
        treestore->erase( row );

    } while( it != list_info.begin() );

    m_updated = true;

    if( gotobottom ) goto_bottom();
}


//
// list_infoに示した行を選択
//
void EditTreeView::select_info( const CORE::DATA_INFO_LIST& list_info )
{
    get_selection()->unselect_all();

    CORE::DATA_INFO_LIST::const_iterator it = list_info.begin();
    for( ; it != list_info.end(); ++it ){
        Gtk::TreePath path( ( *it ).path );
        get_selection()->select( path );
    }
}



//
// ソート実行
//
// path : ディレクトリなら中をソート、そうでなければそのレベルでソート
// mode : ソートのモード。詳しくは class compare_path を参照
//
void EditTreeView::sort( const Gtk::TreePath& path, const int mode )
{
    if( ! m_editable ) return;
    if( ! get_row( path ) ) return;

    CORE::DATA_INFO_LIST list_info;

    Gtk::TreePath path_head = path;

    if( is_dir( path_head ) ) path_head.down();
    else while( path_head.prev() );
    if( ! get_row( path_head ) ) return;

    Gtk::TreePath path_parent = path_head;
    if( path_parent.size() >= 2 ) path_parent.up();
    else path_parent = Gtk::TreePath();

#ifdef _DEBUG
    std::cout << "EditTreeView::sort head = " << path_head.to_string() << " parent = " << path_parent.to_string() << std::endl;
#endif


    // ソート
    std::list< Gtk::TreePath > list_path;
    Gtk::TreePath path_tmp = path_head;
    while( get_row( path_tmp ) ){

        list_path.push_back( path_tmp );
        path_tmp.next();
    }
    list_path.sort( compare_path( *this, m_columns, mode ) );

    std::list< Gtk::TreePath >::const_iterator it_path = list_path.begin();
    for( ; it_path != list_path.end(); ++it_path ){

        const Gtk::TreeRow row = get_row( *it_path );
        if( row ){

#ifdef _DEBUG
            std::cout << ( *it_path ).to_string()
                      << " type = " << row[ m_columns.m_type ]
                      << " name = " << row[ m_columns.m_name ] << std::endl;
#endif

            CORE::DATA_INFO info;
            path2info( info, *it_path );
            list_info.push_back( info );
            get_info_in_dir( list_info, *it_path );
        }
    }

    // path_parent の中を全て削除
    if( m_undo_buffer ){
        CORE::DATA_INFO_LIST list_info_selected;
        const bool dir = false;
        get_info_in_selection( list_info_selected, dir );
        m_undo_buffer->set_list_info_selected( list_info_selected );  // undo したときに選択する列
    }

    CORE::DATA_INFO_LIST list_info_delete;
    get_info_in_dir( list_info_delete, path_parent );
    delete_rows( list_info_delete, Gtk::TreePath() );

    if( m_undo_buffer ){
        m_undo_buffer->set_list_info( CORE::DATA_INFO_LIST(), list_info_delete );
    }

    // list_info を path_parent の中に追加
    const bool before = false;
    const bool scroll = false;
    const bool force = false;
    const bool cancel_undo_commit = false;
    const int check_dup = 0; // 項目の重複チェックをしない
    append_info( list_info, path_parent, before, scroll, force, cancel_undo_commit, check_dup );

    set_scroll( path_parent );
}


////////////////////////////////

//
// EditTreeViewの項目の反復子
//
// path から反復開始
// path が empty の時はルートから反復する
//
EditTreeViewIterator::EditTreeViewIterator( EditTreeView& treeview, EditColumns& columns, const Gtk::TreePath path )
    : m_treeview( treeview ),
      m_columns( columns ),
      m_end( false ),
      m_path( path )
{
    const bool root = ( m_path.empty() );
    if( root ){

        Gtk::TreeModel::Children children = m_treeview.get_model()->children();
        if( ! children.empty() ) m_path = m_treeview.get_model()->get_path( children.begin() );
        else{
            m_end = true;
            return;
        }
    }

    Gtk::TreeModel::Row row = m_treeview.get_row( m_path );
    if( ! row ) m_end = true;
    else{

        m_depth = m_path.size();
        if( ! root ) ++m_depth;
    }
}


Gtk::TreeModel::Row EditTreeViewIterator::operator * ()
{
    return m_treeview.get_row( m_path );
}


void EditTreeViewIterator::operator ++ ()
{
    if( m_end ) return;

    Gtk::TreeModel::Row row = m_treeview.get_row( m_path );

    while( 1 ){

        if( row ){

            switch( row[ m_columns.m_type ] ){

                case TYPE_DIR:
                    m_path.down();
                    break;

                default:
                    m_path.next();
                    break;
            }
        }

        else{

            if( m_path.size() > m_depth ){
                m_path.up();
                m_path.next();
            }
            else{
                m_end = true;
                break;
            }
        }

        row = m_treeview.get_row( m_path );
        if( row ) break;
    }
}
