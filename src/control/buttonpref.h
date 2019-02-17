// ライセンス: GPL2

// マウスボタン設定ダイアログ

// ButtonPref が本体で、ButtonPrefの各行をダブルクリックすると ButtonDiag が開いて個別に操作の設定が出来る
// ButtonDiag の各行をダブルクリックすると ButtonInputDiag が開いてキー入力が出来る


#ifndef _BUTTONPREFPREF_H
#define _BUTTONPREFPREF_H

#include "mousekeypref.h"
#include "control.h"

namespace CONTROL
{
    //
    // ボタン入力をラベルに表示するダイアログ
    //
    class ButtonInputDiag : public CONTROL::InputDiag
    {
      public:

        ButtonInputDiag( Gtk::Window* parent, const std::string& url, const int id );
    };


    ///////////////////////////////////////


    //
    // 個別のボタン設定ダイアログ
    //
    class ButtonDiag : public CONTROL::MouseKeyDiag
    {
      public:

        ButtonDiag( Gtk::Window* parent, const std::string& url, const int id, const std::string& str_motions );

      protected:

        InputDiag* create_inputdiag() override;
        std::string get_default_motions( const int id ) override;
        std::vector< int > check_conflict( const int mode, const std::string& str_motion ) override;
    };


    ///////////////////////////////////////


    //
    // ボタン設定ダイアログ
    //
    class ButtonPref : public CONTROL::MouseKeyPref
    {
      public:

        ButtonPref( Gtk::Window* parent, const std::string& url );

      protected:

        MouseKeyDiag* create_setting_diag( const int id, const std::string& str_motions ) override;
        std::string get_str_motions( const int id ) override;
        std::string get_default_motions( const int id ) override;
        void set_motions( const int id, const std::string& str_motions ) override;
        bool remove_motions( const int id ) override;

      private:

        void slot_cancel_clicked() override;
    };

}

#endif
