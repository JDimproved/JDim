// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "gtkmmversion.h"

#include "usrcmdmanager.h"
#include "command.h"
#include "cache.h"
#include "type.h"

#include "skeleton/msgdiag.h"
#include "skeleton/prefdiag.h"

#include "xml/tools.h"

#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"

#include "dbtree/interface.h"

#include "dbimg/imginterface.h"

#include "config/globalconf.h"

CORE::Usrcmd_Manager* instance_usrcmd_manager = NULL;

CORE::Usrcmd_Manager* CORE::get_usrcmd_manager()
{
    if( ! instance_usrcmd_manager ) instance_usrcmd_manager = new Usrcmd_Manager();
    assert( instance_usrcmd_manager );

    return instance_usrcmd_manager;
}


void CORE::delete_usrcmd_manager()
{
    if( instance_usrcmd_manager ) delete instance_usrcmd_manager;
    instance_usrcmd_manager = NULL;
}

///////////////////////////////////////////////

class ReplaceTextDiag : public SKELETON::PrefDiag
{
    Gtk::VBox m_vbox;
    Gtk::Entry m_entry;
    Gtk::Label m_label;

public:

    ReplaceTextDiag( Gtk::Window* parent, const std::string& title )
        : SKELETON::PrefDiag( parent, "" ), m_label( title + "を置き換えるテキストを入力してください。" )
    {
        resize( 640, 1 );

        m_vbox.pack_start( m_label, Gtk::PACK_SHRINK );
        m_vbox.pack_start( m_entry, Gtk::PACK_SHRINK );

        get_vbox()->set_spacing( 8 );
        get_vbox()->pack_start( m_vbox );

        set_title( "テキスト入力" );
        show_all_children();
    }

    const Glib::ustring get_text() { return m_entry.get_text(); }
};

///////////////////////////////////////////////

using namespace CORE;

Usrcmd_Manager::Usrcmd_Manager()
{
    std::string xml;
    if( CACHE::load_rawdata( CACHE::path_usrcmd(), xml ) ) m_document.init( xml );
    else txt2xml();

    analyze_xml();
}


//
// 旧式の設定ファイル(テキスト形式)をxmlに変換する
//
void Usrcmd_Manager::txt2xml()
{
    m_document.clear();

    // 旧設定ファイルからユーザーコマンドを読み込み
    const std::string file_usrcmd = CACHE::path_usrcmd_old();
    std::string usrcmd;

    if( CACHE::load_rawdata( file_usrcmd, usrcmd ) ){

        XML::Dom* root = m_document.appendChild( XML::NODE_TYPE_ELEMENT, std::string( ROOT_NODE_NAME_USRCMD ) );
        XML::Dom* subdir = root;

        std::list< std::string > list_usrcmd = MISC::get_lines( usrcmd );
        list_usrcmd = MISC::remove_commentline_from_list( list_usrcmd );
        list_usrcmd = MISC::remove_space_from_list( list_usrcmd );
        list_usrcmd = MISC::remove_nullline_from_list( list_usrcmd );

        // ユーザコマンドが 3つ以上 (廃止した max_show_usrcmd 設定の初期値 )より
        // 大きい場合はサブディレクトリ化する
        if( list_usrcmd.size() >= 3 * 2 ){
            subdir = root->appendChild( XML::NODE_TYPE_ELEMENT, XML::get_name( TYPE_DIR ) );
            subdir->setAttribute( "name", "ユーザコマンド" );
            subdir->setAttribute( "open", "y" );
        }

        std::list< std::string >::iterator it;
        for( it = list_usrcmd.begin(); it != list_usrcmd.end(); ++it ){

            const std::string name = *( it++ );
            if( it == list_usrcmd.end() ) break;
            const std::string cmd = *it;

            XML::Dom* usrcmd = subdir->appendChild( XML::NODE_TYPE_ELEMENT, XML::get_name( TYPE_USRCMD ) );
            usrcmd->setAttribute( "name", name );
            usrcmd->setAttribute( "data", cmd );
        }

#ifdef _DEBUG
        std::cout << "Usrcmd_Manager::txt2xml\n";
        std::cout << "nodes = " << m_document.childNodes().size() << std::endl;
        std::cout << m_document.get_xml() << std::endl;
#endif
        save_xml();
    }
}


//
// XML に含まれるコマンド情報を取り出してデータベースを更新
//
void Usrcmd_Manager::analyze_xml()
{
#ifdef _DEBUG
    std::cout << "Usrcmd_Manager::analyze_xml\n";
#endif

    m_list_cmd.clear();

    XML::DomList usrcmds = m_document.getElementsByTagName( XML::get_name( TYPE_USRCMD ) );

    std::list< XML::Dom* >::iterator it = usrcmds.begin();
    while( it != usrcmds.end() )
    {
        const std::string cmd = (*it)->getAttribute( "data" );

#ifdef _DEBUG
        const std::string name = (*it)->getAttribute( "name" );
        std::cout << (*it)->nodeName() << " " <<  name << " " << cmd << std::endl;
#endif

        set_cmd( cmd );

        ++it;
    }

    m_size = m_list_cmd.size();
}


void Usrcmd_Manager::set_cmd( const std::string& cmd )
{
    std::string cmd2 = MISC::remove_space( cmd );
    m_list_cmd.push_back( cmd2 );

#ifdef _DEBUG
    std::cout << "Usrcmd_Manager::set_cmd ";
    std::cout << "[" << m_list_cmd.size()-1 << "] " << cmd2 << std::endl;
#endif
}



//
// XML 保存
//
void Usrcmd_Manager::save_xml()
{
#ifdef _DEBUG
    std::cout << "Usrcmd_Manager::save_xml\n";
    std::cout << m_document.get_xml() << std::endl;
#endif

    CACHE::save_rawdata( CACHE::path_usrcmd(), m_document.get_xml() );
}


//
// 実行
//
void Usrcmd_Manager::exec( const int comnum, // コマンド番号
                           const std::string& url,
                           const std::string& link,
                           const std::string& selection, // 選択文字
                           const int number // レス番号
    )
{
    if( comnum >= m_size ) return;

    exec( m_list_cmd[ comnum ],
          url,
          link,
          selection,
          number );
}


void Usrcmd_Manager::exec( const std::string command, // コマンド
                           const std::string& url,
                           const std::string& link,
                           const std::string& selection, // 選択文字
                           const int number // レス番号
    )
{
    std::string cmd = command;

    if( cmd.find( "$ONLY" ) != std::string::npos ){

        std::list< std::string > lines = MISC::split_line( cmd );
        if( lines.size() >= 2 ){

            std::list< std::string >::iterator it = lines.begin();
            ++it; ++it;
            cmd = *it;
            ++it;
            for( ; it != lines.end(); ++it ) cmd += " " + ( *it );
        }
    }

#ifdef _DEBUG
    std::cout << "Usrcmd_Manager::exec\n"
              << "command = " << cmd << std::endl
              << "url = " << url << std::endl
              << "link = " << link << std::endl
              << "selection = " << selection << std::endl
              << "number = " << number << std::endl;
#endif

    bool use_browser = false;
    if( cmd.find( "$VIEW" ) == 0 ){
        use_browser = true;
        cmd = cmd.substr( 5 );
        cmd = MISC::remove_space( cmd );
    }

    bool show_dialog = false;
    if( cmd.find( "$DIALOG" ) == 0 ){
        show_dialog = true;
        cmd = cmd.substr( 7 );
        cmd = MISC::remove_space( cmd );
    }

    cmd = replace_cmd( cmd, url, link, selection, number );

#ifdef _DEBUG
    std::cout << "exec " << cmd << std::endl;
#endif

    if( cmd.empty() ) return;

    if( use_browser ) CORE::core_set_command( "open_url_browser", cmd );
    else if( show_dialog ){
        SKELETON::MsgDiag mdiag( NULL, cmd );
        mdiag.run();
    }
    else Glib::spawn_command_line_async( cmd );
}


//
// 文字列置換用テキストダイアログ表示
//
bool Usrcmd_Manager::show_replacetextdiag( std::string& texti, const std::string& title )
{
    if( ! texti.empty() ) return true;

    ReplaceTextDiag textdiag( NULL, title );
    if( textdiag.run() != Gtk::RESPONSE_OK ) return false;
    texti = textdiag.get_text();
    return true;
}


// コマンド置換
// cmdの$URLをurl, $LINKをlink, $TEXT*をtext, $NUMBERをnumberで置き換えて出力
// text は UTF-8 であること
std::string Usrcmd_Manager::replace_cmd( const std::string& cmd,
                                         const std::string& url,
                                         const std::string& link,
                                         const std::string& text,
                                         const int number )
{
    std::string cmd_out = cmd;
    const std::string oldhostl = DBTREE::article_org_host( link );
    const std::string oldhost = DBTREE::article_org_host( url );

    cmd_out = MISC::replace_str( cmd_out, "$URL", DBTREE::url_readcgi( url, 0, 0 ) );
    cmd_out = MISC::replace_str( cmd_out, "$DATURL", DBTREE::url_dat( url ) );
    cmd_out = MISC::replace_str( cmd_out, "$LOGPATH", CACHE::path_root() );
    cmd_out = MISC::replace_str( cmd_out, "$LOCALDATL", CACHE::path_dat( link ) );
    cmd_out = MISC::replace_str( cmd_out, "$LOCALDAT", CACHE::path_dat( url ) );
    cmd_out = MISC::replace_str( cmd_out, "$LINK", link );
    cmd_out = MISC::replace_str( cmd_out, "$NUMBER", MISC::itostr( number ) );

    if( cmd_out.find( "$CACHEDIMG" ) != std::string::npos ){
        cmd_out = MISC::replace_str( cmd_out, "$CACHEDIMG", DBIMG::get_cache_path( link ) );
    }

    // ホスト名(http://含む)
    cmd_out = MISC::replace_str( cmd_out, "$SERVERL", MISC::get_hostname( link ) );
    cmd_out = MISC::replace_str( cmd_out, "$SERVER", MISC::get_hostname( url ) );

    // ホスト名(http://含まない)
    cmd_out = MISC::replace_str( cmd_out, "$OLDHOSTNAMEL", MISC::get_hostname( oldhostl, false ) );
    cmd_out = MISC::replace_str( cmd_out, "$OLDHOSTNAME", MISC::get_hostname( oldhost, false ) );
    cmd_out = MISC::replace_str( cmd_out, "$OLDHOSTL", MISC::get_hostname( oldhostl, false ) );
    cmd_out = MISC::replace_str( cmd_out, "$OLDHOST", MISC::get_hostname( oldhost, false ) );
    cmd_out = MISC::replace_str( cmd_out, "$HOSTNAMEL", MISC::get_hostname( link, false ) );
    cmd_out = MISC::replace_str( cmd_out, "$HOSTNAME", MISC::get_hostname( url, false ) );
    cmd_out = MISC::replace_str( cmd_out, "$HOSTL", MISC::get_hostname( link, false ) );
    cmd_out = MISC::replace_str( cmd_out, "$HOST", MISC::get_hostname( url, false ) );

    // スレが属する板のID
    cmd_out = MISC::replace_str( cmd_out, "$BBSNAMEL", DBTREE::board_id( link ) );
    cmd_out = MISC::replace_str( cmd_out, "$BBSNAME", DBTREE::board_id( url ) );

    // スレのID
    cmd_out = MISC::replace_str( cmd_out, "$DATNAMEL", DBTREE::article_key( link ) );
    cmd_out = MISC::replace_str( cmd_out, "$DATNAME", DBTREE::article_key( url ) );

    cmd_out = MISC::replace_str( cmd_out, "$TITLE", DBTREE::article_subject( url ) );
    cmd_out = MISC::replace_str( cmd_out, "$BOARDNAME", DBTREE::board_name( url ) );

    // 範囲選択した文字列
    std::string texti = text;

    if( cmd_out.find( "$TEXTIU" ) != std::string::npos ){
        if( ! show_replacetextdiag( texti, "$TEXTIU" ) ) return std::string();
        cmd_out = MISC::replace_str( cmd_out, "$TEXTIU", MISC::charset_url_encode_split( texti, "UTF-8" ) );
    }
    if( cmd_out.find( "$TEXTIX" ) != std::string::npos ){
        if( ! show_replacetextdiag( texti, "$TEXTIX" ) ) return std::string();
        cmd_out = MISC::replace_str( cmd_out, "$TEXTIX", MISC::charset_url_encode_split( texti, "EUC-JP" ) );
    }
    if( cmd_out.find( "$TEXTIE" ) != std::string::npos ){
        if( ! show_replacetextdiag( texti, "$TEXTIE" ) ) return std::string();
        cmd_out = MISC::replace_str( cmd_out, "$TEXTIE", MISC::charset_url_encode_split( texti, "MS932" ) );
    }
    if( cmd_out.find( "$TEXTI" ) != std::string::npos ){
        if( ! show_replacetextdiag( texti, "$TEXTI" ) ) return std::string();
        cmd_out = MISC::replace_str( cmd_out, "$TEXTI",  texti );
    }

    if( cmd_out.find( "$TEXTU" ) != std::string::npos ){
        cmd_out = MISC::replace_str( cmd_out, "$TEXTU", MISC::charset_url_encode_split( texti, "UTF-8" ) );
    }
    if( cmd_out.find( "$TEXTX" ) != std::string::npos ){
        cmd_out = MISC::replace_str( cmd_out, "$TEXTX", MISC::charset_url_encode_split( texti, "EUC-JP" ) );
    }
    if( cmd_out.find( "$TEXTE" ) != std::string::npos ){
        cmd_out = MISC::replace_str( cmd_out, "$TEXTE", MISC::charset_url_encode_split( texti, "MS932" ) );
    }
    if( cmd_out.find( "$TEXT" ) != std::string::npos ){
        cmd_out = MISC::replace_str( cmd_out, "$TEXT",  texti );
    }

    // 入力ダイアログを表示
    std::string input;

    if( cmd_out.find( "$INPUTU" ) != std::string::npos ){
        if( ! show_replacetextdiag( input, "$INPUTU" ) ) return std::string();
        cmd_out = MISC::replace_str( cmd_out, "$INPUTU", MISC::charset_url_encode_split( input, "UTF-8" ) );
    }
    if( cmd_out.find( "$INPUTX" ) != std::string::npos ){
        if( ! show_replacetextdiag( input, "$INPUTX" ) ) return std::string();
        cmd_out = MISC::replace_str( cmd_out, "$INPUTX", MISC::charset_url_encode_split( input, "EUC-JP" ) );
    }
    if( cmd_out.find( "$INPUTE" ) != std::string::npos ){
        if( ! show_replacetextdiag( input, "$INPUTE" ) ) return std::string();
        cmd_out = MISC::replace_str( cmd_out, "$INPUTE", MISC::charset_url_encode_split( input, "MS932" ) );
    }
    if( cmd_out.find( "$INPUT" ) != std::string::npos ){
        if( ! show_replacetextdiag( input, "$INPUT" ) ) return std::string();
        cmd_out = MISC::replace_str( cmd_out, "$INPUT",  input );
    }

    // 結果のエンコード
    cmd_out = MISC::replace_str( cmd_out, "$OUTU", "" );
    cmd_out = MISC::replace_str( cmd_out, "$OUTX", "" );
    cmd_out = MISC::replace_str( cmd_out, "$OUTE", "" );

#ifdef _DEBUG
    std::cout << "Usrcmd_Manager::replace_cmd\n";
    std::cout << "cmd = " << cmd << std::endl;
    std::cout << "out = " << cmd_out << std::endl;
#endif

    return cmd_out;
}


//
// メニューをアクティブにするか
//
bool Usrcmd_Manager::is_sensitive( int num, const std::string& link, const std::string& selection )
{
    const unsigned int max_selection_str = 1024;

    if( num >= m_size ) return false;

    const std::string cmd = m_list_cmd[ num ];

    if( cmd.find( "$LINK" ) != std::string::npos
        || cmd.find( "$SERVERL" ) != std::string::npos
        || cmd.find( "$HOSTNAMEL" ) != std::string::npos
        || cmd.find( "$HOSTL" ) != std::string::npos
        || cmd.find( "$OLDHOSTNAMEL" ) != std::string::npos
        || cmd.find( "$OLDHOSTL" ) != std::string::npos
        || cmd.find( "$BBSNAMEL" ) != std::string::npos
        || cmd.find( "$DATNAMEL" ) != std::string::npos
        || cmd.find( "$LOCALDATL" ) != std::string::npos
        ){
        if( link.empty() ) return false;
    }

    if( cmd.find( "$TEXT" ) != std::string::npos && cmd.find( "$TEXTI" ) == std::string::npos ){

        if( selection.empty() || selection.length() > max_selection_str ) return false;
    }

    if( cmd.find( "$CACHEDIMG" ) != std::string::npos ){

        if( link.empty() ) return false;
        if( DBIMG::get_type_ext( link ) == DBIMG::T_UNKNOWN ) return false;
        if( ! DBIMG::is_cached( link ) ) return false;
    }

    return true;
}


//
// メニューを隠すか
//
bool Usrcmd_Manager::is_hide( int num, const std::string& url )
{
    if( num >= m_size ) return false;

    const std::string cmd = m_list_cmd[ num ];

#ifdef _DEBUG
    std::cout << "Usrcmd_manager::is_hide cmd = " << cmd << std::endl
              << "url = " << url << std::endl;
#endif

    if( cmd.find( "$ONLY" ) != std::string::npos ){

        std::list< std::string > lines = MISC::split_line( cmd );
        if( lines.size() >= 2 ){

            JDLIB::Regex regex;
            const size_t offset = 0;
            const bool icase = false;
            const bool newline = true;
            const bool usemigemo = false;
            const bool wchar = false;

            std::list< std::string >::iterator it = lines.begin();
            ++it;
            const std::string query = *it;
#ifdef _DEBUG
            std::cout << "query = " << query << std::endl;
#endif
            if( ! regex.exec( query, url, offset, icase, newline, usemigemo, wchar ) ) return true;
        }
    }

    return false;
}


//
// ユーザコマンドの登録とメニュー作成
//
std::string Usrcmd_Manager::create_usrcmd_menu( Glib::RefPtr< Gtk::ActionGroup >& action_group )
{
    int dirno = 0;
    int cmdno = 0;

    return create_usrcmd_menu( action_group, &m_document, dirno, cmdno );
}

// ユーザコマンドの登録とメニュー作成(再帰用)
std::string Usrcmd_Manager::create_usrcmd_menu( Glib::RefPtr< Gtk::ActionGroup >& action_group,
                                                XML::Dom* dom, int& dirno, int& cmdno )
{
    std::string menu;
    if( ! dom ) return menu;

    XML::DomList domlist = dom->childNodes();
    std::list< XML::Dom* >::iterator it = domlist.begin();
    while( it != domlist.end() )
    {
        if( (*it)->nodeType() == XML::NODE_TYPE_ELEMENT )
        {
#ifdef _DEBUG
            std::cout << "name = " << (*it)->nodeName() << std::endl;
#endif
            const int type = XML::get_type( (*it)->nodeName() );

            if( type == TYPE_DIR ){

                const std::string name = (*it)->getAttribute( "name" );
#ifdef _DEBUG
                std::cout << "[" << dirno << "] " << name << std::endl;
#endif
                const std::string dirname = "usrcmd_dir" + MISC::itostr( dirno );
                action_group->add( Gtk::Action::create( dirname, name ) );
                ++dirno;

                menu += "<menu action='" + dirname + "'>";
                menu += create_usrcmd_menu( action_group, *it, dirno, cmdno );
                menu += "</menu>";
            }

            else if( type == TYPE_SEPARATOR ){

                menu += "<separator/>";
            }

            else if( type == TYPE_USRCMD ){

                    const std::string name = (*it)->getAttribute( "name" );
#ifdef _DEBUG
                    std::cout << "[" << cmdno << "] " << name << std::endl;
#endif
                    const std::string cmdname = "usrcmd" + MISC::itostr( cmdno );
                    action_group->add( Gtk::Action::create( cmdname, name ) );
                    ++cmdno;

                    menu += "<menuitem action='" + cmdname + "'/>";
            }

            else if( (*it)->hasChildNodes() ) menu += create_usrcmd_menu( action_group, *it, dirno, cmdno );
        }
        ++it;
    }

    return menu;
}


Glib::RefPtr< Gtk::Action > Usrcmd_Manager::get_action( Glib::RefPtr< Gtk::ActionGroup >& action_group, const int num )
{
    const std::string str_cmd = "usrcmd" + MISC::itostr( num );
    return action_group->get_action( str_cmd );
}


//
// 選択不可かどうか判断して visible や sensitive を切り替える
//
void Usrcmd_Manager::toggle_sensitive( Glib::RefPtr< Gtk::ActionGroup >& action_group,
                                       const std::string& url_article,
                                       const std::string& url_link,
                                       const std::string& str_select
    )
{
    for( int i = 0; i < m_size; ++i ){

        Glib::RefPtr< Gtk::Action > act = get_action( action_group, i );
        if( act ){

#if GTKMM_CHECK_VERSION(2,6,0)
            if( is_hide( i, url_article ) ) act->set_visible( false );
            else{

                act->set_visible( true );

                if( is_sensitive( i, url_link, str_select ) ) act->set_sensitive( true );
                else{

                    act->set_sensitive( false );
                    if( CONFIG::get_hide_usrcmd() ) act->set_visible( false );
                }
            }
#else
            if( is_sensitive( i, url_link, str_select ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
#endif
        }
    }
}
