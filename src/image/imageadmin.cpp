// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageadmin.h"
#include "imagewin.h"

#include "dbimg/imginterface.h"

#include "skeleton/view.h"
#include "skeleton/msgdiag.h"
#include "skeleton/filediag.h"

#include "command.h"
#include "cache.h"
#include "viewfactory.h"
#include "global.h"
#include "session.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"

IMAGE::ImageAdmin *instance_imageadmin = NULL;

IMAGE::ImageAdmin* IMAGE::get_admin()
{
    if( ! instance_imageadmin ) instance_imageadmin = new IMAGE::ImageAdmin(  URL_IMAGEADMIN );
    assert( instance_imageadmin );

    return instance_imageadmin;
}

void IMAGE::delete_admin()
{
    if( instance_imageadmin ) delete instance_imageadmin;
    instance_imageadmin = NULL;
}


using namespace IMAGE;

ImageAdmin::ImageAdmin( const std::string& url )
    : SKELETON::Admin( url )
    , m_scroll( SCROLL_NO )
{
    m_scrwin.add( m_iconbox );
    m_scrwin.set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_NEVER );
    m_scrwin.set_size_request( ICON_SIZE ,  ICON_SIZE + 4);

    m_left.set_label( "<" );
    m_right.set_label( ">" );

    m_left.signal_pressed().connect( sigc::mem_fun( *this, &ImageAdmin::slot_press_left ) );
    m_right.signal_pressed().connect( sigc::mem_fun( *this, &ImageAdmin::slot_press_right ) );
    m_left.signal_released().connect( sigc::mem_fun( *this, &ImageAdmin::slot_release_left ) );
    m_right.signal_released().connect( sigc::mem_fun( *this, &ImageAdmin::slot_release_right ) );


    m_tab.pack_start( m_scrwin );
    m_tab.pack_end( m_right, Gtk::PACK_SHRINK );
    m_tab.pack_end( m_left, Gtk::PACK_SHRINK );
    m_tab.show_all_children();
}



ImageAdmin::~ImageAdmin()
{

#ifdef _DEBUG
    std::cout << "ImageAdmin::~ImageAdmin\n";
#endif 

    // 開いているURLを保存
    SESSION::set_image_URLs( get_URLs() );
    SESSION::set_image_page( get_current_page() );

    close_window();
}


//
// 起動中
//
const bool ImageAdmin::is_booting()
{
    ImageWin* win = dynamic_cast< ImageWin* >( get_jdwin() );

    if( win && win->is_booting() ) return true;

    return Admin::is_booting();
}



// 前回開いていたURLを復元
void ImageAdmin::restore()
{
    std::list< std::string > list_tmp;
    std::list< std::string >::iterator it_tmp;

    list_tmp = SESSION::image_URLs();
    it_tmp = list_tmp.begin();
    for( ; it_tmp != list_tmp.end(); ++it_tmp ){

        if( !(*it_tmp).empty() ){
            COMMAND_ARGS command_arg;
            command_arg.command = "open_view";
            command_arg.url = (*it_tmp);

            open_view( command_arg );
        }
    }

    SKELETON::View* view = get_nth_icon( SESSION::image_page() );
    if( view ){
        switch_img( view->get_url() );
        switch_admin();
    }
}


void ImageAdmin::switch_admin()
{
    if( ! has_focus() ) CORE::core_set_command( "switch_image" );
}


//
// ページが含まれていないか
//
bool ImageAdmin::empty()
{
    return ( m_list_view.size() == 0 );
}



//
// 含まれているタブの数
//
int ImageAdmin::get_tab_nums()
{
    return m_iconbox.children().size();
}



//
// 含まれているページのURLのリスト取得
//
std::list< std::string > ImageAdmin::get_URLs()
{
    std::list< std::string > urls;
    Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().begin();
    for(; it !=  m_iconbox.children().end(); ++it ){
        SKELETON::View* view = dynamic_cast< SKELETON::View* >( it->get_widget() );
        if( view ) urls.push_back( view->get_url() );
    }

    return urls;
}



//
// コアからのクロック入力
//
// 各viewにクロックを渡すだけ
//
void ImageAdmin::clock_in()
{
    // アイコンにクロックを送る
    Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().begin();
    for(; it !=  m_iconbox.children().end(); ++it ){
        SKELETON::View* view = dynamic_cast< SKELETON::View* >( it->get_widget() );
        if( view ) view->clock_in();
    }

    // 画像が表示されている場合viewにクロックを回す
    if( SESSION::is_img_shown() ){

        // アクティブなviewにだけクロックを送る
        SKELETON::View* view = get_current_view();
        if( view ) view->clock_in();
    }

    // タブのスクロール
    if( m_scroll != SCROLL_NO ){

        const int timing = 100; // msec

        ++m_counter_scroll;
        if( timing / TIMER_TIMEOUT <= m_counter_scroll ){
            scroll_tab( m_scroll );
            m_counter_scroll = 0;
        }
    }

    if( get_jdwin() ) get_jdwin()->clock_in();
}



//
// 現在表示されているページ番号
//
int ImageAdmin::get_current_page()
{
    int pos;
    SKELETON::View* view = get_current_view();
    if( !view ) return -1;
    get_icon( view->get_url(), pos );

    return pos;
}



//
// ローカルなコマンド
//
void ImageAdmin::command_local( const COMMAND_ARGS& command )
{
    ImageWin* win = dynamic_cast< ImageWin* >( get_jdwin() );

    // 切り替え
    if( command.command == "switch_image" ) switch_img( command.url );

    // 画像強制表示
    if( command.command == "show_image" ){
        clock_in();
        clock_in();
        clock_in();
    }

    // すべて保存
    else if( command.command  == "save_all" ) save_all();

    // 並び替え
    else if( command.command  == "reorder" ) reorder( command.arg1, command.arg2 );

    else if( command.command  == "close_other_views" ) close_other_views( command.url );

    else if( command.command  == "close_left_views" ) close_left_views( command.url );

    else if( command.command  == "close_right_views" ) close_right_views( command.url );

    else if( command.command  == "close_all_views" ) close_other_views( std::string() );

    // 画面のスクロール
    else if( command.command == "scroll_up" ){
        SKELETON::View* view = get_current_view();
        if( view ) view->scroll_up();
    }
    else if( command.command == "scroll_down" ){
        SKELETON::View* view = get_current_view();
        if( view ) view->scroll_down();
    }
    else if( command.command == "scroll_left" ){
        SKELETON::View* view = get_current_view();
        if( view ) view->scroll_left();
    }
    else if( command.command == "scroll_right" ){
        SKELETON::View* view = get_current_view();
        if( view ) view->scroll_right();
    }

    // window 開け閉じ可能/不可
    else if( command.command == "enable_fold_win" ){
        if( win ) win->set_enable_fold( true );
    }

    else if( command.command == "disable_fold_win" ){
        if( win ) win->set_enable_fold( false );
    }
}



//
// 画像を開く
//
void ImageAdmin::open_view( const COMMAND_ARGS& command )
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::open_view url = " << command.url << std::endl;
#endif

    // まだ表示されていない
    if( ! get_view( command.url ) ){

        // アイコン作成 & 表示
        SKELETON::View* icon = Gtk::manage( CORE::ViewFactory( CORE::VIEW_IMAGEICON, command.url ) );
        if( icon ){
            icon->set_size_request( ICON_SIZE ,  ICON_SIZE );
            icon->show_view();
            m_iconbox.pack_start( *icon, Gtk::PACK_SHRINK );
            m_iconbox.show_all_children();
        }

        // view作成
        SKELETON::View* view = Gtk::manage( CORE::ViewFactory( CORE::VIEW_IMAGEVIEW, command.url ) );
        if( view ){
            view->show_view();
            m_list_view.push_back( view );
        }
    }

    open_window();
    switch_img( command.url );
}



//
// タブの切替え
//
void ImageAdmin::tab_left()
{
    if( m_iconbox.children().size() == 1 ) return;

    SKELETON::View* view;
    std::string url_to;
    SKELETON::View* icon = get_current_icon();
    Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().begin();
    for(; it !=  m_iconbox.children().end(); ++it ){
        view = dynamic_cast< SKELETON::View* >( it->get_widget() );
        if( view ){
            if( view == icon ) break;
            url_to = view->get_url();
        }
    }

    // 一番最後へ戻る
    if( url_to.empty() ){
        it = m_iconbox.children().end();
        view = dynamic_cast< SKELETON::View* >( (--it)->get_widget() );
        if( view ) url_to = view->get_url();
    }

    if( !url_to.empty() ) switch_img( url_to );
    focus_current_view();
}


void ImageAdmin::tab_right()
{
    if( m_iconbox.children().size() == 1 ) return;

    std::string url_to;
    SKELETON::View* icon = get_current_icon();
    Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().begin();
    for(; it != m_iconbox.children().end(); ++it ){
        SKELETON::View* view = dynamic_cast< SKELETON::View* >( it->get_widget() );
        if( view == icon ){

            ++it;
            // 一番最初へ戻る
            if( it == m_iconbox.children().end() ) it = m_iconbox.children().begin();

            view = dynamic_cast< SKELETON::View* >( it->get_widget() );
            if( view ) url_to = view->get_url();
            break;
        }
    }

    if( !url_to.empty() ) switch_img( url_to );
    focus_current_view();
}



//
// タブの最初に移動
//
void ImageAdmin::tab_head()
{
    if( m_iconbox.children().size() == 1 ) return;

    std::string url_to;
    Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().begin();

    SKELETON::View* view = dynamic_cast< SKELETON::View* >( it->get_widget() );
    if( view ) url_to = view->get_url();

    if( !url_to.empty() ) switch_img( url_to );

    switch_admin();
}


//
// タブの最後に移動
//
void ImageAdmin::tab_tail()
{
    if( m_iconbox.children().size() == 1 ) return;

    std::string url_to;
    Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().end();
    it--;
    
    SKELETON::View* view = dynamic_cast< SKELETON::View* >( it->get_widget() );
    if( view ) url_to = view->get_url();

    if( !url_to.empty() ) switch_img( url_to );

    switch_admin();
}




//
// タブアイコンの並び替え
//
void ImageAdmin::reorder( const std::string& url_from, const std::string& url_to )
{
    SKELETON::View* view_from = get_icon( url_from );

    int pos;
    get_icon( url_to, pos );

    if( view_from && pos != -1 ){
        
#ifdef _DEBUG
        std::cout << "ImageAdmin::reorder " << url_from << "\n-> " << url_to << " pos = " << pos << std::endl;
#endif    

        m_iconbox.reorder_child( *view_from, pos );
    }
}





//
// 指定したビューを再描画
//
void ImageAdmin::redraw_view( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::redraw_view url = " << url << std::endl;
#endif

    SKELETON::View* view = get_view( url );
    if( view ) view->redraw_view();

    view = get_icon( url );
    if( view ) view->redraw_view();
}




//
// 現在のビューを再描画
//
void ImageAdmin::redraw_current_view()
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::redraw_current_view\n";
#endif

    SKELETON::View* view = get_current_view();
    if( view ) view->redraw_view();

    view = get_current_icon();
    if( view ) view->redraw_view();
}



//
// 閉じる
//
void ImageAdmin::close_view( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::close_view : url = " << url << std::endl;
#endif

    // 次に表示するviewのURL
    std::string url_next = std::string();

    SKELETON::View* icon = get_icon( url );
    SKELETON::View* view = get_view( url );

    if( ! icon && ! view ) return;

    // 現在表示中のviewを閉じた場合は次か前の画像に切り替える
    if( view && view == get_current_view() ){

        m_view.remove();

        SKELETON::View* view_prev = NULL;
        SKELETON::View* view_next = NULL;        
        Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().begin();
        for(; it !=  m_iconbox.children().end(); ++it ){
            SKELETON::View* view_tmp = dynamic_cast< SKELETON::View* >( it->get_widget() );
            if( view_tmp->get_url() == url ){
                if( ++it != m_iconbox.children().end() ) view_next = dynamic_cast< SKELETON::View* >( it->get_widget() );
                break;
            }
            view_prev = view_tmp;
        }

        if( view_next ) url_next = view_next->get_url();
        else if( view_prev ) url_next = view_prev->get_url();
    }

    if( icon ){
        m_iconbox.remove( *icon );
        delete icon;
    }

    if( view ){
        m_list_view.remove( view );
        delete view;
    }

    if( empty() ){

        close_window();
        CORE::core_set_command( "empty_page", get_url() );
    }
    else if( ! url_next.empty() ) switch_img( url_next );
}


//
// ウィンドウ開く
//
void ImageAdmin::open_window()
{
    ImageWin* win = dynamic_cast< ImageWin* >( get_jdwin() );

    if( ! SESSION::get_embedded_img() && ! win && ! empty() ){
        win = new IMAGE::ImageWin();
        set_jdwin( win );
        win->pack_remove_tab( false, m_tab );
        win->pack_remove_view( false, m_view );
        win->show_all();
    }
    else if( win && win->is_hide() ){
        win->show();
        win->focus_in();
    }
}


//
// ウィンドウ閉じる
//
void ImageAdmin::close_window()
{
    ImageWin* win = dynamic_cast< ImageWin* >( get_jdwin() );

    if( win ){
        win->pack_remove_tab( true, m_tab );
        win->pack_remove_view( true, m_view );

        delete_jdwin();
    }
}


//
// url 以外の画像を閉じる
//
void ImageAdmin::close_other_views( const std::string& url )
{
    Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().begin();
    for(; it !=  m_iconbox.children().end(); ++it ){
        SKELETON::View* view = dynamic_cast< SKELETON::View* >( it->get_widget() );
        if( view && view->get_url() != url ) set_command( "close_view", view->get_url() );
    }
}


//
// url の左側の画像を閉じる
//
void ImageAdmin::close_left_views( const std::string& url )
{
    Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().begin();
    for(; it !=  m_iconbox.children().end(); ++it ){
        SKELETON::View* view = dynamic_cast< SKELETON::View* >( it->get_widget() );
        if( view->get_url() == url ) return;
        if( view ) set_command( "close_view", view->get_url() );
    }
}


//
// url の右側の画像を閉じる
//
void ImageAdmin::close_right_views( const std::string& url )
{
    Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().begin();
    for(; it !=  m_iconbox.children().end(); ++it ){
        SKELETON::View* view = dynamic_cast< SKELETON::View* >( it->get_widget() );
        if( view->get_url() == url ) break;
    }
    ++it;
    for(; it !=  m_iconbox.children().end(); ++it ){
        SKELETON::View* view = dynamic_cast< SKELETON::View* >( it->get_widget() );
        if( view ) set_command( "close_view", view->get_url() );
    }
}


//
// 現在のviewをフォーカスする
//
// 他のクラスからは直接呼ばないで、set_command()経由で呼ぶこと
//
void ImageAdmin::focus_view( int)
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::focus_view\n";
#endif

    SKELETON::View* view_icon = get_current_icon();
    if( view_icon ) {

        if( get_jdwin() ) get_jdwin()->focus_in();

        focus_out_all();

        view_icon->focus_view();

        SKELETON::View* view = get_current_view();
        if( view ){
            set_url( view->get_url(), view->get_url() );
            set_title( view->get_url(), view->get_title() );
            set_status( view->get_url(), view->get_status() );
        }
    }
}

void ImageAdmin::focus_current_view()
{
    focus_view( 0 );
}


//
// 現在のviewをフォーカスアウトする
//
void ImageAdmin::focus_out()
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::focus_out\n";
#endif

    // 画像ビューが隠れないようにフォーカスアウトする前に transient 指定をしておく
    ImageWin* win = dynamic_cast< ImageWin* >( get_jdwin() );
    if( win ) win->set_transient( true );

    SKELETON::Admin::focus_out();
}



//
// 全アイコンのフォーカスをはずす
//
void ImageAdmin::focus_out_all()
{
    Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().begin();
    for(; it !=  m_iconbox.children().end(); ++it ){
        SKELETON::View* view = dynamic_cast< SKELETON::View* >( it->get_widget() );
        if( view ) view->focus_out();
    }
}




//
// 画像切り替え
//
void ImageAdmin::switch_img( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::switch_img url = " << url << std::endl;
#endif

    // 画像切り替え
    int page = 0;
    std::list< SKELETON::View* >::iterator it_view;
    for( it_view = m_list_view.begin(); it_view != m_list_view.end(); ++it_view ){

        SKELETON::View* view = ( *it_view );
        if( view->get_url() == url ){

            if( view != get_current_view() ){
#ifdef _DEBUG
                std::cout << "view was toggled.\n";
#endif
                m_view.remove();
                m_view.add( *view );
                m_view.show_all_children();
            }
            break;
        }
    }

    focus_out_all();

    // アイコン切り替え
    SKELETON::View* view_icon = get_icon( url, page );
    if( view_icon ) view_icon->set_command( "switch_icon" );

    // タブをスクロール
    Gtk::Adjustment* adjust = m_scrwin.get_hadjustment();
    if( page != -1 && adjust ){
        double pos = adjust->get_value();
        double upper =  m_list_view.size() * ICON_SIZE;
        double width = adjust->get_page_size();
        double pos_to = page * ICON_SIZE;

#ifdef _DEBUG
        std::cout << "pos = " << pos << std::endl;
        std::cout << "page = " << page << std::endl;
        std::cout << "pos_to = " << pos_to << std::endl;
        std::cout << "upper = " << upper << std::endl;
        std::cout << "width = " << width << std::endl;
#endif

        if( pos_to <= pos || pos_to >= pos + width ){

            if( pos_to + width >= upper ) pos_to = upper - width;
            adjust->set_value( pos_to );
        }
    }

    if( has_focus() ) focus_current_view();
}



//
// アイコン取得
//
// pos にアイコンの位置が入る(見付からないときは-1)
// 
SKELETON::View* ImageAdmin::get_icon( const std::string& url, int& pos )
{
    Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().begin();
    for( pos = 0; it !=  m_iconbox.children().end(); ++it, ++pos ){
        SKELETON::View* view = dynamic_cast< SKELETON::View* >( it->get_widget() );
        if( view && view->get_url() == url ) return view;
    }

    pos = -1;
    return NULL;
}

// 簡易版
SKELETON::View* ImageAdmin::get_icon( const std::string& url)
{
    int pos;
    return get_icon( url, pos );
}


//
// アイコン取得(番号で)
//
SKELETON::View* ImageAdmin::get_nth_icon( unsigned int n )
{
    if( n >= m_iconbox.children().size() ) return NULL;

    Gtk::Box_Helpers::BoxList::iterator it = m_iconbox.children().begin();
    for( unsigned int i = 0; i < n; ++i, ++it );

    return dynamic_cast< SKELETON::View* >( it->get_widget() );
}



//
// カレントアイコン取得
//
SKELETON::View* ImageAdmin::get_current_icon()
{
    SKELETON::View* view = get_current_view();
    if( !view ) return NULL;
    return get_icon( view->get_url() );
}




//
// view 取得
//
SKELETON::View* ImageAdmin::get_view( const std::string& url )
{
    std::list< SKELETON::View* >::iterator it_view;
    for( it_view = m_list_view.begin(); it_view != m_list_view.end(); ++it_view ){
        if( ( *it_view )->get_url() == url ) return ( *it_view );
    }

    return NULL;
}



//
// カレントview 取得
//
SKELETON::View* ImageAdmin::get_current_view()
{
    return dynamic_cast< SKELETON::View* >( m_view.get_child() );
}



//
// スクロール
//
void ImageAdmin::scroll_tab( int scroll )
{
    if( scroll == SCROLL_NO ) return;

#ifdef _DEBUG
    std::cout << "ImageAdmin::scroll_tab " << scroll << std::endl;
#endif

    Gtk::Adjustment* adjust = m_scrwin.get_hadjustment();
    if( adjust ){
        double pos = adjust->get_value();
        double upper = adjust->get_upper();
        double width = adjust->get_page_size();

#ifdef _DEBUG
        std::cout << "pos = " << pos << std::endl;
        std::cout << "upper = " << upper << std::endl;
        std::cout << "width = " << width << std::endl;
#endif

        if( upper == width ) return;

        if( scroll == SCROLL_LEFT ) pos -= ICON_SIZE;
        else pos += ICON_SIZE;

        if( pos <= 0 ) pos = 0;

        else if( pos + width >= upper ) pos = upper - width;

        // ICON_SIZEの倍数にする
        else pos = ICON_SIZE * ( ( (int)pos ) / ICON_SIZE );

        adjust->set_value( pos );
    }
}


//左押した
void ImageAdmin::slot_press_left()
{
    m_scroll = SCROLL_LEFT;
    m_counter_scroll = 0;
    scroll_tab( m_scroll );
}

//右押した
void ImageAdmin::slot_press_right()
{
    m_scroll = SCROLL_RIGHT;
    m_counter_scroll = 0;
    scroll_tab( m_scroll );
}


//左離した
void ImageAdmin::slot_release_left()
{
    m_scroll = SCROLL_NO;
}

// 右離した
void ImageAdmin::slot_release_right()
{
    m_scroll = SCROLL_NO;
}

//
// すべて保存
//
void ImageAdmin::save_all()
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::save_all\n";
#endif

    int overwrite = 0; // -1 なら全てNO、1ならすべてYES

    std::list< std::string > list_urls = get_URLs();

    // ディレクトリ選択
    SKELETON::FileDiag diag( get_win(), "save", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER );

    diag.set_current_folder( SESSION::dir_img_save() );
    
    if( diag.run() == Gtk::RESPONSE_ACCEPT ){

        diag.hide();

        std::string path_dir = diag.get_filename();
        if( path_dir.empty() ) return;
        if( path_dir.c_str()[ path_dir.length()-1 ] != '/' ) path_dir += "/";
        
#ifdef _DEBUG
        std::cout << "dir = " << path_dir << std::endl;
#endif

        if( CACHE::jdmkdir( path_dir ) ){

            SESSION::set_dir_img_save( path_dir );

            std::list< std::string >::iterator it = list_urls.begin();
            for( ; it != list_urls.end(); ++it ){

                std::string url = (*it);
                if( ! DBIMG::is_cached( url ) ) continue;

                std::string path_from = CACHE::path_img( url );
                std::string path_to = path_dir + MISC::get_filename( url );

#ifdef _DEBUG
                std::cout << "from = " << path_from  << std::endl;
                std::cout << "to   = " << path_to  << std::endl;
#endif

                // 既にファイルがある場合
                if( CACHE::file_exists( path_to ) == CACHE::EXIST_FILE ){

                    // すべて上書き
                    if( overwrite == 1 ) CACHE::jdcopy( path_from, path_to ); 
                    else if( overwrite != -1 ){
                    
                        switch_img( url );

                        for(;;){

                            SKELETON::MsgDiag mdiag( get_win(), "ファイルが存在します。ファイル名を変更しますか？", 
                                                     false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE );
                            mdiag.add_button( Gtk::Stock::NO, Gtk::RESPONSE_NO );
                            mdiag.add_button( Gtk::Stock::YES, Gtk::RESPONSE_YES );
                            mdiag.add_button( "上書き", Gtk::RESPONSE_YES + 100 );
                            mdiag.add_button( "すべていいえ", Gtk::RESPONSE_NO + 200 );
                            mdiag.add_button( "すべて上書き", Gtk::RESPONSE_YES + 200 );

                            int ret = mdiag.run();
                            mdiag.hide();

                            switch( ret ){

                                // すべて上書き
                                case Gtk::RESPONSE_YES + 200:
                                    overwrite = 1;
                                    // 上書き
                                case Gtk::RESPONSE_YES + 100:
                                    CACHE::jdcopy( path_from, path_to );
                                    break;

                                    // 名前変更
                                case Gtk::RESPONSE_YES:
                                    if( ! DBIMG::save( url, get_win(), path_to ) ) continue;
                                    break;

                                    //  すべていいえ
                                case Gtk::RESPONSE_NO + 200:
                                    overwrite = -1;
                                    break;

                                default:
                                    break;
                            }

                            break;
                        }

                    }

                }
                else CACHE::jdcopy( path_from, path_to );
            }

        }
        else MISC::ERRMSG( "can't create " + path_dir );
    }
}
