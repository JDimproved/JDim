// ライセンス: GPL2

//
// コンフィグファイルのローダ
//

#ifndef _CONFLOADER_H
#define _CONFLOADER_H

#include <string>
#include <list>

namespace JDLIB
{
    struct ConfData
    {
        std::string name;
        std::string value;
    };


    class ConfLoader
    {
        std::string m_file;
        std::list< ConfData > m_data;
        bool m_broken{};

    public:

        // file : 設定ファイル
        // str_conf : 設定文字列
        // もしstr_confがemptyの時はfileから読み込む
        ConfLoader( const std::string& file, std::string str_conf );

        bool empty() const noexcept { return m_data.empty(); }
        bool is_broken() const noexcept { return m_broken; }

        // 保存
        void save();

        // 値を変更
        // name が無い場合は綱目を追加
        void update( const std::string& name, const std::string& value );
        void update( const std::string& name, const bool value );
        void update( const std::string& name, const int value );
        void update( const std::string& name, const double value );

        // 値取得
        std::string get_option_str( const std::string& name, const std::string& dflt, const size_t maxlength = 0 );
        bool get_option_bool( const std::string& name, const bool dflt );
        int get_option_int( const std::string& name, const int dflt, const int min, const int max );
        double get_option_double( const std::string& name, const double dflt, const double min , const double max );
    };
}

#endif
