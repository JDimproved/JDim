// ライセンス: GPL2

//
// AA 管理クラス
//

#ifndef _AAMANAGER_H
#define _AAMANAGER_H

#include <string>
#include <list>
#include <vector>
#include <map>

namespace CORE
{
    class AAManager
    {
        std::list< int > m_history; // 履歴
        std::vector< std::string > m_vec_label; // メニューに表示するラベル
        std::vector< std::string > m_vec_aa;  // AA (1行AAの場合はラベルと同じ)
        std::map< int, char > m_map_shortcut; // ショートカットキー

    public:

        AAManager();
        virtual ~AAManager();

        int get_size(){ return m_vec_label.size(); }
        int get_historysize(){ return m_history.size(); }

        const std::string get_label( const int id );
        const std::string get_aa( const int id );

        // ショートカットキー取得
        const std::string id2shortcut( const int id );

        // ショートカットからid取得
        int shortcut2id( const char key );

        // id 番を履歴に追加
        void append_history( const int id );

        // num 番目の履歴をIDに変換
        int history2id( const int num );

        void save_history();

      private:
        void load_label();
        void load_history();
    };


    CORE::AAManager* get_aamanager();
    void delete_aamanager();
}

#endif
