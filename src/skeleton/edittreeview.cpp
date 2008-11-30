// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "edittreeview.h"
#include "editcolumns.h"
#include "msgdiag.h"

#include "xml/document.h"

#include "dndmanager.h"
#include "global.h"
#include "type.h"


#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif


#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif


using namespace SKELETON;


EditTreeView::EditTreeView( EditColumns& columns,
                        const bool use_usr_fontcolor, const std::string& fontname, const int colorid_text, const int colorid_bg, const int colorid_bg_even )
    : DragTreeView( use_usr_fontcolor, fontname, colorid_text, colorid_bg, colorid_bg_even ),
      m_columns( columns ),
      m_updated( false )
{
    setup();
}


EditTreeView::EditTreeView( EditColumns& columns )
    : DragTreeView( false, "", 0, 0, 0 ),
      m_columns( columns )
{
    setup();
}


void EditTreeView::setup()
{
#ifdef _DEBUG
    std::cout << "EditTreeView::Setup\n";
#endif

    m_editable = false;
    m_dnd_counter = 0;
    CORE::get_dnd_manager()->sig_dnd_end().connect( sigc::mem_fun(*this, &EditTreeView::slot_receive_dnd_end ) );
}



EditTreeView::~EditTreeView()
{}


//
// 編集可能にする
//
void EditTreeView::set_editable_view( const bool editable )
{
    m_editable = editable;

    if( m_editable ){

        // D&D 設定
        std::list< Gtk::TargetEntry > targets;
        targets.push_back( Gtk::TargetEntry( "text/plain", Gtk::TARGET_SAME_APP, 0 ) );
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
    if( m_editable && CORE::DND_Now_dnd() ){

        ++m_dnd_counter;
        if( m_dnd_counter >= 250 / TIMER_TIMEOUT ){

            m_dnd_counter = 0;

            Gtk::TreeModel::Path path = get_path_under_mouse();
            Gtk::Adjustment* adjust = get_vadjustment();

            if( get_row( path ) && adjust ){

                int height = get_height();
                int step = (int)adjust->get_step_increment() / 2;
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

#if GTKMMVER >= 280
    unset_model();
#endif

    // 開いてるツリーの格納用
    std::list< Gtk::TreePath > list_path_expand;

    // Domノードから Gtk::TreeStore をセット
    document.set_treestore( treestore, m_columns, root_name, list_path_expand );

#if GTKMMVER >= 280
    set_treestore( treestore );
#endif

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


//
// 全てのツリーに m_columns.m_expand の値をセットする( tree2xml()で使用 )
//
void EditTreeView::set_expanded_row( Glib::RefPtr< Gtk::TreeStore >& treestore, const Gtk::TreeModel::Children& children )
{
    Gtk::TreeModel::iterator it = children.begin();
    while( it != children.end() )
    {
        Gtk::TreePath path = treestore->get_path( *it );

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
    if( ypad ) m_ren_text->property_ypad() = ypad;

    col->pack_start( *m_ren_text, true );
    col->add_attribute( *m_ren_text, "text", EDITCOL_NAME );
    col->add_attribute( *m_ren_text, "underline", EDITCOL_UNDERLINE );
    col->add_attribute( *m_ren_text, "foreground_gdk", EDITCOL_FGCOLOR );
    col->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );

    // 実際の描画時に偶数行に色を塗る
    col->set_cell_data_func( *col->get_first_cell_renderer(), sigc::mem_fun( *this, &DragTreeView::slot_cell_data ) );    
    col->set_cell_data_func( *m_ren_text, sigc::mem_fun( *this, &DragTreeView::slot_cell_data ) );    

    append_column( *col );

    return col;
}


//
// 新規ディレクトリ作成
//
const Gtk::TreePath EditTreeView::create_newdir( const Gtk::TreePath& path )
{
    Gtk::TreeModel::Path path_new;
    const bool subdir = true;
    const bool after = true;
    path_new = append_row( std::string(), "新規ディレクトリ", std::string(), TYPE_DIR, path, subdir, after );

    set_cursor( path_new );
    rename_row( path_new );

    return path_new;
}


//
// コメント挿入
//
const Gtk::TreePath EditTreeView::create_newcomment( const Gtk::TreePath& path )
{
    Gtk::TreeModel::Path path_new;
    const bool subdir = true;
    const bool after = true;
    path_new = append_row( std::string(), "コメント", std::string(), TYPE_COMMENT, path, subdir, after );

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

    Gtk::TreeModel::Row row = get_row( Gtk::TreePath( path ) );
    if( row ) row[ m_columns.m_name ] = text;

    m_ren_text->property_editable() = false;
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
}


//
// D&D中にマウスを動かした
//
// 編集可能の時は下線を引く
// 他のwidgetがソースの時も呼び出される。ラッグ中は on_motion_notify_event() は呼び出されない
//
bool EditTreeView::on_drag_motion( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time )
{
    Gtk::TreeModel::Path path = get_path_under_mouse();

#ifdef _DEBUG
    std::cout << "EditTreeView::on_drag_motion x = " << x << " y = " << y << " path = " << path.to_string() << std::endl;
#endif

    if( m_editable ) draw_underline_while_dragging( path );

    return DragTreeView::on_drag_motion( context, x, y, time );
}


//
// D&Dでドロップされた
//
// 他のwidgetがソースの時も呼ばれるのに注意
//
bool EditTreeView::on_drag_drop( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time )
{
    Gtk::TreeModel::Path path = get_path_under_mouse();

#ifdef _DEBUG
    std::cout << "EditTreeView::on_drag_drop\n";
    std::cout << "dest x = " << x << " y = " << y << " path = " << path.to_string() << std::endl;
#endif    

    bool after = true;

    if( m_editable ){

        draw_underline( m_drag_path_uline, false );

        // セル内の座標を見て真ん中より上だったら上に挿入
        if( get_row( path ) ){

            int cell_x, cell_y, cell_w, cell_h;
            get_cell_xy_wh( cell_x, cell_y, cell_w, cell_h );
            if( cell_y < cell_h / 2 ) after = false;

#ifdef _DEBUG    
            std::cout << "cell height = " << cell_h << " cell_y = " << cell_y << std::endl;
#endif
        }

        // 選択した行の移動
        if( is_dragging() ) move_selected_row( path, after );
    }

    m_sig_drag_drop.emit( path, after );

    return DragTreeView::on_drag_drop( context, x, y, time );
}


//
// D&D 終了
//
// このtreeがソースでない時は呼び出されない
//
void EditTreeView::on_drag_end( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG
    std::cout << "EditTreeView::on_drag_end\n";
#endif

    if( m_editable ) draw_underline( m_drag_path_uline, false );

    return DragTreeView::on_drag_end( context );
}


//
// D&Dマネージャから D&D 終了シグナルを受けたときに呼び出される
//
void EditTreeView::slot_receive_dnd_end()
{
#ifdef _DEBUG
    std::cout << "EditTreeView::slot_receive_dnd_end\n";
#endif

    if( m_editable ) draw_underline( m_drag_path_uline, false );
}


//
// ドラッグ中にマウスカーソルの下に下線を引く
//
void EditTreeView::draw_underline_while_dragging( Gtk::TreePath path )
{
    if( ! m_editable ) return;

    draw_underline( m_drag_path_uline, false );

    int cell_x, cell_y, cell_w, cell_h;
    get_cell_xy_wh( cell_x, cell_y, cell_w, cell_h );

    // 真ん中より上の場合
    if( cell_y < cell_h / 2 ){

        Gtk::TreeModel::Path path_tmp = prev_path( path );
        if( get_row( path_tmp ) ) path = path_tmp;

        if( ! is_dir( path ) ) draw_underline( path, true );
    }
    else draw_underline( path, true );

    m_drag_path_uline = path;
}


//
// draw == true なら pathに下線を引く
//
void EditTreeView::draw_underline( const Gtk::TreePath& path, bool draw )
{
    if( ! m_editable ) return;

    Gtk::TreeModel::Row row = get_row( path );
    if( ! row ) return;

    row[ m_columns.m_underline ] = draw;
}


//
// path は ディレクトリか
//
const bool EditTreeView::is_dir( Gtk::TreeModel::iterator& it )
{
    Gtk::TreeModel::Row row = ( *it );
    if( ! row ) return false;

    if( row[ m_columns.m_type ] == TYPE_DIR ) return true;

    return false;
}

const bool EditTreeView::is_dir( const Gtk::TreePath& path )
{
    if( path.get_depth() <= 0 ) return false;
    Gtk::TreeModel::iterator it = get_model()->get_iter( path );
    return is_dir( it );
}



//
// ディレクトリ内を全選択
//
void EditTreeView::select_all_dir( Gtk::TreeModel::Path path_dir )
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
// 行の再帰コピー
//
// subdir = true　かつdestがディレクトリならサブディレクトリをその下に作ってそこにコピーする。false ならdestの後にコピー
// after = false の場合はdestの前に挿入する
// dest が NULL なら一番下にappend
//
// 成功したら dest にコピーした行のiteratorが入る
//
const bool EditTreeView::copy_row( const Gtk::TreeModel::iterator& src, Gtk::TreeModel::iterator& dest, const bool subdir, const bool after )
{
    if( ! m_editable ) return false;

    Glib::RefPtr< Gtk::TreeStore > treestore = Glib::RefPtr< Gtk::TreeStore >::cast_dynamic( get_model() );
    if( ! treestore ) return false;

    if( ! src ) return false;
    if( dest && src == dest ) return false;

    Gtk::TreeModel::Row row_src = ( *src );
    Gtk::TreeModel::Row row_dest = ( *dest );

    bool src_is_dir = false, dest_is_dir = false;
    const int type = row_src[ m_columns.m_type ];
    if( type == TYPE_DIR ) src_is_dir = true;
    if( row_dest && row_dest[ m_columns.m_type ] == TYPE_DIR ) dest_is_dir = true;

#ifdef _DEBUG
    std::cout << "EditTreeView::copy_row " << row_src[ m_columns.m_name ] << std::endl;
    if( src_is_dir ) std::cout << "src is directory\n";
    if( dest_is_dir ) std::cout << "dest is directory\n";
#endif    

    Gtk::TreeModel::iterator it_new;

    // destがNULLなら一番下に追加
    if( ! dest  ) it_new = treestore->append();

    // destの下にサブディレクトリ作成
    else if( subdir && after && dest_is_dir ){
        it_new = treestore->prepend( row_dest.children() );
    }

    // destの後に追加
    else if( after ) it_new = treestore->insert_after( dest );

    // destの前に追加
    else it_new = treestore->insert( dest );

    Gtk::TreeModel::Row row_new = ( *it_new );
    m_columns.copy_row( row_src, row_new );

    // srcがdirならサブディレクトリ内の行も再帰的にコピー
    if( src_is_dir ){
        Gtk::TreeModel::iterator it_tmp = it_new;
        Gtk::TreeModel::iterator it_child = row_src.children().begin();
        bool subdir_tmp = true;
        for( ; it_child != row_src.children().end(); ++it_child ){
            copy_row( it_child, it_tmp, subdir_tmp, true );
            subdir_tmp = false;
        }
    }

    dest = it_new;
    return true;
}


//
// 選択した行をpathの所にまとめて移動
//
// after = true なら path の後に移動。falseなら前
//
void EditTreeView::move_selected_row( const Gtk::TreePath& path, bool after )
{
    if( ! m_editable ) return;

    Glib::RefPtr< Gtk::TreeStore > treestore = Glib::RefPtr< Gtk::TreeStore >::cast_dynamic( get_model() );
    if( ! treestore ) return;

    std::list< Gtk::TreeModel::iterator > list_it = get_selected_iterators();
    std::vector< bool > vec_cancel;
    vec_cancel.resize( list_it.size() );
    std::fill( vec_cancel.begin(), vec_cancel.end(), false );

    // 移動できるかチェック
    std::list< Gtk::TreeModel::iterator >::iterator it_src = list_it.begin();
    for( int i = 0 ; it_src != list_it.end(); ++i, ++it_src ){

        if( vec_cancel[ i ] ) continue;

        Gtk::TreeModel::Path path_src = treestore->get_path( *it_src );

        // 移動先と送り側が同じならキャンセル
        if( path_src.to_string() == path.to_string() ) return;

        // 移動先がサブディレクトリに含まれないかチェック
        if( is_dir( ( *it_src ) ) ){
            if( path.to_string().find( path_src.to_string() ) != Glib::ustring::npos ){
                SKELETON::MsgDiag mdiag( NULL, "移動先は送り側のディレクトリのサブディレクトリです", false, Gtk::MESSAGE_ERROR );
                mdiag.run();
                return;
            }
        }

        // path_srcのサブディレクトリ内の行も範囲選択内に含まれていたらその行の移動をキャンセル
        std::list< Gtk::TreeModel::iterator >::iterator it_tmp = it_src;
        ++it_tmp;
        for( int i2 = 1 ; it_tmp != list_it.end(); ++i2, ++it_tmp ){

            Gtk::TreeModel::Path path_tmp = treestore->get_path( *it_tmp );
            if( path_tmp.to_string().find( path_src.to_string() ) != Glib::ustring::npos ){
                vec_cancel[ i + i2 ] = true;
            }
        }
    }

    // 移動開始

    std::list< Gtk::TreeModel::Row > list_destrow;

    Gtk::TreeModel::iterator it_dest = treestore->get_iter( path );
    Gtk::TreeModel::iterator it_dest_bkup = it_dest;
    bool after_bkup = after;
    bool subdir = after;
    it_src = list_it.begin();
    for( int i = 0 ; it_src != list_it.end(); ++i, ++it_src ){

        if( vec_cancel[ i ] ) continue;

        // コピーして削除
        if( copy_row( ( *it_src ), it_dest, subdir, after ) ) treestore->erase( ( *it_src ) );
        subdir = false;
        after = true;
        list_destrow.push_back( *it_dest );
    }

    // 移動先がディレクトリなら開く
    if( is_dir( it_dest_bkup ) && after_bkup ) expand_row( treestore->get_path( *it_dest_bkup ), false );

    // 範囲選択
    get_selection()->unselect_all();
    std::list< Gtk::TreeModel::Row >::iterator it_destrow = list_destrow.begin();
    for( ; it_destrow != list_destrow.end(); ++it_destrow ){

        Gtk::TreeModel::Row row_tmp = ( *it_destrow );
        get_selection()->select( row_tmp );

        if( row_tmp[ m_columns.m_type ] == TYPE_DIR ){
            expand_row( treestore->get_path( row_tmp ), false );
            select_all_dir( treestore->get_path( row_tmp ) );
        }
    }

    m_updated = true;
}


//
// 行追加
//
// after = false ならpath_dest の前に追加する( デフォルト after = true )
// path_dest がNULLなら一番最後に作る
// path_dest がディレクトリであり、かつ subdir = true なら path_dest の下に追加。
// path_dest がディレクトリでない、または subdir = falseなら path_dest の後に追加
// 戻り値は追加した行のpath
//
const Gtk::TreeModel::Path EditTreeView::append_row( const std::string& url, const std::string& name, const Glib::ustring data, const int type,
                                                   const Gtk::TreeModel::Path path_dest, const bool subdir, const bool after  )
{
    Glib::RefPtr< Gtk::TreeStore > treestore = Glib::RefPtr< Gtk::TreeStore >::cast_dynamic( get_model() );
    if( ! treestore ) return Gtk::TreeModel::Path();

#ifdef _DEBUG
    std::cout << "EditTreeView::append_row " << url << " " << name << std::endl;
#endif    

    Gtk::TreeModel::Row row_new;

    // 一番下に追加
    if( ! get_row( path_dest ) ) row_new = *( treestore->append() );
    else{

        Gtk::TreeModel::Row row_dest = get_row( path_dest );
        if( row_dest )
        {
            // path_destがディレクトリなら下に追加してディレクトリを開く
            if( subdir && after && row_dest[ m_columns.m_type ] == TYPE_DIR ){
                row_new = *( treestore->prepend( row_dest.children() ) );
                expand_row( path_dest, false );
            }

            // destの下に追加
            else if( after ) row_new = *( treestore->insert_after( row_dest ) );

            // destの前に追加
            else row_new = *( treestore->insert( row_dest ) );
        }
    }

    m_columns.setup_row( row_new, url, name, data, type );

    m_updated = true;

    return treestore->get_path( row_new );
}


//
// 選択した行をまとめて削除
//
void EditTreeView::delete_selected_rows()
{
    Glib::RefPtr< Gtk::TreeStore > treestore = Glib::RefPtr< Gtk::TreeStore >::cast_dynamic( get_model() );
    if( ! treestore ) return;

    std::list< Gtk::TreeModel::iterator > list_it = get_selected_iterators();
    if( ! list_it.size() ) return;

    // ディレクトリが含まれていないか無いか確認
    std::list< Gtk::TreeModel::iterator >::iterator it = list_it.begin();
    for( ; it != list_it.end(); ++it ){

        if( is_dir( (*it ) ) ){
            SKELETON::MsgDiag mdiag( NULL, "ディレクトリを削除するとディレクトリ内の行も全て削除されます。削除しますか？",
                                      false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            if( mdiag.run() != Gtk::RESPONSE_YES ) return;
            break;
        }
    }

    // カーソルを一番最後の行の次の行に移動する
    it = list_it.end();
    --it;

    // ディレクトリを閉じないとそのディレクトリの先頭の列が next になり、ディレクトリが削除されるため
    // キーボードフォーカスが外れる
    if( is_dir( *it ) ) collapse_row( treestore->get_path( *it ) ); 
    Gtk::TreePath next = next_path( treestore->get_path( *it ), true );

    // もしnextが存在しなかったら全ての行を削除してから一番下に移動
    const bool gotobottom = ( ! get_row( next ) );
    if( ! gotobottom ) set_cursor( next );

#ifdef _DEBUG
    std::cout << "EditTreeView::delete_selected_rows : ";
    std::cout << treestore->get_path( *it ).to_string() << " -> " << next.to_string() << std::endl;
#endif

    // まとめて削除
    // ディレクトリ内の行を同時に選択している場合があるので後から消す
    it = list_it.end();
    while( it != list_it.begin() ) treestore->erase( ( *(--it) ) );

    m_updated = true;

    if( gotobottom ) goto_bottom();
}
