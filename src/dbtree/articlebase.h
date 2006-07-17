// ライセンス: 最新のGPL

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

#include "skeleton/lockable.h"

#include "jdlib/constptr.h"
#include "jdlib/heap.h"

namespace DBTREE
{
    class NodeTreeBase;
    class NODE;

    class ArticleBase : public SKELETON::Lockable
    {
        JDLIB::HEAP m_heap;

        // 情報ファイルのパス
        // デストラクタの中でCACHE::path_article_ext_info()を呼ぶとabortするので
        // ArticleBase::read_info()が呼ばれたときにパスを取得しておく
        std::string m_path_article_info;
        std::string m_path_article_ext_info;

        // m_nodetree は参照が外れたら自動でクリアされる
        JDLIB::ConstPtr< NodeTreeBase > m_nodetree;

        std::string m_url;           // dat ファイルのURL
        std::string m_id;            // ID ( .datなどの拡張子付き  (例) 1234567.dat )
        std::string m_key;           // ID から拡張子を取った物
        std::string m_date_modified; // サーバのデータが更新された時間
        time_t m_since_time;         // スレが立った時刻
        std::string m_since_date;    // スレ立て月日( string型 )
        int m_code;                  // HTTPコード
        std::string m_str_code;      // HTTPコード(文字列)
        std::string m_ext_err;       // HTTPコード以外のエラーメッセージ
        size_t m_lng_dat;            // dat ファイルのサイズ
        int m_status;                // 状態 ( global.h で定義 )

        // 移転する前にこのスレがあった旧ホスト名( 移転していないなら m_url に含まれているホスト名と同じ )
        // 詳しくはコンストラクタの説明を参照せよ
        std::string m_org_host;

        std::string m_subject;       // サブジェクト
        int m_number;                // サーバ上にあるレスの数
        int m_number_new;            // 新着数( ロードした時の差分読み込み数)
        int m_number_load;           // キャッシュにあるレスの数
        int m_number_before_load;    // ロード前のレスの数( m_number_new を計算するのに使う )
        int m_number_seen;           // どこまで読んだか
        struct timeval m_access_time;  // ユーザが最後にロードした時間
        struct timeval m_write_time;   // 最終書き込み時間
        std::string m_write_time_date; // 書き込み月日( string型 )
        std::string m_write_name;      // 書き込み時の名前
        std::string m_write_mail;      // 書き込み時のメアド
        bool m_write_fixname;          // 書き込み時名前固定
        bool m_write_fixmail;          // 書き込み時メール固定
        std::list< std::string > m_list_abone_id;   // あぼーんするID
        std::list< std::string > m_list_abone_name; // あぼーんする名前
        std::list< std::string > m_list_abone_word; // あぼーんする文字列
        std::list< std::string > m_list_abone_regex; // あぼーんする正規表現

        // あぼーん
        JDLIB::ConstPtr< char > m_abone; // あぼーん判定のキャッシュ
        
        // ブックマーク
        JDLIB::ConstPtr< char > m_bookmark; // ブックマーク判定キャッシュ

        // HDDにキャッシュされているか
        bool m_cached;

        // 情報ファイルを読みこんだらtrueにして2度読みしないようにする
        bool m_read_info; 

        // subject.txtに含まれているなら true
        // 実際にDAT落ちしたかどうかはサーバにアクセスして302か404が帰ってきたらDAT落ちと判定
        bool m_current;

        // true ならunlock_impl()がコールバックされたときに情報保存
        bool m_save_info;

      protected:
        void set_key( const std::string& key ){ m_key = key; }
        void set_since_time( time_t since ){ m_since_time = since; }
        void set_since_date( std::string since ){ m_since_date = since; }
        
      public:

        ArticleBase( const std::string& datbase, const std::string& id, bool cached );
        virtual ~ArticleBase();

        const bool empty();

        const std::string& get_url() const { return m_url; }

        // 移転があったときなどにURLを更新
        void update_url( const std::string& datbase );

        // 移転する前のオリジナルのURL
        const std::string get_org_url();

        // 移転する前のオリジナルのホスト名
        const std::string& get_org_host() const { return m_org_host; }
        void set_org_host( const std::string& host );

        const std::string& get_id() const { return m_id; }
        const std::string& get_key() const { return m_key; }
        const std::string& get_subject() const { return m_subject; }
        const int get_number() const { return m_number; }
        const int get_number_new() const { return m_number_new; }
        const int get_number_load() const { return m_number_load; }
        const int get_number_seen() const{  return m_number_seen; }

        // キャッシュにあるdatファイルのサイズ
        const size_t get_lng_dat();

        // nodetree の number 番のレスのヘッダノードのポインタを返す
        NODE* res_header( int number );

        // number番のレスの発言者の名前
        const std::string get_name( int number );

        // number番のレスの発言者ID( スレIDではなくて名前の横のID )
        const std::string get_id_name( int number );

        // 指定したた発言者ID の重複数( = 発言数 )
        // (注) 下の get_num_id_name( int number )と違って検索するので遅い
        int get_num_id_name( const std::string& id );

        // number番のた発言者ID の重複数( = 発言数 )
        int get_num_id_name( int number );

        // 指定した発言者IDを持つレス番号をリストにして取得
        std::list< int > get_res_id_name( const std::string& id_name );

        // str_num で指定したレス番号をリストにして取得
        // str_num は "from-to"　の形式 (例) 3から10をセットしたいなら "3-10"
        std::list< int > get_res_str_num( const std::string& str_num );

        // ブックマークをつけたレス番号をリストにして取得
        std::list< int > get_res_bm();

        // number番のレスを参照しているレス番号をリストにして取得
        std::list< int > get_res_reference( int number );

        // URL を含むレス番号をリストにして取得
        std::list< int > get_res_with_url();

        // query を含むレス番号をリストにして取得
        // mode_or == true なら OR抽出
        std::list< int > get_res_query( const std::string& query, bool mode_or );

        // number番のレスの文字列を返す
        // ref == true なら先頭に ">" を付ける        
        const std::string get_res_str( int number, bool ref = false );

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

        // 最終書き込み時間
        const time_t& get_write_time() const { return m_write_time.tv_sec; }
        const std::string& get_write_date() const { return m_write_time_date; }

        // スレ立て時刻
        const time_t& get_since_time() const { return m_since_time; };
        const std::string& get_since_date() const { return m_since_date; }

        // 更新時間
        time_t get_time_modified();
        const std::string& get_date_modified() { return m_date_modified; }

        // http コード
        const int get_code() const { return m_code; }
        const std::string& get_str_code() const { return m_str_code; }

        // エラーメッセージ
        const std::string& get_ext_err() const { return m_ext_err; }

        // 状態 ( global.hで定義 )
        const int get_status() const{ return m_status; }
        void reset_status();
        
        void set_subject( const std::string& subject );
        void set_number( int number );
        void set_number_load( int number_load );
        void set_number_seen( int number_seen );
        void update_writetime();

        void delete_cache();

        // HDDにキャッシュされているか
        const bool is_cached() const { return m_cached; }  

        // subject.txtに含まれているなら true
        const bool is_current() const { return m_current; };    
        void set_current( bool current ){ m_current = current; }

        // あぼーん情報
        std::list< std::string > get_abone_list_id(){ return m_list_abone_id; }
        std::list< std::string > get_abone_list_name(){ return m_list_abone_name; }
        std::list< std::string > get_abone_list_word(){ return m_list_abone_word; }
        std::list< std::string > get_abone_list_regex(){ return m_list_abone_regex; }

        // number番のレスがあぼーんされているか
        const bool abone( int number );

        // あぼーん状態のリセット(情報セットと状態更新を同時におこなう)
        void reset_abone( std::list< std::string >& ids, std::list< std::string >& names
                          , std::list< std::string >& words, std::list< std::string >& regexs );

        // あぼ〜んに追加(reset_abone()と違って個別におこなう)
        void add_abone_id( const std::string& id );
        void add_abone_name( const std::string& name );
        void add_abone_word( const std::string& word );

        // レスのブックマーク
        int get_num_bookmark();
        bool is_bookmarked( int number );
        void set_bookmark( int number, bool set );

        // 情報ファイル読み込み
        void read_info();

        // スレッドのロード開始
        const bool is_loading();
        void stop_load();
        void download_dat();

      private:

        JDLIB::ConstPtr< NodeTreeBase >& get_nodetree();
        virtual NodeTreeBase* create_nodetree(){ return NULL; }

        void slot_node_updated();
        void slot_load_finished();
        virtual void unlock_impl();

        // レス番号のリストからあぼーんしている番号を取り除く
        std::list< int > remove_abone_from_list( std::list< int >& list_num );

        // あぼーん状態の更新
        void update_abone();
        void check_abone( int from_number, int to_number );

        // from_number番から to_number 番までのレスが参照しているレスの参照数を更新
        void update_reference( int from_number, int to_number );

        // from_number番から to_number 番までの発言数の更新
        void update_id_name( int from_number, int to_number );

        // 情報ファイル書き込み
        void save_info();
        void save_navi2ch_info();
    };
}

#endif
