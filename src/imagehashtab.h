// SPDX-License-Identifier: GPL-2.0-or-later

// 全体あぼーん設定のNG 画像ハッシュタブ

#ifndef JDIM_IMAGEHASHTAB_H
#define JDIM_IMAGEHASHTAB_H

#include "config/defaultconf.h"
#include "config/globalconf.h"
#include "dbimg/imginterface.h"
#include "jdlib/miscgtk.h"
#include "jdlib/misctime.h"
#include "skeleton/msgdiag.h"

#include "environment.h"

#include <gtkmm.h>

#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <ios>


namespace CORE
{

/** @brief NG 画像ハッシュを設定するタブ
 *
 * シグナルハンドラを安全に切断するため sigc::trackable を継承する
 */
class ImageHashTab : public sigc::trackable
{
    struct Columns : Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<std::uint64_t> m_col_hash_row; ///< ハッシュA
        Gtk::TreeModelColumn<std::uint64_t> m_col_hash_col; ///< ハッシュB
        Gtk::TreeModelColumn<int> m_col_threshold; ///< しきい値
        Gtk::TreeModelColumn<Glib::ustring> m_col_last_matched; ///< 最後にマッチした日時
        Gtk::TreeModelColumn<Glib::ustring> m_col_source_url; ///< ハッシュのソースURL
        Gtk::TreeModelColumn<std::time_t> m_col_last_matched_time; ///< 最後にマッチした日時(time_t)

        Columns()
        {
            add( m_col_hash_row );
            add( m_col_hash_col );
            add( m_col_threshold );
            add( m_col_last_matched );
            add( m_col_source_url );
            add( m_col_last_matched_time );
        }
        ~Columns() noexcept override = default;
    };

    Gtk::Window* m_parent;
    Gtk::Box m_box;

    Gtk::Grid m_grid;

    // NG 画像ハッシュの有効・無効切り替え
    Gtk::ListBox m_listbox;
    Gtk::Box m_hbox_check_enable_hash;
    Gtk::Box m_vbox_check_enable_hash;
    Gtk::Switch m_switch_enable_hash;
    Gtk::Label m_label_enable_hash;

    // 初期設定のしきい値
    Gtk::Box m_hbox_initial_threshold;
    Gtk::Label m_label_initial_threshold;
    Gtk::SpinButton m_spin_initial_threshold;
    Gtk::Button m_button_reset_initial_threshold;

    // 選択した行に対する操作
    Gtk::Box m_box_tool;
    Gtk::Label m_label_tool;
    Gtk::Button m_button_delete;
    Gtk::MenuButton m_button_set_threshold;
    Gtk::Button m_button_copy;
    Glib::RefPtr<Gio::Menu> m_menu_set_threshold;
    Glib::RefPtr<Gio::SimpleActionGroup> m_action_group;
    Gtk::Label m_label_num_of_items;

    // 注意事項
    Glib::RefPtr<Glib::Binding> m_binding_notes; ///< ToggleButtonとRevealerをバインドする
    Gtk::ToggleButton m_toggle_notes;
    Gtk::Revealer m_revealer_notes;
    Gtk::Label m_label_notes;

    Gtk::LinkButton m_link_manual;

    // NG 画像ハッシュのリスト
    Gtk::ScrolledWindow m_scroll;
    Gtk::TreeView m_treeview;
    Glib::RefPtr<Gtk::ListStore> m_list_store;
    Columns m_columns;
    int m_col;
    Gtk::SortType m_previous_sortmode{};

public:

    explicit ImageHashTab( Gtk::Window* parent )
        : m_parent{ parent }
        , m_box{ Gtk::ORIENTATION_VERTICAL, 8 }
        , m_hbox_check_enable_hash{ Gtk::ORIENTATION_HORIZONTAL, 8 }
        , m_vbox_check_enable_hash{ Gtk::ORIENTATION_VERTICAL, 0 }
        , m_label_enable_hash{ "(実験的な機能) NG 画像ハッシュを有効にする(_A)", true }
        , m_hbox_initial_threshold{ Gtk::ORIENTATION_HORIZONTAL, 8 }
        , m_label_initial_threshold{ "初期設定のしきい値(_I):", true }
        , m_button_reset_initial_threshold{ "リセット" }
        , m_box_tool{ Gtk::ORIENTATION_HORIZONTAL, 0 }
        , m_label_tool{ "選択した行に対する操作:" }
        , m_button_delete{ "削除(_D)", true }
        , m_menu_set_threshold{ Gio::Menu::create() }
        , m_action_group{ Gio::SimpleActionGroup::create() }
        , m_label_notes{ "・設定が無効のときはハッシュ値の計算やあぼ〜ん判定を行いません。\n"
                         "・ハッシュ値を削除したりしきい値を変更してもあぼ〜んされた画像は解除されません。\n"
                         "・判定基準のしきい値を大きくすると誤判定する可能性が高くなります。\n"
                         "・マイナスのしきい値に設定したハッシュ値はあぼ〜んの判定を行いません。" }
        , m_link_manual{ ENVIRONMENT::get_jdhelpimghash(), "オンラインマニュアル(_M)" }
        , m_col{ -2 }
    {
        m_grid.set_row_spacing( 8 );
        m_grid.set_column_spacing( 8 );
        m_grid.attach( m_listbox, 0, 0, 2, 1 );
        m_grid.attach( m_label_initial_threshold, 0, 1, 1, 1 );
        m_grid.attach( m_hbox_initial_threshold, 1, 1, 1, 1 );
        m_grid.attach( m_label_tool, 0, 2, 1, 1 );
        m_grid.attach( m_box_tool, 1, 2, 1, 1 );

        // 設定オプションのセットアップ
        m_switch_enable_hash.set_active( CONFIG::get_enable_img_hash() );
        m_switch_enable_hash.set_hexpand( false );
        m_switch_enable_hash.set_valign( Gtk::ALIGN_CENTER );
        m_switch_enable_hash.show();
        m_label_enable_hash.set_halign( Gtk::ALIGN_START );
        m_label_enable_hash.set_hexpand( true );
        m_label_enable_hash.set_line_wrap( true );
        m_label_enable_hash.set_mnemonic_widget( m_switch_enable_hash );
        m_label_enable_hash.show();

        m_label_enable_hash.set_ellipsize( Pango::ELLIPSIZE_END );

        m_hbox_check_enable_hash.pack_start( m_label_enable_hash, Gtk::PACK_EXPAND_WIDGET );
        m_hbox_check_enable_hash.pack_start( m_switch_enable_hash, Gtk::PACK_SHRINK );
        m_hbox_check_enable_hash.pack_start( m_toggle_notes, Gtk::PACK_SHRINK );
        m_hbox_check_enable_hash.show();

        m_vbox_check_enable_hash.pack_start( m_hbox_check_enable_hash );
        m_vbox_check_enable_hash.pack_start( m_revealer_notes );
        m_vbox_check_enable_hash.show();
        m_listbox.append( m_vbox_check_enable_hash );
        m_listbox.set_selection_mode( Gtk::SELECTION_NONE );
        m_listbox.signal_row_activated().connect( sigc::mem_fun( *this, &ImageHashTab::slot_listbox_row_activated ) );
        m_listbox.show();

        m_label_initial_threshold.set_halign( Gtk::ALIGN_START );
        m_label_initial_threshold.set_ellipsize( Pango::ELLIPSIZE_END );
        m_label_initial_threshold.set_mnemonic_widget( m_spin_initial_threshold );
        m_label_initial_threshold.show();
        m_spin_initial_threshold.set_increments( 1, 1 );
        m_spin_initial_threshold.set_range( -1, 128 ); // -1: 判定しない、 128: 最大値
        m_spin_initial_threshold.set_tooltip_text( "ハッシュ値を登録したとき設定する判定基準のしきい値" );
        m_spin_initial_threshold.set_value( CONFIG::get_img_hash_initial_threshold() );
        m_spin_initial_threshold.show();
        m_button_reset_initial_threshold.show();
        m_button_reset_initial_threshold.signal_clicked().connect(
            sigc::mem_fun( *this, &ImageHashTab::slot_reset_initial_threshold ) );
        m_link_manual.set_halign( Gtk::ALIGN_END );
        m_link_manual.set_use_underline( true );
        m_link_manual.show();
        m_hbox_initial_threshold.pack_start( m_spin_initial_threshold, Gtk::PACK_SHRINK );
        m_hbox_initial_threshold.pack_start( m_button_reset_initial_threshold, Gtk::PACK_SHRINK );
        m_hbox_initial_threshold.pack_end( m_link_manual, Gtk::PACK_SHRINK );
        m_hbox_initial_threshold.show();

        m_label_tool.set_halign( Gtk::ALIGN_START );
        m_label_tool.show();
        m_button_delete.signal_clicked().connect( sigc::mem_fun( *this, &ImageHashTab::slot_delete ) );
        m_button_delete.show();

        m_action_group->add_action_with_parameter( "set-threshold", Glib::Variant<gint32>::variant_type(),
                                                   sigc::mem_fun( *this, &ImageHashTab::slot_set_threshold ) );

        // マジックナンバー 9999 は設定不可能な値で m_spin_initial_threshold が保持する値の代わり
        m_menu_set_threshold->append( "初期設定", "image-hash.set-threshold(9999)" );
        m_menu_set_threshold->append( "あぼ〜んしない(-1)", "image-hash.set-threshold(-1)" );
        m_menu_set_threshold->append( "同一画像(0)", "image-hash.set-threshold(0)" );

        m_button_set_threshold.set_label( "しきい値(_T)" );
        m_button_set_threshold.set_use_underline( true );
        m_button_set_threshold.set_menu_model( m_menu_set_threshold );
        m_button_set_threshold.show();

        m_button_copy.set_hexpand( false );
        m_button_copy.set_image_from_icon_name( "edit-copy-symbolic" );
        m_button_copy.set_tooltip_text( "選択した行をクリップボードにコピー" );
        m_button_copy.signal_clicked().connect( sigc::mem_fun( *this, &ImageHashTab::slot_copy_clicked ) );
        m_button_copy.show();

        m_toggle_notes.set_label( "注意事項" );
        m_toggle_notes.show();
        m_label_notes.set_ellipsize( Pango::ELLIPSIZE_END );
        m_label_notes.property_margin() = 4;
        m_label_notes.show();
        m_revealer_notes.add( m_label_notes );
        m_revealer_notes.show();

        m_binding_notes = Glib::Binding::bind_property( m_toggle_notes.property_active(),
                                                        m_revealer_notes.property_reveal_child() );

        m_label_num_of_items.set_ellipsize( Pango::ELLIPSIZE_END );
        m_label_num_of_items.set_hexpand( false );
        m_label_num_of_items.show();

        m_box_tool.set_hexpand( true );
        m_box_tool.pack_start( m_button_delete, Gtk::PACK_SHRINK );
        m_box_tool.pack_start( m_button_set_threshold, Gtk::PACK_SHRINK );
        m_box_tool.pack_start( m_button_copy, Gtk::PACK_SHRINK );
        m_box_tool.pack_end( m_label_num_of_items, Gtk::PACK_SHRINK );
        m_box_tool.show();

        // あぼーん設定一覧のセットアップ
        m_list_store = Gtk::ListStore::create( m_columns );
        m_list_store->set_sort_column( Gtk::TreeSortable::DEFAULT_UNSORTED_COLUMN_ID,
                                       Gtk::SortType::SORT_ASCENDING );
        m_list_store->set_sort_func( 2, sigc::mem_fun( *this, &ImageHashTab::slot_compare_row_threshold ) );
        m_list_store->set_sort_func( 3, sigc::mem_fun( *this, &ImageHashTab::slot_compare_row_last_matched ) );
        m_list_store->set_sort_func( 4, sigc::mem_fun( *this, &ImageHashTab::slot_compare_row_source_url ) );
        m_treeview.set_model( m_list_store );
        m_treeview.get_selection()->set_mode( Gtk::SELECTION_MULTIPLE );

        m_treeview.append_column_numeric( "ハッシュA", m_columns.m_col_hash_row, "%" PRIX64 );
        m_treeview.append_column_numeric( "ハッシュB", m_columns.m_col_hash_col, "%" PRIX64 );
        m_treeview.append_column_numeric_editable("しきい値", m_columns.m_col_threshold, "%d" );
        m_treeview.append_column( "最後にマッチした日時", m_columns.m_col_last_matched );
        m_treeview.append_column( "ハッシュのソースURL", m_columns.m_col_source_url );
        const int num_column = m_treeview.append_column_numeric( "最後にマッチした日時(time_t)",
                                                                 m_columns.m_col_last_matched_time, "%" PRId64 );
        m_treeview.set_vexpand( true );
        m_treeview.show();

        Gtk::TreeView::Column* column = m_treeview.get_column( 2 );
        column->set_clickable( true );
        column->signal_clicked().connect( sigc::bind( sigc::mem_fun( *this, &ImageHashTab::slot_col_clicked ), 2 ) );
        column = m_treeview.get_column( 3 );
        column->set_clickable( true );
        column->signal_clicked().connect( sigc::bind( sigc::mem_fun( *this, &ImageHashTab::slot_col_clicked ), 3 ) );
        column = m_treeview.get_column( 4 );
        column->set_clickable( true );
        column->signal_clicked().connect( sigc::bind( sigc::mem_fun( *this, &ImageHashTab::slot_col_clicked ), 4 ) );

        auto renderer = dynamic_cast<Gtk::CellRendererText*>( m_treeview.get_column_cell_renderer( 2 ) );
        renderer->signal_edited().connect( sigc::mem_fun( *this, &ImageHashTab::slot_edited_threshold ) );

        m_scroll.set_vexpand( true );
        m_scroll.add( m_treeview );
        m_scroll.show();

        // 読み込んだあぼーん設定を一覧に追加
        JDLIB::span<const DBIMG::AboneImgHash> span = DBIMG::get_span_abone_imghash();
        for( const DBIMG::AboneImgHash& abone_imghash : span ) {
            Gtk::TreeModel::Row row = *m_list_store->append();
            row[ m_columns.m_col_hash_row ] = abone_imghash.dhash.row_hash;
            row[ m_columns.m_col_hash_col ] = abone_imghash.dhash.col_hash;
            row[ m_columns.m_col_threshold ] = abone_imghash.threshold;
            row[ m_columns.m_col_last_matched ] = MISC::timettostr( abone_imghash.last_matched, MISC::TIME_WEEK );
            row[ m_columns.m_col_source_url ] = abone_imghash.source_url;
            row[ m_columns.m_col_last_matched_time ] = abone_imghash.last_matched;
        }
        m_label_num_of_items.set_text( Glib::ustring::compose( "設定数: %1", span.size() ) );
        m_treeview.get_column( num_column - 1 )->set_visible( false );

        m_box.set_hexpand( true );
        m_box.set_vexpand( true );
        m_box.property_margin() = 8;
        m_box.insert_action_group( "image-hash", m_action_group );
        m_box.pack_start( m_grid, Gtk::PACK_SHRINK );
        m_box.pack_start( m_scroll, Gtk::PACK_EXPAND_WIDGET );
        m_box.show();

        m_treeview.grab_focus();
    }

    Gtk::Widget& get_widget() { return m_box; }

    /**
     * @brief ダイアログのOKボタンが押されたときに設定を反映する処理
     */
    void slot_ok_clicked()
    {
        CONFIG::set_enable_img_hash( m_switch_enable_hash.get_active() );
        CONFIG::set_img_hash_initial_threshold( m_spin_initial_threshold.get_value_as_int() );
        save_abone_imghash_list();
    }

private:

    /**
     * @brief 設定の状態をキャッシュに保存する
     */
    void save_abone_imghash_list()
    {
        Gtk::TreeModel::Children children = m_list_store->children();
        std::vector<DBIMG::AboneImgHash> new_abone_imghash_list;
        new_abone_imghash_list.reserve( children.size() );
        for( const Gtk::TreeRow& row : children ) {
            std::uint64_t row_hash = row[ m_columns.m_col_hash_row ];
            std::uint64_t col_hash = row[ m_columns.m_col_hash_col ];
            int threshold = row[ m_columns.m_col_threshold ];
            Glib::ustring source_url = row[ m_columns.m_col_source_url ];
            std::time_t last_matched = row[ m_columns.m_col_last_matched_time ];

            new_abone_imghash_list.push_back(
                DBIMG::AboneImgHash{ { row_hash, col_hash }, threshold, last_matched, source_url.raw() } );
        }
        DBIMG::update_abone_imghash_list( new_abone_imghash_list );
        DBIMG::save_abone_imghash_list();
    }

    /**
     * @brief ラベルをクリックしたときトグルボタンの状態を切り替える
     */
    void slot_listbox_row_activated( Gtk::ListBoxRow* )
    {
        m_switch_enable_hash.set_active( ! m_switch_enable_hash.get_active() );
    }

    /**
     * @brief しきい値の初期設定をリセットする
     */
    void slot_reset_initial_threshold()
    {
        m_spin_initial_threshold.set_value( CONFIG::CONF_IMG_HASH_INITIAL_THRESHOLD );
    }

    /**
     * @brief 選択された行の設定を取り除く
     * @details ダイアログのOKボタンを押すまでキャッシュには反映されない。
     */
    void slot_delete()
    {
        std::vector<Gtk::TreeModel::Path> paths = m_treeview.get_selection()->get_selected_rows();
        if( paths.empty() ) return;

        SKELETON::MsgDiag mdiag( *m_parent, "削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
        mdiag.set_secondary_text( "ハッシュ値を削除しても画像のあぼ〜んは維持されるため個別に解除する必要があります。" );
        if( mdiag.run() != Gtk::RESPONSE_YES ) return;

        // 先頭のiterから削除すると後続のイテレーターが指すiterがずれるため末尾のiterから削除していく
        Gtk::TreeModel::iterator it_next = Gtk::TreeModel::const_iterator();
        for( auto rit = paths.rbegin(); rit != paths.rend(); ++rit ) {
            Gtk::TreeModel::iterator iter = m_list_store->get_iter( *rit );
            if( ! iter ) continue;

            it_next = m_list_store->erase( iter );
        }

        if( it_next ) {
            m_treeview.get_selection()->select( it_next );
        }
        std::size_t n_children;
        if( auto model = m_treeview.get_model(); model ) {
            n_children = model->children().size();
        }
        else {
            n_children = 0;
        }
        m_label_num_of_items.set_text( Glib::ustring::compose( "設定数: %1", n_children ) );
    }

    /**
     * @brief 選択された行のしきい値に値をセットする
     */
    void slot_set_threshold( const Glib::VariantBase& threshold )
    {
        std::vector<Gtk::TreeModel::Path> paths = m_treeview.get_selection()->get_selected_rows();
        if( paths.empty() ) return;

        auto v = Glib::VariantBase::cast_dynamic<Glib::Variant<gint32>>( threshold );
        gint32 value = v.get();
        // マジックナンバー 9999 は設定不可能な値で m_spin_initial_threshold が保持する値の代わり
        if( value == 9999 ) value = m_spin_initial_threshold.get_value_as_int();

        for( Gtk::TreeModel::Path& path : paths ) {
            Gtk::TreeModel::iterator it = m_list_store->get_iter( path );
            if( ! it ) continue;

            it->set_value( m_columns.m_col_threshold, value );
        }
    }

    /**
     * @brief 選択された行の設定をクリップボードにコピーする
     */
    void slot_copy_clicked()
    {
        std::vector<Gtk::TreeModel::Path> paths = m_treeview.get_selection()->get_selected_rows();
        if( paths.empty() ) return;

        Glib::ustring copied;

        for( Gtk::TreeModel::Path& path : paths ) {
            Gtk::TreeModel::iterator it = m_list_store->get_iter( path );
            if( ! it ) continue;

            Gtk::TreeModel::Row row = *it;
            const std::uint64_t row_hash = row[ m_columns.m_col_hash_row ];
            const std::uint64_t col_hash = row[ m_columns.m_col_hash_col ];
            const int threshold = row[ m_columns.m_col_threshold ];
            const std::size_t last_matched = row[ m_columns.m_col_last_matched_time ];
            const Glib::ustring& source_url = row[ m_columns.m_col_source_url ];

            copied.append( Glib::ustring::compose( "%1 %2 %3 %4 %5 %6\n",
                                                   Glib::ustring::format(std::uppercase, std::hex, row_hash),
                                                   Glib::ustring::format(std::uppercase, std::hex, col_hash),
                                                   threshold, DBIMG::kImgHashReserved, last_matched, source_url ) );
        }
        MISC::CopyClipboard( copied.raw() );

        // コピーしたことを表すためボタンのアイコンを2秒間チェックマークに変更する
        constexpr int timeout_ms = 2000; // 単位はミリ秒
        Glib::signal_timeout().connect_once( sigc::mem_fun( *this, &ImageHashTab::slot_button_timeout ), timeout_ms );
        m_button_copy.set_image_from_icon_name( "object-select-symbolic" );
    }

    /**
     * @brief コピーボタンのアイコンを元に戻す
     */
    void slot_button_timeout()
    {
        m_button_copy.set_image_from_icon_name( "edit-copy-symbolic" );
    }

    /**
     * @brief 項目を比較するテンプレート関数
     */
    template<typename Col, typename T = typename Col::ElementType>
    int slot_compare_row( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b, const Col& col ) const
    {
        Gtk::TreeModel::Row row_a = *a;
        Gtk::TreeModel::Row row_b = *b;
        const T& value_a = row_a[ col ];
        const T& value_b = row_b[ col ];
        return value_a < value_b ? -1 : 1;
    }

    /**
     * @brief 項目のしきい値を比較する
     */
    int slot_compare_row_threshold( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b ) const
    {
        return slot_compare_row( a, b, m_columns.m_col_threshold );
    }

    /**
     * @brief 項目の最後にマッチした日時を比較する
     */
    int slot_compare_row_last_matched( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b ) const
    {
        return slot_compare_row( a, b, m_columns.m_col_last_matched_time );
    }

    /**
     * @brief 項目のハッシュのソースURLを辞書順で比較する
     */
    int slot_compare_row_source_url( const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b ) const
    {
        return slot_compare_row( a, b, m_columns.m_col_source_url );
    }

    /**
     * @brief 列の項目名をクリックしたときに一覧をソートする
     */
    void slot_col_clicked( const int col )
    {
        if( m_col != col ) {
            m_list_store->set_sort_column( col, Gtk::SORT_ASCENDING );
            m_previous_sortmode = Gtk::SORT_ASCENDING;
        }
        else if(  m_previous_sortmode == Gtk::SORT_ASCENDING ) {
            m_list_store->set_sort_column( col, Gtk::SORT_DESCENDING );
            m_previous_sortmode = Gtk::SORT_DESCENDING;
        }
        else {
            m_list_store->set_sort_column( col, Gtk::SORT_ASCENDING );
            m_previous_sortmode = Gtk::SORT_ASCENDING;
        }
        m_col = col;
    }

    /**
     * @brief セルのしきい値を編集したときに値を正常な範囲に補正する
     */
    void slot_edited_threshold( const Glib::ustring& path, const Glib::ustring& new_text )
    {
        int new_value = std::atoi( new_text.c_str() );
        if( new_value < -1 ) new_value = -1;
        else if( new_value > 128 ) new_value = 128;
        else return;

        auto it = m_list_store->get_iter( path );
        it->set_value( m_columns.m_col_threshold, new_value );
    }
};

} // namespace CORE

#endif // JDIM_IMAGEHASHTAB_H
