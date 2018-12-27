//
//  SystemSession.cpp
//  Prismatik
//
//  Created by zomfg on 27/12/2018.
//

#include "SystemSession.hpp"
#if defined(Q_OS_WIN)
#include "WindowsSession.hpp"
#elif defined(Q_OS_MACOS)
#include "MacOSSession.h"
#endif

namespace SystemSession {
    
    EventFilter* EventFilter::create()
    {
#if defined(Q_OS_WIN)
		return new WindowsEventFilter();
#elif defined(Q_OS_MACOS)
        return new MacOSEventFilter();
#endif
        return nullptr;
    }
    
}
