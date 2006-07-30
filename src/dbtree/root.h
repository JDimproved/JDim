// ライセンス: 最新のGPL

// データベースのルートクラス
//
// クラス図  [ Root ] ---> [ BoardBase ] ---> [ ArticleBase ] ---> [ NodeTreeBase ]
//

#ifndef _ROOT_H
#define _ROOT_H

#include "skeleton/loadable.h"

#include <string>
#include <list>


namespace DBTREE
{
    class BoardBase;

    // 鯖移転テーブル
    //
    // テーブルを更新するタイミングは下のふたつ
    //
    // (1) bbsmenuを読み込んで移転していた場合( Root::set_board() )
    //
    // 現在のホストを新ホストに移動する。キャッシュも移動する
    //
    // (2) 参照しようとした板が無かった時( Root::get_board() )
    //
    // 参照した古いホストの移動先を現在のホストに設定する。キャッシュは移動しない
    //
    struct MOVETABLE
    {
        std::string old_root;
        std::string new_root;
        std::string path_board;
    };

    
    class Root : public SKELETON::Loadable
    {
        // Boardクラス のキャッシュ
        // Boardクラスは一度作ったら~Root()以外ではdeleteしないこと
        std::list< BoardBase* > m_list_board;

        // 鯖移転テーブル
        std::list< MOVETABLE > m_movetable;

        std::string m_xml_bbsmenu;
        char* m_rawdata;
        size_t m_lng_rawdata;
        std::string m_xml_etc; // 外部板のXML
        std::string m_move_info;

        // NULL board クラス
        BoardBase* m_board_null;
        
      public:

        Root();
        ~Root();

        // 板一覧、外部板一覧のxml
        const std::string& xml_bbsmenu() const { return m_xml_bbsmenu; }
        const std::string& xml_etc() const { return m_xml_etc; }

        // Board クラスのポインタ取得
        BoardBase* get_board( const std::string& url, int count = 0 );

        // bbsmenuのダウンロード
        void download_bbsmenu();

        // 配下の全boardbaseクラスのレスあぼーん状態を更新する
        void update_abone_all_board();

        // 配下の全boardbaseクラスに、全articlebaseクラスのあぼーん状態の更新をさせる
        void update_abone_all_article();

      private:

        // bbsmenuのダウンロード用関数
        void clear();
        virtual void receive_data( const char* data, size_t size );
        virtual void receive_finish();
        void bbsmenu2xml( const std::string& menu );

        // XML に含まれる板情報を取り出してデータベースを更新
        void update_boards( const std::string xml );

        void load_cache();
        bool set_board( const std::string& url, const std::string& name );
        int is_moved( const std::string& root,
                       const std::string& path_board,
                       const std::string& name,
                       BoardBase** board_old );

        void load_etc();
        void load_movetable();
        void save_movetable();

        // urlのタイプ判定
        bool is_2ch( const std::string& url );
        bool is_JBBS( const std::string& url );
        bool is_machi( const std::string& url );
    };
}


#endif
