// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "control.h"
#include "controlid.h"
#include "controlutil.h"
#include "command.h"
#include "global.h"

#include "config/globalconf.h"
#include "config/keyconfig.h"
#include "config/mouseconfig.h"
#include "config/buttonconfig.h"


using namespace CONTROL;


// ホイールマウスジェスチャ用( 全てのビューで共通 )
bool mg_wheel_done;


Control::Control()
{
    add_mode( CONTROL::MODE_COMMON );

    MG_reset();
    MG_wheel_reset();
}



void Control::add_mode( int mode )
{
    m_mode.push_back( mode );;
}




///////////////////////////////////////
//
// キー操作


// 戻り値はコントロールID
int Control::key_press( GdkEventKey* event )
{
    guint key = event->keyval;
    bool ctrl = ( event->state ) & GDK_CONTROL_MASK;
    bool shift = ( event->state ) & GDK_SHIFT_MASK;
    bool alt = ( event->state ) & GDK_MOD1_MASK;

#ifdef _DEBUG
    std::cout << "Control::key_press key = " << std::hex << key;
    if( ctrl ) std::cout << " ctrl";
    if( shift ) std::cout << " shift";
    if( alt ) std::cout << " alt";
    std::cout << "\n";
#endif    

    int control = CONTROL::None;
    std::vector< int >::iterator it = m_mode.begin();
    for( ; it != m_mode.end(); ++it ){
        control = CONFIG::get_keyconfig()->get_id( *it, key, ctrl, shift, alt, false );
        if( control != CONTROL::None ) break;
    }

    return control;
}



///////////////////////////////////////
//
// マウスのボタン操作


// 戻り値はコントロールID
int Control::button_press( GdkEventButton* event )
{
    guint button = event->button;
    bool ctrl = ( event->state ) & GDK_CONTROL_MASK;
    bool shift = ( event->state ) & GDK_SHIFT_MASK;
    bool alt = ( event->state ) & GDK_MOD1_MASK;

    int control = CONTROL::None;
    std::vector< int >::iterator it = m_mode.begin();
    for( ; it != m_mode.end(); ++it ){
        control = CONFIG::get_buttonconfig()->get_id( *it, button, ctrl, shift, alt, false );
        if( control != CONTROL::None ) break;
    }

    return control;
}

// eventがidに割り当てられていたらtrue
bool Control::button_alloted( GdkEventButton* event, int id )
{
    guint button = event->button;
    bool ctrl = ( event->state ) & GDK_CONTROL_MASK;
    bool shift = ( event->state ) & GDK_SHIFT_MASK;
    bool alt = ( event->state ) & GDK_MOD1_MASK;
    bool dblclick = ( event->type == GDK_2BUTTON_PRESS );

    return CONFIG::get_buttonconfig()->alloted( id, button, ctrl, shift, alt, dblclick );
}


// ID からevent取得
bool Control::get_eventbutton( int id, GdkEventButton& event )
{
    guint button;
    bool ctrl;
    bool shift;
    bool alt;
    bool dblclick;

    if( CONFIG::get_buttonconfig()->get_motion( id, button, ctrl, shift, alt, dblclick ) ){
        if( dblclick ) event.type = GDK_2BUTTON_PRESS;
        event.button = button;
        event.state = ( GDK_CONTROL_MASK & ctrl ) | ( GDK_SHIFT_MASK & shift ) | ( GDK_MOD1_MASK & alt );
        return true;
    }

    return false;
}




///////////////////////////////////////
//
// マウスジェスチャ


void Control::MG_reset()
{
    m_mg = false;
    m_mg_x = 0;
    m_mg_y = 0;
    m_mg_value = 0;
    m_mg_lng = 0;
    m_mg_direction = std::string();
}



bool Control::MG_start( GdkEventButton* event )
{
    MG_reset();

    if( ! button_alloted( event, CONTROL::GestureButton ) ) return false;

    m_mg = true;
    m_mg_x = ( int ) event->x;
    m_mg_y = ( int ) event->y;

#ifdef _DEBUG
    std::cout << "Control::MG_start\n";
#endif

    return true;
}



bool Control::MG_motion( GdkEventMotion* event )
{
    if( ! m_mg ) return false;
    if( m_mg_lng >= MAX_MG_LNG ) return false;

    if( m_mg_direction.empty() ){
        m_mg_direction = "■";
        CORE::core_set_command( "set_mginfo", "", "" );
    }

    int x = ( int ) event->x;
    int y = ( int ) event->y;

    int dx = x - m_mg_x;
    int dy = y - m_mg_y;

    int direction = 0;
    std::string str_direction;
    int radius = CONFIG::get_mouse_radius();

    if( dx < 0 && -dx > radius ){
        direction = 4;
        str_direction = "←";
    }
    else if( dx > 0 && dx > radius ){
        direction = 6;
        str_direction = "→";
    }
    else if( dy < 0 && -dy > radius ){
        direction = 8;
        str_direction = "↑";
    }
    else if( dy > 0 && dy > radius ){
        direction = 2;
        str_direction = "↓";
    }

#ifdef _DEBUG
    std::cout << " x = " << x << " y = " << y 
              << " mg_x = " << m_mg_x << " mg_y = " << m_mg_y 
              << " dx = " << dx << " dy = " << dy;
#endif

    if( direction ){

        m_mg_x = x;
        m_mg_y = y;

        // 方向更新
        if( m_mg_value % 10 != direction ){
            
            m_mg_value = m_mg_value * 10 + direction;
            ++m_mg_lng;
            m_mg_direction += str_direction;

            int control = CONTROL::None;
            std::vector< int >::iterator it = m_mode.begin();
            for( ; it != m_mode.end(); ++it ){
                control = CONFIG::get_mouseconfig()->get_id( *it, m_mg_value, false, false, false, false );
                if( control != CONTROL::None ) break;
            }

            CORE::core_set_command( "set_mginfo", "", m_mg_direction + CONTROL::get_label( control ) );
        }
    }

#ifdef _DEBUG
    std::cout << " dir = " << direction << " val = " << m_mg_value << std::endl;
#endif

    return true;
}



// 戻り値はコントロールID
int Control::MG_end( GdkEventButton* event )
{
    if( ! m_mg ) return None;

#ifdef _DEBUG
    std::cout << "Control::MG_end val = " << m_mg_value << std::endl;
#endif

    int control = CONTROL::None;
    std::vector< int >::iterator it = m_mode.begin();
    for( ; it != m_mode.end(); ++it ){
        control = CONFIG::get_mouseconfig()->get_id( *it, m_mg_value, false, false, false, false );
        if( control != CONTROL::None ) break;
    }

    std::string str_command = CONTROL::get_label( control );

    if( m_mg_lng ){

        if( control == CONTROL::None ){
            str_command = "Cancel";
            control = CONTROL::CancelMG;
        }

        m_mg_direction += " " + str_command;
        CORE::core_set_command( "set_mginfo", "", m_mg_direction );
    }

    MG_reset();

    return control;
}


///////////////////////////////////////
//
// ホイールマウスジェスチャ


void Control::MG_wheel_reset()
{
    mg_wheel_done = false;
}


// ホイールマウスジェスチャ開始
bool Control::MG_wheel_start( GdkEventButton* event )
{
    MG_wheel_reset();

    if( ! button_alloted( event, CONTROL::GestureButton ) ) return false;

#ifdef _DEBUG
    std::cout << "Control::MG_wheel_start\n";
#endif

    return true;
}


// ホイールマウスジェスチャ。 戻り値はコントロールID
int Control::MG_wheel_scroll( GdkEventScroll* event )
{
    int control = CONTROL::None;

    guint direction = event->direction;

    // 押しているボタンは event から取れないので
    // get_pointer()から取る
    int x, y;
    Gdk::ModifierType mask;
    Gdk::Display::get_default()->get_pointer( x, y, mask );

    int button = 0;
    GdkEventButton ev;
    get_eventbutton( CONTROL::GestureButton, ev );
    switch( ev.button ){
        case 1: button = Gdk::BUTTON1_MASK; break;
        case 2: button = Gdk::BUTTON2_MASK; break;
        case 3: button = Gdk::BUTTON3_MASK; break;
    }

    bool ctrl = false;
    bool shift = false;
    bool alt = false;

    if( direction == GDK_SCROLL_LEFT ){
        button = 6;

        std::vector< int >::iterator it = m_mode.begin();
        for( ; it != m_mode.end(); ++it ){
            control = CONFIG::get_buttonconfig()->get_id( *it, button, ctrl, shift, alt, false );
            if( control != CONTROL::None ) break;
        }
    }

    else if( direction == GDK_SCROLL_RIGHT ){
        button = 7;

        std::vector< int >::iterator it = m_mode.begin();
        for( ; it != m_mode.end(); ++it ){
            control = CONFIG::get_buttonconfig()->get_id( *it, button, ctrl, shift, alt, false );
            if( control != CONTROL::None ) break;
        }
    }

    else if( ( mask & button ) && direction == GDK_SCROLL_UP ) control = CONTROL::TabLeft;

    else if( ( mask & button ) && direction == GDK_SCROLL_DOWN ) control = CONTROL::TabRight;

#ifdef _DEBUG
    std::cout << "Control::MG_wheel_scroll control = " << control << std::endl;
#endif

    if( control != CONTROL::None ){
        CORE::core_set_command( "set_mginfo", "", CONTROL::get_label( control ) );
        mg_wheel_done = true;
    }

    return control;
}


// ホイールマウスジェスチャ終了
// もしジェスチャが実行されたら true が戻る
bool Control::MG_wheel_end( GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "Control::MG_wheel_end\n";
#endif

    bool ret = mg_wheel_done;
    MG_wheel_reset();

    return ret;
}
