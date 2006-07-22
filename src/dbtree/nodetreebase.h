// ライセンス: 最新のGPL

//
// ノードツリー( DOMみたいな木構造 )のベースクラス および DAT & HTMLパーサ
//

#ifndef _NODETREEBASE_H
#define _NODETREEBASE_H

#include "node.h"

#include "skeleton/loadable.h"

#include "jdlib/heap.h"

namespace JDLIB
{
    class LOADERDATA;
}

namespace DBTREE
{
    //ノードツリーのベースクラス
    class NodeTreeBase : public SKELETON::Loadable
    {
        typedef sigc::signal< void > SIG_UPDATED;
        typedef sigc::signal< void > SIG_FINISHED;
        SIG_UPDATED m_sig_updated;
        SIG_UPDATED m_sig_finished;

        std::string m_url;
        std::string m_default_noname;

        // コード変換前の生データのサイズ ( byte )
        size_t m_lng_dat; 

        // true ならレジューム読み込み
        bool m_resume;

        // 現在処理中のヘッダ番号( つまりロード中でないなら総レス数になる )
        int m_id_header; 

        // サーバー側であぼーんがあったりしてスレが壊れている
        bool m_broken;  

        JDLIB::HEAP m_heap;
        NODE** m_vec_header;  // レスのヘッダのポインタの配列
        
        std::string m_subject;

        // あぼーん情報
        // 実体は親のarticlebaseクラスが持っていてcopy_abone_info()でコピーする
        std::list< std::string > m_list_abone_id;   // あぼーんするID
        std::list< std::string > m_list_abone_name; // あぼーんする名前
        std::list< std::string > m_list_abone_word; // あぼーんする文字列
        std::list< std::string > m_list_abone_regex; // あぼーんする正規表現
        bool m_abone_transparent; // 透明あぼーん
        bool m_abone_chain; // 連鎖あぼーん

        // ロード用変数
        char* m_buffer_lines;
        int m_byte_buffer_lines_left;
        char* m_parsed_text;
        
        // キャッシュ保存用ファイルハンドラ
        FILE *m_fout;
        
        // パース用雑用変数
        NODE* m_node_previous;
        int m_id_node;

        // その他のエラーメッセージ
        std::string m_ext_err;

      protected:

        void set_resume( bool resume ) { m_resume = resume; }
        void set_broken( bool broken ) { m_broken = broken; }
        const int id_header() const { return m_id_header; }
        void set_ext_err( const std::string& ext_err ){ m_ext_err = ext_err; }

      public:

        NodeTreeBase( const std::string url, const std::string& date_modified );
        virtual ~NodeTreeBase();

        bool empty();

        SIG_UPDATED& sig_updated() { return m_sig_updated; }
        SIG_FINISHED& sig_finished() { return m_sig_finished; }

        // キャッシュかららロード
        void load_cache();

        const std::string& get_url() const { return m_url; }
        const std::string& get_subject() const { return m_subject; }
        const int get_res_number();
        const size_t get_lng_dat() const { return m_lng_dat; }
        const bool is_broken() const{ return m_broken; }
        const std::string& get_ext_err() const { return m_ext_err; }

        // number番のレスのヘッダノードのポインタを返す
        NODE* res_header( int number );

        // number番の名前
        const std::string get_name( int number );

        // number番のID
        const std::string get_id_name( int number );

        // 指定したID の重複数( = 発言数 )
        // 下のget_num_id_name( int number )と違って検索するので遅い
        int get_num_id_name( const std::string& id );

        // number番のID の重複数( = 発言数 )
        int get_num_id_name( int number );

        // 指定した発言者IDを持つレス番号をリストにして取得
        std::list< int > get_res_id_name( const std::string& id_name );

        // str_num で指定したレス番号をリストにして取得
        // str_num は "from-to"　の形式 (例) 3から10をセットしたいなら "3-10"
        std::list< int > get_res_str_num( const std::string& str_num );

        // URL を含むレス番号をリストにして取得
        std::list< int > get_res_with_url();

        // number番のレスを参照しているレス番号をリストにして取得
        std::list< int > get_res_reference( int number );

        // query を含むレス番号をリストにして取得
        // mode_or == true なら OR抽出
        std::list< int > get_res_query( const std::string& query, bool mode_or );


        // number番のレスの文字列を返す
        // ref == true なら先頭に ">" を付ける        
        const std::string get_res_str( int number, bool ref = false );
        
        // 明示的にhtml を加える
        // パースして追加したノードのポインタを返す
        // html は UTF-8　であること
        NODE* append_html( const std::string& html );

        // 明示的にdat を加える
        // パースして追加したノードのポインタを返す
        // dat は UTF-8　であること
        NODE* append_dat( const std::string& dat );

        // ロード開始
        void download_dat();

        // あぼーんしているか
        bool get_abone( int number );

        // あぼーん情報を親クラスのarticlebaseからコピーする
        void copy_abone_info( std::list< std::string >& list_abone_id, std::list< std::string >& list_abone_name,
                              std::list< std::string >& list_abone_word, std::list< std::string >& list_abone_regex,
                              bool& abone_transparent, bool& abone_chain );

        // 全レスのあぼーん状態の更新
        // 発言数や参照数も更新する
        void update_abone_all();


      protected:

        virtual void clear();
        virtual void init_loading();

        // ロード用デー多作制
        virtual void create_loaderdata( JDLIB::LOADERDATA& data ){}

        // 保存前にrawデータを加工
        // デフォルトでは何もしない
        virtual char* process_raw_lines( char* rawlines ){ return rawlines; }

        // raw データを dat に変換
        // デフォルトでは何もしない
        virtual const char* raw2dat( char* rawlines, int& byte ){
            byte = strlen( rawlines );
            return rawlines;
        }

        virtual void receive_data( const char* data, size_t size );
        virtual void receive_finish();

      private:

        NODE* createNode();        
        NODE* create_header_node();
        NODE* createBrNode();
        NODE* createSpNode( const int& type );
        NODE* create_node_downleft();
        NODE* create_linknode( const char* text, int n, const char* link, int n_link, int color_text, bool bold );
        NODE* create_ancnode( const char* text, int n, const char* link, int n_link, int color_text, bool bold,
                              ANCINFO* ancinfo, int lng_ancinfo );
        NODE* create_imgnode( const char* text, int n, const char* link, int n_link, int color_text, bool bold );
        NODE* createTextNode( const char* text, int color_text, bool bold = false );
        NODE* createTextNodeN( const char* text, int n, int color_text, bool bold = false );

        // 以下、構文解析用関数
        void add_raw_lines( char* rawines );
        const char* add_one_dat_line( const char* datline );

        void parseName( NODE* header, const char* str, int lng );
        void parseMail( const char* str, int lng );
        void parse_date_id( NODE* header, const char* str, int lng );
        void parse_html( const char* str, int lng, int color_text, bool digitlink = false, bool bold = false );
        void parseBr( );

        bool check_anchor( int mode, const char* str_in, int& n, char* str_out, char* str_link, int lng_link, ANCINFO* ancinfo );

        // レス番号のリストからあぼーんしている番号を取り除く
        std::list< int > remove_abone_from_list( std::list< int >& list_num );

        // あぼーんのクリア
        void clear_abone();

        // from_number番から to_number 番までのレスのあぼーん状態を更新
        void update_abone( int from_number, int to_number );

        // あぼーんチェック
        bool check_abone_id( int number );
        bool check_abone_name( int number );
        bool check_abone_word( int number );
        bool check_abone_regex( int number );
        bool check_abone_chain( int number );


        // 参照数(num_reference)と色のクリア
        void clear_reference();

        // from_number番から to_number 番までのレスが参照しているレスの参照数を更新
        void update_reference( int from_number, int to_number );

        // number番のレスが参照しているレスのレス番号の参照数(num_reference)と色をチェック
        void check_reference( int number );


        // 発言数とIDの色のクリア
        void clear_id_name();

        // from_number番から to_number 番までの発言数の更新
        void update_id_name( int from_number, int to_number );

        // number番のレスの発言数をチェック
        void check_id_name( int number );

        // 発言数( num_id_name )の更新
        // IDノードの色も変更する
        void set_num_id_name( NODE* header, int num_id_name );
    };
}

#endif
