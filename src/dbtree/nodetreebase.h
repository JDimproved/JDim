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
#include "jdlib/jdregex.h"

#include <map>
#include <set>
#include <string_view>
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
        std::string m_url_readcgi;
        std::string m_default_noname;

        // コード変換前の生データのサイズ ( byte )
        std::size_t m_lng_dat{};

        // レジュームのモード
        int m_resume;

        // レジューム時のチェック用
        bool m_resume_cached{};
        // 生データの先頭から RESUME_CHKSIZE バイト分を入れる
        char m_resume_head[ RESUME_CHKSIZE ];

        // レジューム中にスキップした生データサイズ
        std::size_t m_resume_lng{};

        // 現在処理中のヘッダ番号( つまりロード中でないなら総レス数になる )
        int m_id_header; 

        // サーバー側であぼーんがあったりしてスレが壊れている
        bool m_broken{};

        JDLIB::HEAP m_heap;
        std::vector< NODE* > m_vec_header;  // レスのヘッダのポインタの配列
        
        std::string m_subject;

        // 参照で色を変える回数
        int m_num_reference[ LINK_NUM ];

        // 発言数で色を変える回数
        int m_num_id[ LINK_NUM ];

        // 連続投稿したIDをスレのNG IDに追加 (回数)
        int m_abone_consecutive{}; ///< あぼーんにする連続投稿回数
        int m_consecutive_count{}; ///< 連続投稿した回数をカウントする
        const char* m_prev_link_id{}; ///< 前のレスのIDを保持して比較する
        std::vector<const char*> m_vec_abone_consecutive; ///< スレのNG IDに追加するIDを一時保存しておく

        // あぼーん情報
        // 実体は親のarticlebaseクラスが持っていてcopy_abone_info()でコピーする
        std::list< std::string > m_list_abone_id;   // あぼーんするID
        std::list< std::string > m_list_abone_name; // あぼーんする名前
        std::list< std::string > m_list_abone_word; // あぼーんする文字列
        std::list< JDLIB::RegexPattern > m_list_abone_regex; // あぼーんする正規表現

        std::list< std::string > m_list_abone_id_board;   // あぼーんするID(板レベル)
        std::list< std::string > m_list_abone_name_board; // あぼーんする名前(板レベル)
        std::list< std::string > m_list_abone_word_board; // あぼーんする文字列(板レベル)
        std::list< JDLIB::RegexPattern > m_list_abone_regex_board; // あぼーんする正規表現(板レベル)

        std::list< std::string > m_list_abone_word_global; // あぼーんする文字列(全体)
        std::list< JDLIB::RegexPattern > m_list_abone_regex_global; // あぼーんする正規表現(全体)
        std::unordered_set< int > m_abone_reses; // レスあぼーん情報
        bool m_abone_transparent{}; // 透明あぼーん
        bool m_abone_chain{}; // 連鎖あぼーん
        bool m_abone_age{}; // age ているレスはあぼーん
        bool m_abone_default_name{}; // デフォルト名無しのレスはあぼーん
        bool m_abone_noid{}; // ID無しのレスはあぼーん
        bool m_abone_board{}; // 板レベルでのあぼーんを有効にする
        bool m_abone_global{}; // 全体レベルでのあぼーんを有効にする

        // 自分が書き込んだレスか
        std::unordered_set< int > m_posts;

        // 自分の書き込みにレスしているか
        std::unordered_set< int > m_refer_posts;

        // 未来のレスに対するアンカーがある時に使用する
        // check_reference() を参照
        std::map< int, std::vector< int > > m_map_future_refer;

        // ロード用変数
        std::string m_buffer_lines;
        std::string m_parsed_text; // HTMLパーサに使うバッファ
        std::string m_buffer_write; // 書き込みチェック用バッファ
        std::string m_buf_text; ///< 画面に表示するテキスト用バッファ, parse_html() で使う
        std::string m_buf_link; ///< 編集したリンク用バッファ, parse_html() で使う
        bool m_check_update{}; // HEADによる更新チェックのみ
        bool m_check_write{}; // 自分の書き込みかチェックする
        bool m_loading_newthread{}; // 新スレ読み込み中

        // キャッシュ保存用ファイルハンドラ
        FILE* m_fout{};

        // パース用雑用変数
        NODE* m_node_previous{};

        // AA判定用
        JDLIB::RegexPattern m_aa_regex;

        // その他のエラーメッセージ
        std::string m_ext_err;

        /** @brief 各IDと発言数、レス番号のマッピング
         *
         * @details レスの順番( = 何番目の投稿 )を記録するため std::set を使ってレス番号順にソートする
         */
        std::unordered_map< std::string, std::set<int> > m_map_id_name_resnumber;

      protected:

        void set_resume( const bool resume );
        void set_broken( const bool broken ) { m_broken = broken; }
        int id_header() const noexcept { return m_id_header; }
        void set_ext_err( const std::string& ext_err ){ m_ext_err = ext_err; }

      public:

        NodeTreeBase( const std::string& url, const std::string& date_modified );
        ~NodeTreeBase() override;

        bool empty() const noexcept { return m_url.empty(); }
        void update_url( const std::string& url );

        SIG_UPDATED& sig_updated() { return m_sig_updated; }
        SIG_FINISHED& sig_finished() { return m_sig_finished; }

        // キャッシュかららロード
        void load_cache();

        const std::string& get_url() const { return m_url; }
        const std::string& get_subject() const { return m_subject; }
        int get_res_number() const;
        size_t get_lng_dat() const { return m_lng_dat; }
        bool is_broken() const{ return m_broken; }
        const std::string& get_ext_err() const { return m_ext_err; }
        bool is_checking_update() const { return m_check_update; }

        virtual int get_res_number_max() const noexcept { return -1; }
        // スレの最大DATサイズ(KB)
        virtual std::size_t get_dat_volume_max() const noexcept { return 0; }

        // number番のレスのヘッダノードのポインタを返す
        const NODE* res_header( int number ) const;
        NODE* res_header( int number )
        {
            return const_cast<NODE*>( static_cast<const NodeTreeBase&>( *this ).res_header( number ) );
        }

        // number番の名前
        std::string get_name( int number ) const;

        // number番の名前の重複数( = 発言数 )
        int get_num_name( int number ) const;

        // 指定した発言者の名前のレス番号をリストにして取得
        std::list< int > get_res_name( const std::string& name ) const;

        // number番のレスの時刻を文字列で取得
        // 内部で regex　を使っているので遅い
        std::string get_time_str( int number ) const;

        // number番のID
        std::string get_id_name( int number ) const;

        // 指定したID の重複数( = 発言数 )
        // 下のget_num_id_name( int number )と違って検索するので遅い
        int get_num_id_name( const std::string& id ) const;

        // number番のID の重複数( = 発言数 )
        int get_num_id_name( const int number ) const;

        // 指定した発言者IDを持つレス番号をリストにして取得
        std::list< int > get_res_id_name( const std::string& id_name ) const;

        // str_num で指定したレス番号をリストにして取得
        // str_num は "from-to"　の形式 (例) 3から10をセットしたいなら "3-10"
        // list_jointは出力で true のスレは前のスレに連結される (例) "3+4" なら 4が3に連結
        std::list< int > get_res_str_num( const std::string& str_num, std::list< bool >& list_joint ) const;

        // URL を含むレス番号をリストにして取得
        std::list< int > get_res_with_url() const;

        // ツリーに含まれてる 画像URL をリストにして取得
        std::list<std::string> get_imglinks() const;

        // number番のレスを参照しているレス番号をリストにして取得
        std::list< int > get_res_reference( const int number ) const;

        // res_num に含まれるレスを参照しているレス番号をリストにして取得
        std::list< int > get_res_reference( const std::list< int >& res_num ) const;

        // 高参照レスの番号をリストにして取得
        std::list< int > get_highly_referened_res() const;

        // query を含むレス番号をリストにして取得
        // mode_or == true なら OR抽出
        std::list< int > get_res_query( const std::string& query, const bool mode_or ) const;


        // number番のレスの文字列を返す
        // ref == true なら先頭に ">" を付ける
        std::string get_res_str( int number, bool ref = false ) const;

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
        bool get_abone( int number, Abone* abone = nullptr ) const;

        // あぼーん情報を親クラスのarticlebaseからコピーする
        void copy_abone_info( const std::list< std::string >& list_abone_id,
                              const std::list< std::string >& list_abone_name,
                              const std::list< std::string >& list_abone_word,
                              const std::list< std::string >& list_abone_regex,
                              const std::unordered_set< int >& abone_reses,
                              const bool abone_transparent, const bool abone_chain, const bool abone_age,
                              const bool abone_default_name, const bool abone_noid,
                              const bool abone_board, const bool abone_global );

        // 全レスのあぼーん状態の更新
        // 発言数や参照数も更新する
        void update_abone_all();

        // 自分が書き込んだレスか
        void copy_post_info( const std::unordered_set< int >& posts ){ m_posts = posts; }
        const std::unordered_set< int >& get_posts() const noexcept { return m_posts; }

        // 自分の書き込みにレスしたか
        bool is_refer_posted( const int number ) const;

        // 書き込みマークセット
        void set_posted( const int number, const bool set );

        // 書き込み履歴のリセット
        void clear_post_history();

      protected:

        virtual void clear();
        virtual void init_loading();

        // ロード用データ作成
        virtual void create_loaderdata( JDLIB::LOADERDATA& data ){}

        // 保存前にrawデータを加工
        // NOTE: 派生クラスでのrawデータ加工が不要になったため、メンバー関数を削除しました。
        virtual char* process_raw_lines( std::string& rawlines ) = delete;

        // raw データを dat に変換
        // デフォルトでは何もしない
        virtual const char* raw2dat( char* rawlines, int& byte ){
            byte = strlen( rawlines );
            return rawlines;
        }

        void receive_data( std::string_view buf ) override;
        void receive_finish() override;
        void sweep_buffer();

        // 拡張属性を取り出す
        virtual void parse_extattr( std::string_view str ) {}

      private:

        NODE* create_node();
        NODE* create_node_header();
        NODE* create_node_block();
        NODE* create_node_idnum();
        NODE* create_node_br();
        NODE* create_node_hr();
        NODE* create_node_space( const int type, const int bg );
        NODE* create_node_multispace( std::string_view text, const int bg, const char fontid = FONT_MAIN );
        NODE* create_node_link( std::string_view text, std::string_view link, const int color_text,
                                const int color_back, const bool bold, const char fontid = FONT_MAIN );
        NODE* create_node_anc( std::string_view text, std::string_view link,
                               const int color_text, const bool bold,
                               const ANCINFO* ancinfo, const int lng_ancinfo, const char fontid = FONT_MAIN );
        NODE* create_node_sssp( std::string_view link );
        NODE* create_node_img( std::string_view text, std::string_view link, const int color_text,
                               const bool bold, const char fontid = FONT_MAIN );
        NODE* create_node_text( std::string_view text, const int color_text, const bool bold = false, const char fontid = FONT_MAIN );
        NODE* create_node_ntext( const char* text, const int n, const int color_text, const int color_back = 0,
                                 const bool bold = false, const char fontid = FONT_MAIN );
        NODE* create_node_thumbnail( std::string_view text, std::string_view link, std::string_view thumb,
                                     const int color_text, const bool bold, const char fontid = FONT_MAIN );

        // 以下、構文解析用関数
        void add_raw_lines( std::string& buffer_lines );
        const char* add_one_dat_line( const char* datline );

        void parse_name( NODE* header, std::string_view str, const int color_name );
        void parse_mail( NODE* header, std::string_view str );
        void parse_date_id( NODE* header, std::string_view str );

        // HTMLパーサ
        // digitlink : true の時は先頭に数字が現れたらアンカーにする( parse_name() などで使う )
        //             false なら数字の前に >> がついてるときだけアンカーにする
        // bold : ボールド表示
        void parse_html( const char* str, std::size_t lng_str, const int color_text,
                         bool digitlink, const bool bold, const char fontid = FONT_MAIN );

        // 書き込みログ比較用文字列作成
        // m_buffer_write に作成した文字列をセットする
        void parse_write( std::string_view str, const std::size_t max_lng_write );

        bool check_anchor( const int mode, const char* str_in, int& n_in, std::string& str_out, std::string& str_link,
                           ANCINFO* ancinfo ) const;
        /// リンクが現れたかチェックして文字列を取得する関数
        int check_link( const char* str_in, int& lng_in, std::string& str_text, std::string& str_link ) const;

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


        // 発言数と何番目の投稿とIDの色のクリア
        void clear_id_name();

        // from_number 番から to_number 番までの発言数と何番目の投稿を更新
        void update_id_name( const int from_number, const int to_number );

        // number番のレスの発言数をチェック
        void check_id_name( const int number ) = delete;

        // 発言数( num_id_name )と何番目の投稿( posting_order )を更新
        // IDノードの色も変更する
        void set_num_id_name( HEADERINFO& headinfo, const int num_id_name, const int posting_order );


        // from_number番から to_number 番までのレスのフォント判定を更新
        void update_fontid( const int from_number, const int to_number );

        // number番のレスのフォント判定を更新
        void check_fontid( const int number );

      public:
        // http://ime.nu/ などをリンクから削除
        static bool remove_imenu( std::string& str_link );

        // あぼーんした理由をテキストで取得する
        static const char* get_abone_reason( Abone abone );
    };
}

#endif
