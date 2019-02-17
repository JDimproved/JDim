// ライセンス: GPL2
//
// 座標などのウィンドウ情報とかのセッション情報
//

#ifndef _SESSION_H
#define _SESSION_H

#include "type.h"
#include "data_info.h"

#include <list>
#include <vector>
#include <string>

namespace ARTICLE
{
    class DrawAreaBase;
}

namespace SESSION
{
    // focused_admin の値。どこにフォーカスしているか
    // Core::slot_focus_in_event, Core::slot_focus_out_event などを参照
    enum
    {
        FOCUS_SIDEBAR = 0,
        FOCUS_BOARD,
        FOCUS_ARTICLE,
        FOCUS_IMAGE,
        FOCUS_MESSAGE,
        FOCUS_NOT, // どこもフォーカスされていない

        FOCUS_NUM
    };

    // ペーンモード
    enum
    {
        MODE_2PANE = 0,
        MODE_3PANE,
        MODE_V3PANE,

        MODE_PANE_NUM
    };

    // メインウィンドウの右ペーンに表示中のnotebook
    enum
    {
        PAGE_ARTICLE = 0,
        PAGE_IMAGE,
        PAGE_BOARD,

        PAGE_NUM
    };

    // メインツールバーの位置
    enum
    {
        TOOLBAR_POS_NORMAL = 0, // メニューバーの下に表示
        TOOLBAR_POS_RIGHT, // サイドバーの右に表示

        TOOLBAR_POS_NUM
    };

    // 画像のfitモード
    enum
    {
        IMG_FIT_NORMAL = 0,  // 縦と横で小さい方をウィンドウに合わせる
        IMG_FIT_WIDTH,       // 常に横をウィンドウに合わせる

        IMG_FIT_NUM
    };

    void init_session();
    void save_session();

    // ブート中
    bool is_booting();
    void set_booting( const bool boot );

    // 終了中
    bool is_quitting();
    void set_quitting( const bool quit );

    // 入れ替えなどのタブ操作中
    // ビューの再描画などを禁止する
    bool is_tab_operating( const std::string& url_admin );
    void set_tab_operating( const std::string& url_admin, const bool operating );

    int get_mode_pane();
    void set_mode_pane( const int mode );
    
    bool is_online();
    void set_online( const bool mode );

    // 2chログイン中
    bool login2ch();
    void set_login2ch( const bool login );

    // BEログイン中
    bool loginbe();
    void set_loginbe( const bool login );

    // P2ログイン中
    bool loginp2();
    void set_loginp2( const bool login );

    // サイドバー表示中
    bool show_sidebar();
    void set_show_sidebar( const bool showbar );

    // メニューバー
    bool show_menubar();
    void set_show_menubar( const bool show );

    // メインツールバー表示
    bool get_show_main_toolbar();
    void set_show_main_toolbar( const bool show );

    // メインツールバー位置
    int get_toolbar_pos();
    void set_toolbar_pos( const int pos );

    // 板一覧のツールバー表示
    bool get_show_bbslist_toolbar();
    void set_show_bbslist_toolbar( const bool show );

    // スレ一覧のツールバー表示
    bool get_show_board_toolbar();
    void set_show_board_toolbar( const bool show );

    // スレビューのツールバー表示
    bool get_show_article_toolbar();
    void set_show_article_toolbar( const bool show );

    // スレ一覧のタブ表示
    bool get_show_board_tab();
    void set_show_board_tab( const bool show );

    // スレビューのタブ
    bool get_show_article_tab();
    void set_show_article_tab( const bool show );

    // メインステータスバー表示
    bool get_show_main_statbar();
    void set_show_main_statbar( const bool show );

    // フォーカスされているadmin
    int focused_admin();
    void set_focused_admin( const int admin );

    // 各windowの座標
    int get_x_win_main(); // メインウィンドウ
    int get_y_win_main();
    void set_x_win_main( const int x );
    void set_y_win_main( const int y );

    int get_x_win_img(); // 画像ウィンドウ
    int get_y_win_img();
    void set_x_win_img( const int x );
    void set_y_win_img( const int y );

    int get_x_win_mes(); // 書き込みウィンドウ
    int get_y_win_mes();
    void set_x_win_mes( const int x );
    void set_y_win_mes( const int y );


    // 各windowのサイズ
    int get_width_win_main(); // メインウィンドウ
    int get_height_win_main();
    void set_width_win_main( const int width );
    void set_height_win_main( const int height );

    int get_width_win_img(); // 画像ウィンドウ
    int get_height_win_img();
    void set_width_win_img( const int width );
    void set_height_win_img( const int height );

    int get_width_win_mes(); // 書き込みウィンドウ
    int get_height_win_mes();
    void set_width_win_mes( const int width );
    void set_height_win_mes( const int height );


    // 各window がフォーカスされているか
    bool is_focus_win_main(); // メインウィンドウ
    void set_focus_win_main( const bool set );

    bool is_focus_win_img(); // 画像ウィンドウ
    void set_focus_win_img( const bool set );

    bool is_focus_win_mes(); // 書き込みウィンドウ
    void set_focus_win_mes( const bool set );


    // 各window が最大化されているか
    bool is_maximized_win_main(); // メインウィンドウ
    void set_maximized_win_main( const bool maximized );

    bool is_maximized_win_img(); // 画像ウィンドウ
    void set_maximized_win_img( const bool set );

    bool is_maximized_win_mes(); // 書き込みウィンドウ
    void set_maximized_win_mes( const bool maximized );


    // 各window が最小化されているか
    bool is_iconified_win_main(); // メインウィンドウ
    void set_iconified_win_main( const bool set );

    bool is_iconified_win_img(); // 画像ウィンドウ
    void set_iconified_win_img( const bool set );

    bool is_iconified_win_mes(); // 書き込みウィンドウ
    void set_iconified_win_mes( const bool set );


    // 各window が画面に表示されているか
    bool is_shown_win_main(); // メインウィンドウ
    void set_shown_win_main( const bool set );

    bool is_shown_win_img(); // 画像ウィンドウ
    void set_shown_win_img( const bool set );

    bool is_shown_win_mes(); // 書き込みウィンドウ
    void set_shown_win_mes( const bool set );

    // windowがフルスクリーンか
    bool is_full_win_main(); // メインウィンドウ
    void set_full_win_main( const bool set );


    // ダイアログ表示中
    bool is_dialog_shown();
    void set_dialog_shown( const bool set );

    // サイドバーを閉じる前にフォーカスされていたadmin
    int focused_admin_sidebar();
    void set_focused_admin_sidebar( const int admin );

    /// メインウィンドウのペインの敷居の位置
    int hpane_main_pos();
    int vpane_main_pos();
    int hpane_main_r_pos();
    int vpane_main_mes_pos();
    void set_hpane_main_pos( const int pos );
    void set_vpane_main_pos( const int pos );
    void set_hpane_main_r_pos( const int pos );
    void set_vpane_main_mes_pos( const int pos );

    // 前回閉じたときに開いていたメインnotebookのページ番号
    int notebook_main_page();
    void set_notebook_main_page( const int page );

    // 前回閉じたときに開いていたbbslistの開いてるページ番号
    int bbslist_page();
    void set_bbslist_page( const int page );


    // 前回閉じたときに開いていたスレ一覧のページ番号とURL
    int board_page();
    void set_board_page( const int page );
    const std::list< std::string >& get_board_URLs();
    void set_board_URLs( const std::list< std::string >& urls );

    // スレ一覧のロック状態
    const std::list< bool >& get_board_locked();
    void set_board_locked( const std::list< bool >& locked );

    // スレ一覧の切り替え履歴    
    const std::list< std::string >& get_board_switchhistory();
    void set_board_switchhistory( const std::list< std::string >& hist );

    // 前回閉じたときに開いていたスレタブのページ番号とURL
    int article_page();
    void set_article_page( const int page );
    const std::list< std::string >& get_article_URLs();
    void set_article_URLs( const std::list< std::string >& urls );

    // スレタブのロック状態
    const std::list< bool >& get_article_locked();
    void set_article_locked( const std::list< bool >& locked );

    // スレタブの切り替え履歴    
    const std::list< std::string >& get_article_switchhistory();
    void set_article_switchhistory( const std::list< std::string >& hist );

    // 前回閉じたときに開いていたimageのページ番号とURL
    int image_page();
    void set_image_page( const int page );
    const std::list< std::string >& image_URLs();
    void set_image_URLs( const std::list< std::string >& urls );

    // 画像タブのロック状態
    const std::list< bool >& get_image_locked();
    void set_image_locked( const std::list< bool >& locked );

    // 現在開いているサイドバーのページ
    int get_sidebar_current_page();

    // 現在開いているサイドバーのurl
    std::string get_sidebar_current_url();

    // ツールバー等の項目名 -> ID 変換
    int parse_item( const std::string& item_name );

    // サイドバーのツールバー項目
    const std::string& get_items_sidebar_toolbar_str();
    std::string get_items_sidebar_toolbar_default_str();
    void set_items_sidebar_toolbar_str( const std::string& items );
    int get_item_sidebar_toolbar( const int num );

    // メインツールバーの項目
    const std::string& get_items_main_toolbar_str();
    std::string get_items_main_toolbar_default_str();
    void set_items_main_toolbar_str( const std::string& items_str );
    int get_item_main_toolbar( const int num );

    // スレビューのツールバーの項目
    const std::string& get_items_article_toolbar_str();
    std::string get_items_article_toolbar_default_str();
    void set_items_article_toolbar_str( const std::string& items_str );
    int get_item_article_toolbar( const int num );

    // 検索ビューのツールバーの項目
    const std::string& get_items_search_toolbar_str();
    std::string get_items_search_toolbar_default_str();
    void set_items_search_toolbar_str( const std::string& items_str );
    int get_item_search_toolbar( const int num );

    // スレ一覧のツールバー項目
    const std::string& get_items_board_toolbar_str();
    std::string get_items_board_toolbar_default_str();
    void set_items_board_toolbar_str( const std::string& items );
    int get_item_board_toolbar( const int num );

    // 書き込みビューのツールバー項目
    const std::string& get_items_msg_toolbar_str();
    std::string get_items_msg_toolbar_default_str();
    void set_items_msg_toolbar_str( const std::string& items );
    int get_item_msg_toolbar( const int num );

    // スレ一覧の列項目
    const std::string& get_items_board_col_str();
    std::string get_items_board_col_default_str();
    void set_items_board_col_str( const std::string& items );
    int get_item_board_col( const int num );

    // スレ一覧のコンテキストメニュー項目
    const std::string& get_items_board_menu_str();
    std::string get_items_board_menu_default_str();
    void set_items_board_menu_str( const std::string& items_str );
    int get_item_board_menu( const int num );

    // スレビューのコンテキストメニュー項目
    const std::string& get_items_article_menu_str();
    std::string get_items_article_menu_default_str();
    void set_items_article_menu_str( const std::string& items_str );
    int get_item_article_menu( const int num );

    // スレ一覧の列幅
    int col_mark();
    int col_id();
    int col_board();
    int col_subject();
    int col_number();
    int col_load();
    int col_new();
    int col_since();
    int col_write();
    int col_access();
    int col_speed();
    int col_diff();
    void set_col_mark( const int width );
    void set_col_id( const int width );
    void set_col_board( const int width );
    void set_col_subject( const int width );
    void set_col_number( const int width );
    void set_col_load( const int width );
    void set_col_new( const int width );
    void set_col_since( const int width );
    void set_col_write( const int width );
    void set_col_access( const int width );
    void set_col_speed( const int width );
    void set_col_diff( const int width );

    // スレ一覧の since の表示モード
    int get_col_since_time();
    void set_col_since_time( const int mode );

    // スレ一覧の 最終書込 の表示モード
    int get_col_write_time();
    void set_col_write_time( const int mode );

    // スレ一覧の 最終取得 の表示モード
    int get_col_access_time();
    void set_col_access_time( const int mode );

    // 現在開いているarticle の ARTICLE::DrawAreaBase
    ARTICLE::DrawAreaBase* get_base_drawarea();

    // 現在開いているarticle のurl
    std::string get_article_current_url();


    // 埋め込みimage使用
    bool get_embedded_img();
    void set_embedded_img( const bool set );


    // 埋め込みmessageを使用
    bool get_embedded_mes();
    void set_embedded_mes( const bool set );

    // 書き込み後にmessageを閉じる
    bool get_close_mes();
    void set_close_mes( const bool set );


    // 最後にdatを読み書きしたディレクトリ
    const std::string& get_dir_dat();
    void set_dir_dat( const std::string& dir );

    // 最後に画像を保存したディレクトリ
    const std::string& dir_img_save();
    void set_dir_img_save( const std::string& dir );

    // 下書きファイルのディレクトリ
    const std::string& get_dir_draft();
    void set_dir_draft( const std::string& dir );

    // ポップアップメニュー表示中
    bool is_popupmenu_shown();
    void set_popupmenu_shown( const bool shown );

    // JD終了時に削除するスレのリスト
    const std::vector< std::string >& get_delete_list();
    void append_delete_list( const std::string& url );
    void remove_delete_list( const std::string& url );

    // 実況実行中のスレ
    bool is_live( const std::string& url );
    void append_live( const std::string& url );
    void remove_live( const std::string& url );

    // 画像のfitモード
    int get_img_fit_mode();
    void toggle_img_fit_mode();

    // お気に入り挿入ダイアログで最後に保存したディレクトリ名
    const std::string& get_dir_select_favorite();
    void set_dir_select_favorite( const std::string& dir );

    // 各履歴を取得
    void get_history( const std::string& url, CORE::DATA_INFO_LIST& info_list );

    // サイドバーの指定したidのディレクトリに含まれるスレのアドレスを取得
    void get_sidebar_threads( const std::string& url, const int dirid, std::vector< std::string >& list_url );

    // サイドバーの指定したidのディレクトリの名前を取得
    std::string get_sidebar_dirname( const std::string& url, const int dirid );
}


#endif
