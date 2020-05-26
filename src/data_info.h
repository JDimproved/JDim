// データクラス

// 関数の引数で使ったり、共有バッファ(sharedbuffer.h)と組み合わて使う

#ifndef _DATA_INFO_H
#define _DATA_INFO_H

#include <string>
#include <vector>

namespace Gtk
{
    class Window;
}

namespace CORE
{
    class DATA_INFO
    {
      public:

        int type{}; // type.h で定義されているタイプ
        Gtk::Window* parent{};
        std::string url;
        std::string name;
        std::string path; // treeview の path
        std::string data;
        size_t dirid{};  // ディレクトリID、ディレクトリで無い場合は0
        bool expanded{};
    };

    typedef std::vector< CORE::DATA_INFO > DATA_INFO_LIST;
}

#endif
