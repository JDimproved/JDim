// ライセンス: 最新のGPL

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
        
    public:

        // file : 設定ファイル
        // str_conf : 設定文字列
        // もしstr_confがemptyの時はfileから読み込む
        ConfLoader( const std::string& file, std::string str_conf );

        const bool empty();

        // 保存
        void save();

        // 値を変更
        // name が無い場合は綱目を追加
        void update( const std::string& name, const std::string& value );
        void update( const std::string& name, const int value );
        void update( const std::string& name, const double value );

        // 値取得
        std::string get_option( const std::string& name, std::string dflt );
        int get_option( const std::string& name, int dflt );
        double get_option( const std::string& name, double dflt );
    };
}

#endif
