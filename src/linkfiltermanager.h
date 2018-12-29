// ライセンス: GPL2

//
// リンクフィルタの管理クラス
//

#ifndef _LINKFILTERMANAGER_H
#define _LINKFILTERMANAGER_H

#include <string>
#include <vector>

namespace CORE
{

    struct LinkFilterItem
    {
        std::string url;
        std::string cmd;
    };

    class Linkfilter_Manager
    {
        std::vector< LinkFilterItem > m_list_cmd;

    public:

        Linkfilter_Manager();
        virtual ~Linkfilter_Manager() noexcept {}

        std::vector< LinkFilterItem >& get_list(){ return  m_list_cmd; }
        void save_xml();

        // 実行
        // 実行したら true を返す
        const bool exec( const std::string& url, const std::string& link, const std::string& selection );

      private:

        void xml2list( const std::string& xml );
    };

    ///////////////////////////////////////
    // インターフェース

    Linkfilter_Manager* get_linkfilter_manager();
    void delete_linkfilter_manager();
}


#endif
