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


namespace XML
{
    class Document;
};

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
    const std::string url_subject( const std::string& url ); // 板の subject.txt の URL
    const std::string url_root( const std::string& url );
    const std::string url_boardbase( const std::string& url );
    const std::string url_datbase( const std::string& url );

    // dat型のurlに変換
    const std::string url_dat( const std::string& url, int& num_from, int& num_to, std::string& num_str ); 

    // dat型のurlに変換(簡易版)
    const std::string url_dat( const std::string& url ); 

    // read.cgi型のurlに変換
    const std::string url_readcgi( const std::string& url, int num_from, int num_to );

    const std::string url_bbscgibase( const std::string& url );
    const std::string url_subbbscgibase( const std::string& url );
    const std::string url_bbscgi( const std::string& url );
    const std::string url_subbbscgi( const std::string& url );
    const std::string url_bbscgi_new( const std::string& url );
    const std::string url_subbbscgi_new( const std::string& url );

    // 板が移転したかチェックする
    // 移転した時は移転後のURLを返す
    const std::string is_board_moved( const std::string& url );

    const std::string is_board_moved( const std::string& url,
                                        std::string& old_root,
                                        std::string& old_path_board,
                                        std::string& new_root,
                                        std::string& new_path_board
        );

    // 板移転
    const bool move_board( const std::string& url_old, const std::string& url_new );

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
    
    // board 系
    const std::string board_path( const std::string& url );
    const std::string board_id( const std::string& url );
    const time_t board_time_modified( const std::string& url ); // 板の更新時間( time_t )
    const std::string board_date_modified( const std::string& url ); // 板の更新時間( 文字列 )
    void board_set_date_modified( const std::string& url, const std::string& date ); // 板の更新時間( 文字列 )をセット
    const std::string& board_get_modified_localrule( const std::string& url );
    void board_set_modified_localrule( const std::string& url, const std::string& modified );
    const std::string& board_get_modified_setting( const std::string& url );
    void board_set_modified_setting( const std::string& url, const std::string& modified );
    const std::string board_name( const std::string& url );
    const std::string board_subjecttxt( const std::string& url );
    const std::string board_charset( const std::string& url );
    const std::string board_cookie_for_write( const std::string& url );
    const std::list< std::string >& board_list_cookies_for_write( const std::string& url );
    void board_set_list_cookies_for_write( const std::string& url, const std::list< std::string>& list_cookies );        
    const std::string board_hana_for_write( const std::string& url );
    void board_set_hana_for_write( const std::string& url, const std::string& hana );
    const std::string board_basicauth( const std::string& url );
    const std::string board_ext( const std::string& url );
    const int board_code( const std::string& url );
    const std::string board_str_code( const std::string& url );
    void board_save_info( const std::string& url );
    void board_download_subject( const std::string& url );
    void board_stop_load( const std::string& url );
    std::list< DBTREE::ArticleBase* >& board_list_subject( const std::string& url );
    const int board_view_sort_column( const std::string& url );
    void board_set_view_sort_column( const std::string& url, int column );
    const int board_view_sort_mode( const std::string& url );
    void board_set_view_sort_mode( const std::string& url, int mode );
    const int board_view_sort_pre_column( const std::string& url );
    void board_set_view_sort_pre_column( const std::string& url, int column );
    const int board_view_sort_pre_mode( const std::string& url );
    void board_set_view_sort_pre_mode( const std::string& url, int mode );
    const bool board_check_noname( const std::string& url );
    void board_set_check_noname( const std::string& url, bool check );
    const int board_get_mode_local_proxy( const std::string& url );
    const std::string& board_get_local_proxy( const std::string& url );
    const int board_get_local_proxy_port( const std::string& url );
    void board_set_mode_local_proxy( const std::string& url, int mode );
    void board_set_local_proxy( const std::string& url, const std::string& proxy );
    void board_set_local_proxy_port( const std::string& url, int port );
    const int board_get_mode_local_proxy_w( const std::string& url );
    const std::string& board_get_local_proxy_w( const std::string& url );
    const int board_get_local_proxy_port_w( const std::string& url );
    void board_set_mode_local_proxy_w( const std::string& url, int mode );
    void board_set_local_proxy_w( const std::string& url, const std::string& proxy );
    void board_set_local_proxy_port_w( const std::string& url, int port );
    const std::string& board_get_write_name( const std::string& url );
    const std::string& board_get_write_mail( const std::string& url );
    void board_set_write_name( const std::string& url, const std::string& name );
    void board_set_write_mail( const std::string& url, const std::string& mail );
    void board_update_writetime( const std::string& url );
    const time_t board_write_time( const std::string& url );
    const time_t board_write_pass( const std::string& url );
    const time_t board_samba_sec( const std::string& url );
    void board_set_samba_sec( const std::string& url, time_t sec );
    time_t board_write_leftsec( const std::string& url );
    const int board_get_number_max_res( const std::string& url );
    void board_set_number_max_res( const std::string& url, const int number );
    const time_t board_get_live_sec( const std::string& url );
    void board_set_live_sec( const std::string& url, time_t sec );
    const time_t board_last_access_time( const std::string& url );


    // 全板の情報ファイル読み込み
    void read_boardinfo_all();

    // キャッシュ内のログ検索
    // datファイルのURL(read.cgi型)を返す
    std::list< std::string > search_cache_all( const std::string& url, const std::string& query, bool mode_or, bool& stop );
    std::list< std::string > search_cache( const std::string& url, const std::string& query, bool mode_or, bool& top );
    
    // article 系
    const bool article_is_cached( const std::string& url ); // キャッシュにあるかどうか
    const std::string article_id( const std::string& url ); // 拡張子込み "12345.dat" みたいに
    const std::string article_key( const std::string& url ); // idから拡張子を取ったもの。書き込み用
    const time_t article_since_time( const std::string& url );
    const std::string article_since_date( const std::string& url );
    const time_t article_time_modified( const std::string& url ); // スレの更新時間( time_t )
    const std::string article_date_modified( const std::string& url ); // スレの更新時間( 文字列 )
    void article_set_date_modified( const std::string& url, const std::string& date ); // スレの更新時間( 文字列 )をセット
    const int article_hour( const std::string& url );
    const time_t article_write_time( const std::string& url );
    const std::string article_write_date( const std::string& url );
    const time_t article_write_pass( const std::string& url );
    const int article_status( const std::string& url );
    void article_reset_status( const std::string& url );
    const int article_code( const std::string& url );
    const std::string article_str_code( const std::string& url );
    const std::string article_ext_err( const std::string& url );
    const std::string article_subject( const std::string& url );
    const int article_number( const std::string& url );    
    const int article_number_load( const std::string& url );
    const int article_number_seen( const std::string& url );
    void article_set_number_seen( const std::string& url, int seen );
    const int article_number_new( const std::string& url );    
    const bool article_is_loading( const std::string& url );
    void article_download_dat( const std::string& url, const bool check_update );
    void article_stop_load( const std::string& url );
    const int article_get_speed( const std::string& url );

    // キャッシュ削除
    // cache_only == true の時はキャッシュだけ削除してスレ情報は消さない
    void delete_article( const std::string& url, const bool cache_only );  

    // キャッシュ保存
    bool article_save_dat( const std::string& url, const std::string& path_to );

    void article_update_writetime( const std::string& url );
    size_t article_lng_dat( const std::string& url );

    // agent, プロキシ

    const std::string& get_agent( const std::string& url );
    const std::string get_proxy_host( const std::string& url ); // 読み込み用
    const int get_proxy_port( const std::string& url );
    const std::string get_proxy_host_w( const std::string& url ); // 書き込み用
    const int get_proxy_port_w( const std::string& url );

    // ローカルルール
    const std::string localrule( const std::string& url );

    // setting.txt
    const std::string settingtxt( const std::string& url );

    // 書き込み関係

    // デフォルトの名無し名
    const std::string default_noname( const std::string& url );

    // 最大改行数/2
    const int line_number( const std::string& url );

    // 最大書き込みバイト数
    const int message_count( const std::string& url );


    // 書き込み時の名前とメール
    const std::string& write_name( const std::string& url );
    void set_write_name( const std::string& url, const std::string& str );
    const bool write_fixname( const std::string& url );
    void set_write_fixname( const std::string& url, bool set );

    const std::string& write_mail( const std::string& url );
    void set_write_mail( const std::string& url, const std::string& str );
    const bool write_fixmail( const std::string& url );
    void set_write_fixmail( const std::string& url, bool set );

    // ポストするメッセージの作成
    const std::string create_write_message( const std::string& url,
                                            const std::string& name, const std::string& mail, const std::string& msg );
    const std::string create_newarticle_message( const std::string& url, const std::string& subject,
                                                 const std::string& name, const std::string& mail, const std::string& msg );

    // あぼーん関係

    // 板レベルのあぼーん
    // グローバルなあぼーん情報は globalconf が管理
    std::list< std::string > get_abone_list_id_board( const std::string& url );
    std::list< std::string > get_abone_list_name_board( const std::string& url );
    std::list< std::string > get_abone_list_word_board( const std::string& url );
    std::list< std::string > get_abone_list_regex_board( const std::string& url );

    // 板レベルのあぼーん情報の同時セットと更新
    void reset_abone_board( const std::string& url,
                            std::list< std::string >& ids, std::list< std::string >& names,
                            std::list< std::string >& words, std::list< std::string >& regexs );

    // 個別のあぼーん情報のセットと更新
    void add_abone_id_board( const std::string& url, const std::string& id );
    void add_abone_name_board( const std::string& url, const std::string& name );
    void add_abone_word_board( const std::string& url, const std::string& word );

    // 全boardbaseクラスのあぼーん状態の更新
    // ビューの再描画も同時に行われる
    void update_abone_all_board();

    // 指定したboardbaseクラスのあぼーん状態の更新
    // ビューの再描画も同時に行われる
    void update_abone_board( const std::string& url );

    // スレあぼーん
    // グローバルなあぼーん情報は globalconf が管理
    std::list< std::string > get_abone_list_thread( const std::string& url );
    std::list< std::string > get_abone_list_word_thread( const std::string& url );
    std::list< std::string > get_abone_list_regex_thread( const std::string& url );
    std::vector< char > get_abone_vec_res( const std::string& url );
    const int get_abone_number_thread( const std::string& url );
    const int get_abone_hour_thread( const std::string& url );

    // スレあぼーん状態のリセット(情報セットと状態更新を同時におこなう)
    void reset_abone_thread( const std::string& url,
                             std::list< std::string >& threads, std::list< std::string >& words, std::list< std::string >& regexs,
                             const int number, const int hour );

    //
    // 各articlebase別のあぼーん情報
    //

    // 全articlebaseクラスのあぼーん状態の更新
    void update_abone_all_article();

    // レスあぼーん
    // グローバルなあぼーん情報は globalconf が管理
    std::list< std::string > get_abone_list_id( const std::string& url );
    std::list< std::string > get_abone_list_name( const std::string& url );
    std::list< std::string > get_abone_list_word( const std::string& url );
    std::list< std::string > get_abone_list_regex( const std::string& url );

    // 全あぼーん情報の同時セットと更新
    void reset_abone( const std::string& url, std::list< std::string >& ids, std::list< std::string >& names,
                      std::list< std::string >& words, std::list< std::string >& regexs,
                      std::vector< char >& vec_abone_res,
                      bool transparent, bool chain);

    // 個別のあぼーん情報のセットと更新
    void set_abone_res( const std::string& url, const int number, const bool set );
    void add_abone_id( const std::string& url, const std::string& id );
    void add_abone_name( const std::string& url, const std::string& name );
    void add_abone_word( const std::string& url, const std::string& word );

    // 透明あぼーん
    const bool get_abone_transparent( const std::string& url );
    void set_abone_transparent( const std::string& url, bool set );

    // 連鎖あぼーん
    const bool get_abone_chain( const std::string& url );
    void set_abone_chain( const std::string& url, bool set );


    //　ブックマーク関係

    // スレのブックマーク
    const bool is_bookmarked_thread( const std::string& url );
    void set_bookmarked_thread( const std::string& url, bool bookmarked );

    // レスのブックマーク
    const int get_num_bookmark( const std::string& url );
    const bool is_bookmarked( const std::string& url, int number );
    void set_bookmark( const std::string& url, int number, bool set );
}

#endif
