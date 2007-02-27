// ライセンス: GPL2

//
// AA 管理クラス
//

#ifndef _AAMANAGER_H
#define _AAMANAGER_H

#include <string>
#include <list>

namespace CORE
{
    class AAManager
    {
        bool m_save;

        std::list< std::string > m_list_label;
        std::list< std::string > m_list_aa;

    public:

        AAManager();
        virtual ~AAManager();

        int get_size(){ return m_list_label.size(); }

        // num 番のアイテムを先頭に移動
        void move_to_top( int num );

        std::list< std::string >& get_labels(){ return m_list_label; }
        const std::string& get_aa( int num );
    };


    CORE::AAManager* get_aamanager();
    void delete_aamanager();
}

#endif
