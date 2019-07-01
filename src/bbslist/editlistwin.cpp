// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "editlistwin.h"
#include "selectlistview.h"
#include "toolbar.h"
#include "bbslistadmin.h"

#include "skeleton/compentry.h"
#include "skeleton/undobuffer.h"

#include "jdlib/miscutil.h"

#include "viewfactory.h"

using namespace BBSLIST;

enum
{
    EDITWIN_WIDTH = 800,
    EDITWIN_HEIGHT = 500
};


EditListWin::EditListWin( const std::string& url, Glib::RefPtr< Gtk::TreeStore >& treestore )
    : Gtk::Window( Gtk::WINDOW_TOPLEVEL ),
      m_label( "マウスの中ボタンドラッグで行の複数選択が可能です。" )
{
    // ツールバー
    m_toolbar = Gtk::manage( new EditListToolBar() );
    m_toolbar->open_buttonbar();
    m_vbox.pack_start( *m_toolbar, Gtk::PACK_SHRINK );
    m_toolbar->show_toolbar();

    // Adminクラスが無いのでツールバーのボタン等のシグナルを直接つなぐ
    m_toolbar->get_button_close()->signal_clicked().connect( sigc::mem_fun( this, &EditListWin::slot_close ) );

    m_toolbar->get_entry_search()->signal_changed().connect( sigc::mem_fun( *this, &EditListWin::slot_changed_search ) );
    m_toolbar->get_entry_search()->signal_activate().connect( sigc::mem_fun( *this, &EditListWin::slot_active_search ) );
    m_toolbar->get_entry_search()->signal_operate().connect( sigc::mem_fun( *this, &EditListWin::slot_operate_search ) );
    m_toolbar->get_button_up_search()->signal_clicked().connect( sigc::mem_fun( this, &EditListWin::slot_up_search ) );
    m_toolbar->get_button_down_search()->signal_clicked().connect( sigc::mem_fun( this, &EditListWin::slot_down_search ) );
    m_toolbar->get_button_undo()->signal_clicked().connect( sigc::mem_fun( this, &EditListWin::slot_undo ) );
    m_toolbar->get_button_redo()->signal_clicked().connect( sigc::mem_fun( this, &EditListWin::slot_redo ) );

    // ラベル
    m_vbox.pack_start( m_label, Gtk::PACK_SHRINK );

    // ビュー
    m_selectview = dynamic_cast< SelectListView* > ( Gtk::manage( CORE::ViewFactory( CORE::VIEW_SELECTLIST, url ) ) );
    if( m_selectview ){

        m_selectview->set_parent_win( this );
        m_selectview->copy_treestore( treestore );
        m_selectview->sig_close_dialog().connect( sigc::mem_fun(*this, &EditListWin::hide ) );
        m_selectview->sig_focus_entry_search().connect( sigc::mem_fun(*this, &EditListWin::slot_focus_entry_search ) );

        m_vbox.pack_start( *m_selectview );
        m_selectview->focus_view();
    }

    // UNDOバッファの監視
    BBSLIST::get_undo_buffer_favorite()->sig_undo().connect( sigc::mem_fun(*this, &EditListWin::slot_undo_buffer_changed ) );
    BBSLIST::get_undo_buffer_favorite()->sig_redo().connect( sigc::mem_fun(*this, &EditListWin::slot_undo_buffer_changed ) );
    BBSLIST::get_undo_buffer_favorite()->sig_commit().connect( sigc::mem_fun(*this, &EditListWin::slot_undo_buffer_changed ) );

    add( m_vbox );
    set_title( "お気に入りの編集" );
    resize( EDITWIN_WIDTH, EDITWIN_HEIGHT );

    show_all_children();
}


EditListWin::~EditListWin() noexcept = default;


void EditListWin::clock_in()
{
    if( m_selectview ) m_selectview->clock_in();
}

void EditListWin::append_item()
{
    if( m_selectview ) m_selectview->append_item();
}

//
// 閉じる
//
void EditListWin::slot_close()
{
#ifdef _DEBUG
    std::cout << "EditListWin::slot_close\n";
#endif

    hide();
}


//
// 検索関係
//
void EditListWin::slot_focus_entry_search()
{
#ifdef _DEBUG
    std::cout << "EditListWin::slot_focus_search\n";
#endif

    if( m_toolbar ) m_toolbar->focus_entry_search();
}


void EditListWin::slot_changed_search()
{
#ifdef _DEBUG
    std::cout << "EditListWin::slot_changed_search\n";
#endif

    if( m_selectview && m_toolbar ) m_selectview->set_search_query( m_toolbar->get_search_text() );
}


void EditListWin::slot_active_search()
{
#ifdef _DEBUG
    std::cout << "EditListWin::slot_active_search\n";
#endif

    if( m_selectview ) m_selectview->exec_search();
}


void EditListWin::slot_operate_search( const int controlid )
{
#ifdef _DEBUG
    std::cout << "EditListWin::slot_operate_search id = " << controlid << std::endl;
#endif

    if( m_selectview ) m_selectview->operate_search( MISC::itostr( controlid ) );
}


void EditListWin::slot_up_search()
{
#ifdef _DEBUG
    std::cout << "EditListWin::slot_up_search\n";
#endif

    if( m_selectview ) m_selectview->up_search();
}


void EditListWin::slot_down_search()
{
#ifdef _DEBUG
    std::cout << "EditListWin::slot_down_search\n";
#endif

    if( m_selectview ) m_selectview->down_search();
}


void EditListWin::slot_undo()
{
#ifdef _DEBUG
    std::cout << "EditListWin::slot_undo\n";
#endif

    if( m_selectview ) m_selectview->undo();
}


void EditListWin::slot_redo()
{
#ifdef _DEBUG
    std::cout << "EditListWin::slot_redo\n";
#endif

    if( m_selectview ) m_selectview->redo();
}


//
// UNDOバッファを監視してボタン状態を更新する
//
void EditListWin::slot_undo_buffer_changed()
{
#ifdef _DEBUG
    std::cout << "EditListWin::slot_undo_buffer_changed\n";
#endif

    m_toolbar->get_button_undo()->set_sensitive( BBSLIST::get_undo_buffer_favorite()->get_enable_undo() );
    m_toolbar->get_button_redo()->set_sensitive( BBSLIST::get_undo_buffer_favorite()->get_enable_redo() );
}
