//
//  MacOSSession.h
//  Prismatik
//
//  Created by zomfg on 27/12/2018.
//
#pragma once
//#ifdef Q_OS_MACOS
#ifndef MACOSSESSIONCHANGEDETECTOR_H
#define MACOSSESSIONCHANGEDETECTOR_H
#include "SystemSession.hpp"

namespace SystemSession {
    class MacOSEventFilter : public EventFilter
    {
    public:
        MacOSEventFilter();
        ~MacOSEventFilter();
        bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) Q_DECL_OVERRIDE;
    private:
        class NativeWrapper;
        NativeWrapper* _wrapper;
    };
}

#endif /* MACOSSESSIONCHANGEDETECTOR_H */
//#endif /* Q_OS_MACOS */
