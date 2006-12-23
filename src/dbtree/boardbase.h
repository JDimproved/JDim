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

        // ビュワーでソートをする列番号、ソード順
        int m_view_sort_column;
        bool m_view_sort_ascend;
        int m_view_sort_pre_column;
        bool m_view_sort_pre_ascend;

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

        // あぼーん情報
        std::list< std::string > m_list_abone_thread; // あぼーんするスレのタイトル
        std::list< std::string > m_list_abone_word_thread; // あぼーんする文字列
        std::list< std::string > m_list_abone_regex_thread; // あぼーんする正規表現

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

        char* m_rawdata;
        int m_lng_rawdata;

        // 情報ファイルを読みこんだらtrueにして2度読みしないようにする
        bool m_read_info;

        // クッキー, 書き込み時に必要
        std::list< std::string > m_list_cookies_for_write;

        // hana, 2ch書き込み時に必要
        std::string m_hana_for_write;   

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
        const bool get_abone_thread( ArticleBase* article );

      public:

        BoardBase( const std::string& root, const std::string& path_board, const std::string& name );
        virtual ~BoardBase();
        bool empty();

        // boardviewに表示するスレッドのリストを取得
        std::list< ArticleBase* >& get_list_subject(){ return m_list_subject; }

        // boardviewでソートする列番号とソート順
        const int get_view_sort_column() const { return m_view_sort_column; }
        void set_view_sort_column( int column ){ m_view_sort_column = column; }
        const bool get_view_sort_ascend() const { return m_view_sort_ascend; }
        void set_view_sort_ascend( bool ascend ){ m_view_sort_ascend = ascend; }

        const int get_view_sort_pre_column() const { return m_view_sort_pre_column; }
        void set_view_sort_pre_column( int column ) { m_view_sort_pre_column = column; }
        const bool get_view_sort_pre_ascend() const { return m_view_sort_pre_ascend; }
        void set_view_sort_pre_ascend( bool ascend ){ m_view_sort_pre_ascend = ascend; }

        // 名無し書き込み不可
        const bool get_check_noname() const { return m_check_noname; }
        void set_check_noname( bool check );

        // url がこの板のものかどうか
        virtual bool equal( const std::string& url );

        // 移転などで板のルートを変更する
        void update_root( const std::string& root );

        const std::string& get_root() const{ return m_root; }
        const std::string& get_path_board() const { return m_path_board; }
        const std::string& get_ext() const { return m_ext; }
        const std::string& get_id() const { return m_id; }
        const std::string& get_charset() const { return m_charset; }
        const std::string& get_name() const { return m_name; }
        const std::string& get_subjecttxt() const { return m_subjecttxt; }

        // ダウンロード、書き込み時のエージェント名やプロキシ
        virtual const std::string& get_agent();
        virtual const std::string get_proxy_host();
        virtual const int get_proxy_port();
        virtual const std::string get_proxy_host_w();
        virtual const int get_proxy_port_w();

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

        // hana
        const std::string& hana_for_write() { return m_hana_for_write; }
        void set_hana_for_write( const std::string& hana ){ m_hana_for_write = hana; }

        // basic認証
        const std::string& get_basicauth() const { return m_basicauth; }
        void set_basicauth( const std::string& basicauth ){ m_basicauth = basicauth; }

        // スレの url を dat型のurlに変換して出力
        // url がスレッドのURLで無い時はempty()が返る
        // もしurlが移転前の旧ホストのものだったら対応するarticlebaseクラスに旧ホスト名を知らせる
        // (例) url =  "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345/12-15"のとき、
        // "http://www.hoge2ch.net/hogeboard/dat/12345.dat",  num_from = 12, num_to = 15
        const std::string url_dat( const std::string& url, int& num_from, int& num_to ); 

        // スレの url を read.cgi型のurlに変換
        // url がスレッドのURLで無い時はempty()が返る
        // num_from と num_to が 0 で無い時はスレ番号を付ける
        // (例) "http://www.hoge2ch.net/hogeboard/dat/12345.dat",  num_from = 12, num_to = 15 のとき
        // "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345/12-15"
        virtual const std::string url_readcgi( const std::string& url, int num_from, int num_to );

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
        virtual const std::string url_readcgipath();

        // bbscgi のURLのベースアドレス
        const std::string url_bbscgibase();
        
        // subbbscgi のURLのベースアドレス
        const std::string url_subbbscgibase();

        // article クラスのポインタ取得
        // それぞれの違いはソースのコメントを参照
        ArticleBase* get_article( const std::string id );
        ArticleBase* get_article_create( const std::string id );
        ArticleBase* get_article_fromURL( const std::string url );

        void download_subject();

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

        // スレあぼーん情報
        std::list< std::string > get_abone_list_thread(){ return m_list_abone_thread; }
        std::list< std::string > get_abone_list_word_thread(){ return m_list_abone_word_thread; }
        std::list< std::string > get_abone_list_regex_thread(){ return m_list_abone_regex_thread; }

        // スレあぼーん状態の更新
        void update_abone_thread();

        // スレあぼーん状態のリセット(情報セットと状態更新を同時におこなう)
        void reset_abone_thread( std::list< std::string >& threads, std::list< std::string >& words, std::list< std::string >& regexs );

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

        // 板情報の取得
        void read_info();

        // 情報保存
        void save_info();
        
      private:

        void clear();

        // キャッシュのファイル名が正しいかどうか
        virtual bool is_valid( const std::string& filename ){ return false; }

        virtual void create_loaderdata( JDLIB::LOADERDATA& data );
        virtual void receive_data( const char* data, size_t size );
        virtual void receive_finish();

        virtual ArticleBase* append_article( const std::string& id, bool cached );
        virtual void parse_subject( const char* str_subject_txt ){}

        void read_board_info();
        void append_all_article_in_cache();

        void save_summary();
        void save_board_info();
        void save_jdboard_info();

        // setting.txtの読み込み及びダウンロード
        virtual void load_setting(){}
        virtual void download_setting(){}
    };
}

#endif
