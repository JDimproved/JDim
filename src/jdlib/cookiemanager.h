// ライセンス: GPL2

//
// HTTPクッキー管理マネージャ
//

#ifndef JDIM_COOKIEMANAGER_H
#define JDIM_COOKIEMANAGER_H

#include <string>


namespace JDLIB
{

//
// HTTPクッキー管理マネージャの抽象クラス
//
class CookieManager
{
public:

    virtual ~CookieManager() = default;

    // HTTPヘッダー Set-Cookie: の値からクッキーを登録する
    // hostname : inputにドメインがない場合はhostnameに登録
    // input : inputに渡す前にヘッダー名と空白は取り除いておく
    virtual void feed( const std::string& hostname, const std::string& input ) = 0;

    // ホスト名からクッキーを取得する
    // 返ってくる文字列は "name1=value1; name2=value2" の形式
    virtual std::string get_cookie_by_host( const std::string& hostname ) const = 0;

    // ホスト名からクッキーを削除する
    // ホスト名に含まれる上位レベルのドメインもまとめて削除する
    virtual void delete_cookie_by_host( const std::string& hostname ) = 0;
};


// クッキー管理マネージャのシングルトン オブジェクトを取得する
CookieManager* get_cookie_manager();
// シングルトン オブジェクトのクッキー管理マネージャを削除する
void delete_cookie_manager();

} // namespace JDLIB

#endif // JDIM_COOKIEMANAGER_H
