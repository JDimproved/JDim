// ライセンス: GPL2

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

                std::string str_label = CONTROL::get_label_with_mnemonic( id );
                std::string str_motion = CONTROL::get_motion( id );

                ( *it_item ).remove();
                Gtk::Label *label = Gtk::manage( new Gtk::Label( str_label + ( str_motion.empty() ? "" : "  " ), true ) );
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



// controllabel.h に登録されている文字列の末尾にショートカットを付けた文字列を取得する
const std::string CONTROL::get_label_with_mnemonic( int id )
{
    unsigned int pos;

    std::string label = CONTROL::get_label ( id );
    pos = label.find( "...", 0);
    if ( pos != std::string::npos )
    {
        switch ( id )
        {
            case 19:    //名前を付けて保存...
                label.erase( pos, 3 );
                label += "(_S)...";
                break;

            case 26:    //プロパティ...
                label.erase( pos, 3 );
                label += "(_P)...";
                break;
        }
    }
    else
    {
        switch ( id )
        {
            case CONTROL::PreBookMark:  //前のブックマークへ移動
                label += "(_P)";
                break;

            case CONTROL::NextBookMark: //次のブックマークへ移動
                label += "(_X)";
                break;

            case CONTROL::Home: //先頭へ移動
                label += "(_H)";
                break;

            case CONTROL::End:  //最後へ移動
                label += "(_E)";
                break;

            case CONTROL::Quit:    //閉じる
                label += "(_C)";
                break;

            case CONTROL::Delete:    //削除
                label += "(_D)";
                break;

            case CONTROL::Reload:    //再読み込み
                label += "(_R)";
                break;

            case CONTROL::StopLoading:    //読み込み中止
                label += "(_T)";
                break;

            case CONTROL::Copy: //コピー
                label += "(_C)";
                break;

            case CONTROL::AppendFavorite:    //お気に入りに追加
                label += "(_A)";
                break;

            case CONTROL::Search:    //検索
                label += "(_S)";
                break;

            case CONTROL::SearchNext:    //次検索
                label += "(_N)";
                break;

            case CONTROL::SearchPrev:    //前検索
                label += "(_P)";
                break;

            case CONTROL::GotoNew:  //新着へ移動
                label += "(_N)";
                break;

            case CONTROL::CancelMosaic: //モザイク解除
                label += "(_M)";
                break;

            case CONTROL::ZoomFitImage: //画面に画像サイズを合わせる
                label += "(_A)";
                break;

            case CONTROL::ZoomInImage:  //ズームイン
                label += "(_I)";
                break;

            case CONTROL::ZoomOutImage: //ズームアウト
                label += "(_Z)";
                break;

            case CONTROL::OrgSizeImage: //元の画像サイズ
                label += "(_N)";
                break;
        }
    }

    return label;
}
