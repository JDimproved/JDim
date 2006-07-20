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
        const size_t lng_dat() const { return m_lng_dat; }
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

        // URL を含むレス番号をリストにして取得
        std::list< int > get_res_with_url();

        // number番のレスを参照しているレス番号をリストにして取得
        std::list< int > get_res_reference( int number );

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

        // あぼーんのクリア
        void clear_abone();

        // あぼーんチェック
        bool check_abone_id( int number, std::list< std::string >& list_id );
        bool check_abone_name( int number, std::list< std::string >& list_name );
        bool check_abone_word( int number, std::list< std::string >& list_word );
        bool check_abone_regex( int number, std::list< std::string >& list_regex );
        bool check_abone_chain( int number );

        // 参照数(num_reference)と色のクリア
        void clear_reference();

        // number番のレスが参照しているレスのレス番号の参照数(num_reference)と色を更新する
        void update_reference( int number );

        // 発言数とIDの色のクリア
        void clear_id_name();

        // number番のレスの発言数を更新
        void update_id_name( int number );

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
        NODE* create_linknode( const char* text, int n, const char* link, int n_link, int color_text, bool bold = false,
                               bool img = false, int anc_from = 0, int anc_to = 0 );
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

        bool check_anchor( int mode, const char* str_in, int& n, char* str_out, char* str_link, int lng_link,
                           int& anc_from, int& anc_to );
        int str_to_int( const char* str, int& n );

        // 発言数( num_id_name )の更新
        // IDノードの色も変更する
        void set_num_id_name( NODE* header, int num_id_name );
    };
}

#endif
