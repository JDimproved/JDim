// ライセンス: GPL2
//
// 板情報クラスのベース
//

#ifndef _BOARDBASE_H
#define _BOARDBASE_H

#include "skeleton/loadable.h"

#include <string>
#include <list>
#include <vector>
#include <ctime>

#ifdef _WIN32
#include <sys/time.h>
#endif

namespace JDLIB
{
    class LOADERDATA;
    class Iconv;
}


namespace DBTREE
{
    // ローカルプロキシ設定
    enum
    {
        PROXY_GLOBAL = 0,  // 全体設定使用
        PROXY_DISABLE, // 全体設定無効
        PROXY_LOCAL,     // ローカル設定使用

        PROXY_NUM
    };

    struct ARTICLE_INFO
    {
        std::string id;
        std::string subject;
        int number;
    };

    typedef std::vector< ARTICLE_INFO > ARTICLE_INFO_LIST;

    class Root;
    class ArticleBase;
    class ArticleHash;

    class BoardBase : public SKELETON::Loadable
    {
        // ArticleBaseクラス のキャッシュ
        // ArticleBaseクラスは一度作ったら~BoardBase()以外ではdeleteしないこと
        ArticleHash* m_hash_article;

        // subject.txt から作ったArticleBaseクラスのポインタのリスト
        // subject.txt と同じ順番で、ロードされるたびに更新される
        std::vector< ArticleBase* > m_list_subject; 

        // ダウンロード中に parse_subject() でsubject.txtを解析して結果を入れておく
        // ダウンロード終了後に regist_article() でスレを登録する
        ARTICLE_INFO_LIST m_list_artinfo; 

        // 状態 ( global.h で定義 )
        int m_status;                

        // 一度でも m_list_subject が作られた(=subject.txtを開いた)らtrue
        bool m_list_subject_created;

        // ローカルルールのmodified 時刻
        std::string m_modified_localrule;

        // setting.txtの modified 時刻
        std::string m_modified_setting;

        // subjectダウンロード指示時(BoardBase::download_subject)にオンラインだったか
        bool m_is_online;

        // subjectダウンロード指示時(BoardBase::download_subject)にブート中だったか
        bool m_is_booting;

        // ビュワーでソートをする列番号、ソード順
        int m_view_sort_column;
        int m_view_sort_mode;
        int m_view_sort_pre_column;
        int m_view_sort_pre_mode;

        // 名無し書き込み不可
        bool m_check_noname;

        // 過去ログも表示する
        bool m_show_oldlog;

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

        // dat型のurlに変換する時のquery ( url_dat()で使用する )
        std::string m_query_dat;
        std::string m_query_cgi;
        std::string m_query_kako;

        // ローカルあぼーん情報(板内の全レス対象)
        std::list< std::string > m_list_abone_id; // あぼーんするID
        std::list< std::string > m_list_abone_name; // あぼーんする名前
        std::list< std::string > m_list_abone_word; // あぼーんする文字列
        std::list< std::string > m_list_abone_regex; // あぼーんする正規表現

        // ローカルスレあぼーん情報
        std::list< std::string > m_list_abone_thread; // あぼーんするスレのタイトル
        std::list< std::string > m_list_abone_thread_remove; // あぼーんするスレのタイトル( dat 落ち判定用、remove_old_abone_thread()を参照せよ )
        std::list< std::string > m_list_abone_word_thread; // あぼーんする文字列
        std::list< std::string > m_list_abone_regex_thread; // あぼーんする正規表現
        int m_abone_number_thread; // レスの数
        int m_abone_hour_thread; // スレ立てからの経過時間

        // 読み込み用ローカルプロキシ設定
        int m_mode_local_proxy;
        std::string m_local_proxy;
        int m_local_proxy_port;
        std::string m_local_proxy_basicauth; // basic 認証用の「ユーザID:パスワード」の組

        // 書き込み用ローカルプロキシ設定
        int m_mode_local_proxy_w;
        std::string m_local_proxy_w;
        int m_local_proxy_port_w;
        std::string m_local_proxy_basicauth_w; // basic 認証用の「ユーザID:パスワード」の組

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
        JDLIB::Iconv* m_iconv;
        std::string m_rawdata;
        std::string m_rawdata_left;

        // 情報ファイルを読みこんだらtrueにして2度読みしないようにする
        bool m_read_info;

        // キャッシュにあるこの板に属するスレをデータベースに登録したか
        bool m_append_articles;

        // 移転を調査するために url_boardbase を読んでいる
        bool m_read_url_boardbase;

        // 書き込み時に必要なキーワード( hana=mogera や suka=pontan など )
        // 書き込み時のメッセージに付加する
        std::string m_keyword_for_write;   

        // basic 認証用の「ユーザID:パスワード」の組
        std::string m_basicauth;

        // get_article_fromURL()のキャッシュ
        std::string m_get_article_url;
        ArticleBase* m_get_article;

        // remove_old_abone_thread() をキャンセルするか
        bool m_cancel_remove_abone_thread;

        // Null artice クラス
        ArticleBase* m_article_null;

      protected:

        ARTICLE_INFO_LIST& get_list_artinfo(){ return m_list_artinfo; }

        ArticleHash* get_hash_article(){ return m_hash_article; }
        std::list< std::string >& get_url_update_views(){ return  m_url_update_views; }

        ArticleBase* get_article_null();
        ArticleBase* get_article( const std::string& datbase, const std::string& id );
        ArticleBase* get_article_create( const std::string& datbase, const std::string& id );

        void set_path_dat( const std::string& str ){ m_path_dat = str; }
        void set_path_readcgi( const std::string& str ){ m_path_readcgi = str; }
        void set_path_bbscgi( const std::string& str ){  m_path_bbscgi = str; }
        void set_path_subbbscgi( const std::string& str ){ m_path_subbbscgi = str; }
        void set_subjecttxt( const std::string& str ){ m_subjecttxt = str; }
        void set_ext( const std::string& str ){ m_ext = str; }
        void set_id( const std::string& str ){ m_id = str; }
        void set_charset( const std::string& str ){ m_charset = str; }

        // articleがスレあぼーんされているか
        bool is_abone_thread( ArticleBase* article );

        // m_url_update_views に登録されている view に update_board コマンドを送る
        void send_update_board();

        // クッキー
        virtual std::string get_hap() const { return {}; }
        virtual void set_hap( const std::string& hap ){}

        // クッキーの更新 (クッキーをセットした時に実行)
        virtual void update_hap(){}

      public:

        BoardBase( const std::string& root, const std::string& path_board, const std::string& name );
        ~BoardBase();
        bool empty();

        // 状態 ( global.hで定義 )
        int get_status() const { return m_status; }

        // boardviewに表示するスレッドのリストを取得
        std::vector< ArticleBase* >& get_list_subject(){ return m_list_subject; }

        // ローカルルールの modified 時刻
        const std::string& get_modified_localrule() const { return m_modified_localrule; }
        void set_modified_localrule( const std::string& modified ){ m_modified_localrule = modified; }

        // setting.txtの modified 時刻
        const std::string& get_modified_setting() const { return m_modified_setting; }
        void set_modified_setting( const std::string& modified ){ m_modified_setting = modified; }

        // boardviewでソートする列番号とソート順
        int get_view_sort_column() const { return m_view_sort_column; }
        void set_view_sort_column( int column ){ m_view_sort_column = column; }
        int get_view_sort_mode() const { return m_view_sort_mode; }
        void set_view_sort_mode( int mode ){ m_view_sort_mode = mode; }

        int get_view_sort_pre_column() const { return m_view_sort_pre_column; }
        void set_view_sort_pre_column( int column ) { m_view_sort_pre_column = column; }
        int get_view_sort_pre_mode() const { return m_view_sort_pre_mode; }
        void set_view_sort_pre_mode( int mode ){ m_view_sort_pre_mode = mode; }

        // 名無し書き込み不可
        bool get_check_noname() const { return m_check_noname; }
        void set_check_noname( const bool check ){ m_check_noname = check; }

        // 過去ログも表示する
        bool get_show_oldlog() const { return m_show_oldlog; }
        void set_show_oldlog( const bool show ){ m_show_oldlog = show; }

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

        // ユーザーエージェント
        virtual const std::string& get_agent(); // ダウンロード用
        virtual const std::string& get_agent_w(); // 書き込み用

        // ダウンロード時のプロキシ
        virtual std::string get_proxy_host();
        virtual int get_proxy_port();
        virtual std::string get_proxy_basicauth();

        // 書き込み時のプロキシ
        virtual std::string get_proxy_host_w();
        virtual int get_proxy_port_w();
        virtual std::string get_proxy_basicauth_w();

        // ローカルルール
        virtual std::string localrule();

        // SETTING.TXT
        virtual std::string settingtxt();

        // 書き込みの時のデフォルト名
        virtual std::string default_noname();

        // 最大改行数/2
        virtual int line_number();

        // 最大書き込みバイト数
        virtual int message_count();

        // 特殊文字書き込み可能か( pass なら可能、 change なら不可 )
        virtual std::string get_unicode();

        // 板のホストを指定してクッキーのやり取り
        virtual std::string cookie_for_request() const;
        void set_list_cookies( const std::list< std::string >& list_cookies );
        void delete_cookies();

        // 書き込み時に必要なキーワード( hana=mogera や suka=pontan など )
        // 書き込み時のメッセージに付加する
        const std::string& get_keyword_for_write() const { return m_keyword_for_write; }
        void set_keyword_for_write( const std::string& keyword ){ m_keyword_for_write = keyword; }

        // 書き込み時に必要なキーワード( hana=mogera や suka=pontan など )を
        // 確認画面のhtmlから解析する      
        virtual void analyze_keyword_for_write( const std::string& html ){}

        // 書き込み時のリファラ
        virtual std::string get_write_referer(){ return url_boardbase(); }

        // basic認証
        const std::string& get_basicauth() const { return m_basicauth; }
        void set_basicauth( const std::string& basicauth ){ m_basicauth = basicauth; }

        // スレの url を dat型のurlに変換して出力
        // url がスレッドのURLで無い時はempty()が返る
        // もしurlが移転前の旧ホストのものだったら対応するarticlebaseクラスに旧ホスト名を知らせる
        // (例) url =  "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345/12-15"のとき、
        // "http://www.hoge2ch.net/hogeboard/dat/12345.dat",  num_from = 12, num_to = 15, num_str = "12-15"
        virtual std::string url_dat( const std::string& url, int& num_from, int& num_to, std::string& num_str ); 

        // スレの url を read.cgi型のurlに変換
        // url がスレッドのURLで無い時はempty()が返る
        // num_from と num_to が 0 で無い時はスレ番号を付ける
        // (例) "http://www.hoge2ch.net/hogeboard/dat/12345.dat",  num_from = 12, num_to = 15 のとき
        // "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345/12-15"
        virtual std::string url_readcgi( const std::string& url, int num_from, int num_to );

        // subject.txt の URLを取得
        // (例) "http://www.hoge2ch.net/hogeboard/subject.txt"
        std::string url_subject();

        // ルートアドレス
        // (例) "http://www.hoge2ch.net/hogeboard/" なら "http://www.hoge2ch.net/"
        std::string url_root();

        // 板のベースアドレス
        // (例) "http://www.hoge2ch.net/hogeboard/"
        std::string url_boardbase();

        // dat ファイルのURLのベースアドレスを返す
        // (例) "http://www.hoge2ch.net/hogeboard/dat/12345.dat" なら "http://www.hoge2ch.net/hogeboard/dat/"
        std::string url_datbase();

        // dat ファイルのURLのパスを返す
        // (例) "http://www.hoge2ch.net/hogeboard/dat/12345.dat" なら "/hogeboard/dat/"
        virtual std::string url_datpath();

        // read.cgi のURLのベースアドレスを返す
        // (例) "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345" なら "http://www.hoge2ch.net/test/read.cgi/hogeboard/"
        std::string url_readcgibase();

        // read.cgi のURLのパスを返す
        // (例) "http://www.hoge2ch.net/test/read.cgi/hogeboard/12345" なら "/test/read.cgi/hogeboard/"
        std::string url_readcgipath();

        // bbscgi のURLのベースアドレス
        // (例) "http://www.hoge2ch.net/test/bbs.cgi/" ( 最後に '/' がつく )
        std::string url_bbscgibase();

        // subbbscgi のURLのベースアドレス
        // (例) "http://www.hoge2ch.net/test/subbbs.cgi/"  ( 最後に '/' がつく )
        std::string url_subbbscgibase();

        // article クラスのポインタ取得
        ArticleBase* get_article_fromURL( const std::string& url );

        // subject.txt ダウンロード
        // url_update_view : CORE::core_set_command( "update_board" ) を送信するビューのアドレス
        // read_from_cache : まだスレ一覧を開いていないときにキャッシュのsubject.txtを読み込む
        virtual void download_subject( const std::string& url_update_view, const bool read_from_cache );

        // 新スレ作成用のメッセージ変換
        virtual std::string create_newarticle_message( const std::string& subject, const std::string& name,
                                                       const std::string& mail, const std::string& msg )
        {
            return {};
        }

        // 新スレ作成用のbbscgi のURL
        virtual std::string url_bbscgi_new() { return {}; }
        
        // 新スレ作成用のsubbbscgi のURL
        virtual std::string url_subbbscgi_new() { return {}; }

        // 配下の全articlebaseクラスのあぼーん状態の更新
        void update_abone_all_article();

        // 板レベルでのあぼーん情報
        const std::list< std::string >& get_abone_list_id_board(){ return m_list_abone_id; }
        const std::list< std::string >& get_abone_list_name_board(){ return m_list_abone_name; }
        const std::list< std::string >& get_abone_list_word_board(){ return m_list_abone_word; }
        const std::list< std::string >& get_abone_list_regex_board(){ return m_list_abone_regex; }

        // 板レベルでのあぼーん状態のリセット(情報セットとスレビューの表示更新を同時におこなう)
        void reset_abone_board( const std::list< std::string >& ids,
                                const std::list< std::string >& names,
                                const std::list< std::string >& words,
                                const std::list< std::string >& regexs );

        // 板レベルでのあぼ〜ん状態更新(reset_abone()と違って各項目ごと個別におこなう。スレビューの表示更新も同時におこなう)
        void add_abone_id_board( const std::string& id );
        void add_abone_name_board( const std::string& name );
        void add_abone_word_board( const std::string& word );

        // スレあぼーん情報
        const std::list< std::string >& get_abone_list_thread(){ return m_list_abone_thread; }
        const std::list< std::string >& get_abone_list_thread_remove(){ return m_list_abone_thread_remove; }
        const std::list< std::string >& get_abone_list_word_thread(){ return m_list_abone_word_thread; }
        const std::list< std::string >& get_abone_list_regex_thread(){ return m_list_abone_regex_thread; }
        int get_abone_number_thread(){ return m_abone_number_thread; }
        int get_abone_hour_thread(){ return m_abone_hour_thread; }

        // subject.txtのロード後にdat落ちしたスレッドをスレあぼーんのリストから取り除く
        void remove_old_abone_thread();

        // スレあぼーん情報を更新した時に対応するスレ一覧の表示を更新する
        // CONFIG::set_abone_number_thread() などでグローバル設定をした後などに呼び出す
        // redraw : スレ一覧の表示更新を行う
        void update_abone_thread( const bool redraw );

        // スレあぼーん状態のリセット(情報セットとスレ一覧の表示更新を同時におこなう)
        // redraw : スレ一覧の表示更新を行う
        void reset_abone_thread( const std::list< std::string >& threads,
                                 const std::list< std::string >& words,
                                 const std::list< std::string >& regexs,
                                 const int number,
                                 const int hour,
                                 const bool redraw
            );

        // ローカルプロキシ設定
        int get_mode_local_proxy() const { return m_mode_local_proxy; }
        const std::string& get_local_proxy() const { return m_local_proxy; }
        int get_local_proxy_port() const { return m_local_proxy_port; }
        const std::string& get_local_proxy_basicauth() const { return m_local_proxy_basicauth; }

        void set_mode_local_proxy( int mode ){ m_mode_local_proxy = mode; }
        void set_local_proxy( const std::string& proxy );
        void set_local_proxy_port( int port ){ m_local_proxy_port = port; }

        int get_mode_local_proxy_w() const { return m_mode_local_proxy_w; }
        const std::string& get_local_proxy_w() const { return m_local_proxy_w; }
        int get_local_proxy_port_w() const { return m_local_proxy_port_w; }
        const std::string& get_local_proxy_basicauth_w() const { return m_local_proxy_basicauth_w; }

        void set_mode_local_proxy_w( int mode ){ m_mode_local_proxy_w = mode; }
        void set_local_proxy_w( const std::string& proxy );
        void set_local_proxy_port_w( int port ){ m_local_proxy_port_w = port; }

        // 書き込み時のデフォルトの名前とメアド
        const std::string& get_write_name() const { return  m_write_name; }
        const std::string& get_write_mail() const { return m_write_mail; }

        void set_write_name( const std::string& name ){ m_write_name = name; }
        void set_write_mail( const std::string& mail ){ m_write_mail = mail; }

        // 最終書き込み時間
        void update_writetime();
        time_t get_write_time() const { return m_write_time.tv_sec; } // 秒
        time_t get_write_pass(); // 経過時間(秒)
        time_t get_samba_sec() const { return m_samba_sec; } // samba(秒)
        void set_samba_sec( time_t sec ){ m_samba_sec = sec; }
        time_t get_write_leftsec(); // 書き込み可能までの残り秒

        // 全書き込み履歴クリア
        void clear_all_post_history();

        // 全スレの書き込み時間とスレ立て時間の文字列をリセット
        void reset_all_since_date();
        void reset_all_write_date();
        void reset_all_access_date();

        // 実況の秒数
        time_t get_live_sec() const{ return m_live_sec; }
        void set_live_sec( time_t sec ){ m_live_sec = sec; }

        // 最終アクセス時刻
        time_t get_last_access_time() const{ return m_last_access_time; }

        // 最大レス数
        int get_number_max_res() const { return m_number_max_res; }
        void set_number_max_res( const int number );

        // datの最大サイズ(Kバイト)
        virtual int get_max_dat_lng() const { return 0; }

        // 板情報の取得
        virtual void read_info();

        // 情報保存
        virtual void save_info();
        
        // 配下の全articlebaseクラスの情報保存
        void save_articleinfo_all();

        // キャッシュ内のログ検索
        // ArticleBase のアドレスをリスト(list_article)にセットして返す
        // query が空の時はキャッシュにあるログを全てヒットさせる
        // bm がtrueの時、しおりが付いている(スレ一覧でしおりを付けた or レスに一つでもしおりが付いている)スレのみを対象に検索する
        virtual void search_cache( std::vector< ArticleBase* >& list_article,
                                   const std::string& query,
                                   const bool mode_or, // 今のところ無視
                                   const bool bm,
                                   const bool stop // 呼出元のスレッドで true にセットすると検索を停止する
            );

        // datファイルのインポート
        // 成功したらdat型のurlを返す
        virtual std::string import_dat( const std::string& filename );

        // 更新可能状態にしてお気に入りやスレ一覧のタブのアイコンに更新マークを表示
        // update == true の時に表示。falseなら戻す
        void show_updateicon( const bool update );

        // 板の更新チェック時に、更新チェックを行うスレのアドレスのリスト
        // キャッシュが存在し、かつdat落ちしていないで新着数が0のスレを速度の順でソートして返す
        std::list< std::string > get_check_update_articles();

      private:

        void clear();

        // デフォルト最大レス数( 0 : 未設定 )
        virtual int get_default_number_max_res() { return 0; }

        // キャッシュのファイル名が正しいかどうか
        virtual bool is_valid( const std::string& filename ) { return false; }

        virtual void create_loaderdata( JDLIB::LOADERDATA& data );
        void receive_data( const char* data, size_t size ) override;
        void receive_finish() override;

        // url_boardbase をロードして移転したかどうか解析開始
        bool start_checkking_if_board_moved();

        virtual ArticleBase* append_article( const std::string& datbase, const std::string& id, const bool cached );
        virtual void parse_subject( const char* str_subject_txt ){}
        virtual void regist_article( const bool is_online ){}

        std::list< std::string > get_filelist_in_cache();

        void read_board_info();
        virtual void append_all_article_in_cache();

        void save_summary();
        void save_board_info();
        void save_jdboard_info();

        // ローカルルールとsetting.txtの読み込み及びダウンロード
        virtual void load_rule_setting(){}
        virtual void download_rule_setting(){}

        // レス数であぼーん(グローバル)
        // 2ch以外の板ではキャンセルする
        virtual int get_abone_number_global() { return 0; }
    };
}

#endif
