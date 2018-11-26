// ライセンス: GPL2

// View履歴クラス
//

#ifndef _VIEW_HISTORY_H
#define _VIEW_HISTORY_H

#include "viewhistoryitem.h"

#include <string>
#include <vector>

namespace HISTORY
{
    class ViewHistory
    {
        friend class History_Manager; // History_Manager 以外からは直接操作禁止

        std::vector< ViewHistoryItem* > m_items;

        int m_history_top;
        int m_history_current;
        int m_history_end;

        ViewHistory();
        virtual ~ViewHistory();

        int get_size() const { return m_items.size(); }
        const ViewHistoryItem* get_item( const int pos ){ return m_items[ pos ]; }

        int get_top() const { return m_history_top; }
        int get_cur() const { return m_history_current; }
        int get_end() const { return m_history_end; }

        void set_top( const int top );
        void set_cur( const int cur );
        void set_end( const int end );

        const std::string& get_current_url();
        const std::string& get_current_title();

        // URL更新
        void replace_current_url( const std::string& url ); // 現在のアドレス
        void replace_url( const std::string& url_old, const std::string& url_new ); // 全体

        // タイトル更新
        void replace_current_title( const std::string& title );

        // item の取得
        std::vector< ViewHistoryItem* >& get_items_back( const int count );
        std::vector< ViewHistoryItem* >& get_items_forward( const int count );

        // 戻る / 進む 可能かの判定
        bool can_back( const int count );
        bool can_forward( const int count );

        // 追加
        void append( const std::string& url );

        // 戻る / 進む
        // exec = true のときは履歴の位置を変更する
        // false の時はitemの取得のみ
        const ViewHistoryItem* back( const int count, const bool exec ); 
        const ViewHistoryItem* forward( const int count, const bool exec );
        const ViewHistoryItem* back_forward( const bool back, const int count, const bool exec );
    };
}

#endif
