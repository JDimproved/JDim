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
#include "httpcode.h"

#include "history/historymanager.h"

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

    m_left.set_focus_on_click( false );
    m_right.set_focus_on_click( false );

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

    close_window();
}


void ImageAdmin::save_session()
{
    Admin::save_session();

    // 開いているURLを保存
    SESSION::set_image_URLs( get_URLs() );
    SESSION::set_image_locked( get_locked() );
    SESSION::set_image_page( get_current_page() );
}


// 前回開いていたURLを復元
void ImageAdmin::restore( const bool only_locked )
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::restore\n";
#endif

    int set_page_num = 0;

    std::list< std::string > list_url = SESSION::image_URLs();
    std::list< std::string >::iterator it_url= list_url.begin();

    std::list< bool > list_locked = SESSION::get_image_locked();
    std::list< bool >::iterator it_locked = list_locked.begin();

    for( int page = 0; it_url != list_url.end(); ++it_url, ++page ){

        // タブのロック状態
        bool lock = false;
        if( it_locked != list_locked.end() ){
            if( (*it_locked ) ) lock = true;
            ++it_locked;
        }

        // ロックされているものだけ表示
        if( only_locked && ! lock ) continue;

        if( page == SESSION::image_page() ) set_page_num = get_tab_nums();

        COMMAND_ARGS command_arg = url_to_openarg( *it_url, true, lock );
        if( ! command_arg.url.empty() ) open_view( command_arg );        
    }

    SKELETON::View* view = get_nth_icon( set_page_num );
    if( view ){
        switch_img( view->get_url() );
        switch_admin();
    }
}


COMMAND_ARGS ImageAdmin::url_to_openarg( const std::string& url, const bool tab, const bool lock )
{
    COMMAND_ARGS command_arg;
    command_arg.command = "open_view";
    command_arg.url = url;
    if( lock ) command_arg.arg3 = "lock";

    return command_arg;
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
    return static_cast< int >( m_iconbox.get_children().size() );
}



//
// 含まれているページのURLのリスト取得
//
const std::list< std::string > ImageAdmin::get_URLs()
{
    std::list< std::string > urls;
    m_iconbox.foreach( [&urls]( Gtk::Widget& w ) {
        auto view = dynamic_cast< SKELETON::View* >( &w );
        if( view ) {
            urls.push_back( view->get_url() );
        }
    } );
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
    m_iconbox.foreach( []( Gtk::Widget& w ) {
        auto view = dynamic_cast< SKELETON::View* >( &w );
        if( view ) {
            view->clock_in();
        }
    } );

    // 画像が表示されている場合viewにクロックを回す
    if( SESSION::is_shown_win_img() ){

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

    else if( command.command  == "close_error_views" ) close_error_views( command.arg1 );

    else if( command.command  == "close_noerror_views" ) close_noerror_views();

    else if( command.command  == "close_all_views" ) close_other_views( std::string() );

    // キャッシュに無いviewを削除
    else if( command.command  == "close_nocached_views" ) close_error_views( "nocached" );

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

    else if( command.command == "set_imgtab_operating" ){

        if( command.arg1 == "true" ) SESSION::set_tab_operating( get_url(), true );
        else{

            SESSION::set_tab_operating( get_url(), false );

            if( ! empty() ){
                update_status_of_all_views();
            }
        }
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
            if( command.arg3 == "lock" ) icon->lock();
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
    update_status_of_all_views();
    switch_img( command.url );
}



//
// タブの切替え
//
void ImageAdmin::tab_left( const bool updated )
{
    std::string url_to;

    // gtk2のGlib::ListHandleのイテレーターはデクリメント不可なので型変換する
    const std::vector< Gtk::Widget* > widgets = m_iconbox.get_children();
    if( widgets.size() <= 1 ) return;

    const SKELETON::View* const icon = get_current_icon();
    for( auto&& widget : widgets ) {
        auto view = dynamic_cast< SKELETON::View* >( widget );
        if( view ) {
            if( view == icon ) break;
            url_to = view->get_url();
        }
    }
    // 一番最後へ戻る
    if( url_to.empty() ) {
        auto view = dynamic_cast< SKELETON::View* >( widgets.back() );
        if( view ) {
            url_to = view->get_url();
        }
    }

    if( !url_to.empty() ) switch_img( url_to );
    focus_current_view();
}


void ImageAdmin::tab_right( const bool updated )
{
    std::string url_to;

    const auto widgets = m_iconbox.get_children();
    if( widgets.size() <= 1 ) return;

    auto it = std::find( widgets.begin(), widgets.end(), static_cast< Gtk::Widget* >( get_current_icon() ) );
    if( ++it == widgets.end() ) {
        it = widgets.begin();
    }
    auto view = dynamic_cast< SKELETON::View* >( *it );
    if( view ) {
        url_to = view->get_url();
    }

    if( !url_to.empty() ) switch_img( url_to );
    focus_current_view();
}



//
// タブの最初に移動
//
void ImageAdmin::tab_head()
{
    const auto widgets = m_iconbox.get_children();
    if( widgets.size() <= 1 ) return;

    auto view = dynamic_cast< SKELETON::View* >( *widgets.begin() );
    std::string url_to;

    if( view ) url_to = view->get_url();

    if( !url_to.empty() ) switch_img( url_to );

    switch_admin();
}


//
// タブの最後に移動
//
void ImageAdmin::tab_tail()
{
    const std::vector< Gtk::Widget* > widgets = m_iconbox.get_children();
    if( widgets.size() <= 1 ) return;

    auto view = dynamic_cast< SKELETON::View* >( widgets.back() );
    std::string url_to;

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

        update_status_of_all_views();
    }
}


//
// 全てのビューのステータス表示内容更新の予約
//
void ImageAdmin::update_status_of_all_views()
{
    if( SESSION::is_tab_operating( get_url() ) ) return;

#ifdef _DEBUG
    std::cout << "ImageAdmin::update_status_of_all_views\n";
#endif    

    std::list< SKELETON::View* >::iterator it_view;
    for( it_view = m_list_view.begin(); it_view != m_list_view.end(); ++it_view ){
        if( *it_view ) (*it_view)->set_command( "update_status" );
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
    if( DBIMG::is_cached( url ) && icon->is_locked() ) return;

    // 現在表示中のviewを閉じた場合は次か前の画像に切り替える
    if( view && view == get_current_view() ){

        m_view.remove();

        SKELETON::View* view_prev = nullptr;
        SKELETON::View* view_next = nullptr;
        const auto widgets = m_iconbox.get_children();
        for( auto it = widgets.begin(); it != widgets.end(); ++it ) {
            auto view_tmp = dynamic_cast< SKELETON::View* >( *it );
            if( view_tmp->get_url() == url ) {
                if( ++it != widgets.end() ) {
                    view_next = dynamic_cast< SKELETON::View* >( *it );
                }
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
    else if( ! url_next.empty() ){
        update_status_of_all_views();
        switch_img( url_next );
    }
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
        win->pack_remove_end( false, m_view );
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
        win->pack_remove_end( true, m_view );
        delete_jdwin();
    }
}


//
// url 以外の画像を閉じる
//
void ImageAdmin::close_other_views( const std::string& url )
{
    set_command( "set_imgtab_operating", "", "true" );

    m_iconbox.foreach( [this, &url]( Gtk::Widget& w ) {
        auto view = dynamic_cast< SKELETON::View* >( &w );
        if( view && view->get_url() != url ) {
            set_command( "close_view", view->get_url() );
        }
    } );

    set_command( "set_imgtab_operating", "", "false" );
}


//
// url の左側の画像を閉じる
//
void ImageAdmin::close_left_views( const std::string& url )
{
    set_command( "set_imgtab_operating", "", "true" );

    for( auto&& widget : m_iconbox.get_children() ) {
        auto view = dynamic_cast< SKELETON::View* >( widget );
        if( view->get_url() == url ) break;
        if( view ) {
            set_command( "close_view", view->get_url() );
        }
    }

    set_command( "set_imgtab_operating", "", "false" );
}


//
// url の右側の画像を閉じる
//
void ImageAdmin::close_right_views( const std::string& url )
{
    set_command( "set_imgtab_operating", "", "true" );

    const auto widgets = m_iconbox.get_children();
    auto it = widgets.begin();
    for( ; it != widgets.end(); ++it ) {
        auto view = dynamic_cast< SKELETON::View* >( *it );
        if( view->get_url() == url ) break;
    }
    for( ++it; it != widgets.end(); ++it ) {
        auto view = dynamic_cast< SKELETON::View* >( *it );
        if( view ) {
            set_command( "close_view", view->get_url() );
        }
    }

    set_command( "set_imgtab_operating", "", "false" );
}


//
// エラーになっている画像を閉じる
//
void ImageAdmin::close_error_views( const std::string mode )
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::close_error_views\n";
#endif

    set_command( "set_imgtab_operating", "", "true" );

    for( auto&& widget : m_iconbox.get_children() ) {
        auto view = dynamic_cast< SKELETON::View* >( widget );
        if( view ){

            const std::string url = view->get_url();

            if( DBIMG::is_cached ( url ) ) continue;
            if( mode != "all" && DBIMG::is_loading ( url ) ) continue;
            if( DBIMG::is_wait ( url ) ) continue;

            const int code = DBIMG::get_code( url );
            if( ( code == HTTP_FORBIDDEN || code == HTTP_NOT_FOUND )  // 404,403 は無条件で閉じる
                || ( mode == "notimeout" && code != HTTP_TIMEOUT && code != HTTP_TEMP_UNAV ) // timeout,503以外
                || ( mode == "nocached"  && code == HTTP_INIT ) // キャッシュに無い(削除済み)の画像
                || mode == "all"  // 読み込み中も含めて閉じる
                ){
                set_command( "close_view", url );
                if( mode == "all" ) DBIMG::stop_load( url );
            }
        }
    }

    set_command( "set_imgtab_operating", "", "false" );
}


//
// エラーでない画像を閉じる
//
void ImageAdmin::close_noerror_views()
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::close_noerror_views\n";
#endif

    set_command( "set_imgtab_operating", "", "true" );

    for( auto&& widget : m_iconbox.get_children() ) {
        auto view = dynamic_cast< SKELETON::View* >( widget );
        if( view ){

            const std::string url = view->get_url();

            if( ! DBIMG::is_cached ( url ) ) continue;

            const int code = DBIMG::get_code( url );
            if( code == HTTP_OK ) set_command( "close_view", url );
        }
    }

    set_command( "set_imgtab_operating", "", "false" );
}


void ImageAdmin::restore_lasttab()
{
    HISTORY::restore_history( URL_HISTCLOSEIMGVIEW );
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
        if( view ) update_status( view, false );
    }
}

void ImageAdmin::focus_current_view()
{
    focus_view( 0 );
}


//
// 全アイコンのフォーカスをはずす
//
void ImageAdmin::focus_out_all()
{
    m_iconbox.foreach( []( Gtk::Widget& w ) {
        auto view = dynamic_cast< SKELETON::View* >( &w );
        if( view ) {
            view->focus_out();
        }
    } );
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
    auto adjust = m_scrwin.get_hadjustment();
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
    pos = 0;
    for( auto&& widget : m_iconbox.get_children() ) {
        auto view = dynamic_cast< SKELETON::View* >( widget );
        if( view && view->get_url() == url ) return view;
        ++pos;
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
SKELETON::View* ImageAdmin::get_nth_icon( const unsigned int n )
{
    const std::vector< Gtk::Widget* > widgets = m_iconbox.get_children();
    if( n >= widgets.size() ) return nullptr;

    return dynamic_cast< SKELETON::View* >( widgets[ n ] );
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
    SKELETON::View* view = get_current_view();
    if( view && view->get_url() == url ) return view;

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

    auto adjust = m_scrwin.get_hadjustment();
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
// 画像ファイルのコピー
//
bool ImageAdmin::copy_file( const std::string& url, const std::string& path_from, const std::string& path_to )
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::copy_file url = " << url << std::endl
              << "from = " << path_from << std::endl
              << "to = " << path_to << std::endl;
#endif

    if( ! CACHE::jdcopy( path_from, path_to ) ){

        // ビューを切り替えてURLやステータス更新
        switch_img( url );
        SKELETON::View* view = get_current_view();
        if( view ) update_status( view, true );

        SKELETON::MsgDiag mdiag( get_win(), path_to + "\n\nの保存に失敗しました。\nハードディスクの容量やパーミッションなどを確認してください。\n\n画像保存をキャンセルしました。原因を解決してからもう一度保存を行ってください。" );
        mdiag.run();

        return false;
    }

    return true;
}


//
// すべて保存
//
void ImageAdmin::save_all()
{
#ifdef _DEBUG
    std::cout << "ImageAdmin::save_all\n";
#endif

    int overwrite = Gtk::RESPONSE_NO;
    bool use_name_in_cache = false;

    std::list< std::string > list_urls = get_URLs();

    // ディレクトリ選択
    SKELETON::FileDiag diag( get_win(), "保存先選択", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER );

    diag.set_current_folder( SESSION::dir_img_save() );
    
    if( diag.run() == Gtk::RESPONSE_ACCEPT ){

        diag.hide();

        std::string path_dir = MISC::recover_path( diag.get_filename() );
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
                if( ! DBIMG::is_cached( url ) || DBIMG::get_mosaic( url ) ) continue;

                std::string path_from = DBIMG::get_cache_path( url );
                std::string path_to = path_dir + MISC::get_filename( url );

#ifdef _DEBUG
                std::cout << "from = " << path_from  << std::endl;
                std::cout << "to   = " << path_to  << std::endl;
#endif

                // 既にファイルがある場合
                if( CACHE::file_exists( path_to ) == CACHE::EXIST_FILE ){

                    // すべて上書き
                    if( overwrite == SKELETON::OVERWRITE_YES_ALL ){
                        if( ! copy_file( url, path_from, path_to ) ) return;
                    }

                    // ファイル名として画像キャッシュでのファイル名を使用
                    else if( use_name_in_cache ){
                        if( ! copy_file( url, path_from, path_dir + CACHE::filename_img( url ) ) ) return;
                    }

                    // すべていいえ
                    else if( overwrite == SKELETON::OVERWRITE_NO_ALL ){}

                    else{

                        // ビューを切り替えてURLやステータス更新
                        switch_img( url );
                        SKELETON::View* view = get_current_view();
                        if( view ) update_status( view, true );

                        for(;;){

                            SKELETON::MsgOverwriteDiag mdiag( get_win() );

                            overwrite = mdiag.run();
                            mdiag.hide();

                            switch( overwrite ){

                                // すべて上書き
                                case SKELETON::OVERWRITE_YES_ALL:

                                    // 上書き
                                case SKELETON::OVERWRITE_YES:

                                    if( ! copy_file( url, path_from, path_to ) ) return;
                                    break;

                                    // 名前変更
                                case Gtk::RESPONSE_YES:
                                {
                                    SKELETON::MsgDiag mdiag_name( get_win(),
                                                                  "ファイル名として画像キャッシュでのファイル名を使用しますか？\n\nいいえを選ぶとファイル選択ダイアログを開きます。",
                                                                  false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );

                                    mdiag_name.add_button( "すべてはい", SKELETON::OVERWRITE_YES_ALL );
                                    mdiag_name.set_default_response( Gtk::RESPONSE_YES );
                                    const int ret = mdiag_name.run();

                                    if( ret == Gtk::RESPONSE_YES || ret == SKELETON::OVERWRITE_YES_ALL ){

                                        if( ret == SKELETON::OVERWRITE_YES_ALL ) use_name_in_cache = true;
                                        if( ! copy_file( url, path_from, path_dir + CACHE::filename_img( url ) ) ) return;
                                    }
                                    else{

                                        if( ! DBIMG::save( url, get_win(), path_to ) ) continue;
                                    }
                                    break;
                                }

                                default:
                                    break;
                            }

                            break;
                        }

                    }

                }
                else if( ! copy_file( url, path_from, path_to ) ) return;
            }

        }
        else{

            SKELETON::MsgDiag mdiag( get_win(), path_dir + "\n\nの作成に失敗しました。\nハードディスクの容量やパーミッションなどを確認してください。\n\n画像保存をキャンセルしました。原因を解決してからもう一度保存を行ってください。" );
            mdiag.run();
        }
    }
}


// ページがロックされているかリストで取得
std::list< bool > ImageAdmin::get_locked()
{
    std::list< bool > locked;

    m_iconbox.foreach( [&locked]( Gtk::Widget& w ) {
        const auto view = dynamic_cast< SKELETON::View* >( &w );
        if( view ) {
            locked.push_back( view->is_locked() );
        }
    } );

    return locked;
}


// タブのロック/アンロック
const bool ImageAdmin::is_lockable( const int page )
{
    SKELETON::View* view = get_nth_icon( page );
    if( view ) return view->is_lockable();

    return false;
}

const bool ImageAdmin::is_locked( const int page )
{
    SKELETON::View* view = get_nth_icon( page );
    if( view ) return view->is_locked();

    return false;
}

void ImageAdmin::lock( const int page )
{
    SKELETON::View* view = get_nth_icon( page );
    if( view ) return view->lock();
}

void ImageAdmin::unlock( const int page )
{
    SKELETON::View* view = get_nth_icon( page );
    if( view ) return view->unlock();
}
