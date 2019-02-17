// ライセンス: GPL2

// キーボード設定ダイアログ

// KeyPref が本体で、KeyPrefの各行をダブルクリックすると KeyDiag が開いて個別に操作の設定が出来る
// KeyDiag の各行をダブルクリックすると KeyInputDiag が開いてキー入力が出来る

#ifndef _KEYPREFPREF_H
#define _KEYPREFPREF_H

#include "mousekeypref.h"

namespace CONTROL
{
    //
    // キーボード入力をラベルに表示するダイアログ
    //
    class KeyInputDiag : public CONTROL::InputDiag
    {
      public:

        KeyInputDiag( Gtk::Window* parent, const std::string& url, const int id );
    };


    ///////////////////////////////////////


    //
    // 個別のショートカットキー設定ダイアログ
    //
    class KeyDiag : public CONTROL::MouseKeyDiag
    {
      public:

        KeyDiag( Gtk::Window* parent, const std::string& url, const int id, const std::string& str_motions );

      protected:

        InputDiag* create_inputdiag() override;
        std::string get_default_motions( const int id ) override;
        std::vector< int > check_conflict( const int mode, const std::string& str_motion ) override;
    };


    ///////////////////////////////////

    //
    // キーボード設定ダイアログ
    //
    class KeyPref : public CONTROL::MouseKeyPref
    {
      public:

        KeyPref( Gtk::Window* parent, const std::string& url );

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
