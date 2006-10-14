// ライセンス: 最新のGPL

//
// ユーザーコマンドの管理クラス
//

#ifndef _USRCMDMANAGER_H
#define _USRCMDMANAGER_H

#include <string>
#include <vector>

namespace CORE
{
    class Usrcmd_Manager
    {
        int m_size;

        std::vector< std::string > m_list_label;
        std::vector< std::string > m_list_cmd;
        std::vector< bool > m_list_openbrowser;

    public:
        Usrcmd_Manager();
        virtual ~Usrcmd_Manager(){}

        const int get_size() const { return m_size; }

        // 実行
        void exec( int num, const std::string& url, const std::string& link, const std::string& selection );

        bool is_sensitive( int num, const std::string& link, const std::string& selection );
        const std::string get_label( int num );

      private:

        void set_cmd( const std::string& label, const std::string& cmd );
    };

    ///////////////////////////////////////
    // インターフェース

    Usrcmd_Manager* get_usrcmd_manager();
    void delete_usrcmd_manager();
}


#endif
