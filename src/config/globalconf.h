// ライセンス: GPL2
//
// グローバル設定
//

#ifndef _GLOBALCONF_H
#define _GLOBALCONF_H

#include <string>
#include <list>

namespace CONFIG
{
    class ConfigItems;

    ConfigItems* get_confitem();
    void delete_confitem();

    // 設定読み込み、書き込み
    bool load_conf();
    void save_conf();

    // 設定の一時的なバックアップと復元
    void bkup_conf();
    void restore_conf();


    /////////////////////////////////////////////
    
    // 前回開いたviewを復元するか
    bool get_restore_board();
    void set_restore_board( const bool restore );
    bool get_restore_article();
    void set_restore_article( const bool restore );
    bool get_restore_image();
    void set_restore_image( const bool restore );

    // 自前でウィンドウ配置を管理する
    bool get_manage_winpos();
    void set_manage_winpos( const bool manage );

    // 色 ( # + 12桁の16進数 の形式 )
    const std::string& get_color( const int id );
    void set_color( const int id, const std::string& color );
    void reset_colors();

    // ツリービューでgtkrcの設定を使用するか
    bool get_use_tree_gtkrc();
    void set_use_tree_gtkrc( const bool use );

    // スレビューの選択色でgtkrcの設定を使用するか
    bool get_use_select_gtkrc();
    void set_use_select_gtkrc( const bool use );

    // ツリービューの行間スペース
    int get_tree_ypad();

    // ツリービューにエクスパンダを表示
    bool get_tree_show_expanders();

    // ツリービューのレベルインデント調整量(ピクセル)
    int get_tree_level_indent();

    // カテゴリを開いたときにツリービューをスクロールする
    bool get_scroll_tree();

    // ツリービューの選択を表示中のビューと同期する ( 0: 同期しない 1: 同期する 2: 同期する(フォルダを開く) )
    int get_select_item_sync();
    void set_select_item_sync( const int sync );

    // 各ビューと枠との間の余白
    int get_view_margin();

    // スクロールバーを左に配置
    bool get_left_scrbar();

    // スレ一覧で古いスレも表示 	 
    bool get_show_oldarticle();

    // フォント
    const std::string& get_fontname( const int id );
    void set_fontname( const int id, const std::string& fontname );
    void reset_fonts();
    bool get_aafont_enabled();

    // レスを参照するときに前に付ける文字
    const std::string get_ref_prefix();

    // 参照文字( ref_prefix ) の後のスペースの数
    int ref_prefix_space();

    // レスにアスキーアートがあると判定する正規表現
    const std::string get_regex_res_aa();
    void set_regex_res_aa( const std::string& regex );

    // JD ホームページのアドレス
    const std::string& get_url_jdhp();

    // 2chの認証サーバのアドレス
    const std::string& get_url_login2ch();

    // p2の認証サーバのアドレス
    const std::string& get_url_loginp2();
    void set_url_loginp2( const std::string& url );

    // BEの認証サーバのアドレス
    const std::string& get_url_loginbe();

    // bbsmenu.htmlのURL
    const std::string& get_url_bbsmenu();    

    // bbsmenu.html内にあるリンクは全て板とみなす
    bool use_link_as_board();

    // 板移転時に確認ダイアログを表示する
    bool get_show_movediag();
    void set_show_movediag( const bool show );

    // スレタイ検索用メニュータイトルアドレス
    const std::string& get_menu_search_title();
    const std::string& get_url_search_title();

    // スレタイ検索用正規表現
    const std::string& get_regex_search_title();

    // WEB検索用メニュータイトルアドレス
    const std::string& get_menu_search_web();
    const std::string& get_url_search_web();

    // p2 書き込み用アドレス
    const std::string& get_url_writep2();
    const std::string& get_url_resp2();

    // 2ch にアクセスするときのエージェント名
    const std::string& get_agent_for2ch();

    // 2ch にログインするときのX-2ch-UA
    const std::string& get_x_2ch_ua();

    // 2ch 読み込み用プロクシとポート番号
    bool get_use_proxy_for2ch();
    const std::string& get_proxy_for2ch();
    int get_proxy_port_for2ch();
    const std::string& get_proxy_basicauth_for2ch();

    void set_use_proxy_for2ch( const bool set );
    void set_proxy_for2ch( const std::string& proxy );
    void set_proxy_port_for2ch( const int port );

    // 2ch 書き込み用プロクシとポート番号
    bool get_use_proxy_for2ch_w();
    const std::string& get_proxy_for2ch_w();
    int get_proxy_port_for2ch_w();
    const std::string& get_proxy_basicauth_for2ch_w();

    void set_use_proxy_for2ch_w( const bool set );
    void set_proxy_for2ch_w( const std::string& proxy );
    void set_proxy_port_for2ch_w( const int port );

    // 2ch外にアクセスするときのエージェント名
    const std::string& get_agent_for_data();

    // 2chの外にアクセスするときのプロクシとポート番号
    bool get_use_proxy_for_data();
    const std::string& get_proxy_for_data();
    int get_proxy_port_for_data();
    const std::string& get_proxy_basicauth_for_data();

    void set_use_proxy_for_data( const bool set );
    void set_proxy_for_data( const std::string& proxy );
    void set_proxy_port_for_data( const int port );

    // ローダのバッファサイズ
    int get_loader_bufsize();  // 一般
    int get_loader_bufsize_board();  // スレ一覧読み込み用

    // ローダのタイムアウト値
    int get_loader_timeout();
    int get_loader_timeout_post();
    int get_loader_timeout_data();
    int get_loader_timeout_checkupdate();

    // ipv6使用
    bool get_use_ipv6();
    void set_use_ipv6( const bool set );

    // 同一ホストに対する最大コネクション数( 1 または 2 )
    int get_connection_num();

    // 2chのクッキー:HAP
    bool get_use_cookie_hap();
    const std::string& get_cookie_hap();
    const std::string& get_cookie_hap_bbspink();
    void set_cookie_hap( const std::string& cookie_hap );
    void set_cookie_hap_bbspink( const std::string& cookie_hap );

    // 2chの過去ログ取得時にofflaw2を使用する
    bool get_use_offlaw2_2ch();

    // リンクをクリックしたときに実行するコマンド
    const std::string& get_command_openurl();
    void set_command_openurl( const std::string& command );

    // ブラウザ設定ダイアログのコンボボックスの番号
    int get_browsercombo_id();
    void set_browsercombo_id( const int id );

    // レス番号の上にマウスオーバーしたときに参照ポップアップ表示する
    bool get_refpopup_by_mo();

    // 名前の上にマウスオーバーしたときにポップアップ表示する
    bool get_namepopup_by_mo();

    // IDの上にマウスオーバーしたときにIDをポップアップ表示する
    bool get_idpopup_by_mo();

    // 画像のスムージングレベル(0-2, 2が最も高画質かつ低速)
    int get_imgemb_interp();
    int get_imgmain_interp();
    int get_imgpopup_interp();

    // 画像ポップアップサイズ
    int get_imgpopup_width();
    int get_imgpopup_height();

    // 画像ポップアップを使用する
    bool get_use_image_popup();
    void set_use_image_popup( const bool use );

    // 画像ビューを使用する
    bool get_use_image_view();
    void set_use_image_view( const bool image_view );

    // インライン画像表示をする
    bool get_use_inline_image();
    void set_use_inline_image( const bool inline_img );

    // ssspアイコン表示
    bool get_show_ssspicon();
    void set_show_sssp_icon( const bool show );

    // インライン画像の最大幅と高さ
    int get_embimg_width();
    int get_embimg_height();

    // 埋め込み画像ビューを閉じたときにタブも閉じる
    bool get_hide_imagetab();

    // 画像ビューでdeleteを押したときに確認ダイアログを表示する
    bool get_show_delimgdiag();
    void set_show_delimgdiag( const bool show );

    // 画像にモザイクかける
    bool get_use_mosaic();
    void set_use_mosaic( const bool mosaic );

    // モザイクの大きさ
    int get_mosaic_size();

    // 画像をデフォルトでウィンドウサイズに合わせる
    bool get_zoom_to_fit();
    void set_zoom_to_fit( const bool fit );

    // 画像キャッシュ削除の日数
    int get_del_img_day();
    void set_del_img_day( const int day );

    // 画像あぼーん削除の日数
    int get_del_imgabone_day();
    void set_del_imgabone_day( const int day );

    // ダウンロードする画像の最大ファイルサイズ(Mbyte)
    int get_max_img_size();

    // 画像の最大サイズ(Mピクセル)
    int get_max_img_pixel();

    // 画像のメモリキャッシュ枚数
    int get_imgcache_size();

    // スレ一覧で指定した値(時間)よりも後に立てられたスレを新着とみなす
    int get_newthread_hour();

    // スレ一覧でインクリメント検索をする
    bool get_inc_search_board();

    // スレ一覧でdeleteを押したときに確認ダイアログを表示する
    bool get_show_deldiag();
    void set_show_deldiag( const bool show );

    // スレ一覧をロードする前にキャッシュにある一覧を表示
    bool get_show_cached_board();

    // スレ一覧でお知らせスレ(924)のアイコンを表示する
    bool get_show_924();
    
    // ツリービューのスクロール量(マウスホイール上下・行数)
    int get_tree_scroll_size();

    // スレビューのスクロール量(マウスホイール上下・行数)
    int get_scroll_size();

    // スレビューのスクロール量(キー上下・行数)
    int get_key_scroll_size();

    // スレビューの高速スクロール量(キー上下・ ページ高 - 行高 * key_fastscroll_size )
    int get_key_fastscroll_size();

    // スレビューでリロード後に一番下までスクロール
    bool get_jump_after_reload();
    void set_jump_after_reload( const bool set );

    // スレビューでリロード後に新着までスクロール
    bool get_jump_new_after_reload();
    void set_jump_new_after_reload( const bool set );

    // 実況モード
    int get_live_mode();
    void set_live_mode( const int mode );

    // 実況速度
    int get_live_speed();
    void set_live_speed( const int speed );

    // 実況のスクロールモードを切り替えるしきい値
    int get_live_threshold();
    void set_live_threshode( const int th );

    // 板一覧でカテゴリを常にひとつだけ開く
    bool get_open_one_category();

    // お気に入りでカテゴリを常にひとつだけ開く
    bool get_open_one_favorite();

    // デフォルトの書き込み名
    const std::string get_write_name();

    // デフォルトのメールアドレス
    const std::string get_write_mail();

    // 書き込み時に書き込み確認ダイアログを出すかどうか
    bool get_always_write_ok();
    void set_always_write_ok( const bool write_ok );

    // 書き込みログを保存
    bool get_save_post_log();
    void set_save_post_log( const bool save );

    // 書き込みログの最大サイズ
    size_t get_maxsize_post_log();

    // 書き込み履歴を保存
    bool get_save_post_history();
    void set_save_post_history( const bool save );

    // 書き込み中のダイアログを表示しない
    bool get_hide_writing_dialog();

    // 編集中のメッセージの保存確認ダイアログを表示する
    bool get_show_savemsgdiag();
    void set_show_savemsgdiag( const bool show );

    // 書き込みビューでテキストを折り返す
    bool get_message_wrap();
    void set_message_wrap( const bool wrap );

    // 非アクティブ時に書き込みビューを折りたたむ
    bool get_fold_message();
    void set_fold_message( const bool fold );

    // 非アクティブ時に画像ビューを折りたたむ
    bool get_fold_image();

    // 書き込み欄の日本語のON/OFF状態を保存
    bool get_keep_im_status();

    // ポップアップとカーソルの間のマージン
    int get_margin_popup();
    void set_margin_popup( const int margin );

    // 画像ポップアップとカーソルの間のマージン
    int get_margin_imgpopup_x(); // 水平方向
    int get_margin_imgpopup(); // 垂直方向

    // ポップアップが消えるまでの時間(ミリ秒)
    int get_hide_popup_msec();

    // マウスジェスチャを有効
    bool get_enable_mg();

    // マウスジェスチャの判定開始半径
    int get_mouse_radius();

    // 数字入力ジャンプの待ち時間(ミリ秒)
    int get_numberjmp_msec();

    // 履歴メニューの表示数
    int get_history_size();

    // 履歴ビューの表示数
    int get_historyview_size();

    // AA履歴の保持数
    int get_aahistory_size();

    // 0以上なら多重ポップアップの説明を表示する
    int get_instruct_popup();

    // スレビューを開いたときにスレ一覧との切り替え方法を説明する
    bool get_instruct_tglart();
    void set_instruct_tglart( const bool set );

    // 画像ビューを開いたときにスレビューとの切り替え方法を説明する
    bool get_instruct_tglimg();
    void set_instruct_tglimg( const bool set );

    // スレビューでdeleteを押したときに確認ダイアログを表示する
    bool get_show_delartdiag();
    void set_show_delartdiag( const bool show );

    // 下線位置調整
    double get_adjust_underline_pos();
    void set_adjust_underline_pos( const double pos );

    // スレ表示の行間調整
    double get_adjust_line_space();
    void set_adjust_line_space( const double space );

    // スレ表示でリンクの下に下線を引く
    bool get_draw_underline();

    // スレビューで文字幅の近似を厳密にする
    bool get_strict_char_width();
    void set_strict_char_width( const bool strictwidth );

    // スレビューで発言数(ID)をカウントする
    bool get_check_id();

    // レス参照で色を変える回数
    int get_num_reference_high();
    int get_num_reference_low();

    // 発言数で色を変える回数
    int get_num_id_high();
    int get_num_id_low();

    // datのパース時にURL判定を甘くする(^なども含める)
    bool get_loose_url();

    // ユーザーコマンドで選択できない項目を非表示にする
    bool get_hide_usrcmd();
    void set_hide_usrcmd( const bool hide );

    // スレビューで再読み込みボタンを押したときに全タブを更新する
    bool get_reload_allthreads();

    // タブに表示する文字列の最小値
    int get_tab_min_str();

    // タブにアイコンを表示するか
    bool get_show_tab_icon();

    // タブ上でマウスホイールを回転してタブを切り替える
    bool get_switchtab_wheel();

    // 他のビューを開くときのタブの位置 ( 0: 一番右端 1:右隣 2:左隣 )
    int get_newtab_pos();

    // ツリービューで選択したビューを開くときのタブの位置 ( 0: 一番右端 1:右隣 2:左隣 )
    int get_opentab_pos();

    // 次スレ検索を開くときのタブの位置 ( 0: 次スレ検索タブ 1:新しいタブ 2:アクティブなタブを置き換え )
    int get_boardnexttab_pos();

    // スレビューに書き込みマークを表示するか
    bool get_show_post_mark();
    void set_show_post_mark( const bool show );

    // ボタンをフラットにするか
    bool get_flat_button();
    void set_flat_button( const bool set );

    // ツールバーの背景描画
    bool get_draw_toolbarback();
    void set_draw_toolbarback( const bool set );

    // boardviewでのスレの全体あぼーん
    std::list< std::string >& get_list_abone_word_thread(); // ワード
    std::list< std::string >& get_list_abone_regex_thread(); // 正規表現

    void set_list_abone_word_thread( std::list< std::string >& word );
    void set_list_abone_regex_thread( std::list< std::string >& regex );

    int get_remove_old_abone_thread(); // dat落ちしたスレをNGスレタイトルリストから取り除くか( 0: ダイアログ表示 1: 取り除く 2: 除かない )
    void set_remove_old_abone_thread( const int remove ); 

    int get_abone_number_thread();
    void set_abone_number_thread( const int number );

    int get_abone_hour_thread();
    void set_abone_hour_thread( const int hour );

    // articleviewでのレスの全体あぼーん
    const std::list< std::string >& get_list_abone_name(); // 名前
    const std::list< std::string >& get_list_abone_word(); // ワード
    const std::list< std::string >& get_list_abone_regex(); // 正規表現

    void set_list_abone_name( const std::list< std::string >& name );
    void set_list_abone_word( const std::list< std::string >& word );
    void set_list_abone_regex( const std::list< std::string >& regex );

    // デフォルトで透明、連鎖あぼーんをするか
    bool get_abone_transparent();
    void set_abone_transparent( const bool set );

    bool get_abone_chain();
    void set_abone_chain( const bool set );

    // NG正規表現によるあぼーん時に大小と全半角文字の違いを無視
    bool get_abone_icase();
    void set_abone_icase( const bool set );

    bool get_abone_wchar();
    void set_abone_wchar( const bool set );

    // 右ペーンが空の時にサイドバーを閉じるか
    bool get_expand_sidebar();

    // 3ペーン時にスレ一覧やスレビューを最大化するか
    bool get_expand_rpane();

    // ペーンの境界をクリックしてサイドバーを開け閉めする
    bool get_open_sidebar_by_click();

    // 次スレ検索の類似度のしきい値
    int get_threshold_next();

    // 次スレを開いたときにお気に入りのアドレスと名前を自動更新
    int get_replace_favorite_next();
    void set_replace_favorite_next( const int mode );

    // お気に入りの自動更新をするかダイアログを出す
    bool show_diag_replace_favorite();
    void set_show_diag_replace_favorite( const bool show );

    // スレをお気に入りに追加したときにしおりをセットする
    bool get_bookmark_drop();

    // お気に入りの更新チェック時に板の更新もチェックする
    bool get_check_update_board();

    // 起動時にお気に入りを自動でチェックする
    bool get_check_update_boot();

    // お気に入り登録時に重複項目を登録するか ( 0: 登録する 1: ダイアログ表示  2: 登録しない )
    int get_check_favorite_dup();
    void set_check_favorite_dup( const int check );

    // お気に入り登録時に挿入先ダイアログを表示する ( 0 : 表示する 1: 表示せず先頭に追加 2: 表示せず最後に追加 )
    int get_show_favorite_select_diag();

    // Ctrl+qでウィンドウを閉じない
    bool get_disable_close();
    void set_disable_close( const bool disable );

    // メニューバーを非表示にした時にダイアログを表示
    bool get_show_hide_menubar_diag();
    void set_show_hide_menubar_diag( const bool set );

    // 状態変更時にメインステータスバーの色を変える
    bool get_change_stastatus_color();

    // まちBBSの取得に offlaw.cgi を使用する
    bool get_use_machi_offlaw();
    void set_use_machi_offlaw( const bool set );

    // 書き込み履歴のあるスレを削除する時にダイアログを表示
    bool get_show_del_written_thread_diag();
    void set_del_written_thread_diag( const bool set );

    // スレを削除する時に画像キャッシュも削除する ( 0: ダイアログ表示 1: 削除 2: 削除しない )
    int get_delete_img_in_thread();
    void set_delete_img_in_thread( const int set );

    // FIFOの作成などにエラーがあったらダイアログを表示する
    bool get_show_diag_fifo_error();
    void set_show_diag_fifo_error( const bool set );

    // 指定した分ごとにセッションを自動保存 (0: 保存しない)
    int get_save_session();

#ifdef HAVE_MIGEMO_H
    // migemo-dictの場所
    const std::string& get_migemodict_path();
#endif
}


#endif
