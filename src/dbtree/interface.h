// ライセンス: 最新のGPL

//
// データベースへのインターフェース関数
//

#ifndef _INTERFACE_H
#define _INTERFACE_H

#include <string>
#include <list>

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
    const std::string url_subject( const std::string& url );    
    const std::string url_root( const std::string& url );
    const std::string url_boardbase( const std::string& url );
    const std::string url_datbase( const std::string& url );

    // dat型のurlに変換
    const std::string url_dat( const std::string& url, int& num_from, int& num_to ); 

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

    // bbslist系
    const std::string& get_xml_bbsmenu();
    const std::string& get_xml_etc();
    void download_bbsmenu();
    
    // board 系
    const std::string board_path( const std::string& url );
    const std::string board_id( const std::string& url );
    const time_t board_time_modified( const std::string& url );
    const std::string board_name( const std::string& url );
    const std::string board_subjecttxt( const std::string& url );
    const std::string board_charset( const std::string& url );
    const std::string board_cookie_for_write( const std::string& url );
    const std::list< std::string >& board_list_cookies_for_write( const std::string& url );
    void board_set_list_cookies_for_write( const std::string& url, const std::list< std::string>& list_cookies );        
    const std::string board_hana_for_write( const std::string& url );
    void board_set_hana_for_write( const std::string& url, const std::string& hana );        
    const std::string board_ext( const std::string& url );
    const std::string board_str_code( const std::string& url );
    void board_save_info( const std::string& url );
    void board_download_subject( const std::string& url );
    void board_stop_load( const std::string& url );
    std::list< DBTREE::ArticleBase* >& board_list_subject( const std::string& url );
    const int board_view_sort_column( const std::string& url );
    void board_set_view_sort_column( const std::string& url, int column );
    const bool board_view_sort_ascend( const std::string& url );
    void board_set_view_sort_ascend( const std::string& url, bool ascend );
    const bool board_check_noname( const std::string& url );
    void board_set_check_noname( const std::string& url, bool check );

    
    // article 系
    const bool article_is_cached( const std::string& url ); // キャッシュにあるかどうか
    const std::string article_id( const std::string& url ); // 拡張子込み "12345.dat" みたいに
    const std::string article_key( const std::string& url ); // idから拡張子を取ったもの。書き込み用
    const time_t article_since_time( const std::string& url );
    const std::string article_since_date( const std::string& url );
    const time_t article_time_modified( const std::string& url );
    const std::string article_date_modified( const std::string& url );
    const time_t article_write_time( const std::string& url );
    const std::string article_write_date( const std::string& url );
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
    void article_download_dat( const std::string& url );

    void delete_article( const std::string& url );  // キャッシュ削除
    void article_update_writetime( const std::string& url );
    size_t article_lng_dat( const std::string& url );

    // agent, プロキシ

    const std::string& get_agent( const std::string& url );
    const std::string get_proxy_host( const std::string& url ); // 読み込み用
    const int get_proxy_port( const std::string& url );
    const std::string get_proxy_host_w( const std::string& url ); // 書き込み用
    const int get_proxy_port_w( const std::string& url );

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


    // あぼーん
    std::list< std::string > get_abone_list_id( const std::string& url );
    std::list< std::string > get_abone_list_name( const std::string& url );
    std::list< std::string > get_abone_list_word( const std::string& url );
    std::list< std::string > get_abone_list_regex( const std::string& url );
    void reset_abone( const std::string& url, std::list< std::string >& ids, std::list< std::string >& names,
                      std::list< std::string >& words, std::list< std::string >& regexs );
    void add_abone_id( const std::string& url, const std::string& id );
    void add_abone_name( const std::string& url, const std::string& name );
    void add_abone_word( const std::string& url, const std::string& word );

    const bool get_abone_transparent( const std::string& url );
    void set_abone_transparent( const std::string& url, bool set );

    // ブックマーク
    const bool is_bookmarked( const std::string& url, int number );
    void set_bookmark( const std::string& url, int number, bool set );
}

#endif
