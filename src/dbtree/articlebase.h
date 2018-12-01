// ライセンス: GPL2

//
// スレ情報のベースクラス
//
// 新スレ用の id は 0000000000(.各板別の拡張子) とする。
//

#ifndef _ARTICLEBASE_H
#define _ARTICLEBASE_H

#include <string>
#include <sys/time.h>
#include <list>
#include <vector>
#include <ctime>

#include "skeleton/lockable.h"

#include "jdlib/constptr.h"

namespace DBTREE
{
    class NodeTreeBase;
    struct NODE;

    class ArticleBase : public SKELETON::Lockable
    {
        // 情報ファイルのパス
        // デストラクタの中でCACHE::path_article_ext_info()を呼ぶとabortするので
        // ArticleBase::read_info()が呼ばれたときにパスを取得しておく
        std::string m_path_article_info;
        std::string m_path_article_ext_info;

        // m_nodetree は参照が外れたら自動でクリアされる
        JDLIB::ConstPtr< NodeTreeBase > m_nodetree;

        std::string m_url;           // dat ファイルのURL
        std::string m_datbase;       // ベースアドレス
        std::string m_id;            // ID ( .datなどの拡張子付き  (例) 1234567.dat )
        std::string m_key;           // ID から拡張子を取った物
        std::string m_date_modified; // サーバのデータが更新された時間
        time_t m_since_time;         // スレが立った時刻
        std::string m_since_date;    // スレ立て月日( string型 )
        int m_code;                  // HTTPコード
        std::string m_str_code;      // HTTPコード(文字列)
        std::string m_ext_err;       // HTTPコード以外のエラーメッセージ
        int m_status;                // 状態 ( global.h で定義 )

        // 移転する前にこのスレがあった旧ホスト名( 移転していないなら m_url に含まれているホスト名と同じ )
        // 詳しくはコンストラクタの説明を参照せよ
        std::string m_org_host;

        std::string m_subject;       // サブジェクト
        int m_number;                // サーバ上にあるレスの数
        int m_number_diff;            // レス増分( subject.txt をロードした時の m_number の増分 )
        int m_number_new;            // 新着数( ロードした時の差分読み込み数)
        int m_number_load;           // キャッシュにあるレスの数
        int m_number_before_load;    // ロード前のレスの数( m_number_new を計算するのに使う )
        int m_number_seen;           // どこまで読んだか
        int m_number_max;            // 規定の最大レス数(0:未設定)
        struct timeval m_access_time;  // ユーザが最後にロードした時間
        std::string m_access_date;     // ユーザが最後にロードした月日( string型 )
        struct timeval m_check_update_time;  // 最終更新チェック時間
        struct timeval m_write_time;   // 最終書き込み時間
        std::string m_write_time_date; // 最終書き込み月日( string型 )
        std::string m_write_name;      // 書き込み時の名前
        std::string m_write_mail;      // 書き込み時のメアド
        bool m_write_fixname;          // 書き込み時名前固定
        bool m_write_fixmail;          // 書き込み時メール固定

        // あぼーん情報
        std::list< std::string > m_list_abone_id;   // あぼーんするID
        std::list< std::string > m_list_abone_name; // あぼーんする名前
        std::list< std::string > m_list_abone_word; // あぼーんする文字列
        std::list< std::string > m_list_abone_regex; // あぼーんする正規表現
        std::vector< char > m_vec_abone_res; // レスあぼーん情報
        bool m_abone_transparent; // 透明あぼーん
        bool m_abone_chain; // 連鎖あぼーん
        bool m_abone_age; // age ているレスをあぼーん
        bool m_abone_board; // 板レベルでのあぼーんを有効にする
        bool m_abone_global; // 全体レベルでのあぼーんを有効にする

        // 「スレ」がスレ一覧でブックマークされているか
        bool m_bookmarked_thread;          

        // 「レス」のブックマーク
        std::vector< char > m_vec_bookmark; // ブックマーク判定キャッシュ

        // 自分が書き込んだレスか
        std::vector< char > m_vec_posted;

        // HDDにキャッシュされているか
        bool m_cached;

        // 情報ファイルを読みこんだらtrueにして2度読みしないようにする
        bool m_read_info; 

        // true ならunlock_impl()がコールバックされたときに情報保存
        bool m_save_info;

        // 前スレのアドレス
        // スレが未取得で、この変数がemptyで無いとき download_dat()を呼び出すと
        // ロード終了時に次スレ移行チェックと前スレの情報のコピーをする
        std::string m_url_pre_article;

        // スレッド924か
        bool m_924;

      protected:

        void set_key( const std::string& key ){ m_key = key; }
        void set_since_time( const time_t since ){ m_since_time = since; }
        void set_is_924( const bool is924 ){ m_924 = is924; }

        // dat落ちしたスレをロードするか
        virtual const bool is_load_olddat(){ return false; }

      public:

        ArticleBase( const std::string& datbase, const std::string& id, bool cached );
        virtual ~ArticleBase();

        const bool empty();

        const std::string& get_url() const { return m_url; }

        // ID がこのスレのものかどうか
        virtual const bool equal( const std::string& datbase, const std::string& id );

        // 移転があったときなどにdatファイルのベースアドレスを更新
        void update_datbase( const std::string& datbase );

        // 移転する前のオリジナルのURL
        const std::string get_org_url();

        // 移転する前のオリジナルのホスト名
        const std::string& get_org_host() const { return m_org_host; }
        void set_org_host( const std::string& host );

        const std::string& get_id() const { return m_id; }
        const std::string& get_key() const { return m_key; }
        const std::string& get_subject() const { return m_subject; }
        const int get_number() const { return m_number; }
        const int get_number_diff() const { return m_number_diff; }
        const int get_number_new() const { return m_number_new; }
        const int get_number_load() const { return m_number_load; }
        const int get_number_seen() const{  return m_number_seen; }

        void set_number_max( const int number ){ m_number_max = number; }

        // スレ速度
        const int get_speed();

        // キャッシュにあるdatファイルのサイズ
        const size_t get_lng_dat();

        // nodetree の number 番のレスのヘッダノードのポインタを返す
        NODE* res_header( int number );

        // number番のレスの発言者の名前
        const std::string get_name( int number );

        // number番の名前の重複数( = 発言数 )
        int get_num_name( int number );

        // 指定した発言者の名前のレス番号をリストにして取得
        std::list< int > get_res_name( const std::string& name );

        // number番のレスの時刻を文字列で取得
        // 内部で regex　を使っているので遅い
        const std::string get_time_str( int number );

        // number番のレスの発言者ID( スレIDではなくて名前の横のID )
        const std::string get_id_name( int number );

        // 指定した発言者ID の重複数( = 発言数 )
        // (注) 下の get_num_id_name( int number )と違って検索するので遅い
        int get_num_id_name( const std::string& id );

        // number番の発言者ID の重複数( = 発言数 )
        int get_num_id_name( int number );

        // 指定した発言者IDを持つレス番号をリストにして取得
        std::list< int > get_res_id_name( const std::string& id_name );

        // str_num で指定したレス番号をリストにして取得
        // str_num は "from-to"　の形式 (例) 3から10をセットしたいなら "3-10"
        // list_jointは出力で true のスレは前のスレに連結される (例) "3+4" なら 4が3に連結
        std::list< int > get_res_str_num( const std::string& str_num, std::list< bool >& list_joint );

        // ブックマークをつけたレス番号をリストにして取得
        std::list< int > get_res_bm();

        // 書き込みしたレス番号をリストにして取得
        std::list< int > get_res_posted();

        // number番のレスを参照しているレス番号をリストにして取得
        std::list< int > get_res_reference( const int number );

        // res_num に含まれるレスを参照しているレス番号をリストにして取得
        std::list< int > get_res_reference( const std::list< int >& res_num );

        // URL を含むレス番号をリストにして取得
        std::list< int > get_res_with_url();

        // query を含むレス番号をリストにして取得
        // mode_or == true なら OR抽出
        const std::list< int > get_res_query( const std::string& query, const bool mode_or );

        // number番のレスの文字列を返す
        // ref == true なら先頭に ">" を付ける        
        const std::string get_res_str( int number, bool ref = false );

        // number　番のレスの生文字列を返す
        const std::string get_raw_res_str( int number );

        // 書き込み時の名前とメアド
        const std::string& get_write_name() const { return m_write_name; }
        void set_write_name( const std::string& str ){ m_save_info = true; m_write_name = str; }
        const bool get_write_fixname() const { return m_write_fixname; }
        void set_write_fixname( bool set ){ m_save_info = true;  m_write_fixname = set; }

        const std::string& get_write_mail() const { return m_write_mail; }
        void set_write_mail( const std::string& str ){ m_save_info = true;  m_write_mail = str; }
        const bool get_write_fixmail() const { return m_write_fixmail; }
        void set_write_fixmail( bool set ){ m_save_info = true; m_write_fixmail = set; }

        // 書き込みメッセージ作成
        virtual const std::string create_write_message( const std::string& name, const std::string& mail, const std::string& msg )
        { return std::string(); }

        // bbscgi のURL
        virtual const std::string url_bbscgi() { return std::string(); }
        
        // subbbscgi のURL
        virtual const std::string url_subbbscgi() { return std::string(); }

        // 最終アクセス時間
        const std::string get_access_time_str();
        const time_t& get_access_time() const { return m_access_time.tv_sec; } // 秒
        const std::string& get_access_date(); // string型
        void reset_access_date(){ m_access_date = std::string(); }

        // 最終書き込み時間
        const time_t& get_write_time() const { return m_write_time.tv_sec; } // 秒
        const std::string& get_write_date(); // string型
        void reset_write_date(){ m_write_time_date = std::string(); }

        // 書き込み数
        const int get_num_posted();

        // 自分の書き込みか
        const bool is_posted( const int number );

        // 自分の書き込みにレスしたか
        const bool is_refer_posted( const int number );

        // 書き込みマークセット
        void set_posted( const int number, const bool set );

        // 書き込み履歴のリセット
        void clear_post_history();

        // スレ立て時刻
        const time_t& get_since_time() const { return m_since_time; };
        const std::string& get_since_date();
        void reset_since_date(){ m_since_date = std::string(); }

        // 更新時間
        time_t get_time_modified();
        const std::string& get_date_modified() { return m_date_modified; }
        void set_date_modified( const std::string& date ){ m_date_modified = date; }

        // スレが立ってからの経過時間( 時間 )
        const int get_hour();

        // http コード
        const int get_code() const { return m_code; }
        const std::string& get_str_code() const { return m_str_code; }

        // エラーメッセージ
        const std::string& get_ext_err() const { return m_ext_err; }

        // DAT落ちかどうかなどの状態 ( global.hで定義 )
        const int get_status() const{ return m_status; }
        void set_status( const int status ){ m_status = status; }
        
        void set_subject( const std::string& subject );
        void set_number( const int number, const bool is_online );
        void set_number_load( const int number_load );
        void set_number_seen( const int number_seen );
        void update_writetime();

        // キャッシュ削除
        // cache_only == true の時はキャッシュだけ削除してスレ情報は消さない
        virtual void delete_cache( const bool cache_only );

        // キャッシュ保存
        bool save_dat( const std::string& path_to );

        // HDDにキャッシュされているか
        const bool is_cached() const { return m_cached; }
        void set_cached( const bool set ){ m_cached = set; }

        // キャッシュがarticlebaseに読み込まれている(nodetree!=NULL)か
        const bool is_cache_read() const { return ( m_nodetree ); }

        // キャッシュがあって、かつ新着の読み込みが可能
        const bool enable_load();

        // キャッシュはあるが規定のレス数を越えていて、かつ全てのレスが既読
        const bool is_finished();

        // あぼーん情報
        const std::list< std::string >& get_abone_list_id(){ return m_list_abone_id; }
        const std::list< std::string >& get_abone_list_name(){ return m_list_abone_name; }
        const std::list< std::string >& get_abone_list_word(){ return m_list_abone_word; }
        const std::list< std::string >& get_abone_list_regex(){ return m_list_abone_regex; }
        const std::vector< char >& get_abone_vec_res(){ return m_vec_abone_res; }

        // 透明
        const bool get_abone_transparent();

        // 連鎖
        const bool get_abone_chain();

        // ageあぼーん
        const bool get_abone_age() const { return m_abone_age; }

        // 板レベルでのあぼーん
        const bool get_abone_board() const { return m_abone_board; }

        // 全体レベルでのあぼーん
        const bool get_abone_global() const { return m_abone_global; }        

        // number番のレスがあぼーんされているか
        const bool get_abone( int number );

        // 全レスのあぼーん状態の更新
        void update_abone();

        // あぼーん状態のリセット(情報セットと状態更新を同時におこなう)
        void reset_abone( const std::list< std::string >& ids,
                          const std::list< std::string >& names,
                          const std::list< std::string >& words,
                          const std::list< std::string >& regexs,
                          const std::vector< char >& vec_abone_res,
                          const bool transparent, const bool chain, const bool age,
                          const bool board, const bool global
            );

        // あぼ〜ん状態更新(reset_abone()と違って各項目ごと個別におこなう)
        void add_abone_id( const std::string& id );
        void add_abone_name( const std::string& name );
        void add_abone_word( const std::string& word );
        void set_abone_res( const int num_from, const int num_to, const bool set );
        void set_abone_transparent( const bool set ); // 透明
        void set_abone_chain( const bool set ); // 連鎖
        void set_abone_age( const bool set ); // age
        void set_abone_board( const bool set ); // 板レベルでのあぼーん
        void set_abone_global( const bool set ); // 全体レベルでのあぼーん


        // 「スレ」のブックマーク
        void set_bookmarked_thread( const bool bookmarked );
        const bool is_bookmarked_thread() const { return m_bookmarked_thread; }

        // 「レス」のブックマーク
        const int get_num_bookmark();
        const bool is_bookmarked( const int number );
        void set_bookmark( const int number, const bool set );

        // 情報ファイル読み込み
        void read_info();

        // 情報ファイル書き込み
        // キャッシュがあって、force = true の時は強制書き込み
        virtual void save_info( const bool force );

        const bool is_loading() const; // ロード中か
        const bool is_checking_update();  // 更新チェック中か

        // スレッドのロード停止
        void stop_load();

        // スレッドのロード開始
        // DAT落ちの場合はロードしないので、強制的にリロードしたいときは reset_status() で
        // ステータスをリセットしてからロードする
        // check_update : true の時はHEADによる更新チェックをおこなう
        virtual void download_dat( const bool check_update );

        // 前スレのアドレスを指定
        // 前スレのアドレスをセットしてからdownload_dat()を呼び出すと
        // ロード終了時( slot_load_finished() )に次スレ移行チェックをする
        void set_url_pre_article( const std::string& url_pre_article );

        // url_src で示されるスレの情報をコピー
        void copy_article_info( const std::string& url_src );

        // スレッド924か
        const bool is_924() const{ return m_924; }

      private:

        // 更新チェック可能
        virtual const bool enable_check_update(){ return true; }

        // NodeTree作成
        // もしNodeTreeが作られていなかったら作成
        JDLIB::ConstPtr< NodeTreeBase >& get_nodetree();

        virtual NodeTreeBase* create_nodetree(){ return NULL; }

        void reset_status();

        void slot_node_updated();
        void slot_load_finished();
        virtual void unlock_impl();

        // お気に入りのアイコンとスレビューのタブのアイコンに更新マークを表示
        // update == true の時に表示。falseなら戻す
        void show_updateicon( const bool update );

        // navi2ch互換情報ファイル書き込み
        void save_navi2ch_info();
    };
}

#endif
