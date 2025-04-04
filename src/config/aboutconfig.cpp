// ライセンス: GPL2

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define _DEBUG
#include "jddebug.h"

#include "aboutconfig.h"
#include "aboutconfigdiag.h"

#include "globalconf.h"
#include "configitems.h"
#include "defaultconf.h"

#include "skeleton/msgdiag.h"

#include "colorid.h"

enum
{
    CONFTYPE_STR = 0,
    CONFTYPE_INT,
    CONFTYPE_BOOL,
    CONFTYPE_COMMENT
};


using namespace CONFIG;


AboutConfig::AboutConfig( Gtk::Window* parent )
    : SKELETON::PrefDiag( parent, "", true )
    , m_hbox_search{ Gtk::ORIENTATION_HORIZONTAL, 0 }
    , m_label{ "動作保証外です。高度な設定を変更するとJDimが誤作動する場合があります。", false }
{
    CONFIG::bkup_conf();

    pack_widgets();
}


//
// widgetのパック
//
void AboutConfig::pack_widgets()
{
    signal_key_press_event().connect( sigc::mem_fun( *this, &AboutConfig::slot_key_press_event ) );

    m_hbox_search.pack_start( m_label );
    m_hbox_search.pack_end( m_toggle_search, Gtk::PACK_SHRINK );

    m_label.set_hexpand( true );
    m_toggle_search.set_image_from_icon_name( "edit-find-symbolic" );
    // m_toggle_search 以外にも m_search_bar を閉じる操作があるため双方向にバインドする
    m_binding_search = Glib::Binding::bind_property( m_toggle_search.property_active(),
                                                     m_search_bar.property_search_mode_enabled(),
                                                     Glib::BINDING_BIDIRECTIONAL );

    m_search_bar.add( m_search_entry );
    m_search_bar.connect_entry( m_search_entry );
    m_search_bar.set_hexpand( true );
    m_search_bar.set_show_close_button( true );

    m_search_entry.set_hexpand( true );
    m_search_entry.set_width_chars( 50 );
    m_search_entry.signal_changed().connect( sigc::mem_fun( *this, &AboutConfig::slot_entry_changed ) );

    m_liststore = Gtk::ListStore::create( m_columns );
    m_model_filter = Gtk::TreeModelFilter::create( m_liststore );
    m_model_filter->set_visible_func( sigc::mem_fun( *this, &AboutConfig::slot_visible_func ) );

    m_treeview.property_fixed_height_mode() = true;
    m_treeview.set_model( m_model_filter );
    m_treeview.set_search_entry( m_search_entry );
    m_treeview.set_size_request( 700, 400 );
    m_treeview.signal_row_activated().connect( sigc::mem_fun( *this, &AboutConfig::slot_row_activated ) );

    Gtk::TreeViewColumn* column = Gtk::manage( new Gtk::TreeViewColumn( "設定名", m_columns.m_col_name ) );
    column->set_fixed_width( 540 );
    column->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
    column->set_resizable( true );
    m_treeview.append_column( *column );
    Gtk::CellRenderer *cell = column->get_first_cell();
    if( cell ) column->set_cell_data_func( *cell, sigc::mem_fun( *this, &AboutConfig::slot_cell_data ) );

    column = Gtk::manage( new Gtk::TreeViewColumn( "値", m_columns.m_col_value ) );
    column->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
    column->set_resizable( true );
    m_treeview.append_column( *column );
    cell = column->get_first_cell();
    if( cell ) column->set_cell_data_func( *cell, sigc::mem_fun( *this, &AboutConfig::slot_cell_data ) );

    m_scrollwin.add( m_treeview );
    m_scrollwin.set_margin_bottom( 8 );
    m_scrollwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
    m_scrollwin.set_propagate_natural_height( true );
    m_scrollwin.set_propagate_natural_width( true );

    get_content_area()->pack_start( m_hbox_search, Gtk::PACK_SHRINK );
    get_content_area()->pack_start( m_search_bar, Gtk::PACK_SHRINK );
    get_content_area()->pack_start( m_scrollwin );

    set_title( "about:config 高度な設定" );
    show_all_children();

    append_rows();
}


//
// OK
//
void AboutConfig::slot_ok_clicked()
{
    SKELETON::MsgDiag mdiag( nullptr, "一部の設定はJDimを再起動しない限り有効になりません。JDimを再起動してください。" );
    mdiag.run();
}


//
// キャンセルが押されたら設定を戻す
//
void AboutConfig::slot_cancel_clicked()
{
    CONFIG::restore_conf();
}


/** @brief ダイアログ内で Ctrl+F が押されたときは検索ボックスを表示する
 *
 * @param[in] event キーイベントの状態
 * @return 検索ボックスが非表示のときは m_search_bar の処理から返す、それ以外は false (GDK_EVENT_PROPAGATE)
 */
bool AboutConfig::slot_key_press_event( GdkEventKey* event )
{
    if( ( event->state & GDK_CONTROL_MASK ) && event->keyval == 'f' ) {
        if( m_search_bar.get_search_mode() ) {
            m_search_entry.grab_focus();
        }
        else {
            m_search_bar.set_search_mode( true );
            return m_search_bar.handle_event( event );
        }
    }
    return false;
}


/**
 * @brief 検索ボックスに入力されたテキストで設定項目のフィルタリングを実行する
 */
void AboutConfig::slot_entry_changed()
{
    m_model_filter->refilter();
}


/** @brief 検索ボックスに入力されたテキストが設定の名称に含まれるかチェックする
 *
 * @details 入力は1つのキーワードとして検索する。
 * 空白でテキストを区切ってもAND検索やOR検索は行わない。
 * @param[in] iter 設定項目の行
 * @return チェックした行を表示するなら true
 * - 設定の名称に入力テキストが含まれるなら表示する
 * - 検索ボックスが非表示のときや入力テキストが空のときはすべての行を表示する
 */
bool AboutConfig::slot_visible_func( const Gtk::TreeModel::const_iterator& iter )
{
    if( ! m_search_bar.get_search_mode() ) return true;

    Glib::ustring query = m_search_entry.get_text();
    if( query.empty() ) return true;

    const Gtk::TreeModel::Row& row = *iter;
    // 検索中は設定カテゴリの名前や空行を取り除く
    if( row[ m_columns.m_col_type ] == CONFTYPE_COMMENT ) return false;

    // 検索クエリの前後にある空白文字は無視する
    constexpr auto is_not_space = []( gunichar uc ) { return uc != U' ' && uc != U'\u3000'; };
    if( auto it = std::find_if( query.begin(), query.end(), is_not_space ); it != query.begin() ) {
        query.erase( query.begin(), it );
    }
    if( auto rit = std::find_if( query.rbegin(), query.rend(), is_not_space ); rit != query.rbegin() ) {
        query.erase( rit.base(), query.end() );
    }

    const Glib::ustring& name = row[ m_columns.m_col_name ];
    return name.find( query ) != std::string::npos;
}


//
// 実際の描画の際に cellrendere のプロパティをセットするスロット関数
//
void AboutConfig::slot_cell_data( Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& it )
{
    Gtk::TreeModel::Row row = *it;
    Gtk::CellRendererText* rentext = dynamic_cast<Gtk::CellRendererText*>( cell );

    if( row[ m_columns.m_col_drawbg ] ){
        rentext->property_foreground() = CONFIG::get_color( COLOR_CHAR_HIGHLIGHT_TREE );
        rentext->property_foreground_set() = true;
        rentext->property_cell_background() = CONFIG::get_color( COLOR_BACK_HIGHLIGHT_TREE );
        rentext->property_cell_background_set() = true;
    }
    else {
        rentext->property_foreground_set() = false;
        rentext->property_cell_background_set() = false;
    }
}


void AboutConfig::append_rows()
{
    // ネットワーク
    append_row( "■ ネットワーク" );
    append_row(	"JDim ホームページのアドレス", get_confitem()->url_jdimhp, CONF_URL_JDIMHP );
    append_row(	"板一覧を取得するサーバ", get_confitem()->url_bbsmenu, CONF_URL_BBSMENU );
    append_row( "2chログイン認証サーバのアドレス", get_confitem()->url_login2ch, CONF_LOGIN2CH );
    append_row( "BE認証サーバのアドレス", get_confitem()->url_loginbe, CONF_LOGINBE );
    append_row( "2chにアクセスするときのエージェント名", get_confitem()->agent_for2ch, CONF_AGENT_FOR2CH );
    append_row( "2ch以外のサーバにアクセスするときのエージェント名", get_confitem()->agent_for_data, CONF_AGENT_FOR_DATA );
    append_row( "2chログイン認証サーバにアクセスするときのエージェント名", get_confitem()->x_2ch_ua, CONF_X_2CH_UA );
    append_row( "TLSでノンブロッキングI/Oを使用する", get_confitem()->tls_nonblocking, CONF_TLS_NONBLOCKING );
    append_row( "TLSでサーバー証明書を検証する (\"いいえ\"は安全性が低下)", get_confitem()->verify_cert, CONF_VERIFY_CERT );
    append_row( "スレの読み込み時のタイムアウト値(秒)", get_confitem()->loader_timeout, CONF_LOADER_TIMEOUT );
    append_row( "書き込み時のタイムアウト値(秒)", get_confitem()->loader_timeout_post, CONF_LOADER_TIMEOUT_POST );
    append_row( "画像等のデータのロード時のタイムアウト値(秒)", get_confitem()->loader_timeout_img, CONF_LOADER_TIMEOUT_IMG );
    append_row( "更新チェック時のタイムアウト値(秒)", get_confitem()->loader_timeout_checkupdate, CONF_LOADER_TIMEOUT_CHECKUPDATE );
    append_row( "一般データのダウンロード時のバッファサイズ(Kbyte)", get_confitem()->loader_bufsize, CONF_LOADER_BUFSIZE );
    append_row( "スレ一覧のダウンロード時のバッファサイズ(Kbyte)", get_confitem()->loader_bufsize_board, CONF_LOADER_BUFSIZE_BOARD );
    append_row( "同一ホストに対する最大コネクション数( 1 または 2 )", get_confitem()->connection_num, CONF_CONNECTION_NUM );
    append_row( "2chのクッキーを保存する", get_confitem()->use_cookie_hap, CONF_USE_COOKIE_HAP );
    append_row( "2chのクッキー", get_confitem()->cookie_hap, CONF_COOKIE_HAP );
    append_row( "BBSPINKのクッキー", get_confitem()->cookie_hap_bbspink, CONF_COOKIE_HAP_BBSPINK );

    // ツリービュー
    append_row( "" );
    append_row( "■ ツリービュー(板一覧、スレ一覧)" );
    append_row( "ツリービューでマウスホイールを回したときのスクロール量(行数)", get_confitem()->tree_scroll_size, CONF_TREE_SCROLL_SIZE );
    append_row( "ツリービューの行間スペース", get_confitem()->tree_ypad, CONF_TREE_YPAD );
    append_row( "ツリービューのエクスパンダを表示する", get_confitem()->tree_show_expanders, CONF_TREE_SHOW_EXPANDERS );
    append_row( "ツリービューのレベルインデント調整量(ピクセル)", get_confitem()->tree_level_indent, CONF_TREE_LEVEL_INDENT );
    append_row( "カテゴリやディレクトリを開いたときにスクロールする", get_confitem()->scroll_tree, CONF_SCROLL_TREE );
    append_row( "ツリービューの選択を表示中のビューと同期する ( 0: 同期しない 1: 同期する 2: 同期する(フォルダを開く) )", get_confitem()->select_item_sync, CONF_SCROLL_TREE );

    // 板一覧、履歴ビュー
    append_row( "" );
    append_row( "■ 板一覧、履歴ビュー" );
    append_row( "板移転時に確認ダイアログを表示する", get_confitem()->show_movediag, CONF_SHOW_MOVEDIAG );
    append_row( "板一覧内にあるリンクを全て板とみなす", get_confitem()->use_link_as_board, CONF_USE_LINK_AS_BOARD );
    append_row( "板一覧でカテゴリを常にひとつだけ開く", get_confitem()->open_one_category, CONF_OPEN_ONE_CATEGORY );
    append_row( "右ペーンが空の時にサイドバーを最大化する", get_confitem()->expand_sidebar, CONF_EXPAND_SIDEBAR );
    append_row( "ペーン境界をクリックしてサイドバーを開け閉めする", get_confitem()->open_sidebar_by_click, CONF_OPEN_SIDEBAR_BY_CLICK );
    append_row( "更新チェック時に板の更新もチェックする", get_confitem()->check_update_board, CONF_CHECK_UPDATE_BOARD );
    append_row( "履歴ビューの最大表示数", get_confitem()->historyview_size, CONF_HISTORYVIEW_SIZE );

    // お気に入り
    append_row( "" );
    append_row( "■ お気に入り" );
    append_row( "お気に入りでディレクトリを常にひとつだけ開く", get_confitem()->open_one_favorite, CONF_OPEN_ONE_FAVORITE );
    append_row( "スレをお気に入りに追加した時にしおりをセットする", get_confitem()->bookmark_drop, CONF_BOOKMARK_DROP );
    append_row( "起動時にお気に入りを自動でチェックする", get_confitem()->check_update_boot, CONF_CHECK_UPDATE_BOOT );
    append_row( "重複項目を登録する ( 0: 登録する 1: ダイアログ表示 2: 登録しない )", get_confitem()->check_favorite_dup, CONF_CHECK_FAVORITE_DUP );
    append_row( "登録時に挿入先ダイアログ表示 ( 0 : 表示 1: 表示せず先頭に追加 2: 表示せず最後に追加 )",
                get_confitem()->show_favorite_select_diag, CONF_SHOW_FAVORITE_SELECT_DIAG );

    // スレ一覧
    append_row( "" );
    append_row( "■ スレ一覧" );
    append_row( "インクリメント検索をする", get_confitem()->inc_search_board, CONF_INC_SEARCH_BOARD );
    append_row( "deleteを押したときに確認ダイアログを表示", get_confitem()->show_deldiag, CONF_SHOW_DELDIAG );
    append_row( "指定した値(時間)よりも後に立てられたスレを新着とみなす", get_confitem()->newthread_hour, CONF_NEWTHREAD_HOUR );
    append_row( "3ペーン時にスレ一覧やスレビューを最大化する", get_confitem()->expand_rpane, CONF_EXPAND_RPANE );
    append_row( "スレ一覧をロードする前にキャッシュにある一覧を表示する", get_confitem()->show_cached_board, CONF_SHOW_CACHED_BOARD );
    append_row( "お知らせスレ(924)のアイコンを表示する", get_confitem()->show_924, CONF_SHOW_924 );
    append_row( "dat落ちしたスレをNGスレタイトルから除く( 0: ダイアログ表示 1: 除く 2: 除かない )", get_confitem()->remove_old_abone_thread, CONF_REMOVE_OLD_ABONE_THREAD );
    append_row( "dat落ちしたスレを表示する", get_confitem()->show_oldarticle, CONF_SHOW_OLDARTICLE );

    // スレビュー
    append_row( "" );
    append_row( "■ スレビュー" );
    append_row( "マウスホイールを回したときのスクロール速度", get_confitem()->scroll_size, CONF_SCROLL_SIZE );
    append_row( "上下キーを押したときのスクロール速度", get_confitem()->key_scroll_size, CONF_KEY_SCROLL_SIZE );
    append_row( "pageup、pagedownキーを押したときのスクロール速度", get_confitem()->key_fastscroll_size, CONF_KEY_FASTSCROLL_SIZE );
    append_row( "リロード後に末尾に自動的に移動する", get_confitem()->jump_after_reload, CONF_JUMP_AFTER_RELOAD );
    append_row( "リロード後に新着レスに自動的に移動する", get_confitem()->jump_new_after_reload, CONF_JUMP_NEW_AFTER_RELOAD );
    append_row( "ポップアップとカーソルの間の間隔(ピクセル)", get_confitem()->margin_popup, CONF_MARGIN_POPUP );
    append_row( "画像ポップアップとカーソルの間の水平間隔(ピクセル)", get_confitem()->margin_imgpopup_x, CONF_MARGIN_IMGPOPUP_X );
    append_row( "画像ポップアップとカーソルの間の垂直間隔(ピクセル)", get_confitem()->margin_imgpopup, CONF_MARGIN_IMGPOPUP );
    append_row( "ポップアップが消えるまでの時間(ミリ秒)", get_confitem()->hide_popup_msec, CONF_HIDE_POPUP_MSEC );
    append_row( "多重ポップアップの説明を表示する", get_confitem()->instruct_popup, CONF_INSTRUCT_POPUP );    
    append_row( "レス番号の上にマウスオーバーしたときに参照ポップアップ表示する", get_confitem()->refpopup_by_mo, CONF_REFPOPUP_BY_MO );
    append_row( "「名前」の上にマウスオーバーしたときにポップアップ表示する", get_confitem()->namepopup_by_mo, CONF_NAMEPOPUP_BY_MO );
    append_row( "IDの上にマウスオーバーしたときにIDをポップアップ表示する", get_confitem()->idpopup_by_mo, CONF_IDPOPUP_BY_MO );
    append_row( "スレビューとスレ一覧の切り替え方法説明ダイアログを表示する", get_confitem()->instruct_tglart, CONF_INSTRUCT_TGLART );
    append_row( "画像ビューとスレビューの切り替え方法説明ダイアログを表示する", get_confitem()->instruct_tglimg, CONF_INSTRUCT_TGLIMG );
    append_row( "deleteを押したときに確認ダイアログを表示", get_confitem()->show_delartdiag, CONF_SHOW_DELARTDIAG );
    append_row( "リンクの下線を表示する", get_confitem()->draw_underline, CONF_DRAW_UNDERLINE );
    append_row( "レスを引用コピーするときに前に付ける引用文字", get_confitem()->ref_prefix, CONF_REF_PREFIX );
    append_row( "引用文字の後のスペース数", get_confitem()->ref_prefix_space, CONF_REF_PREFIX_SPACE );
    append_row( "RFC規定外の文字(^など)もURL判定に用いる", get_confitem()->loose_url, CONF_LOOSE_URL );
    append_row( "URLのパーセントエンコーディングをデコードして表示する", get_confitem()->percent_decode, CONF_PERCENT_DECODE );
    append_row( "再読み込みボタンを押したときに全タブを更新する", get_confitem()->reload_allthreads, CONF_RELOAD_ALLTHREAD );
    append_row( "発言(同一ID)数とレスの順番(何番目の投稿)をカウントする", get_confitem()->check_id, CONF_CHECK_ID );
    append_row( "レス参照数で色を変える回数(高)", get_confitem()->num_reference_high, CONF_NUM_REFERENCE_HIGH );
    append_row( "レス参照数で色を変える回数(低)", get_confitem()->num_reference_low, CONF_NUM_REFERENCE_LOW );
    append_row( "発言数で色を変える回数(高)", get_confitem()->num_id_high, CONF_NUM_ID_HIGH );
    append_row( "発言数で色を変える回数(低)", get_confitem()->num_id_low, CONF_NUM_ID_LOW );
    append_row( "WEB検索用のメニュー項目名", get_confitem()->menu_search_web, CONF_MENU_SEARCH_WEB );
    append_row( "WEB検索用のアドレス", get_confitem()->url_search_web, CONF_URL_SEARCH_WEB );
    append_row( "デフォルトで透明あぼーん", get_confitem()->abone_transparent, CONF_ABONE_TRANSPARENT );
    append_row( "デフォルトで連鎖あぼーん", get_confitem()->abone_chain, CONF_ABONE_CHAIN );
    append_row( "書き込み履歴のあるスレを削除する時にダイアログを表示", get_confitem()->show_del_written_thread_diag, CONF_SHOW_DEL_WRITTEN_THREAD_DIAG );
    append_row( "スレを削除する時に画像キャッシュも削除する ( 0: ダイアログ表示 1: 削除 2: 削除しない )", get_confitem()->delete_img_in_thread, CONF_DELETE_IMG_IN_THREAD );
    append_row( "最大表示可能レス数", get_confitem()->max_resnumber, CONF_MAX_RESNUMBER );
    append_row( "スレビューのテキストを描画する方法 ( 0: PangoGlyphString 1: PangoLayout )", get_confitem()->text_rendering_method, CONF_TEXT_RENDERING_METHOD );
    append_row( "(実験的な機能) スレビューの検索クエリをタブ間で共有する", get_confitem()->share_query_among_tabs, CONF_SHARE_QUERY_AMONG_TABS );

    // 書き込みウィンドウ
    append_row( "" );
    append_row( "■ 書き込みビュー" );
    append_row( "デフォルトの書き込み名", get_confitem()->write_name, CONF_WRITE_NAME );
    append_row( "デフォルトのメールアドレス", get_confitem()->write_mail, CONF_WRITE_MAIL );
    append_row( "書き込み時に確認ダイアログを表示しない", get_confitem()->always_write_ok, CONF_ALWAYS_WRITE_OK );
    append_row( "書き込み中ダイアログを表示しない", get_confitem()->hide_writing_dialog, CONF_HIDE_WRITING_DIALOG );
    append_row( "編集中のメッセージの保存確認ダイアログを表示する", get_confitem()->show_savemsgdiag, CONF_SHOW_SAVEMSGDIAG ); 
    append_row( "アスキーアートメニューの履歴の保持数", get_confitem()->aahistory_size, CONF_AAHISTORY );
    append_row( "書き込みログの最大サイズ(バイト)", get_confitem()->maxsize_postlog, CONF_MAXSIZE_POSTLOG );
    append_row( "ビューを閉じても書き込み欄の日本語のON/OFF状態を保つ", get_confitem()->keep_im_status, CONF_KEEP_IM_STATUS );

    // 画像
    append_row( "" );
    append_row( "■ 画像" );
    append_row( "画像ポップアップの幅(ピクセル)", get_confitem()->imgpopup_width, CONF_IMGPOPUP_WIDTH );
    append_row( "画像ポップアップの高さ(ピクセル)", get_confitem()->imgpopup_height, CONF_IMGPOPUP_HEIGHT );
    append_row( "画像ビューを開いたときにウィンドウサイズに合わせる", get_confitem()->zoom_to_fit, CONF_ZOOM_TO_FIT );
    append_row( "画像ビューのフォーカスが外れたら折りたたむ", get_confitem()->fold_image, CONF_FOLD_IMAGE );
    append_row( "埋め込み画像ビューを閉じたときにタブも閉じる", get_confitem()->hide_imagetab, CONF_HIDE_IMAGETAB );
    append_row( "指定したサイズ(M byte)より大きい画像を表示するときに警告する", get_confitem()->max_img_size, CONF_MAX_IMG_SIZE );
    append_row( "指定した画素数(M pixel)より大きい画像を表示するときに警告する", get_confitem()->max_img_pixel, CONF_MAX_IMG_PIXEL );
    append_row( "モザイクのレベル", get_confitem()->mosaic_size, CONF_MOSAIC_SIZE );
    append_row( "(実験的な機能) 画像をモザイクで開くときはグレースケール(白黒)で表示する", get_confitem()->use_grayscale_mosaic, CONF_USE_GRAYSCALE_MOSAIC );
    append_row( "画像ビューのスムージングレベル(0-2, 大きい程高画質で低速)", get_confitem()->imgmain_interp, CONF_IMGMAIN_INTERP );
    append_row( "インライン画像のスムージングレベル(0-2, 大きい程高画質で低速)", get_confitem()->imgemb_interp, CONF_IMGEMB_INTERP );
    append_row( "インライン画像の最大幅", get_confitem()->embimg_width, CONF_EMBIMG_WIDTH );
    append_row( "インライン画像の最大高さ", get_confitem()->embimg_height, CONF_EMBIMG_HEIGHT );
    append_row( "ポップアップ画像のスムージングレベル(0-2, 大きい程高画質で低速)", get_confitem()->imgpopup_interp, CONF_IMGPOPUP_INTERP );
    append_row( "画像のメモリキャッシュ枚数", get_confitem()->imgcache_size, CONF_IMGCACHE_SIZE );
    append_row( "deleteを押したときに確認ダイアログを表示", get_confitem()->show_delimgdiag, CONF_SHOW_DELIMGDIAG );

    // ウィンドウ
    append_row( "" );
    append_row( "■ ウィンドウ" );
    append_row( "タブにアイコンを表示する", get_confitem()->show_tab_icon, CONF_SHOW_TAB_ICON );
    append_row( "タブに表示する文字列の最小文字数", get_confitem()->tab_min_str, CONF_TAB_MIN_STR );
    append_row( "タブ上でマウスホイールを回転してタブを切り替える", get_confitem()->switchtab_wheel, CONF_SWITCHTAB_WHEEL );
    append_row( "他のビューを開くときのタブの位置 ( 0: 一番右端 1:右隣 2:左隣 )", get_confitem()->newtab_pos, CONF_NEWTAB_POS );
    append_row( "ツリービューで選択したビューを開くときのタブの位置 ( 0: 一番右端 1:右隣 2:左隣 )", get_confitem()->opentab_pos, CONF_OPENTAB_POS );
    append_row( "各ビューと枠との間の余白", get_confitem()->view_margin, CONF_VIEW_MARGIN );
    append_row( "自前でウィンドウ配置を管理する (Waylandでは設定に関係なく管理しない)", get_confitem()->manage_winpos, CONF_MANAGE_WINPOS );
    append_row( "スレビューのスクロールバーを左に配置する", get_confitem()->left_scrbar, CONF_LEFT_SCRBAR );
    append_row( "メニューバーを非表示にした時にダイアログを表示", get_confitem()->show_hide_menubar_diag, CONF_SHOW_HIDE_MENUBAR_DIAG );
    append_row( "状態変更時にメインステータスバーの色を変える", get_confitem()->change_stastatus_color, CONF_CHANGE_STASTATUS_COLOR );
    append_row( "状態変更時にスレビュータイトルの色を変える", get_confitem()->change_statitle_color, CONF_CHANGE_STATITLE_COLOR );
    append_row( "Client-Side Decorationを使うか( 0: 使わない 1: 使う 2: デスクトップに合わせる )",
                get_confitem()->use_header_bar, CONF_USE_HEADER_BAR );

    // 次スレ検索
    append_row( "" );
    append_row( "■ 次スレ検索" );
    append_row( "類似度判定のしきい値(小さいほど判定が甘くなる、最大10)", get_confitem()->threshold_next, CONF_THRESHOLD_NEXT );
    append_row( "移行時にお気に入りのアドレスと名前を自動更新する(0: しない, 1:する, 2:追加)", get_confitem()->replace_favorite_next, CONF_REPLACE_FAVORITE_NEXT );
    append_row( "お気に入り自動更新の確認ダイアログを表示する", get_confitem()->show_diag_replace_favorite, CONF_SHOW_DIAG_REPLACE_FAVORITE );
    append_row( "次スレ検索を開くときのタブの位置 ( 0: 次スレ検索タブ 1:新しいタブ 2:アクティブなタブを置き換え )", get_confitem()->boardnexttab_pos, CONF_BOARDNEXTTAB_POS );

    // スレタイ検索
    append_row( "" );
    append_row( "■ スレタイ検索" );
    append_row( "スレタイ検索用のメニュー項目名", get_confitem()->menu_search_title, CONF_MENU_SEARCH_TITLE );
    append_row( "スレタイ検索用のアドレス", get_confitem()->url_search_title, CONF_URL_SEARCH_TITLE );
    append_row( "スレタイ検索時にアドレスとスレタイを取得する正規表現", get_confitem()->regex_search_title, CONF_REGEX_SEARCH_TITLE );

    // その他
    append_row( "" );
    append_row( "■ その他" );
    append_row( "履歴メニューの表示数", get_confitem()->history_size, CONF_HISTORY_SIZE );
    append_row( "マウスジェスチャを有効にする", get_confitem()->enable_mg, CONF_ENABLE_MG );
    append_row( "マウスジェスチャの判定開始半径", get_confitem()->mouse_radius, CONF_MOUSE_RADIUS );
    append_row( "数字入力ジャンプの待ち時間(ミリ秒)", get_confitem()->numberjmp_msec, CONF_NUMBERJMP_MSEC );
    append_row( "起動時に開いていたスレ一覧を復元する", get_confitem()->restore_board, CONF_RESTORE_BOARD );
    append_row( "起動時に開いていたスレを復元する", get_confitem()->restore_article, CONF_RESTORE_ARTICLE );
    append_row( "起動時に開いていた画像を復元する", get_confitem()->restore_image, CONF_RESTORE_IMAGE );
#ifdef HAVE_MIGEMO_H
    append_row( "migemoの辞書ファイルの場所(migemo使用時のみ)", get_confitem()->migemodict_path, CONF_MIGEMO_PATH );
#endif
    append_row( "FIFOの作成などにエラーがあったらダイアログを表示する", get_confitem()->show_diag_fifo_error, CONF_SHOW_DIAG_FIFO_ERROR );
    append_row( "指定した分ごとにセッションを自動保存 (0: 保存しない)", get_confitem()->save_session, CONF_SAVE_SESSION );
    append_row( "不正なMS932文字列をUTF-8と見なす", get_confitem()->broken_sjis_be_utf8, CONF_BROKEN_SJIS_BE_UTF8 );
    append_row( "不正な数値文字参照(サロゲートペア)をデコードする", get_confitem()->correct_character_reference, CONF_CORRECT_CHAR_REFERENCE );
    append_row( "(安全でない) スレ一覧とスレビューのプロパティにあるエンコーディング設定を有効にする", get_confitem()->choose_character_encoding, CONF_CHOOSE_CHAR_ENCODING );
}


void AboutConfig::append_row( const std::string& name, std::string& value, const std::string& defaultval )
{
    Gtk::TreeModel::Row row;
    row = *( m_liststore->append() );

    row[ m_columns.m_col_name ]  = name;
    row[ m_columns.m_col_type ] = CONFTYPE_STR;
    row[ m_columns.m_col_drawbg ] = false;
    row[ m_columns.m_col_value_str ] = &value;
    row[ m_columns.m_col_default_str ] = defaultval;

    set_value( row, value );
}


void AboutConfig::append_row( const std::string& name, int& value, const int defaultval )
{
    Gtk::TreeModel::Row row;
    row = *( m_liststore->append() );

    row[ m_columns.m_col_name ]  = name;
    row[ m_columns.m_col_type ] = CONFTYPE_INT;
    row[ m_columns.m_col_drawbg ] = false;
    row[ m_columns.m_col_value_int ] = &value;
    row[ m_columns.m_col_default_int ] = defaultval;

    set_value( row, value );
}


void AboutConfig::append_row( const std::string& name, bool& value, const bool defaultval )
{
    Gtk::TreeModel::Row row;
    row = *( m_liststore->append() );

    row[ m_columns.m_col_name ]  = name;
    row[ m_columns.m_col_type ] = CONFTYPE_BOOL;
    row[ m_columns.m_col_drawbg ] = false;
    row[ m_columns.m_col_value_bool ] = &value;
    row[ m_columns.m_col_default_bool ] = defaultval;

    set_value( row, value );
}


void AboutConfig::append_row( const std::string& comment )
{
    Gtk::TreeModel::Row row;
    row = *( m_liststore->append() );

    row[ m_columns.m_col_name ]  = comment;
    row[ m_columns.m_col_type ] = CONFTYPE_COMMENT;
    static_cast<void>( row ); // cppcheck: unreadVariable
}


void AboutConfig::set_value( Gtk::TreeModel::Row& row, const std::string& value )
{
    row[ m_columns.m_col_value ] = value;

    const std::string defaultval = row[ m_columns.m_col_default_str ];
    if( value != defaultval ) row[ m_columns.m_col_drawbg ] = true;
    else row[ m_columns.m_col_drawbg ] = false;
}


void AboutConfig::set_value( Gtk::TreeModel::Row& row, const int value )
{
    row[ m_columns.m_col_value ] = std::to_string( value );

    const int  defaultval = row[ m_columns.m_col_default_int ];
    if( value != defaultval ) row[ m_columns.m_col_drawbg ] = true;
    else row[ m_columns.m_col_drawbg ] = false;
}


void AboutConfig::set_value( Gtk::TreeModel::Row& row, const bool value )
{
    if( value ) row[ m_columns.m_col_value ] = "はい";
    else row[ m_columns.m_col_value ] = "いいえ";

    const bool defaultval = row[ m_columns.m_col_default_bool ];
    if( value != defaultval ) row[ m_columns.m_col_drawbg ] = true;
    else row[ m_columns.m_col_drawbg ] = false;
}


void AboutConfig::slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
#ifdef _DEBUG
    std::cout << "AboutConfig::slot_row_activated path = " << path.to_string() << std::endl;
#endif

    Gtk::TreeModel::Row row = *( m_model_filter->get_iter( path ) );
    if( ! row ) return;

    const int type = row[ m_columns.m_col_type ];
    if( type == CONFTYPE_COMMENT ) return;

    else if( type == CONFTYPE_STR ) {

        AboutConfigDiagStr pref( this, row[ m_columns.m_col_value_str ], row[ m_columns.m_col_default_str ] );
        pref.run();
        set_value( row, *row[ m_columns.m_col_value_str ] );
    }
    else if( type == CONFTYPE_INT ) {

        AboutConfigDiagInt pref( this, row[ m_columns.m_col_value_int ], row[ m_columns.m_col_default_int ] );
        pref.run();
        set_value( row, *row[ m_columns.m_col_value_int ] );
    }
    else if( type == CONFTYPE_BOOL ) {

        AboutConfigDiagBool pref( this, row[ m_columns.m_col_value_bool ], row[ m_columns.m_col_default_bool ] );
        pref.run();
        set_value( row, *row[ m_columns.m_col_value_bool ] );
    }
}
