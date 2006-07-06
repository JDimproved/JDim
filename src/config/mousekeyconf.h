// ライセンス: 最新のGPL
//
// マウス、キーボード設定のベースクラス
//

#ifndef _MOUSEKEYCONF_H
#define _MOUSEKEYCONF_H

#include <gtkmm.h>
#include <vector>

#define SETMOTION(name,default_motion) do{\
str_motion = cf.get_option( name, default_motion ); \
set_motion( name, str_motion ); \
}while(0)

namespace CONFIG
{
    class MouseKeyItem;
    
    class MouseKeyConf
    {
        std::vector< MouseKeyItem* > m_vec_items;
        
      public:

        MouseKeyConf();
        virtual ~MouseKeyConf();

        // 操作からID取得
        int get_id( const int& mode,
                    const guint& motion, const bool& ctrl, const bool& shift, const bool& alt, const bool& dblclick );

        // ID から操作を取得
        // (注意) リストの一番上にあるものを出力
        bool get_motion( const int& id,
                         guint& motion, bool& ctrl, bool& shift, bool& alt, bool& dblclick );

        // ID が割り当てられているかチェック
        bool alloted( const int& id,
                      const guint& motion, const bool& ctrl, const bool& shift, const bool& alt, const bool& dblclick );

        // 操作文字列取得
        virtual const std::string get_str_motion( int id );

      protected:

        std::vector< MouseKeyItem* >& vec_items(){ return m_vec_items; }

        // 設定ファイル保存
        void save_conf( const std::string& savefile );

        // モーションが重複していないかチェック
        int check_conflict( const int& mode,
                            const guint& motion, const bool& ctrl, const bool& shift, const bool& alt, const bool& dblclick );

        // IDからモードを取得
        int get_mode( const int& id );

        // モーションセット
        void set_motion( const std::string& name, const std::string& str_motion );
        virtual void set_one_motion( const std::string& name, const std::string& str_motion ){}

        // 指定したIDのアイテムを削除
        void remove_items( int id );
    };
}

#endif
