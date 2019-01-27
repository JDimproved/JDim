// ライセンス: GPL2

#include "preference.h"

#include "dbtree/interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "config/globalconf.h"

#include "skeleton/msgdiag.h"

#include "cache.h"
#include "command.h"

#include <string>
#include <list>

using namespace ARTICLE;

Preferences::Preferences( Gtk::Window* parent, const std::string& url, const std::string command )
    : SKELETON::PrefDiag( parent, url )
    ,m_label_name( false, "スレタイトル : ", DBTREE::article_subject( get_url() ) )
    ,m_label_url( false, "スレのURL : ", DBTREE:: url_readcgi( get_url(),0,0 ) )
    ,m_label_url_dat( false, "DATファイルのURL : ", DBTREE:: url_dat( get_url() ) )
    ,m_label_cache( false, "ローカルキャッシュパス : ", std::string() )
    ,m_label_size( false, "サイズ( byte / Kbyte ) : ", std::string() )
    ,m_check_transpabone( "透明あぼ〜ん" )
    ,m_check_chainabone( "連鎖あぼ〜ん" )
    ,m_check_ageabone( "sage以外をあぼ〜ん" )
    ,m_check_boardabone( "板レベルでのあぼ〜んを有効にする" )
    ,m_check_globalabone( "全体レベルでのあぼ〜んを有効にする" )
    ,m_label_since( false, "スレ立て日時 : ", std::string() )
    ,m_label_modified( false, "最終更新日時 : ", std::string() )
    ,m_button_clearmodified( "日時クリア" )
    ,m_label_write( false, "最終書き込み日時 : ", std::string() )
    ,m_bt_clear_post_history( "書き込み履歴クリア" )
    ,m_label_write_name( false, "名前 : ", std::string() )
    ,m_label_write_mail( false, "メール : ", std::string() )
{
    // 一般
    if( DBTREE::article_is_cached( get_url() ) ){

        int size = DBTREE::article_lng_dat( get_url() );

        m_label_cache.set_text( CACHE::path_dat( get_url() ) );
        m_label_size.set_text( MISC::itostr( size )  + " / " + MISC::itostr( size/1024 ) );

        if( DBTREE::article_date_modified( get_url() ).empty() ) m_label_modified.set_text( "未取得" );
        else m_label_modified.set_text(
            MISC::timettostr( DBTREE::article_time_modified( get_url() ), MISC::TIME_WEEK )
            + " ( " + MISC::timettostr( DBTREE::article_time_modified( get_url() ), MISC::TIME_PASSED ) + " )"
            );

        if( DBTREE::article_write_time( get_url() ) ) m_label_write.set_text(
            MISC::timettostr( DBTREE::article_write_time( get_url() ), MISC::TIME_WEEK )
            + " ( " + MISC::timettostr( DBTREE::article_write_time( get_url() ), MISC::TIME_PASSED ) + " )"
            );
    }

    m_label_since.set_text(
            MISC::timettostr( DBTREE::article_since_time( get_url() ), MISC::TIME_WEEK )
            + " ( " + MISC::timettostr( DBTREE::article_since_time( get_url() ), MISC::TIME_PASSED ) + " )"
        );

    m_button_clearmodified.signal_clicked().connect( sigc::mem_fun(*this, &Preferences::slot_clear_modified ) );
    m_hbox_modified.pack_start( m_label_modified );
    m_hbox_modified.pack_start( m_button_clearmodified, Gtk::PACK_SHRINK );    

    m_bt_clear_post_history.signal_clicked().connect( sigc::mem_fun(*this, &Preferences::slot_clear_post_history ) );
    m_hbox_write.pack_start( m_label_write );
    m_hbox_write.pack_start( m_bt_clear_post_history, Gtk::PACK_SHRINK );    

    m_vbox_info.set_border_width( 16 );
    m_vbox_info.set_spacing( 8 );
    m_vbox_info.pack_start( m_label_name, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_url, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_url_dat, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_cache, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_size, Gtk::PACK_SHRINK );

    m_vbox_info.pack_start( m_label_since, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_hbox_modified, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_hbox_write, Gtk::PACK_SHRINK );

    if( DBTREE::write_fixname( get_url() ) ) m_label_write_name.set_text( DBTREE::write_name( get_url() ) );
    if( DBTREE::write_fixmail( get_url() ) ) m_label_write_mail.set_text( DBTREE::write_mail( get_url() ) );
    m_vbox_info.pack_start( m_label_write_name, Gtk::PACK_SHRINK );
    m_vbox_info.pack_start( m_label_write_mail, Gtk::PACK_SHRINK );

    // あぼーん設定

    m_vbox_abone.set_border_width( 16 );
    m_vbox_abone.set_spacing( 8 );

    // 透明あぼーん
    m_check_transpabone.set_active( DBTREE::get_abone_transparent( get_url() ) );

    // 連鎖あぼーん
    m_check_chainabone.set_active( DBTREE::get_abone_chain( get_url() ) );

    // ageあぼーん
    m_check_ageabone.set_active( DBTREE::get_abone_age( get_url() ) );

    // 板レベルあぼーん
    m_check_boardabone.set_active( DBTREE::get_abone_board( get_url() ) );

    // 全体レベルあぼーん
    m_check_globalabone.set_active( DBTREE::get_abone_global( get_url() ) );

    if( CONFIG::get_abone_transparent() ) m_check_transpabone.set_sensitive( false );
    if( CONFIG::get_abone_chain() ) m_check_chainabone.set_sensitive( false );

    m_vbox_abone.pack_start( m_check_transpabone, Gtk::PACK_SHRINK );
    m_vbox_abone.pack_start( m_check_chainabone, Gtk::PACK_SHRINK );
    m_vbox_abone.pack_start( m_check_ageabone, Gtk::PACK_SHRINK );
    m_vbox_abone.pack_start( m_check_boardabone, Gtk::PACK_SHRINK );
    m_vbox_abone.pack_start( m_check_globalabone, Gtk::PACK_SHRINK );

    if( CONFIG::get_abone_transparent() || CONFIG::get_abone_chain() ){
        m_label_abone.set_text( "チェック出来ない場合は設定メニューから「デフォルトで透明/連鎖あぼ〜ん」を解除して下さい" );
        m_label_abone.set_alignment( Gtk::ALIGN_START, Gtk::ALIGN_CENTER );
        m_vbox_abone.pack_start( m_label_abone, Gtk::PACK_SHRINK );
    }

    if( DBTREE::article_is_cached( get_url() ) ){ 

        std::string str_id, str_res, str_name, str_word, str_regex;
        std::list< std::string >::iterator it;

        // id
        std::list< std::string > list_id = DBTREE::get_abone_list_id( get_url() );
        for( it = list_id.begin(); it != list_id.end(); ++it ) if( ! ( *it ).empty() ) str_id += ( *it ) + "\n";
        m_edit_id.set_text( str_id );

        // あぼーんレス番号
        // 連番は 12-34 の様なフォーマットに変換
        std::vector< char > vec_res = DBTREE::get_abone_vec_res( get_url() );
        std::vector< char >::iterator it_res = vec_res.begin();
        int res = 0;
        int pre_res = 0;
        int count = 0;
        for( ; it_res != vec_res.end(); ++res ){

            if( ( *it_res ) ){
                if( ! pre_res ) pre_res = res;
                ++count;
            }

            ++it_res;

            if( !( *it_res ) || it_res == vec_res.end() ){

                if( pre_res ){
                    str_res += MISC::itostr( pre_res );
                    if( count > 1 ) str_res += "-" + MISC::itostr( pre_res + count -1 );
                    str_res += "\n";
                    pre_res = 0;
                    count = 0;
                }
            }
        }


        m_edit_res.set_text( str_res );

        // name
        std::list< std::string > list_name = DBTREE::get_abone_list_name( get_url() );
        for( it = list_name.begin(); it != list_name.end(); ++it ) if( ! ( *it ).empty() ) str_name += ( *it ) + "\n";
        m_edit_name.set_text( str_name );

        // word
        std::list< std::string > list_word = DBTREE::get_abone_list_word( get_url() );
        for( it = list_word.begin(); it != list_word.end(); ++it ) if( ! ( *it ).empty() ) str_word += ( *it ) + "\n";
        m_edit_word.set_text( str_word );

        // regex
        std::list< std::string > list_regex = DBTREE::get_abone_list_regex( get_url() );
        for( it = list_regex.begin(); it != list_regex.end(); ++it ) if( ! ( *it ).empty() ) str_regex += ( *it ) + "\n";
        m_edit_regex.set_text( str_regex );
    }
    else{
        m_edit_id.set_editable( false );
        m_edit_res.set_editable( false );
        m_edit_name.set_editable( false );
        m_edit_word.set_editable( false );
        m_edit_regex.set_editable( false );
    }

    m_label_abone_id.set_text( "ここでIDを削除してもレスが表示されない場合は板全体に対してIDがあぼーん指定されている可能性があります。\n板のプロパティのあぼーん設定も確認してください。" );
    m_vbox_abone.set_spacing( 8 );
    m_vbox_abone_id.pack_start( m_label_abone_id, Gtk::PACK_SHRINK );
    m_vbox_abone_id.pack_start( m_edit_id );

    m_notebook_abone.append_page( m_vbox_abone, "一般" );
    m_notebook_abone.append_page( m_vbox_abone_id, "NG ID" );
    m_notebook_abone.append_page( m_edit_res, "NG レス番号" );
    m_notebook_abone.append_page( m_edit_name, "NG 名前" );
    m_notebook_abone.append_page( m_edit_word, "NG ワード" );
    m_notebook_abone.append_page( m_edit_regex, "NG 正規表現" );

    m_notebook.append_page( m_vbox_info, "一般" );
    const int page_abone = 1;
    m_notebook.append_page( m_notebook_abone, "あぼ〜ん設定" );

    get_vbox()->pack_start( m_notebook );
    set_title( "「" + DBTREE::article_subject( get_url() ) + "」のプロパティ" );
    resize( 600, 400 );
    show_all_children();

    if( command == "show_abone" ) m_notebook.set_current_page( page_abone );
}

Preferences::~Preferences() noexcept
{}


//
// OK 押した
//
void Preferences::slot_ok_clicked()
{
    // あぼーん再設定
    std::list< std::string > list_id = MISC::get_lines( m_edit_id.get_text() );
    std::list< std::string > list_name = MISC::get_lines( m_edit_name.get_text() );
    std::list< std::string > list_word = MISC::get_lines( m_edit_word.get_text() );
    std::list< std::string > list_regex = MISC::get_lines( m_edit_regex.get_text() );

    // あぼーんレス番号
    std::vector< char > vec_abone_res;
    vec_abone_res.resize( DBTREE::article_number_load( get_url() ) + 1 );
    std::list< std::string > list_res = MISC::get_lines( m_edit_res.get_text() );
    std::list< std::string >::iterator it = list_res.begin();
    for( ; it != list_res.end(); ++it ){

        std::string num_str = ( *it );
        int number = atoi( num_str.c_str() );
        if( number >= 1 ){
            int number_end = number;
            size_t pos = num_str.find( "-" );
            if( pos != std::string::npos ) number_end = MIN( (int)vec_abone_res.size(), MAX( number, atoi( num_str.substr( pos + 1 ).c_str() ) ) );
            for( int i = number; i <= number_end; ++i ) vec_abone_res[ i ] = true;
        }
    }

    DBTREE::reset_abone( get_url(), list_id, list_name, list_word, list_regex, vec_abone_res
                         , m_check_transpabone.get_active(), m_check_chainabone.get_active(), m_check_ageabone.get_active(),
                         m_check_boardabone.get_active(), m_check_globalabone.get_active() );

    // viewの再レイアウト
    CORE::core_set_command( "relayout_article", get_url() );
}


void Preferences::slot_clear_modified()
{
    DBTREE::article_set_date_modified( get_url(), "" );

    if( DBTREE::article_date_modified( get_url() ).empty() ) m_label_modified.set_text( "未取得" );
    else m_label_modified.set_text(
        MISC::timettostr( DBTREE::article_time_modified( get_url() ), MISC::TIME_WEEK )
        + " ( " + MISC::timettostr( DBTREE::article_time_modified( get_url() ), MISC::TIME_PASSED ) + " )"
        );

}


void Preferences::slot_clear_post_history()
{
    if( m_label_write.get_text().empty() ) return;

    SKELETON::MsgDiag mdiag( NULL, "書き込み履歴を削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    DBTREE::article_clear_post_history( get_url() );
    m_label_write.set_text( "" );
    CORE::core_set_command( "redraw_article" );

    // BoardViewの行を更新
    CORE::core_set_command( "update_board_item", DBTREE::url_subject( get_url() ), DBTREE::article_id( get_url() ) );
}
