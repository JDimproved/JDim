// ライセンス: 最新のGPL

//
// ファイルローダに渡すデータクラス
//

#ifndef _LOADERDATA_H
#define _LOADERDATA_H

#include <string>

namespace JDLIB
{
    class LOADERDATA
    {
    public:

        bool head;
        
        size_t length; // Content-Length の値
        size_t length_current; // 解凍前の現在までに読み込んでいるサイズ。よって length_current <= length
        size_t size_data; // 解凍したあとの純粋なデータサイズ。よって length < size_data となる場合もある
        
        std::string url;
        std::string protocol;
        std::string host;
        std::string path;
        long port;
        bool use_ssl; // https
        bool async; // 非同期ソケット使用

        std::string str_post;
        
        std::string host_proxy;
        long port_proxy;

        std::string agent;
        std::string referer;
        std::string ex_field;  // 送信時にヘッダに追加するフィールド
        std::string str_header;
        long code;
        std::string str_code;
        std::string date;
        std::string modified;
        size_t byte_readfrom;
        std::string cookie;
        std::string location;
        std::string contenttype;
        size_t size_buf;
        long timeout;
        
        LOADERDATA(){ init(); }

        void init(){

            head = false;
            length = 0;
            length_current = 0;
            size_data = 0;
            
            url.clear();
            protocol.clear();
            host.clear();
            path.clear();
            port = 0;
            use_ssl = false;
            async = true;
            
            str_post.clear();

            host_proxy.clear();
            port_proxy = 0;

            agent.clear();
            referer.clear();
            ex_field.clear();
            
            str_header.clear();
            code = -1;
            str_code.clear();
            date.clear();
            modified.clear();
            byte_readfrom = 0;
            cookie.clear();
            contenttype.clear();
            size_buf = 0;
            timeout = 0;
        }
    };
}

#endif
