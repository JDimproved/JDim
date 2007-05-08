// ライセンス: GPL2

#ifndef _JDDEBUG_H
#define _JDDEBUG_H

#ifndef _DEBUG
#define NDEBUG
#else
#include <iostream>
#endif

#ifdef _DEBUG_CARETMOVE
#include <iostream>
#endif

#include <assert.h>

#ifdef _DEBUG_SHOW_VMSIZE

#include <iostream>
#include <sstream>

// プロセスのvmsizeを表示
#define show_vmsize( msg ) { \
std::stringstream com; \
com << "grep VmSize /proc/" << getpid() << "/status | sed -e \"s/V[^0-9]*//\""; \
std::cerr << msg << ": "; \
system( com.str().c_str() ); \
} while(0)

#else
#define show_vmsize( msg ) while(0)
#endif


#endif
