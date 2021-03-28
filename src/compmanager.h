// ライセンス: GPL2

//
// Entryなどの補完管理クラス
//

#ifndef _COMPMANAGER_H
#define _COMPMANAGER_H

#include <string>
#include <vector>
#include <list>

namespace CORE
{

    enum
    {
        COMP_SEARCH_ARTICLE = 0,
        COMP_NAME,
        COMP_MAIL,
        COMP_SEARCH_BBSLIST,
        COMP_SEARCH_BOARD,

        COMP_SIZE
    };

    typedef std::list<std::string> COMPLIST; 
    typedef std::list<std::string>::iterator COMPLIST_ITERATOR; 

    class Completion_Manager
    {
        std::vector<COMPLIST> m_lists;

    public:
        Completion_Manager();
        virtual ~Completion_Manager() noexcept = default;

        COMPLIST get_list( const int mode, const std::string& query );
        void set_query( const int mode, const std::string& query );

        void clear( const int mode );

        void save_session();

      private:

        // 情報ファイル読み書き
        void load_info( const int mode );
        void save_info( const int mode );
    };

    ///////////////////////////////////////
    // インターフェース

    Completion_Manager* get_completion_manager();
    void delete_completion_manager();
}


#endif
