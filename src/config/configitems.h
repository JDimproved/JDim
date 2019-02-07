// ライセンス: GPL2
//
// 設定項目クラス
//

#ifndef _CONFIGITEMS_H
#define _CONFIGITEMS_H

#include <string>
#include <list>
#include <vector>

namespace JDLIB
{
    class ConfLoader;
}


namespace CONFIG
{
    class ConfigItems
    {
        // 設定をロード済み
        bool m_loaded;

    public:

        // 前回開いたviewを復元するか
        bool restore_board;
        bool restore_article;
        bool restore_image;

        // 自前でウィンドウ配置を管理する
        bool manage_winpos;

        // フォント
        std::vector< std::string > fontname;

        // レスを参照するときに前に付ける文字
        std::string ref_prefix;

        // ref_prefix の後のスペースの数
        int ref_prefix_space;
        std::string ref_prefix_space_str;

        // レスにアスキーアートがあると判定する正規表現
        std::string regex_res_aa;
        bool aafont_enabled;

        // キャッシュのルートディレクトリ
        // 旧バージョンとの互換のため残しているだけで使用していない
        std::string path_cacheroot;

        // 読み込み用プロクシとポート番号
        bool use_proxy_for2ch;
        std::string proxy_for2ch;
        int proxy_port_for2ch;
        std::string proxy_basicauth_for2ch;

        // 書き込み用プロクシとポート番号
        bool use_proxy_for2ch_w;
        std::string proxy_for2ch_w;
        int proxy_port_for2ch_w;
        std::string proxy_basicauth_for2ch_w;

        // 2chの外にアクセスするときのプロクシとポート番号
        bool use_proxy_for_data;
        std::string proxy_for_data;
        int proxy_port_for_data;
        std::string proxy_basicauth_for_data;

        // 2ch にアクセスするときのエージェント名
        std::string agent_for2ch;

        // 2ch外にアクセスするときのエージェント名
        std::string agent_for_data;

        // 2ch にログインするときのX-2ch-UA
        std::string x_2ch_ua;

        // ローダのバッファサイズ
        int loader_bufsize; // 一般
        int loader_bufsize_board; // スレ一覧用

        // ローダのタイムアウト値
        int loader_timeout;
        int loader_timeout_post;
        int loader_timeout_img;
        int loader_timeout_checkupdate;

        // ipv6使用
        bool use_ipv6;

        // 同一ホストに対する最大コネクション数( 1 または 2 )
        int connection_num;

        // 2chのクッキー:HAPを保存する
        bool use_cookie_hap;

        // 2chのクッキー:HAP
        std::string cookie_hap;
        std::string cookie_hap_bbspink;

        // 2chの過去ログ取得時にofflaw2を使用する
        bool use_offlaw2_2ch;

        // リンクをクリックしたときに実行するコマンド
        std::string command_openurl;

        // ブラウザ設定ダイアログのコンボボックスの番号
        int browsercombo_id;

        // レス番号の上にマウスオーバーしたときに参照ポップアップ表示する
        bool refpopup_by_mo;

        // 名前の上にマウスオーバーしたときにポップアップ表示する
        bool namepopup_by_mo;

        // IDの上にマウスオーバーしたときにIDをポップアップ表示する
        bool idpopup_by_mo;

        // 画像のスムージングレベル(0-2, 2が最も高画質かつ低速)
        int imgemb_interp;
        int imgmain_interp;
        int imgpopup_interp;

        // 画像ポップアップサイズ
        int imgpopup_width;
        int imgpopup_height;

        // 画像ポップアップを使用する
        bool use_image_popup;

        // 画像ビューを使用する
        bool use_image_view;

        // インライン画像を表示する
        bool use_inline_image;

        // ssspアイコン表示
        bool show_ssspicon;

        // インライン画像の最大幅と高さ
        int embimg_width;
        int embimg_height;

        // 埋め込み画像ビューを閉じたときにタブも閉じる
        bool hide_imagetab;

        // 画像ビューでdeleteを押したときに確認ダイアログを表示する
        bool show_delimgdiag;

        // 画像にモザイクかける
        bool use_mosaic;

        // モザイクの大きさ
        // 画像を一度mosaic_sizeまで縮めてから表示する
        // 画像のサイズがmosaic_sizeより小さい場合はモザイクをかけない
        int mosaic_size;

        // 画像をデフォルトでウィンドウサイズに合わせる
        bool zoom_to_fit;

        // 画像キャッシュ削除の日数
        int del_img_day;

        // 画像あぼーん削除の日数
        int del_imgabone_day;

        // ダウンロードする画像の最大ファイルサイズ(Mbyte)        
        int max_img_size;

        // 画像の最大サイズ(Mピクセル)
        int max_img_pixel;

        // 画像のメモリキャッシュ枚数
        int imgcache_size;

        // JD ホームページのアドレス
        std::string url_jdhp;

        // JDim ホームページのアドレス
        std::string url_jdimhp;

        // 2chの認証サーバのアドレス
        std::string url_login2ch;

        // p2の認証サーバのアドレス
        std::string url_loginp2;

        // BEの認証サーバのアドレス
        std::string url_loginbe;

        // bbsmenu.htmlのURL
        std::string url_bbsmenu;

        // bbsmenu.htmlの内にあるリンクは全て板とみなす
        bool use_link_as_board;

        // 板移転時に確認ダイアログを表示する
        bool show_movediag;

        // スレタイ検索用メニュータイトルアドレス
        std::string menu_search_title;
        std::string url_search_title;

        // スレタイ検索用正規表現
        std::string regex_search_title;

        // web検索用メニュータイトルアドレス
        std::string menu_search_web;
        std::string url_search_web;

        // p2 書き込み用アドレス
        std::string url_writep2;
        std::string url_resp2;

        // 色
        std::vector< std::string > str_color;

        // 書き込みビューでGTKテーマの設定を使用するか (GTK3版のみ)
        bool use_message_gtktheme;

        // ツリービューでgtkrcの設定を使用するか
        bool use_tree_gtkrc;

        // スレビューの選択色でgtkrcの設定を使用するか
        bool use_select_gtkrc;

        // ツリービューの行間スペース
        int tree_ypad;

        // ツリービューにエクスパンダを表示
        bool tree_show_expanders;

        // ツリービューのレベルインデント調整量(ピクセル)
        int tree_level_indent;

        // カテゴリを開いたときにツリービューをスクロールする
        bool scroll_tree;

        // ツリービューの選択を表示中のビューと同期する ( 0: 同期しない 1: 同期する 2: 同期する(フォルダを開く) )
        int select_item_sync;

        // 各ビューと枠との間の余白
        int view_margin;

        // スクロールバーを左に配置
        bool left_scrbar;

        // スレ一覧で古いスレも表示 	 
        bool show_oldarticle;

        // スレ一覧で指定した値(時間)よりも後に立てられたスレを新着とみなす
        int newthread_hour;

        // スレ一覧でインクリメント検索をする
        bool inc_search_board;

        // スレ一覧でdeleteを押したときに確認ダイアログを表示する
        bool show_deldiag;

        // スレ一覧をロードする前にキャッシュにある一覧を表示
        bool show_cached_board;

        // スレ一覧でお知らせスレ(924)のアイコンを表示する
        bool show_924;

        // ツリービューのスクロール量(マウスホイール上下・行数)
        int tree_scroll_size;

        // スレビューのスクロール量(マウスホイール上下・行数)
        int scroll_size;

        // スレビューのスクロール量(キー上下・行数)
        int key_scroll_size;

        // スレビューの高速スクロール量(キー上下・ ページ高 - 行高 * key_fastscroll_size )
        int key_fastscroll_size;

        // スレビューでリロード後に一番下までスクロール
        bool jump_after_reload;

        // スレビューでリロード後に新着までスクロール
        bool jump_new_after_reload;

        // 実況モード
        int live_mode;

        // 実況速度
        int live_speed;

        // 実況のスクロールモードを切り替えるしきい値
        int live_threshold;

        // 板一覧でカテゴリを常にひとつだけ開く
        bool open_one_category;

        // お気に入りでカテゴリを常にひとつだけ開く
        bool open_one_favorite;

        // デフォルトの書き込み名
        std::string write_name;

        // デフォルトのメールアドレス
        std::string write_mail;

        // 書き込み時に書き込み確認ダイアログを出さない
        bool always_write_ok;

        // 書き込みログを保存
        bool save_postlog;

        // 書き込みログの最大サイズ
        int maxsize_postlog;

        // 書き込み履歴を保存
        bool save_posthist;

        // 「書き込み中」のダイアログを表示しない
        bool hide_writing_dialog;

        // 編集中のメッセージの保存確認ダイアログを表示する
        bool show_savemsgdiag;

        // 書き込みビューでテキストを折り返す
        bool message_wrap;

        // 非アクティブ時に書き込みビューを折りたたむ
        bool fold_message;

        // 非アクティブ時に画像ビューを折りたたむ
        bool fold_image;

        // 書き込み欄の日本語のON/OFF状態を保存
        bool keep_im_status;

        // ポップアップとカーソルの間のマージン
        int margin_popup;

        // 画像ポップアップとカーソルの間のマージン
        int margin_imgpopup_x;  // 水平方向
        int margin_imgpopup; // 垂直方向

        // ポップアップが消えるまでの時間(ミリ秒)
        int hide_popup_msec;

        // マウスジェスチャを有効
        bool enable_mg;

        // マウスジェスチャの判定開始半径
        int mouse_radius;

        // 数字入力ジャンプの待ち時間(ミリ秒)
        int numberjmp_msec;

        // 履歴メニューの表示数
        int history_size;

        // 履歴ビューの表示数
        int historyview_size;

        // AA履歴の保持数
        int aahistory_size;

        // 0以上なら多重ポップアップの説明を表示する
        int instruct_popup;

        // スレビューを開いたときにスレ一覧との切り替え方法を説明する
        bool instruct_tglart;
        bool instruct_tglart_end;

        // 画像ビューを開いたときにスレビューとの切り替え方法を説明する        
        bool instruct_tglimg;
        bool instruct_tglimg_end;

        // スレビューでdeleteを押したときに確認ダイアログを表示する
        bool show_delartdiag;

        // 下線位置
        double adjust_underline_pos;

        // 行間スペース
        double adjust_line_space;

        // リンク下線を表示
        bool draw_underline;

        // スレビューで文字幅の近似を厳密にする        
        bool strict_char_width;

        // スレビューで発言数(ID)をカウントする
        bool check_id;

        // レス参照で色を変える回数
        int num_reference_high;
        int num_reference_low;

        // 発言数で色を変える回数
        int num_id_high;
        int num_id_low;

        // datのパース時にURL判定を甘くする(^なども含める)
        bool loose_url;

        // ユーザーコマンドで選択できない項目を非表示にする
        bool hide_usrcmd;

        // スレビューで再読み込みボタンを押したときに全タブを更新する
        bool reload_allthreads;

        // タブに表示する文字列の最小値        
        int tab_min_str;

        // タブにアイコンを表示するか
        bool show_tab_icon;

        // タブ上でマウスホイールを回転してタブを切り替える
        bool switchtab_wheel;

        // 他のビューを開くときのタブの位置 ( 0: 一番右端 1:右隣 2:左隣 )
        int newtab_pos;

        // ツリービューで選択したビューを開くときのタブの位置 ( 0: 一番右端 1:右隣 2:左隣 )
        int opentab_pos;

        // 次スレ検索を開くときのタブの位置 ( 0: 次スレ検索タブ 1:新しいタブ 2:アクティブなタブを置き換え )
        int boardnexttab_pos;

        // スレビューに書き込みマークを表示するか
        bool show_post_mark;

        // ボタンをフラットにするか
        bool flat_button;

        // ツールバーの背景描画
        bool draw_toolbarback;

        // スレ あぼーん word
        std::list< std::string > list_abone_word_thread;

        // スレ あぼーん regex
        std::list< std::string > list_abone_regex_thread;

        // dat落ちしたスレをNGスレタイトルリストから取り除くか( 0: ダイアログ表示 1: 取り除く 2: 除かない )
        int remove_old_abone_thread;

        // スレ あぼーん レス数
        int abone_number_thread;

        // スレ あぼーん スレ立てからの経過時間
        int abone_hour_thread;

        // あぼーん name
        std::list< std::string > list_abone_name;

        // あぼーん word
        std::list< std::string > list_abone_word;

        // あぼーん regex        
        std::list< std::string > list_abone_regex;

        // デフォルトで透明、連鎖あぼーんをするか
        bool abone_transparent;
        bool abone_chain;

        // NG正規表現によるあぼーん時に大小と全半角文字の違いを無視
        bool abone_icase;
        bool abone_wchar;

        // 右ペーンが空の時にサイドバーを閉じるか
        bool expand_sidebar;

        // 3ペーン時にスレ一覧やスレビューを最大化するか
        bool expand_rpane;

        // ペーンの境界をクリックしてサイドバーを開け閉めする
        bool open_sidebar_by_click;

        // 次スレ検索の類似度のしきい値
        int threshold_next;

        // 次スレを開いたときにお気に入りのアドレスと名前を自動更新
        int replace_favorite_next;

        // お気に入りの自動更新をするかダイアログを出す
        bool show_diag_replace_favorite;

        // スレをお気に入りに追加したときにしおりをセットする
        bool bookmark_drop;

        // お気に入りの更新チェック時に板の更新もチェックする
        bool check_update_board;

        // 起動時にお気に入りを自動でチェックする
        bool check_update_boot;

        // お気に入り登録時に重複項目を登録するか ( 0: 登録する 1: ダイアログ表示  2: 登録しない )
        int check_favorite_dup;

        // お気に入り登録時に挿入先ダイアログを表示する ( 0 : 表示する 1: 表示せず先頭に追加 2: 表示せず最後に追加 )
        int show_favorite_select_diag;

        // Ctrl+qでウィンドウを閉じない
        bool disable_close;

        // メニューバーを非表示にした時にダイアログを表示
        bool show_hide_menubar_diag;

        // 状態変更時にメインステータスバーの色を変える
        bool change_stastatus_color;

        // まちBBSの取得に offlaw.cgi を使用する
        bool use_machi_offlaw;

        // 書き込み履歴のあるスレを削除する時にダイアログを表示
        bool show_del_written_thread_diag;

        // スレを削除する時に画像キャッシュも削除する ( 0: ダイアログ表示 1: 削除 2: 削除しない )
        int delete_img_in_thread;

        // FIFOの作成などにエラーがあったらダイアログを表示する
        bool show_diag_fifo_error;

        // 指定した分ごとにセッションを自動保存 (0: 保存しない)
        int save_session;

#ifdef HAVE_MIGEMO_H
        // migemo-dictの場所
        std::string migemodict_path;
#endif

        /////////////////////////


        ConfigItems();
        virtual ~ConfigItems() noexcept;

        // 設定読み込み
        bool load( const bool restore = false );

        // 保存
        void save();

        // フォントのリセット
        void reset_fonts();

        // 色のリセット
        void reset_colors();

        // プロクシ設定
        void set_proxy_for2ch( const std::string& proxy );
        void set_proxy_for2ch_w( const std::string& proxy );
        void set_proxy_for_data( const std::string& proxy );

      private:

        void save_impl( const std::string& path );

        void set_fonts( JDLIB::ConfLoader& cf );
        void set_colors( JDLIB::ConfLoader& cf );
    };
    
}

#endif
