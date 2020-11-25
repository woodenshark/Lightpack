//
//  MacOSSession.h
//  Prismatik
//
//  Created by zomfg on 27/12/2018.
//
#pragma once

#ifndef MACOSSESSIONCHANGEDETECTOR_H
#define MACOSSESSIONCHANGEDETECTOR_H
#include "SystemSession.hpp"

namespace SystemSession {
    class MacOSEventFilter : public EventFilter
    {
    public:
        MacOSEventFilter();
        ~MacOSEventFilter();
        #if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result);
        #else
        bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) Q_DECL_OVERRIDE;
        #endif
    private:
        class NativeWrapper;
        NativeWrapper* _wrapper;
    };
}

#endif /* MACOSSESSIONCHANGEDETECTOR_H */
