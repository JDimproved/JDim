// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "dispatchmanager.h"

#include "skeleton/dispatchable.h"


bool use_dialog_manager = false;
CORE::DispatchManagerBase* instance_dispmanager = NULL;
CORE::DispatchManagerBase* instance_dialog_dispmanager = NULL;


CORE::DispatchManagerBase* CORE::get_dispmanager()
{
    if( use_dialog_manager ){
        if( ! instance_dialog_dispmanager ) instance_dialog_dispmanager = new CORE::DispatchManagerForDialog();
        return instance_dialog_dispmanager;
    }

    if( ! instance_dispmanager ) instance_dispmanager = new CORE::DispatchManager();
    return instance_dispmanager;
}

void CORE::delete_dispatchmanager()
{
    if( instance_dialog_dispmanager ) delete instance_dialog_dispmanager;
    instance_dialog_dispmanager = NULL;
    
    if( instance_dispmanager ) delete instance_dispmanager;
    instance_dispmanager = NULL;
}

void CORE::enable_dialog_dispmanager()
{
    use_dialog_manager = true;
}

void CORE::disable_dialog_dispmanager()
{
    use_dialog_manager = false;

    if( instance_dialog_dispmanager ) delete instance_dialog_dispmanager;
    instance_dialog_dispmanager = NULL;
}


//////////////////////


using namespace CORE;


DispatchManagerBase::DispatchManagerBase()
{
#ifdef _DEBUG
    std::cout << "DispatchManagerBase::DispatchManagerBase\n";
#endif
}


DispatchManagerBase::~DispatchManagerBase()
{
#ifdef _DEBUG
    std::cout << "DispatchManagerBase::~DispatchManagerBase size = " << m_children.size() << std::endl;
#endif
}


void DispatchManagerBase::add( SKELETON::Dispatchable* child )
{
    // 既にlistに登録されていたらキャンセルする
    std::list< SKELETON::Dispatchable* >::iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it ){
        if( *it == child ){
#ifdef _DEBUG
            std::cout << "DispatchManagerBase::add canceled\n";
#endif
            return;
        }
    }

    m_children.push_back( child );
    emit();

#ifdef _DEBUG
    std::cout << "DispatchManagerBase::add size = " << m_children.size() << std::endl;
#endif
}


void DispatchManagerBase::remove( SKELETON::Dispatchable* child )
{
    size_t size = m_children.size();
    if( ! size  ) return;

    m_children.remove( child );

#ifdef _DEBUG
    if( size != m_children.size() ) std::cout << "!!!!!!!\nDispatchManagerBase::remove size "
                                              << size << " -> " << m_children.size() << "\n!!!!!!!\n";
#endif
}


void DispatchManagerBase::slot_dispatch()
{
    const size_t size = m_children.size();
    if( ! size  ) return;

    SKELETON::Dispatchable* child = *( m_children.begin() );

    // child->callback_dispatch()の中で再び Dispatchable::add()が呼び出されると
    // キャンセルされてしまうので callback_dispatch() を呼び出す前にremoveする
    m_children.remove( child );

    if( child ) child->callback_dispatch();

#ifdef _DEBUG
    std::cout << "DispatchManagerBase::slot_dispatch size = " << size << " -> " << m_children.size() << std::endl;
#endif
}


//////////////////////////////////////////

//
// メインループ用マネージャ
//
DispatchManager::DispatchManager()
    : DispatchManagerBase()
{
#ifdef _DEBUG
    std::cout << "DispatchManager::DispatchManager\n";
#endif

    m_dispatch.connect( sigc::mem_fun( *this, &DispatchManager::slot_dispatch ) );
}


//////////////////////////////////////////


// ダイアログ用マネージャ
// ダイアログでは Glib::Dispatcher が動かないのでタイマーを使って
// 擬似的にdispatchする
DispatchManagerForDialog::DispatchManagerForDialog()
    : DispatchManagerBase()
{
#ifdef _DEBUG
    std::cout << "DispatchManagerForDialog::DispatchManagerForDialog\n";
#endif

    const int timeout = 10;

    sigc::slot< bool > slot_timeout = sigc::bind( sigc::mem_fun(*this, &DispatchManagerForDialog::slot_timeout), 0 );
    m_conn = Glib::signal_timeout().connect( slot_timeout, timeout );
}


DispatchManagerForDialog::~DispatchManagerForDialog()
{
#ifdef _DEBUG
    std::cout << "DispatchManagerForDialog::~DispatchManagerForDialog\n";
#endif

    m_conn.disconnect();
}


bool DispatchManagerForDialog::slot_timeout( int timer_number )
{
    slot_dispatch();

    return true;
}
