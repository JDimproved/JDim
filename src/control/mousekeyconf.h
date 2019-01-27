// ライセンス: GPL2
//
// マウス、キーボード設定のベースクラス
//

#ifndef _MOUSEKEYCONF_H
#define _MOUSEKEYCONF_H

#include "mousekeyitem.h"
#include "controlutil.h"

#include "jdlib/confloader.h"

#include <gtkmm.h>
#include <vector>
#include <map>


namespace CONTROL
{
    class MouseKeyConf
    {
        std::vector< MouseKeyItem > m_vec_items;
        std::map< int, std::string > m_map_default_motions;
        
      public:

        MouseKeyConf();
        virtual ~MouseKeyConf() noexcept;

        // 設定ファイル読み込み 
        virtual void load_conf() = 0;

        // 設定ファイル保存
        void save_conf( const std::string& savefile );

        // 操作からID取得
        int get_id( const int mode,
                    const guint motion, const bool ctrl, const bool shift, const bool alt,
                    const bool dblclick, const bool trpclick );

        // ID から操作を取得
        // (注意) リストの一番上にあるものを出力
        bool get_motion( const int id,
                         guint& motion, bool& ctrl, bool& shift, bool& alt, bool& dblclick, bool& trpclick );

        // ID が割り当てられているかチェック
        bool alloted( const int id,
                      const guint motion, const bool ctrl, const bool shift, const bool alt,
                      const bool dblclick, const bool trpclick );

        // IDから操作文字列取得
        virtual const std::string get_str_motions( const int id );

        // IDからデフォルトの操作文字列取得
        virtual const std::string get_default_motions( const int id );

        // 同じモード内でモーションが重複していないかチェック
        // 戻り値 : コントロールID
        const std::vector< int > check_conflict( const int mode, const std::string& str_motion );

        // スペースで区切られた複数の操作をデータベースに登録
        void set_motions( const int id, const std::string& str_motions );

        // 指定したIDの操作を全て削除
        bool remove_motions( const int id );

      protected:

        std::vector< MouseKeyItem >& vec_items(){ return m_vec_items; }

        // 設定ファイルから読み込んだモーションを登録
        void load_motions( JDLIB::ConfLoader& cf, const std::string& name, const std::string& default_motions );
        void load_keymotions( JDLIB::ConfLoader& cf, const std::string& name, const std::string& default_motions );

        // デフォルト操作を登録
        void set_default_motions( const int id, const std::string& default_motions );

        // ひとつの操作をデータベースに登録
        void set_one_motion( const std::string& name, const std::string& str_motion );
        virtual void set_one_motion_impl( const int id, const int mode, const std::string& name, const std::string& str_motion ) = 0;
    };
}

#endif
