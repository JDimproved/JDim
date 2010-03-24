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
    const bool load_conf();
    void save_conf();

    // 設定の一時的なバックアップと復元
    void bkup_conf();
    void restore_conf();


    /////////////////////////////////////////////
    
    // 前回開いたviewを復元するか
    const bool get_restore_board();
    void set_restore_board( const bool restore );
    const bool get_restore_article();
    void set_restore_article( const bool restore );
    const bool get_restore_image();
    void set_restore_image( const bool restore );

    // 自前でウィンドウ配置を管理する
    const bool get_manage_winpos();
    void set_manage_winpos( const bool manage );

    // 色 ( # + 12桁の16進数 の形式 )
    const std::string& get_color( const int id );
    void set_color( const int id, const std::string& color );
    void reset_colors();

    // ツリービューでgtkrcの設定を使用するか
    const bool get_use_tree_gtkrc();
    void set_use_tree_gtkrc( const bool use );

    // スレビューの選択色でgtkrcの設定を使用するか
    const bool get_use_select_gtkrc();
    void set_use_select_gtkrc( const bool use );

    // ツリービューの行間スペース
    const int get_tree_ypad();

    // カテゴリを開いたときにツリービューをスクロールする
    const bool get_scroll_tree();

    // 各ビューと枠との間の余白
    const int get_view_margin();

    // フォント
    const std::string& get_fontname( const int id );
    void set_fontname( const int id, const std::string& fontname );
    void reset_fonts();

    // レスを参照するときに前に付ける文字
    const std::string get_ref_prefix();

    // 参照文字( ref_prefix ) の後のスペースの数
    const int ref_prefix_space();

    // JD ホームページのアドレス
    const std::string& get_url_jdhp();

    // 2chの認証サーバのアドレス
    const std::string& get_url_login2ch();

    // p2の認証サーバのアドレス
    const std::string& get_url_loginp2();

    // bbsmenu.htmlのURL
    const std::string& get_url_bbsmenu();    

    // bbsmenu.html内にあるリンクは全て板とみなす
    const bool use_link_as_board();

    // 板移転時に確認ダイアログを表示する
    const bool get_show_movediag();
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
    const bool get_use_proxy_for2ch();
    const std::string& get_proxy_for2ch();
    const int get_proxy_port_for2ch();
    const std::string& get_proxy_basicauth_for2ch();

    void set_use_proxy_for2ch( const bool set );
    void set_proxy_for2ch( const std::string& proxy );
    void set_proxy_port_for2ch( const int port );

    // 2ch 書き込み用プロクシとポート番号
    const bool get_use_proxy_for2ch_w();
    const std::string& get_proxy_for2ch_w();
    const int get_proxy_port_for2ch_w();
    const std::string& get_proxy_basicauth_for2ch_w();

    void set_use_proxy_for2ch_w( const bool set );
    void set_proxy_for2ch_w( const std::string& proxy );
    void set_proxy_port_for2ch_w( const int port );

    // 2ch外にアクセスするときのエージェント名
    const std::string& get_agent_for_data();

    // 2chの外にアクセスするときのプロクシとポート番号
    const bool get_use_proxy_for_data();
    const std::string& get_proxy_for_data();
    const int get_proxy_port_for_data();
    const std::string& get_proxy_basicauth_for_data();

    void set_use_proxy_for_data( const bool set );
    void set_proxy_for_data( const std::string& proxy );
    void set_proxy_port_for_data( const int port );

    // ローダのバッファサイズ
    const int get_loader_bufsize();  // 一般
    const int get_loader_bufsize_board();  // スレ一覧読み込み用

    // ローダのタイムアウト値
    const int get_loader_timeout();
    const int get_loader_timeout_post();
    const int get_loader_timeout_data();
    const int get_loader_timeout_checkupdate();

    // ipv6使用
    const bool get_use_ipv6();
    void set_use_ipv6( const bool set );

    // リンクをクリックしたときに実行するコマンド
    const std::string& get_command_openurl();
    void set_command_openurl( const std::string& command );

    // ブラウザ設定ダイアログのコンボボックスの番号
    const int get_browsercombo_id();
    void set_browsercombo_id( const int id );

    // レス番号の上にマウスオーバーしたときに参照ポップアップ表示する
    const bool get_refpopup_by_mo();

    // 名前の上にマウスオーバーしたときにポップアップ表示する
    const bool get_namepopup_by_mo();

    // IDの上にマウスオーバーしたときにIDをポップアップ表示する
    const bool get_idpopup_by_mo();

    // 画像のスムージングレベル(0-2, 2が最も高画質かつ低速)
    const int get_imgemb_interp();
    const int get_imgmain_interp();
    const int get_imgpopup_interp();

    // 画像ポップアップサイズ
    const int get_imgpopup_width();
    const int get_imgpopup_height();

    // 画像ポップアップを使用する
    const bool get_use_image_popup();
    void set_use_image_popup( const bool use );

    // 画像ビューを使用する
    const bool get_use_image_view();
    void set_use_image_view( const bool image_view );

    // インライン画像表示をする
    const bool get_use_inline_image();
    void set_use_inline_image( const bool inline_img );

    // ssspアイコン表示
    const bool get_show_ssspicon();
    void set_show_sssp_icon( const bool show );

    // 画像にモザイクかける
    const bool get_use_mosaic();
    void set_use_mosaic( const bool mosaic );

    // モザイクの大きさ
    const int get_mosaic_size();

    // 画像をデフォルトでウィンドウサイズに合わせる
    const bool get_zoom_to_fit();
    void set_zoom_to_fit( const bool fit );

    // 画像キャッシュ削除の日数
    const int get_del_img_day();
    void set_del_img_day( const int day );

    // 画像あぼーん削除の日数
    const int get_del_imgabone_day();
    void set_del_imgabone_day( const int day );

    // ダウンロードする画像の最大ファイルサイズ(Mbyte)
    const int get_max_img_size();

    // 画像の最大サイズ(Mピクセル)
    const int get_max_img_pixel();

    // 画像のメモリキャッシュ枚数
    const int get_imgcache_size();

    // スレ一覧にdat落ちしたスレも表示する
    const bool get_show_oldarticle();
    void set_show_oldarticle( const bool showarticle );

    // スレ一覧で指定した値(時間)よりも後に立てられたスレを新着とみなす
    const int get_newthread_hour();

    // スレ一覧でインクリメント検索をする
    const bool get_inc_search_board();

    // スレ一覧でdeleteを押したときに確認ダイアログを表示する
    const bool get_show_deldiag();
    void set_show_deldiag( const bool show );

    // スレ一覧をロードする前にキャッシュにある一覧を表示
    const bool get_show_cached_board();

    // スレ一覧でお知らせスレ(924)のアイコンを表示する
    const bool get_show_924();
    
    // ツリービューのスクロール量(マウスホイール上下・行数)
    const int get_tree_scroll_size();

    // スレビューのスクロール量(マウスホイール上下・行数)
    const int get_scroll_size();

    // スレビューのスクロール量(キー上下・行数)
    const int get_key_scroll_size();

    // スレビューの高速スクロール量(キー上下・ ページ高 - 行高 * key_fastscroll_size )
    const int get_key_fastscroll_size();

    // スレビューでリロード後に一番下までスクロール
    const bool get_jump_after_reload();
    void set_jump_after_reload( const bool set );

    // スレビューでリロード後に新着までスクロール
    const bool get_jump_new_after_reload();
    void set_jump_new_after_reload( const bool set );

    // 実況モード
    const int get_live_mode();
    void set_live_mode( const int mode );

    // 実況速度
    const int get_live_speed();
    void set_live_speed( const int speed );

    // 実況のスクロールモードを切り替えるしきい値
    const int get_live_threshold();
    void set_live_threshode( const int th );

    // 板一覧でカテゴリを常にひとつだけ開く
    const bool get_open_one_category();

    // お気に入りでカテゴリを常にひとつだけ開く
    const bool get_open_one_favorite();

    // 書き込み時に書き込み確認ダイアログを出すかどうか
    const bool get_always_write_ok();
    void set_always_write_ok( const bool write_ok );

    // 書き込みログを保存
    const bool get_save_post_log();
    void set_save_post_log( const bool save );

    // 書き込みログの最大サイズ
    const size_t get_maxsize_post_log();

    // 書き込み履歴を保存
    const bool get_save_post_history();
    void set_save_post_history( const bool save );

    // 書き込み中のダイアログを表示しない
    const bool get_hide_writing_dialog();

    // 非アクティブ時に書き込みビューを折りたたむ
    const bool get_fold_message();
    void set_fold_message( const bool fold );

    // 書き込み欄の日本語のON/OFF状態を保存
    const bool get_keep_im_status();

    // ポップアップとカーソルの間のマージン
    const int get_margin_popup();
    void set_margin_popup( const int margin );

    // 画像ポップアップとカーソルの間のマージン
    const int get_margin_imgpopup();

    // マウスジェスチャの判定開始半径
    const int get_mouse_radius();

    // 履歴メニューの表示数
    const int get_history_size();

    // 履歴ビューの表示数
    const int get_historyview_size();

    // AA履歴の保持数
    const int get_aahistory_size();

    // 0以上なら多重ポップアップの説明を表示する
    const int get_instruct_popup();

    // スレビューを開いたときにスレ一覧との切り替え方法を説明する
    const bool get_instruct_tglart();
    void set_instruct_tglart( const bool set );

    // 画像ビューを開いたときにスレビューとの切り替え方法を説明する
    const bool get_instruct_tglimg();
    void set_instruct_tglimg( const bool set );

    // 下線位置調整
    const double get_adjust_underline_pos();
    void set_adjust_underline_pos( const double pos );

    // スレ表示の行間調整
    const double get_adjust_line_space();
    void set_adjust_line_space( const double space );

    // スレ表示でリンクの下に下線を引く
    const bool get_draw_underline();

    // スレビューで文字幅の近似を厳密にする
    const bool get_strict_char_width();
    void set_strict_char_width( const bool strictwidth );

    // スレビューで発言数(ID)をカウントする
    const bool get_check_id();

    // レス参照で色を変える回数
    const int get_num_reference_high();
    const int get_num_reference_low();

    // 発言数で色を変える回数
    const int get_num_id_high();
    const int get_num_id_low();

    // datのパース時にURL判定を甘くする(^なども含める)
    const bool get_loose_url();

    // ユーザーコマンドで選択できない項目を非表示にする
    const bool get_hide_usrcmd();
    void set_hide_usrcmd( const bool hide );

    // スレビューで再読み込みボタンを押したときに全タブを更新する
    const bool get_reload_allthreads();

    // タブに表示する文字列の最小値
    const int get_tab_min_str();

    // タブにアイコンを表示するか
    const bool get_show_tab_icon();

    // スレビューに書き込みマークを表示するか
    const bool get_show_post_mark();
    void set_show_post_mark( const bool show );

    // ボタンをフラットにするか
    const bool get_flat_button();
    void set_flat_button( const bool set );

    // ツールバーの背景描画
    const bool get_draw_toolbarback();
    void set_draw_toolbarback( const bool set );

    // boardviewでのスレの全体あぼーん
    std::list< std::string >& get_list_abone_word_thread(); // ワード
    std::list< std::string >& get_list_abone_regex_thread(); // 正規表現

    void set_list_abone_word_thread( std::list< std::string >& word );
    void set_list_abone_regex_thread( std::list< std::string >& regex );

    const int get_remove_old_abone_thread(); // dat落ちしたスレをNGスレタイトルリストから取り除くか( 0: ダイアログ表示 1: 取り除く 2: 除かない )
    void set_remove_old_abone_thread( const int remove ); 

    const int get_abone_number_thread();
    void set_abone_number_thread( const int number );

    const int get_abone_hour_thread();
    void set_abone_hour_thread( const int hour );

    // articleviewでのレスの全体あぼーん
    const std::list< std::string >& get_list_abone_name(); // 名前
    const std::list< std::string >& get_list_abone_word(); // ワード
    const std::list< std::string >& get_list_abone_regex(); // 正規表現

    void set_list_abone_name( const std::list< std::string >& name );
    void set_list_abone_word( const std::list< std::string >& word );
    void set_list_abone_regex( const std::list< std::string >& regex );

    // デフォルトで透明、連鎖あぼーんをするか
    const bool get_abone_transparent();
    void set_abone_transparent( const bool set );

    const bool get_abone_chain();
    void set_abone_chain( const bool set );

    // 右ペーンが空の時にサイドバーを閉じるか
    const bool get_expand_sidebar();

    // 3ペーン時にスレ一覧やスレビューを最大化するか
    const bool get_expand_rpane();

    // 次スレ検索の類似度のしきい値
    const int get_threshold_next();

    // 次スレを開いたときにお気に入りのアドレスと名前を自動更新
    const int get_replace_favorite_next();
    void set_replace_favorite_next( const int mode );

    // お気に入りの自動更新をするかダイアログを出す
    const bool show_diag_replace_favorite();
    void set_show_diag_replace_favorite( const bool show );

    // スレをお気に入りに追加したときにしおりをセットする
    const bool get_bookmark_drop();

    // お気に入りの更新チェック時に板の更新もチェックする
    const bool get_check_update_board();

    // 起動時にお気に入りを自動でチェックする
    const bool get_check_update_boot();

    // Ctrl+qでウィンドウを閉じない
    const bool get_disable_close();

    // まちBBSの取得に offlaw.cgi を使用する
    const bool get_use_machi_offlaw();
    void set_use_machi_offlaw( const bool set );

    // 書き込み履歴のあるスレを削除する時にダイアログを表示
    const bool get_show_del_written_thread_diag();
    void set_del_written_thread_diag( const bool set );

    // FIFOの作成などにエラーがあったらダイアログを表示する
    const bool get_show_diag_fifo_error();
    void set_show_diag_fifo_error( const bool set );

#ifdef HAVE_MIGEMO_H
    // migemo-dictの場所
    const std::string& get_migemodict_path();
#endif
}


#endif
