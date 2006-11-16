// ライセンス: 最新のGPL

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


// ホイールマウスジェスチャが開始されているか
// タブが切り替わる場合があるので全てのviewで共通の変数とする
bool mg_wheel = false;;
bool mg_wheel_done = false;

Control::Control()
    : m_mode( CONTROL::MODE_COMMON )
{
    MG_reset();
    mg_wheel = false;
    mg_wheel_done = false;
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

    return CONFIG::get_keyconfig()->get_id( m_mode, key, ctrl, shift, alt, false );
}



///////////////////////////////////////
//
// マウスのボタン操作


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
    if( ! button_alloted( event, CONTROL::GestureButton ) ) return false;

    MG_reset();
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

            int control = CONFIG::get_mouseconfig()->get_id( m_mode, m_mg_value, false, false, false, false );
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

    int control = CONFIG::get_mouseconfig()->get_id( m_mode, m_mg_value, false, false, false, false );
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


// ホイールマウスジェスチャ開始
bool Control::MG_wheel_start( GdkEventButton* event )
{
    if( ! button_alloted( event, CONTROL::GestureButton ) ) return false;

#ifdef _DEBUG
    std::cout << "Control::MG_wheel_start\n";
#endif

    mg_wheel = true;
    mg_wheel_done = false;
    return true;
}


// ホイールマウスジェスチャ。 戻り値はコントロールID
int Control::MG_wheel_scroll( GdkEventScroll* event )
{
    if( ! mg_wheel ) return CONTROL::None;

    int control = CONTROL::None;
    if( event->direction == GDK_SCROLL_UP ) control = CONTROL::TabLeft;
    if( event->direction == GDK_SCROLL_DOWN ) control = CONTROL::TabRight;

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
    if( ! mg_wheel ) return false;

#ifdef _DEBUG
    std::cout << "Control::MG_wheel_end\n";
#endif

    bool ret = mg_wheel_done;

    mg_wheel = false;
    mg_wheel_done = false;

    return ret;
}
