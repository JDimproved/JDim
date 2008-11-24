// ライセンス: GPL2
//
// 板情報クラスのベース
//

#ifndef _BOARDBASE_H
#define _BOARDBASE_H

#include "skeleton/loadable.h"

#include <string>
#include <list>

namespace JDLIB
{
    class LOADERDATA;
}


namespace DBTREE
{
    // ローカルプロキシ設定
    enum
    {
        PROXY_GLOBAL,  // 全体設定使用
        PROXY_DISABLE, // 全体設定無効
        PROXY_LOCAL    // ローカル設定使用
    };

    class Root;
    class ArticleBase;

    class BoardBase : public SKELETON::Loadable
    {
        // ArticleBaseクラス のキャッシュ
        // ArticleBaseクラスは一度作ったら~BoardBase()以外ではdeleteしないこと
        std::list< ArticleBase* > m_list_article;

        // subject.txt から作ったArticleBaseクラスのポインタのリスト
        // subject.txt と同じ順番で、ロードされるたびに更新される
        std::list< ArticleBase* > m_list_subject; 

        // 一度でも m_list_subject が作られた(=subject.txtを開いた)らtrue
        bool m_list_subject_created;

        // ローカルルールのmodified 時刻
        std::string m_modified_localrule;

        // setting.txtの modified 時刻
        std::string m_modified_setting;

        // subjectダウンロード後にローカルルールとsetting.txtをロードする
        bool m_load_rule_setting;

        // ビュワーでソートをする列番号、ソード順
        int m_view_sort_column;
        int m_view_sort_mode;
        int m_view_sort_pre_column;
        int m_view_sort_pre_mode;

        // 名無し書き込み不可
        bool m_check_noname;

        //
        // subjectファイルのURLが "http://www.hoge2ch.net/hogeboard/subject.txt"
        // datファイルのURLが "http://www.hoge2ch.net/hogeboard/dat/12345.dat"
        // スレのURLが "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345"  のとき、
        //
        // m_root = "http://www.hoge2ch.net"
        // m_path_board = "/hogeboard"
        // m_path_dat = "/dat"
        // m_path_readcgi = "/test/read.cgi"
        // m_path_bbscgi = "/test/bbs.cgi"    
        // m_path_subbbscgi = "/test/subbbs.cgi"    
        // m_subjecttxt = "subject.txt"
        // m_ext = ".dat"
        // m_id = "hogeboard"
        // m_charset = "MS932"
        //
        // 先頭に'/'を付けて最後に '/' は付けないことにフォーマットを統一
        //
        std::string m_root;
        std::string m_path_board;
        std::string m_path_dat;
        std::string m_path_readcgi;
        std::string m_path_bbscgi;        
        std::string m_path_subbbscgi;        
        std::string m_subjecttxt;
        std::string m_ext;
        std::string m_id;
        std::string m_charset;
        std::string m_name; // 板名

        // ローカルあぼーん情報(板内の全レス対象)
        std::list< std::string > m_list_abone_id; // あぼーんするID
        std::list< std::string > m_list_abone_name; // あぼーんする名前
        std::list< std::string > m_list_abone_word; // あぼーんする文字列
        std::list< std::string > m_list_abone_regex; // あぼーんする正規表現

        // ローカルスレあぼーん情報
        std::list< std::string > m_list_abone_thread; // あぼーんするスレのタイトル
        std::list< std::string > m_list_abone_word_thread; // あぼーんする文字列
        std::list< std::string > m_list_abone_regex_thread; // あぼーんする正規表現
        int m_abone_number_thread; // レスの数
        int m_abone_hour_thread; // スレ立てからの経過時間

        // ローカルプロキシ設定
        int m_mode_local_proxy;
        std::string m_local_proxy;
        int m_local_proxy_port;

        int m_mode_local_proxy_w;
        std::string m_local_proxy_w;
        int m_local_proxy_port_w;

        // 書き込み時のデフォルトの名前とメアド
        std::string m_write_name;      
        std::string m_write_mail;

        // 最終書き込み時間
        struct timeval m_write_time;   

        // samba(秒)
        time_t m_samba_sec;

        // 実況時の更新間隔(秒)
        time_t m_live_sec;

        // 最終アクセス時刻
        time_t m_last_access_time;

        // 最大レス数
        int m_number_max_res;

        // ダウンロード用変数
        std::list< std::string > m_url_update_views; // CORE::core_set_command( "update_board" ) を送信するビューのアドレス
        char* m_rawdata;
        int m_lng_rawdata;

        // 情報ファイルを読みこんだらtrueにして2度読みしないようにする
        bool m_read_info;

        // キャッシュにあるこの板に属するスレをデータベースに登録したか
        bool m_append_articles;

        // 移転を調査するために url_boardbase を読んでいる
        bool m_read_url_boardbase;

        // クッキー, 書き込み時に必要
        std::list< std::string > m_list_cookies_for_write;

        // 書き込み時に必要なキーワード( hana=mogera や suka=pontan など )
        // 書き込み時のメッセージに付加する
        std::string m_keyword_for_write;   

        // basic 認証用の「ユーザID:パスワード」の組
        std::string m_basicauth;

        // get_article_fromURL()のキャッシュ
        std::string m_get_article_url;
        ArticleBase* m_get_article;

        // NULL artice クラス
        ArticleBase* m_article_null;

      protected:

        std::list< ArticleBase* >& get_list_article(){ return m_list_article; }

        ArticleBase* get_article_null();

        void set_path_dat( const std::string& str ){ m_path_dat = str; }
        void set_path_readcgi( const std::string& str ){ m_path_readcgi = str; }
        void set_path_bbscgi( const std::string& str ){  m_path_bbscgi = str; }
        void set_path_subbbscgi( const std::string& str ){ m_path_subbbscgi = str; }
        void set_subjecttxt( const std::string& str ){ m_subjecttxt = str; }
        void set_ext( const std::string& str ){ m_ext = str; }
        void set_id( const std::string& str ){ m_id = str; }
        void set_charset( const std::string& str ){ m_charset = str; }

        // articleがスレあぼーんされているか
        const bool is_abone_thread( ArticleBase* article );

      public:

        BoardBase( const std::string& root, const std::string& path_board, const std::string& name );
        virtual ~BoardBase();
        bool empty();

        // boardviewに表示するスレッドのリストを取得
        std::list< ArticleBase* >& get_list_subject(){ return m_list_subject; }

        // ローカルルールの modified 時刻
        const std::string& get_modified_localrule() const { return m_modified_localrule; }
        void set_modified_localrule( const std::string& modified ){ m_modified_localrule = modified; }

        // setting.txtの modified 時刻
        const std::string& get_modified_setting() const { return m_modified_setting; }
        void set_modified_setting( const std::string& modified ){ m_modified_setting = modified; }

        // boardviewでソートする列番号とソート順
        const int get_view_sort_column() const { return m_view_sort_column; }
        void set_view_sort_column( int column ){ m_view_sort_column = column; }
        const int get_view_sort_mode() const { return m_view_sort_mode; }
        void set_view_sort_mode( int mode ){ m_view_sort_mode = mode; }

        const int get_view_sort_pre_column() const { return m_view_sort_pre_column; }
        void set_view_sort_pre_column( int column ) { m_view_sort_pre_column = column; }
        const int get_view_sort_pre_mode() const { return m_view_sort_pre_mode; }
        void set_view_sort_pre_mode( int mode ){ m_view_sort_pre_mode = mode; }

        // 名無し書き込み不可
        const bool get_check_noname() const { return m_check_noname; }
        void set_check_noname( bool check );

        // url がこの板のものかどうか
        virtual bool equal( const std::string& url );

        // 移転などで板のルートやパスを変更する
        void update_url( const std::string& root, const std::string& path_board );

        const std::string& get_root() const{ return m_root; }
        const std::string& get_path_board() const { return m_path_board; }
        const std::string& get_ext() const { return m_ext; }
        const std::string& get_id() const { return m_id; }
        const std::string& get_charset() const { return m_charset; }
        const std::string& get_name() const { return m_name; }
        void update_name( const std::string& name );
        const std::string& get_subjecttxt() const { return m_subjecttxt; }

        // ダウンロード、書き込み時のエージェント名やプロキシ
        virtual const std::string& get_agent();
        virtual const std::string get_proxy_host();
        virtual const int get_proxy_port();
        virtual const std::string get_proxy_host_w();
        virtual const int get_proxy_port_w();

        // ローカルルール
        virtual const std::string localrule();

        // SETTING.TXT
        virtual const std::string settingtxt();

        // 書き込みの時のデフォルト名
        virtual const std::string default_noname();

        // 最大改行数/2
        virtual const int line_number();

        // 最大書き込みバイト数
        virtual const int message_count();

        // 書き込み用クッキー
        virtual const std::string cookie_for_write();
        const std::list< std::string >& list_cookies_for_write() { return m_list_cookies_for_write; }
        void set_list_cookies_for_write( const std::list< std::string >& list_cookies ){ m_list_cookies_for_write = list_cookies; }

        // 書き込み時に必要なキーワード( hana=mogera や suka=pontan など )
        // 書き込み時のメッセージに付加する
        const std::string& get_keyword_for_write() const { return m_keyword_for_write; }
        void set_keyword_for_write( const std::string& keyword ){ m_keyword_for_write = keyword; }

        // 書き込み時に必要なキーワード( hana=mogera や suka=pontan など )を
        // 確認画面のhtmlから解析する      
        virtual void analyze_keyword_for_write( const std::string& html ){}

        // basic認証
        const std::string& get_basicauth() const { return m_basicauth; }
        void set_basicauth( const std::string& basicauth ){ m_basicauth = basicauth; }

        // スレの url を dat型のurlに変換して出力
        // url がスレッドのURLで無い時はempty()が返る
        // もしurlが移転前の旧ホストのものだったら対応するarticlebaseクラスに旧ホスト名を知らせる
        // (例) url =  "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345/12-15"のとき、
        // "http://www.hoge2ch.net/hogeboard/dat/12345.dat",  num_from = 12, num_to = 15, num_str = 12-15
        virtual const std::string url_dat( const std::string& url, int& num_from, int& num_to, std::string& num_str ); 

        // スレの url を read.cgi型のurlに変換
        // url がスレッドのURLで無い時はempty()が返る
        // num_from と num_to が 0 で無い時はスレ番号を付ける
        // (例) "http://www.hoge2ch.net/hogeboard/dat/12345.dat",  num_from = 12, num_to = 15 のとき
        // "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345/12-15"
        const std::string url_readcgi( const std::string& url, int num_from, int num_to );

        // subject.txt の URLを取得
        // (例) "http://www.hoge2ch.net/hogeboard/subject.txt"
        const std::string url_subject();

        // ルートアドレス
        // (例) "http://www.hoge2ch.net/hogeboard/" なら "http://www.hoge2ch.net/"
        const std::string url_root();

        // 板のベースアドレス
        // (例) "http://www.hoge2ch.net/hogeboard/"
        const std::string url_boardbase();

        // dat ファイルのURLのベースアドレスを返す
        // (例) "http://www.hoge2ch.net/hogeboard/dat/12345.dat" なら "http://www.hoge2ch.net/hogeboard/dat/"
        const std::string url_datbase();

        // dat ファイルのURLのパスを返す
        // (例) "http://www.hoge2ch.net/hogeboard/dat/12345.dat" なら "/hogeboard/dat/"
        virtual const std::string url_datpath();

        // read.cgi のURLのベースアドレスを返す
        // (例) "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345" なら "http://www.hoge2ch.net/test/read.cgi/hogeboard/"
        const std::string url_readcgibase();        

        // read.cgi のURLのパスを返す
        // (例) "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345" なら "/test/read.cgi/hogeboard/"
        const std::string url_readcgipath();

        // bbscgi のURLのベースアドレス
        const std::string url_bbscgibase();
        
        // subbbscgi のURLのベースアドレス
        const std::string url_subbbscgibase();

        // article クラスのポインタ取得
        // それぞれの違いはソースのコメントを参照
        ArticleBase* get_article( const std::string& id );
        ArticleBase* get_article_create( const std::string& id );
        ArticleBase* get_article_fromURL( const std::string& url );

        // subject.txt ダウンロード
        // url_update_view : CORE::core_set_command( "update_board" ) を送信するビューのアドレス
        void download_subject( const std::string& url_update_view );

        // 新スレ作成用のメッセージ変換
        virtual const std::string create_newarticle_message( const std::string& subject,
                                                             const std::string& name, const std::string& mail, const std::string& msg )
        {
            return std::string();
        }

        // 新スレ作成用のbbscgi のURL
        virtual const std::string url_bbscgi_new() { return std::string(); }
        
        // 新スレ作成用のsubbbscgi のURL
        virtual const std::string url_subbbscgi_new() { return std::string(); }


        // 配下の全articlebaseクラスのあぼーん状態の更新
        void update_abone_all_article();

        // あぼーん情報
        std::list< std::string > get_abone_list_id(){ return m_list_abone_id; }
        std::list< std::string > get_abone_list_name(){ return m_list_abone_name; }
        std::list< std::string > get_abone_list_word(){ return m_list_abone_word; }
        std::list< std::string > get_abone_list_regex(){ return m_list_abone_regex; }

        // あぼーん状態のリセット(情報セットと状態更新を同時におこなう)
        void reset_abone( std::list< std::string >& ids, std::list< std::string >& names, std::list< std::string >& words, std::list< std::string >& regexs );

        // あぼ〜ん状態更新(reset_abone()と違って各項目ごと個別におこなう)
        void add_abone_id( const std::string& id );
        void add_abone_name( const std::string& name );
        void add_abone_word( const std::string& word );

        // スレあぼーん情報
        std::list< std::string > get_abone_list_thread(){ return m_list_abone_thread; }
        std::list< std::string > get_abone_list_word_thread(){ return m_list_abone_word_thread; }
        std::list< std::string > get_abone_list_regex_thread(){ return m_list_abone_regex_thread; }
        const int get_abone_number_thread(){ return m_abone_number_thread; }
        const int get_abone_hour_thread(){ return m_abone_hour_thread; }

        // スレあぼーん状態の更新
        void update_abone_thread();

        // スレあぼーん状態のリセット(情報セットと状態更新を同時におこなう)
        void reset_abone_thread( std::list< std::string >& threads, std::list< std::string >& words, std::list< std::string >& regexs,
                                 const int number, const int hour );

        // ローカルプロキシ設定
        const int get_mode_local_proxy() const { return m_mode_local_proxy; }
        const std::string& get_local_proxy() const { return m_local_proxy; }
        const int get_local_proxy_port() const { return m_local_proxy_port; }
        void set_mode_local_proxy( int mode ){ m_mode_local_proxy = mode; }
        void set_local_proxy( const std::string& proxy ){ m_local_proxy = proxy; }
        void set_local_proxy_port( int port ){ m_local_proxy_port = port; }

        const int get_mode_local_proxy_w() const { return m_mode_local_proxy_w; }
        const std::string& get_local_proxy_w() const { return m_local_proxy_w; }
        const int get_local_proxy_port_w() const { return m_local_proxy_port_w; }
        void set_mode_local_proxy_w( int mode ){ m_mode_local_proxy_w = mode; }
        void set_local_proxy_w( const std::string& proxy ){ m_local_proxy_w = proxy; }
        void set_local_proxy_port_w( int port ){ m_local_proxy_port_w = port; }

        // 書き込み時のデフォルトの名前とメアド
        const std::string& get_write_name() const { return  m_write_name; }
        const std::string& get_write_mail() const { return m_write_mail; }

        void set_write_name( const std::string& name ){ m_write_name = name; }
        void set_write_mail( const std::string& mail ){ m_write_mail = mail; }

        // 最終書き込み時間
        void update_writetime();
        const time_t& get_write_time() const { return m_write_time.tv_sec; } // 秒
        const time_t get_write_pass(); // 経過時間(秒)
        const time_t get_samba_sec() const { return m_samba_sec; } // samba(秒)
        void set_samba_sec( time_t sec ){ m_samba_sec = sec; }
        time_t get_write_leftsec(); // 書き込み可能までの残り秒

        // 全書き込み履歴クリア
        void clear_all_post_history();

        // 実況の秒数
        const time_t get_live_sec() const{ return m_live_sec; }
        void set_live_sec( time_t sec ){ m_live_sec = sec; }

        // 最終アクセス時刻
        const time_t get_last_access_time() const{ return m_last_access_time; }

        // 最大レス数
        const int get_number_max_res() const{ return m_number_max_res; }
        void set_number_max_res( const int number ){ m_number_max_res = number; }

        // 板情報の取得
        void read_info();

        // 情報保存
        void save_info();
        
        // キャッシュ内のログ検索
        std::list< std::string > search_cache( const std::string& query, bool mode_or, bool& stop );

      private:

        void clear();

        // m_url_update_views に登録されている view に update_board コマンドを送る
        void send_update_board();

        // キャッシュのファイル名が正しいかどうか
        virtual bool is_valid( const std::string& filename ){ return false; }

        virtual void create_loaderdata( JDLIB::LOADERDATA& data );
        virtual void receive_data( const char* data, size_t size );
        virtual void receive_finish();

        // url_boardbase をロードして移転したかどうか解析開始
        bool start_checkking_if_board_moved();

        virtual ArticleBase* append_article( const std::string& id, bool cached );
        virtual void parse_subject( const char* str_subject_txt ){}

        std::list< std::string > get_filelist_in_cache();

        void read_board_info();
        void append_all_article_in_cache();

        void save_summary();
        void save_board_info();
        void save_jdboard_info();

        // ローカルルールとsetting.txtの読み込み及びダウンロード
        virtual void load_rule_setting(){}
        virtual void download_rule_setting(){}

        // レス数であぼーん(グローバル)
        // 2ch以外の板ではキャンセルする
        virtual const int get_abone_number_global(){ return 0; }
    };
}

#endif
