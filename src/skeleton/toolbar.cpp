// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "admin.h"
#include "imgbutton.h"
#include "backforwardbutton.h"

#include "jdlib/miscutil.h"

#include "controlutil.h"
#include "controlid.h"

#include <list>

using namespace SKELETON;


ToolBar::ToolBar( Admin* admin )
    : m_admin( admin ),
      m_toolbar_shown( false ),
      m_button_close( NULL ),
      m_button_back( NULL ),
      m_button_forward( NULL )
{
    m_buttonbar.set_border_width( 1 );
    m_scrwin.add( m_buttonbar );
    m_scrwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );

    set_size_request( 8 );
}
        

void ToolBar::set_url( const std::string& url )
{
    m_url = url;

    if( m_button_back ) m_button_back->set_url( m_url );
    if( m_button_forward ) m_button_forward->set_url( m_url );
}



bool ToolBar::is_empty()
{
    return ( ! m_buttonbar.get_children().size() );
}


// ツールバーを表示
void ToolBar::show_toolbar()
{
    if( ! m_toolbar_shown ){
        pack_start( m_scrwin, Gtk::PACK_SHRINK );
        show_all_children();
        m_toolbar_shown = true;
    }
}


// ツールバーを隠す
void ToolBar::hide_toolbar()
{
    if( m_toolbar_shown ){
        remove( m_scrwin );
        show_all_children();
        m_toolbar_shown = false;
    }
}


// タブのロック
void ToolBar::lock()
{
    if( m_button_close ) m_button_close->set_sensitive( false );
}


// タブのアンロック
void ToolBar::unlock()
{
    if( m_button_close ) m_button_close->set_sensitive( true );
}


// 更新
void ToolBar::update()
{
    bool empty = is_empty();

    unpack_buttons();
    pack_buttons();

    // ツールバーの中身が空の場合は
    // もう一度unpackとpackを繰り返さないと表示されないようだ
    if( empty ){
        unpack_buttons();
        pack_buttons();
    }

    // 進む、戻るボタンのsensitive状態を更新する
    set_url( m_url );
}


// ボタンのアンパック
void ToolBar::unpack_buttons()
{
    std::list< Gtk::Widget* > lists = m_buttonbar.get_children();
    std::list< Gtk::Widget* >::iterator it = lists.begin();
    for( ; it != lists.end(); ++it ){
        m_buttonbar.remove( *(*it) );
        if( dynamic_cast< Gtk::VSeparator* >( *it ) ) delete *it;
    }
}


// 区切り追加
void ToolBar::pack_separator()
{
    Gtk::VSeparator *sep = Gtk::manage( new Gtk::VSeparator() ); // delete は unpack_buttons() で行う
    m_buttonbar.pack_start( *sep, Gtk::PACK_SHRINK );
    sep->show();
}



//
// 閉じるボタン
//
Gtk::Button* ToolBar::get_button_close()
{
    if( ! m_button_close ){
        m_button_close = Gtk::manage( new ImgButton( Gtk::Stock::CLOSE ) );
        set_tooltip( *m_button_close, CONTROL::get_label_motion( CONTROL::Quit ) );

        m_button_close->signal_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_close ) );
    }

    return m_button_close;
}


void ToolBar::slot_clicked_close()
{
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_close\n";
#endif

    m_admin->operate_view( "close_view", m_url );
}


//
// 戻るボタン
//
Gtk::Button* ToolBar::get_button_back()
{
    if( ! m_button_back ){
        m_button_back = Gtk::manage( new SKELETON::BackForwardButton( m_url, true ) );
        set_tooltip( *m_button_back, CONTROL::get_label_motion( CONTROL::PrevView ) );

        m_button_back->signal_button_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_back ) );
        m_button_back->signal_selected().connect( sigc::mem_fun(*this, &ToolBar::slot_selected_back ) );
    }

    return m_button_back;
}


void ToolBar::slot_clicked_back()
{
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_back : " << m_url << std::endl;
#endif

    m_admin->operate_view( "back_viewhistory", m_url, "1" );
}


void ToolBar::slot_selected_back( const int i )
{
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_selected_back : " << i << " url = " << m_url << std::endl;
#endif

    m_admin->operate_view( "back_viewhistory", m_url, MISC::itostr( i+1 ) );
}


//
// 進むボタン
//
Gtk::Button* ToolBar::get_button_forward()
{
    if( ! m_button_forward ){
        m_button_forward = Gtk::manage( new SKELETON::BackForwardButton( m_url, false ) );
        set_tooltip( *m_button_forward, CONTROL::get_label_motion( CONTROL::NextView ) );

        m_button_forward->signal_button_clicked().connect( sigc::mem_fun(*this, &ToolBar::slot_clicked_forward ) );
        m_button_forward->signal_selected().connect( sigc::mem_fun(*this, &ToolBar::slot_selected_forward ) );
    }

    return m_button_forward;
}


void ToolBar::slot_clicked_forward()
{
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_clicked_forward  : " << m_url << std::endl;
#endif

    m_admin->operate_view( "forward_viewhistory", m_url, "1" );
}


void ToolBar::slot_selected_forward( const int i )
{
    if( m_url.empty() || ! m_admin ) return;

#ifdef _DEBUG
    std::cout << "ToolBar::slot_selected_forward : " << i << " url = " << m_url << std::endl;
#endif

    m_admin->operate_view( "forward_viewhistory", m_url, MISC::itostr( i+1 ) );
}


