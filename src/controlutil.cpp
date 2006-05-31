// ライセンス: 最新のGPL

//#define _DEBUG
#include "jddebug.h"

#include "controlutil.h"
#include "controlid.h"
#include "controllabel.h"

#include "config/keyconfig.h"
#include "config/mouseconfig.h"


// メニューにショートカットキーやマウスジェスチャを表示
void CONTROL::set_menu_motion( Gtk::Menu* menu )
{
    if( !menu ) return;

    Gtk::Menu_Helpers::MenuList& items = menu->items();
    Gtk::Menu_Helpers::MenuList::iterator it_item = items.begin();
    for( ; it_item != items.end(); ++it_item ){

        // menuitemの中の名前を読み込んで ID を取得し、CONTROL::Noneでなかったら
        // ラベルを置き換える
        Gtk::Label* label = dynamic_cast< Gtk::Label* >( (*it_item).get_child() );
        if( label ){
#ifdef _DEBUG
            std::cout << label->get_text() << std::endl;
#endif
            int id = CONTROL::get_id( label->get_text() );
            if( id != CONTROL::None ){

                std::string str_label = CONTROL::get_label( id );
                std::string str_motion = CONTROL::get_motion( id );

                ( *it_item ).remove();
                Gtk::Label *label = Gtk::manage( new Gtk::Label( str_label + ( str_motion.empty() ? "" : "  " ) ) );
                Gtk::Label *label_motion = Gtk::manage( new Gtk::Label( str_motion ) );
                Gtk::HBox *box = Gtk::manage( new Gtk::HBox() );

                box->pack_start( *label, Gtk::PACK_SHRINK );
                box->pack_end( *label_motion, Gtk::PACK_SHRINK );
                (*it_item).add( *box );
                box->show_all();
            }
        }

        if( (*it_item).has_submenu() ) CONTROL::set_menu_motion( (*it_item).get_submenu() );
    }
}


// ラベルからID取得
int CONTROL::get_id( const std::string& label )
{
    for( int id = CONTROL::COMMONMOTION; id < CONTROL::CONTROL_END; ++id ){
        if( label == CONTROL::control_label[ id ][0] ) return id;
    }

    return CONTROL::None;
}



// IDからラベル取得
const std::string CONTROL::get_label( int id )
{
    return CONTROL::control_label[ id ][ 1 ];
}



// IDから操作取得
const std::string CONTROL::get_motion( int id )
{
    std::string str_motion = CONFIG::get_keyconfig()->get_str_motion( id );
    std::string mouse_motion = CONFIG::get_mouseconfig()->get_str_motion( id );
    if( ! mouse_motion.empty() ){
        if( !str_motion.empty() ) str_motion += " , ";
        str_motion += "( " + mouse_motion + " )";
    }

    return str_motion;
}



// IDからラベルと操作の両方を取得
const std::string CONTROL::get_label_motion( int id )
{
    std::string motion = CONTROL::get_motion( id );
    return CONTROL::get_label( id ) + ( motion.empty() ? "" :  "  " ) + motion;
}
