// ライセンス: GPL2

//
// URL を開く
//

#ifndef _OPENURL_H
#define _OPENURL_H

#include "skeleton/prefdiag.h"
#include "skeleton/label_entry.h"

namespace CORE
{
    class OpenURLDialog : public SKELETON::PrefDiag
    {
        SKELETON::LabelEntry m_label_url;

      public:

        explicit OpenURLDialog( const std::string& url );
        ~OpenURLDialog() noexcept override = default;

      protected:

        void slot_ok_clicked() override;
    };
}


#endif
