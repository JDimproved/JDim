// ライセンス: GPL2

//
// データベースへのインターフェース関数
//

#ifndef _INTERFACE_H
#define _INTERFACE_H

#include "etcboardinfo.h"

#include <string>
#include <list>
#include <vector>
#include <ctime>
#include <unordered_set>


namespace XML
{
    class Document;
}

namespace DBTREE
{
    class Root;
    class BoardBase;
    class NodeTreeBase;
    class ArticleBase;

    void create_root();
    void delete_root();

    // 各クラスのポインタ取得
    Root* get_root();
    BoardBase* get_board( const std::string& url );
    ArticleBase* get_article( const std::string& url );

    // urlの変換関係
    std::string url_subject( const std::string& url ); // 板の subject.txt の URL
    std::string url_root( const std::string& url );
    std::string url_boardbase( const std::string& url );
    std::string url_datbase( const std::string& url );

    // dat型のurlに変換
    std::string url_dat( const std::string& url, int& num_from, int& num_to, std::string& num_str );

    // dat型のurlに変換(簡易版)
    std::string url_dat( const std::string& url );

    // read.cgi型のurlに変換
    std::string url_readcgi( const std::string& url, int num_from, int num_to );

    std::string url_bbscgibase( const std::string& url );
    std::string url_subbbscgibase( const std::string& url );
    std::string url_bbscgi( const std::string& url );
    std::string url_subbbscgi( const std::string& url );
    std::string url_bbscgi_new( const std::string& url );
    std::string url_subbbscgi_new( const std::string& url );

    // 板が移転したかチェックする
    // 移転した時は移転後のURLを返す
    std::string is_board_moved( const std::string& url );

    std::string is_board_moved( const std::string& url,
                                std::string& old_root,
                                std::string& old_path_board,
                                std::string& new_root,
                                std::string& new_path_board );

    // 板移転
    bool move_board( const std::string& url_old, const std::string& url_new );

    // 板移転情報の保存の有効切り替え
    void set_enable_save_movetable( const bool set );

    // 移転情報保存
    void save_movetable();

    // bbslist系
    const XML::Document& get_xml_document();
    const std::list< DBTREE::ETCBOARDINFO >& get_etcboards();
    bool add_etc( const std::string& url, const std::string& name, const std::string& basicauth, const std::string& id );
    bool move_etc( const std::string& url_old, const std::string& url_new,
                   const std::string& name_old, const std::string& name_new,
                   const std::string& basicauth, const std::string& boardid );
    bool remove_etc( const std::string& url, const std::string& name );
    void save_etc();
    void download_bbsmenu();
    std::string get_date_modified(); // bbsmenuの更新時間( 文字列 )
    time_t get_time_modified(); // bbsmenuの更新時間( time_t )

    // board 系
    std::string board_path( const std::string& url );
    std::string board_id( const std::string& url );
    time_t board_time_modified( const std::string& url ); // 板の更新時間( time_t )
    std::string board_date_modified( const std::string& url ); // 板の更新時間( 文字列 )
    void board_set_date_modified( const std::string& url, const std::string& date ); // 板の更新時間( 文字列 )をセット
    const std::string& board_get_modified_localrule( const std::string& url );
    void board_set_modified_localrule( const std::string& url, const std::string& modified );
    const std::string& board_get_modified_setting( const std::string& url );
    void board_set_modified_setting( const std::string& url, const std::string& modified );
    std::string board_name( const std::string& url );
    std::string board_subjecttxt( const std::string& url );
    std::string board_charset( const std::string& url );
    std::string board_cookie_for_write( const std::string& url );
    const std::list< std::string >& board_list_cookies_for_write( const std::string& url );
    void board_set_list_cookies_for_write( const std::string& url, const std::list< std::string>& list_cookies );
    void board_reset_list_cookies_for_write( const std::string& url );
    std::string board_keyword_for_write( const std::string& url );
    void board_set_keyword_for_write( const std::string& url, const std::string& keyword );
    void board_analyze_keyword_for_write( const std::string& url, const std::string& html );
    std::string board_basicauth( const std::string& url );
    std::string board_ext( const std::string& url );
    int board_status( const std::string& url );
    int board_code( const std::string& url );
    std::string board_str_code( const std::string& url );
    void board_save_info( const std::string& url );
    void board_download_subject( const std::string& url, const std::string& url_update_view );
    void board_read_subject_from_cache( const std::string& url );
    bool board_is_loading( const std::string& url );
    void board_stop_load( const std::string& url );
    std::vector< DBTREE::ArticleBase* >& board_list_subject( const std::string& url );
    int board_view_sort_column( const std::string& url );
    void board_set_view_sort_column( const std::string& url, int column );
    int board_view_sort_mode( const std::string& url );
    void board_set_view_sort_mode( const std::string& url, int mode );
    int board_view_sort_pre_column( const std::string& url );
    void board_set_view_sort_pre_column( const std::string& url, int column );
    int board_view_sort_pre_mode( const std::string& url );
    void board_set_view_sort_pre_mode( const std::string& url, int mode );
    bool board_check_noname( const std::string& url );
    void board_set_check_noname( const std::string& url, const bool check );
    bool board_show_oldlog( const std::string& url );
    void board_set_show_oldlog( const std::string& url, const bool show );
    int board_get_mode_local_proxy( const std::string& url );
    const std::string& board_get_local_proxy( const std::string& url );
    int board_get_local_proxy_port( const std::string& url );
    const std::string& board_get_local_proxy_basicauth( const std::string& url );
    void board_set_mode_local_proxy( const std::string& url, int mode );
    void board_set_local_proxy( const std::string& url, const std::string& proxy );
    void board_set_local_proxy_port( const std::string& url, int port );
    int board_get_mode_local_proxy_w( const std::string& url );
    const std::string& board_get_local_proxy_w( const std::string& url );
    int board_get_local_proxy_port_w( const std::string& url );
    const std::string& board_get_local_proxy_basicauth_w( const std::string& url );
    void board_set_mode_local_proxy_w( const std::string& url, int mode );
    void board_set_local_proxy_w( const std::string& url, const std::string& proxy );
    void board_set_local_proxy_port_w( const std::string& url, int port );
    const std::string& board_get_write_name( const std::string& url );
    const std::string& board_get_write_mail( const std::string& url );
    void board_set_write_name( const std::string& url, const std::string& name );
    void board_set_write_mail( const std::string& url, const std::string& mail );
    void board_update_writetime( const std::string& url );
    time_t board_write_time( const std::string& url );
    time_t board_write_pass( const std::string& url );
    time_t board_samba_sec( const std::string& url );
    void board_set_samba_sec( const std::string& url, time_t sec );
    time_t board_write_leftsec( const std::string& url );

    // 更新可能状態にしてお気に入りやスレ一覧のタブのアイコンに更新マークを表示
    // update == true の時に表示。falseなら戻す
    void board_show_updateicon( const std::string& url, const bool update );

    // 板の更新チェック時に、更新チェックを行うスレのアドレスのリスト
    // キャッシュが存在し、かつdat落ちしていないで新着数が0のスレを速度の順でソートして返す
    std::list< std::string > board_get_check_update_articles( const std::string& url );

    // datファイルのインポート
    // 成功したらdat型のurlを返す
    std::string board_import_dat( const std::string& url, const std::string& filename );

    // 各板に属する全スレの書き込み履歴のリセット
    void board_clear_all_post_history( const std::string& url );

    int board_get_number_max_res( const std::string& url );
    void board_set_number_max_res( const std::string& url, const int number );

    // datの最大サイズ(Kバイト)
    int board_get_max_dat_lng( const std::string& url );

    time_t board_get_live_sec( const std::string& url );
    void board_set_live_sec( const std::string& url, time_t sec );
    time_t board_last_access_time( const std::string& url );

    // 全スレの書き込み履歴のリセット
    void clear_all_post_history();

    // 全板の情報ファイル読み込み
    void read_boardinfo_all();

    // キャッシュ内のログ検索
    // ArticleBase のアドレスをリスト(list_article)にセットして返す
    // query が空の時はキャッシュにあるログを全てヒットさせる
    // bm がtrueの時、しおりが付いている(スレ一覧でしおりを付けた or レスに一つでもしおりが付いている)スレのみを対象に検索する
    void search_cache_all( std::vector< DBTREE::ArticleBase* >& list_article,
                           const std::string& query, const bool mode_or, const bool bm, const bool stop );
    void search_cache( const std::string& url, std::vector< DBTREE::ArticleBase* >& list_article,
                       const std::string& query, const bool mode_or, const bool bm, const bool stop );

    // article 系
    bool article_is_cached( const std::string& url ); // キャッシュにあるかどうか
    std::string article_id( const std::string& url ); // 拡張子込み "12345.dat" みたいに
    std::string article_key( const std::string& url ); // idから拡張子を取ったもの。書き込み用
    std::string article_org_host( const std::string& url ); // 移転する前のオリジナルのURL
    time_t article_since_time( const std::string& url );
    std::string article_since_date( const std::string& url );
    time_t article_time_modified( const std::string& url ); // スレの更新時間( time_t )
    std::string article_date_modified( const std::string& url ); // スレの更新時間( 文字列 )
    void article_set_date_modified( const std::string& url, const std::string& date ); // スレの更新時間( 文字列 )をセット
    int article_hour( const std::string& url );
    time_t article_write_time( const std::string& url );
    std::string article_write_date( const std::string& url );
    int article_status( const std::string& url );
    int article_code( const std::string& url );
    std::string article_str_code( const std::string& url );
    std::string article_ext_err( const std::string& url );
    std::string article_subject( const std::string& url );
    int article_number( const std::string& url );
    int article_number_load( const std::string& url );
    int article_number_seen( const std::string& url );
    void article_set_number_seen( const std::string& url, int seen );
    int article_number_new( const std::string& url );
    bool article_is_loading( const std::string& url );
    bool article_is_checking_update( const std::string& url );
    void article_download_dat( const std::string& url, const bool check_update );
    void article_set_url_pre_article( const std::string& url, const std::string& url_pre_article );
    void article_copy_article_info( const std::string& url, const std::string& url_src );
    void article_stop_load( const std::string& url );
    int article_get_speed( const std::string& url );

    // 書き込み履歴のリセット
    void article_clear_post_history( const std::string& url );

    // キャッシュ削除
    // cache_only == true の時はキャッシュだけ削除してスレ情報は消さない
    void delete_article( const std::string& url, const bool cache_only );

    // キャッシュ保存
    bool article_save_dat( const std::string& url, const std::string& path_to );

    // 全スレ情報の保存
    void save_articleinfo_all();

    void article_update_writetime( const std::string& url );
    size_t article_lng_dat( const std::string& url );

    // ユーザーエージェント
    const std::string& get_agent( const std::string& url );   // ダウンロード用
    const std::string& get_agent_w( const std::string& url ); // 書き込み用

    // 読み込み用プロキシ
    std::string get_proxy_host( const std::string& url );
    int get_proxy_port( const std::string& url );
    std::string get_proxy_basicauth( const std::string& url );

    // 書き込み用プロキシ
    std::string get_proxy_host_w( const std::string& url );
    int get_proxy_port_w( const std::string& url );
    std::string get_proxy_basicauth_w( const std::string& url );

    // ローカルルール
    std::string localrule( const std::string& url );

    // setting.txt
    std::string settingtxt( const std::string& url );

    // 書き込み関係

    // デフォルトの名無し名
    std::string default_noname( const std::string& url );

    // 最大改行数/2
    int line_number( const std::string& url );

    // 最大書き込みバイト数
    int message_count( const std::string& url );

    // 特殊文字書き込み可能か( pass なら可能、 change なら不可 )
    std::string get_unicode( const std::string& url );

    // 書き込み時の名前とメール
    const std::string& write_name( const std::string& url );
    void set_write_name( const std::string& url, const std::string& str );
    bool write_fixname( const std::string& url );
    void set_write_fixname( const std::string& url, bool set );

    const std::string& write_mail( const std::string& url );
    void set_write_mail( const std::string& url, const std::string& str );
    bool write_fixmail( const std::string& url );
    void set_write_fixmail( const std::string& url, bool set );

    // ポストするメッセージの作成
    std::string create_write_message( const std::string& url, const std::string& name, const std::string& mail,
                                      const std::string& msg );
    std::string create_newarticle_message( const std::string& url, const std::string& subject,
                                           const std::string& name, const std::string& mail,
                                           const std::string& msg );

    // 書き込み時のリファラ
    std::string get_write_referer( const std::string& url );

    // あぼーん関係

    // 板レベルでのあぼーん情報
    // グローバルなあぼーん情報は globalconf が管理
    const std::list< std::string >& get_abone_list_id_board( const std::string& url );
    const std::list< std::string >& get_abone_list_name_board( const std::string& url );
    const std::list< std::string >& get_abone_list_word_board( const std::string& url );
    const std::list< std::string >& get_abone_list_regex_board( const std::string& url );

    // 板レベルでのあぼーん状態のリセット(情報セットとスレビューの表示更新を同時におこなう)
    void reset_abone_board( const std::string& url,
                            const std::list< std::string >& ids,
                            const std::list< std::string >& names,
                            const std::list< std::string >& words,
                            const std::list< std::string >& regexs );

    // 板レベルでのあぼ〜ん状態更新(reset_abone()と違って各項目ごと個別におこなう。スレビューの表示更新も同時におこなう)
    void add_abone_id_board( const std::string& url, const std::string& id );
    void add_abone_name_board( const std::string& url, const std::string& name );
    void add_abone_word_board( const std::string& url, const std::string& word );

    // スレあぼーん情報
    // グローバルなあぼーん情報は globalconf が管理
    const std::list< std::string >& get_abone_list_thread( const std::string& url );
    const std::list< std::string >& get_abone_list_thread_remove( const std::string& url );
    const std::list< std::string >& get_abone_list_word_thread( const std::string& url );
    const std::list< std::string >& get_abone_list_regex_thread( const std::string& url );
    const std::unordered_set< int >& get_abone_reses( const std::string& url );
    int get_abone_number_thread( const std::string& url );
    int get_abone_hour_thread( const std::string& url );

    // subject.txtのロード後にdat落ちしたスレッドをスレあぼーんのリストから取り除く
    void remove_old_abone_thread( const std::string& url );

    // スレあぼーん情報を更新した時、全boardbaseクラスに対応するスレ一覧の表示を更新させる
    // CONFIG::set_abone_number_thread() などでグローバル設定をした後などに呼び出す
    void update_abone_thread();

    // スレあぼーん状態のリセット
    // redraw : スレ一覧の表示更新を行う
    void reset_abone_thread( const std::string& url,
                             const std::list< std::string >& threads,
                             const std::list< std::string >& words,
                             const std::list< std::string >& regexs,
                             const int number, const int hour, const bool redraw );

    //
    // 各articlebase別のあぼーん情報
    //

    // 全articlebaseクラスのあぼーん状態の更新
    void update_abone_all_article();

    // 全articlebaseクラスの書き込み時間とスレ立て時間の文字列をリセット
    void reset_all_since_date();
    void reset_all_write_date();
    void reset_all_access_date();

    // レスあぼーん
    // グローバルなあぼーん情報は globalconf が管理
    const std::list< std::string >& get_abone_list_id( const std::string& url );
    const std::list< std::string >& get_abone_list_name( const std::string& url );
    const std::list< std::string >& get_abone_list_word( const std::string& url );
    const std::list< std::string >& get_abone_list_regex( const std::string& url );

    // 全あぼーん情報の同時セットと更新
    void reset_abone( const std::string& url,
                      const std::list< std::string >& ids,
                      const std::list< std::string >& names,
                      const std::list< std::string >& words,
                      const std::list< std::string >& regexs,
                      const std::vector< char >& vec_abone_res,
                      const bool transparent, const bool chain, const bool age,
                      const bool board, const bool global
        );

    // 個別のあぼーん情報のセットと更新
    void set_abone_res( const std::string& url, const int num_from, const int num_to, const bool set );
    void add_abone_id( const std::string& url, const std::string& id );
    void add_abone_name( const std::string& url, const std::string& name );
    void add_abone_word( const std::string& url, const std::string& word );

    // 透明あぼーん
    bool get_abone_transparent( const std::string& url );
    void set_abone_transparent( const std::string& url, const bool set );

    // 連鎖あぼーん
    bool get_abone_chain( const std::string& url );
    void set_abone_chain( const std::string& url, const bool set );

    // ageあぼーん
    bool get_abone_age( const std::string& url );
    void set_abone_age( const std::string& url, const bool set );

    // 板レベルでのあぼーん
    bool get_abone_board( const std::string& url );
    void set_abone_board( const std::string& url, const bool set );

    // 全体レベルでのあぼーん
    bool get_abone_global( const std::string& url );
    void set_abone_global( const std::string& url, const bool set );

    //　ブックマーク関係

    // スレのブックマーク
    bool is_bookmarked_thread( const std::string& url );
    void set_bookmarked_thread( const std::string& url, const bool bookmarked );

    // レスのブックマーク
    int get_num_bookmark( const std::string& url );
    bool is_bookmarked( const std::string& url, const int number );
    void set_bookmark( const std::string& url, const int number, const bool set );
}

#endif
