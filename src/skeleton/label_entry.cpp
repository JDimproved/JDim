// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "label_entry.h"

using namespace SKELETON;


LabelEntry::LabelEntry( const bool editable, const std::string& label, const std::string& text )
    : m_editable( editable )
{
    m_label.set_text_with_mnemonic( label );
    m_label.set_mnemonic_widget ( m_entry );
    m_color_bg_org = m_entry.get_style()->get_base( Gtk::STATE_NORMAL );

    setup();

    m_entry.set_text( text );
    signal_realize().connect( sigc::mem_fun(*this, &LabelEntry::slot_realize ) );
    signal_style_changed().connect( sigc::mem_fun(*this, &LabelEntry::slot_style_changed ) );

    pack_start( m_label, Gtk::PACK_SHRINK );
    pack_start( m_entry );
}


void LabelEntry::setup()
{
    m_entry.set_editable( m_editable );
    m_entry.set_activates_default( m_editable );
    m_entry.set_has_frame( m_editable );
    m_entry.property_can_focus() = m_editable;
}


void LabelEntry::slot_realize()
{
#ifdef _DEBUG
    std::cout << "LabelEntry::slot_realize\n";
#endif

    slot_style_changed( get_style() );
}


void LabelEntry::slot_style_changed( Glib::RefPtr< Gtk::Style > )
{
#ifdef _DEBUG
    std::cout << "LabelEntry::slot_style_changed\n";
#endif

    //
    // entryの背景色を変更
    //

    m_color_bg = get_style()->get_bg( Gtk::STATE_NORMAL );

    // Gtk::Notebook の中にあるとテーマによっては色が変になるので
    // Gtk::Notebook の背景色を使用する
    Gtk::Widget* parent = get_parent();
    while( parent && ! dynamic_cast< Gtk::Notebook* >( parent ) ) parent = parent->get_parent();
    if( parent ){
#ifdef _DEBUG
        std::cout << "in the notebook\n";
#endif
        m_color_bg = parent->get_style()->get_bg( Gtk::STATE_NORMAL );
    }

    // 背景色変更
    if( ! m_editable ) m_entry.modify_base( Gtk::STATE_NORMAL, m_color_bg );
}


void LabelEntry::set_editable( const bool editable )
{
#ifdef _DEBUG
    std::cout << "LabelEntry::set_editable editable = " << editable << std::endl;
#endif

    m_editable = editable;

    setup();

    if( ! editable ) m_entry.modify_base( Gtk::STATE_NORMAL, m_color_bg );
    else m_entry.modify_base( Gtk::STATE_NORMAL, m_color_bg_org );
}


void LabelEntry::set_visibility( bool visibility )
{
    m_entry.set_visibility( visibility );
}


void LabelEntry::set_text( const std::string& text )
{
    m_entry.set_text( text );
}


const Glib::ustring LabelEntry::get_text()
{
    return m_entry.get_text();
}


void LabelEntry::grab_focus()
{
    if( m_editable ) m_entry.grab_focus();
}


const bool LabelEntry::has_grab()
{
    if( m_editable ) return m_entry.has_grab();
    return false;
}
