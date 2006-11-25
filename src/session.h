// ライセンス: GPL2
//
// 座標などのウィンドウ情報とかのセッション情報
//

#ifndef _SESSION_H
#define _SESSION_H

#include <list>

namespace SESSION
{
    // focused_admin の値。どこにフォーカスしているか
    // Core::slot_focus_in_event, Core::slot_focus_out_event などを参照
    enum
    {
        FOCUS_BBSLIST = 0,
        FOCUS_BOARD,
        FOCUS_ARTICLE,
        FOCUS_IMAGE,
        FOCUS_NO
    };

    void init_session();
    void save_session();

    const int get_mode_pane();
    void set_mode_pane( int mode );
    
    const bool is_online();
    void set_online( bool mode );

    // 2chログイン中
    const bool login2ch();
    void set_login2ch( bool login );

    int x();
    int y();
    int width();
    int height();
    bool maximized();
    bool show_urlbar();
    bool show_sidebar();
   
    void set_x( int x );
    void set_y( int y );
    void set_width( int width );
    void set_height( int height );
    void set_maximized( bool maximized );
    void set_show_urlbar( bool showbar );
    void set_show_sidebar( bool showbar );

    // フォーカスされているadmin
    int focused_admin();
    void set_focused_admin( int admin );

    // サイドバーを閉じる前にフォーカスされていたadmin
    int focused_admin_sidebar();
    void set_focused_admin_sidebar( int admin );

    /// メインウィンドウのペインの敷居の位置
    int hpane_main_pos();
    int vpane_main_pos();
    int hpane_main_r_pos();
    void set_hpane_main_pos( int pos );
    void set_vpane_main_pos( int pos );
    void set_hpane_main_r_pos( int pos );

    // メインnotebookのページ番号
    int notebook_main_page();
    void set_notebook_main_page( int page );

    // bbslistの開いてるページ番号
    int bbslist_page();
    void set_bbslist_page( int page );


    // 前回閉じたときに開いていたboardのページ番号とURL
    int board_page();
    void set_board_page( int page );
    const std::list< std::string >& board_URLs();
    void set_board_URLs( const std::list< std::string >& urls );

    // 前回閉じたときに開いていたarticleのページ番号とURL
    int article_page();
    void set_article_page( int page );
    const std::list< std::string >& article_URLs();
    void set_article_URLs( const std::list< std::string >& urls );


    // 前回閉じたときに開いていたimageのページ番号とURL
    int image_page();
    void set_image_page( int page );
    const std::list< std::string >& image_URLs();
    void set_image_URLs( const std::list< std::string >& urls );


    // board ビューの列幅
    int col_mark();
    int col_id();
    int col_subject();
    int col_number();
    int col_load();
    int col_new();
    int col_since();
    int col_write();
    int col_speed();
    void set_col_mark( int width );
    void set_col_id( int width );
    void set_col_subject( int width );
    void set_col_number( int width );
    void set_col_load( int width );
    void set_col_new( int width );
    void set_col_since( int width );
    void set_col_write( int width );
    void set_col_speed( int width );

    // image が画面に表示されているか
    bool is_img_shown();
    void set_img_shown( bool set );

    // message ウィンドウの位置
    int mes_x();
    int mes_y();
    int mes_width();
    int mes_height();
    bool mes_maximized();
    void set_mes_x( int x );
    void set_mes_y( int y );
    void set_mes_width( int width );
    void set_mes_height( int height );
    void set_mes_maximized( bool maximized );

    // 最後に画像を保存したディレクトリ
    const std::string& dir_img_save();
    void set_dir_img_save( const std::string& dir );
}

#endif
