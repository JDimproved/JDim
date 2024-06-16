// SPDX-License-Identifier: GPL-2.0-or-later

/** @file loginacorn.h
 *
 * @brief どんぐりシステムの警備員ログイン(メールアドレス認証)管理クラス
 * @details セッション管理やログイン、パスワードの保存などを行う
 */

#ifndef JDIM_LOGINACORN_H
#define JDIM_LOGINACORN_H

#include "skeleton/login.h"

#include <string>
#include <string_view>


namespace CORE
{

class LoginAcorn : public SKELETON::Login
{
    enum class Operation
    {
        login, ///< ログイン操作
        status, ///< アカウント名とIDを取得
        logout, ///< ログアウト操作
    };

    std::string m_rawdata;
    Operation m_operation{};

  public:

    LoginAcorn();
    ~LoginAcorn() override = default;

    void start_login() override;
    void logout() override;

  private:

    void reset();
    void receive_data( std::string_view buf ) override;
    void receive_finish() override;
    void receive_finish_redirect();
    void receive_finish_ok();
};

LoginAcorn* get_loginacorn();
void delete_loginacorn();

}

#endif // JDIM_LOGINACORN_H
