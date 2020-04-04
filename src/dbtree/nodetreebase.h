// ライセンス: GPL2

//
// ノードツリー( DOMみたいな木構造 )のベースクラス および DAT & HTMLパーサ
//

#ifndef _NODETREEBASE_H
#define _NODETREEBASE_H

#include "node.h"
#include "fontid.h"

#include "skeleton/loadable.h"

#include "jdlib/heap.h"
#include "jdlib/miscutil.h"

#include <map>
#include <cstring>
#include <unordered_map>
#include <unordered_set>

namespace JDLIB
{
    class LOADERDATA;
}

namespace DBTREE
{
    enum
    {
        LINK_LOW = 0,
        LINK_HIGH,
        LINK_NUM
    };

    constexpr size_t RESUME_CHKSIZE = 64;

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

        // レジュームのモード
        int m_resume;

        // レジューム時のチェック用
        bool m_resume_cached;
        // 生データの先頭から RESUME_CHKSIZE バイト分を入れる
        char m_resume_head[ RESUME_CHKSIZE ];

        // レジューム中にスキップした生データサイズ
        size_t m_resume_lng;

        // 現在処理中のヘッダ番号( つまりロード中でないなら総レス数になる )
        int m_id_header; 

        // サーバー側であぼーんがあったりしてスレが壊れている
        bool m_broken;  

        JDLIB::HEAP m_heap;
        std::vector< NODE* > m_vec_header;  // レスのヘッダのポインタの配列
        
        std::string m_subject;

        // 参照で色を変える回数
        int m_num_reference[ LINK_NUM ];

        // 発言数で色を変える回数
        int m_num_id[ LINK_NUM ];

        // あぼーん情報
        // 実体は親のarticlebaseクラスが持っていてcopy_abone_info()でコピーする
        std::list< std::string > m_list_abone_id;   // あぼーんするID
        std::list< std::string > m_list_abone_name; // あぼーんする名前
        std::list< std::string > m_list_abone_word; // あぼーんする文字列
        std::list< std::string > m_list_abone_regex; // あぼーんする正規表現

        std::list< std::string > m_list_abone_id_board;   // あぼーんするID(板レベル)
        std::list< std::string > m_list_abone_name_board; // あぼーんする名前(板レベル)
        std::list< std::string > m_list_abone_word_board; // あぼーんする文字列(板レベル)
        std::list< std::string > m_list_abone_regex_board; // あぼーんする正規表現(板レベル)

        std::list< std::string > m_list_abone_word_global; // あぼーんする文字列(全体)
        std::list< std::string > m_list_abone_regex_global; // あぼーんする正規表現(全体)
        std::unordered_set< int > m_abone_reses; // レスあぼーん情報
        bool m_abone_transparent; // 透明あぼーん
        bool m_abone_chain; // 連鎖あぼーん
        bool m_abone_age; // age ているレスはあぼーん
        bool m_abone_board; // 板レベルでのあぼーんを有効にする
        bool m_abone_global; // 全体レベルでのあぼーんを有効にする

        // 自分が書き込んだレスか
        std::unordered_set< int > m_posts;

        // 自分の書き込みにレスしているか
        std::unordered_set< int > m_refer_posts;
        // 上記の中で、新着のレスの内、自分の書き込みにレスをしているレス番号（新着返信）
        std::set< int > m_refer_posts_from_newres; // ordered

        // 未来のレスに対するアンカーがある時に使用する
        // check_reference() を参照
        std::map< int, std::vector< int > > m_map_future_refer;

        // ロード用変数
        std::string m_buffer_lines;
        std::string m_parsed_text; // HTMLパーサに使うバッファ
        std::string m_buffer_write; // 書き込みチェック用バッファ
        bool m_check_update; // HEADによる更新チェックのみ
        bool m_check_write; // 自分の書き込みかチェックする
        bool m_loading_newthread; // 新スレ読み込み中
        
        // キャッシュ保存用ファイルハンドラ
        FILE *m_fout;
        
        // パース用雑用変数
        NODE* m_node_previous;

        // AA判定用
        std::string m_aa_regex;

        // その他のエラーメッセージ
        std::string m_ext_err;

        // 各IDと発言数、レス番号のマッピング
        std::unordered_map< std::string, std::unordered_set< int > > m_map_id_name_resnumber;

      protected:

        void set_resume( const bool resume );
        void set_broken( const bool broken ) { m_broken = broken; }
        int id_header() const { return m_id_header; }
        void set_ext_err( const std::string& ext_err ){ m_ext_err = ext_err; }

      public:

        NodeTreeBase( const std::string& url, const std::string& date_modified );
        ~NodeTreeBase();

        bool empty();
        void update_url( const std::string& url );

        SIG_UPDATED& sig_updated() { return m_sig_updated; }
        SIG_FINISHED& sig_finished() { return m_sig_finished; }

        // キャッシュかららロード
        void load_cache();

        const std::string& get_url() const { return m_url; }
        const std::string& get_subject() const { return m_subject; }
        int get_res_number();
        size_t get_lng_dat() const { return m_lng_dat; }
        bool is_broken() const{ return m_broken; }
        const std::string& get_ext_err() const { return m_ext_err; }
        bool is_checking_update() const { return m_check_update; }

        // number番のレスのヘッダノードのポインタを返す
        NODE* res_header( int number );

        // number番の名前
        std::string get_name( int number );

        // number番の名前の重複数( = 発言数 )
        int get_num_name( int number );

        // 指定した発言者の名前のレス番号をリストにして取得
        std::list< int > get_res_name( const std::string& name );

        // number番のレスの時刻を文字列で取得
        // 内部で regex　を使っているので遅い
        std::string get_time_str( int number );

        // number番のID
        std::string get_id_name( int number );

        // 指定したID の重複数( = 発言数 )
        // 下のget_num_id_name( int number )と違って検索するので遅い
        int get_num_id_name( const std::string& id );

        // number番のID の重複数( = 発言数 )
        int get_num_id_name( const int number );

        // 指定した発言者IDを持つレス番号をリストにして取得
        std::list< int > get_res_id_name( const std::string& id_name );

        // str_num で指定したレス番号をリストにして取得
        // str_num は "from-to"　の形式 (例) 3から10をセットしたいなら "3-10"
        // list_jointは出力で true のスレは前のスレに連結される (例) "3+4" なら 4が3に連結
        std::list< int > get_res_str_num( const std::string& str_num, std::list< bool >& list_joint );

        // URL を含むレス番号をリストにして取得
        std::list< int > get_res_with_url();

        // 含まれる URL をリストにして取得
        std::list< std::string > get_urls();

        // number番のレスを参照しているレス番号をリストにして取得
        std::list< int > get_res_reference( const int number );

        // res_num に含まれるレスを参照しているレス番号をリストにして取得
        std::list< int > get_res_reference( const std::list< int >& res_num );

        // 高参照レスの番号をリストにして取得
        std::list< int > get_highly_referened_res();

        // query を含むレス番号をリストにして取得
        // mode_or == true なら OR抽出
        std::list< int > get_res_query( const std::string& query, const bool mode_or );


        // number番のレスの文字列を返す
        // ref == true なら先頭に ">" を付ける        
        std::string get_res_str( int number, bool ref = false );

        // 明示的にhtml を加える
        // パースして追加したノードのポインタを返す
        // html は UTF-8　であること
        NODE* append_html( const std::string& html );

        // 明示的にdat を加える
        // パースして追加したノードのポインタを返す
        // dat は UTF-8　であること
        NODE* append_dat( const std::string& dat );

        // ロード開始
        // check_update : HEADによる更新チェックのみ
        virtual void download_dat( const bool check_update );

        // あぼーんしているか
        bool get_abone( int number );

        // あぼーん情報を親クラスのarticlebaseからコピーする
        void copy_abone_info( const std::list< std::string >& list_abone_id,
                              const std::list< std::string >& list_abone_name,
                              const std::list< std::string >& list_abone_word,
                              const std::list< std::string >& list_abone_regex,
                              const std::unordered_set< int >& abone_reses,
                              const bool abone_transparent, const bool abone_chain, const bool abone_age,
                              const bool abone_board, const bool abone_global );

        // 全レスのあぼーん状態の更新
        // 発言数や参照数も更新する
        void update_abone_all();

        // 自分が書き込んだレスか
        void copy_post_info( const std::unordered_set< int >& posts ){ m_posts = posts; }
        const std::unordered_set< int >& get_posts() const noexcept { return m_posts; }

        // 自分の書き込みにレスしたか
        bool is_refer_posted( const int number );

        // 書き込みマークセット
        void set_posted( const int number, const bool set );

        // 書き込み履歴のリセット
        void clear_post_history();

        // 新着返信レス取得
        const std::set<int>& get_refer_posts_from_newres () const noexcept { return m_refer_posts_from_newres; }

      protected:

        virtual void clear();
        virtual void init_loading();

        // ロード用データ作成
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

        void receive_data( const char* data, size_t size ) override;
        void receive_finish() override;

      private:

        NODE* create_node();
        NODE* create_node_header();
        NODE* create_node_block();
        NODE* create_node_idnum();
        NODE* create_node_br();
        NODE* create_node_hr();
        NODE* create_node_space( const int type );
        NODE* create_node_multispace( const char* text, const int n, const char fontid = FONT_MAIN );
        NODE* create_node_htab();
        NODE* create_node_link( const char* text, const int n, const char* link, const int n_link, const int color_text, const bool bold, const char fontid = FONT_MAIN );
        NODE* create_node_anc( const char* text, const int n, const char* link, const int n_link,
                               const int color_text, const bool bold,
                               const ANCINFO* ancinfo, const int lng_ancinfo, const char fontid = FONT_MAIN );
        NODE* create_node_sssp( const char* link, const int n_link );
        NODE* create_node_img( const char* text, const int n, const char* link, const int n_link, const int color_text,
                               const bool bold, const char fontid = FONT_MAIN );
        NODE* create_node_text( const char* text, const int color_text, const bool bold = false, const char fontid = FONT_MAIN );
        NODE* create_node_ntext( const char* text, const int n, const int color_text, const bool bold = false, const char fontid = FONT_MAIN );
        NODE* create_node_thumbnail( const char* text, const int n, const char* link, const int n_link, const char* thumb, const int n_thumb,
                                     const int color_text, const bool bold, const char fontid = FONT_MAIN );

        // 以下、構文解析用関数
        void add_raw_lines( char* rawines, size_t size );
        const char* add_one_dat_line( const char* datline );

        void parse_name( NODE* header, const char* str, const int lng, const int color_name );
        void parse_mail( NODE* header, const char* str, const int lng );
        void parse_date_id( NODE* header, const char* str, const int lng );

        // HTMLパーサ
        // digitlink : true の時は先頭に数字が現れたらアンカーにする( parse_name() などで使う )
        //             false なら数字の前に >> がついてるときだけアンカーにする
        // bold : ボールド表示
        // ahref : <a href=～></a> からリンクノードを作成する
        void parse_html( const char* str, const int lng, const int color_text,
                         bool digitlink, const bool bold, const bool ahref, const char fontid = FONT_MAIN );

        // 書き込みログ比較用文字列作成
        // m_buffer_write に作成した文字列をセットする
        void parse_write( const char* str, const int lng, const std::size_t max_lng_write );

        bool check_anchor( const int mode, const char* str_in, int& n, char* str_out, char* str_link, int lng_link, ANCINFO* ancinfo );
        int check_link( const char* str_in, const int lng_in, int& n_in, char* str_link, const int lng_link );
        int check_link_impl( const char* str_in, const int lng_in, int& n_in, char* str_link, const int lng_link, const int linktype, const int delim_pos );

        // レジューム時のチェックデータをキャッシュ
        void set_resume_data( const char* data, size_t length );

        // あぼーんのクリア
        void clear_abone();

        // from_number番から to_number 番までのレスのあぼーん状態を更新
        void update_abone( const int from_number, const int to_number );

        // あぼーんチェック
        bool check_abone_res( const int number );
        bool check_abone_id( const int number );
        bool check_abone_name( const int number );
        bool check_abone_mail( const int number );
        bool check_abone_word( const int number );
        bool check_abone_chain( const int number );


        // number番のレスに含まれるレスアンカーをリストにして取得
        std::list< ANCINFO* > get_res_anchors( const int number );

        // 参照数(num_reference)と色のクリア
        void clear_reference();

        // from_number番から to_number 番までのレスが参照しているレスの参照数を更新
        void update_reference( int from_number, int to_number );

        // number番のレスが参照しているレスのレス番号の参照数(num_reference)と色をチェック
        void check_reference( const int number );

        // 参照数を count だけ増やしてして色を変更
        void inc_reference( NODE* head, const int count );


        // 発言数とIDの色のクリア
        void clear_id_name();

        // from_number番から to_number 番までの発言数の更新
        void update_id_name( const int from_number, const int to_number );

        // number番のレスの発言数をチェック
        void check_id_name( const int number );

        // 発言数( num_id_name )の更新
        // IDノードの色も変更する
        void set_num_id_name( NODE* header, const int num_id_name );


        // from_number番から to_number 番までのレスのフォント判定を更新
        void update_fontid( const int from_number, const int to_number );

        // number番のレスのフォント判定を更新
        void check_fontid( const int number );


        // http://ime.nu/ などをリンクから削除
        bool remove_imenu( char* str_link );

        // 文字列中の"&amp;"を"&"に変換する
        int convert_amp( char* text, const int n );
    };


    //
    // リンクが現れたかチェックして文字列を取得する関数
    //   (引数の値は、check_link_impl()を見ること)
    //
    inline int NodeTreeBase::check_link( const char* str_in, const int lng_in, int& n_in, char* str_link, const int lng_link )
    {
        // http://, https://, ftp://, ttp(s)://, tp(s):// のチェック
        int delim_pos = 0;
        const int linktype = MISC::is_url_scheme( str_in, &delim_pos );

        if( linktype == MISC::SCHEME_NONE ) return linktype;

        return check_link_impl( str_in, lng_in, n_in, str_link, lng_link, linktype, delim_pos );
    }
}

#endif
