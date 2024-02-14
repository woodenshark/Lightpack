//
//  MacOSSession.mm
//  Prismatik
//
//  Created by zomfg on 27/12/2018.
//

#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include "MacOSSession.h"
#include "debug.h"

namespace SystemSession {
    static const char* NSNotificationEventType = "mac_generic_NSNotification";
}

@interface MacOSNativeEventListener : NSObject
@end

@implementation MacOSNativeEventListener {
    SystemSession::MacOSEventFilter* _delegate;
}

- (id) initWithDelegate:(SystemSession::MacOSEventFilter*)delegate
{
    self = [super init];
    if (self != nil)
        _delegate = delegate;
    return self;
}

- (void) dealloc
{
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
    _delegate = nil;
    [super dealloc];
}

- (void) receivedNotification:(NSNotification*)notif
{
    _delegate->nativeEventFilter(QByteArray(SystemSession::NSNotificationEventType), notif, NULL);
}

- (void) registerNotification:(NSNotificationName)notifName
{
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(receivedNotification:) name:notifName object:NULL];
}
@end

namespace SystemSession
{

    class MacOSEventFilter::NativeWrapper
    {
    public:
        NativeWrapper(MacOSEventFilter* detector)
        : _client([[MacOSNativeEventListener alloc] initWithDelegate:detector])
        {}

        ~NativeWrapper()
        { [_client release]; }
        
        void registerNotification(NSNotificationName notifName)
        { [_client registerNotification:notifName]; }
        
    private:
        MacOSNativeEventListener* _client;
    };

    MacOSEventFilter::MacOSEventFilter()
    {
        _wrapper = new NativeWrapper(this);

        _wrapper->registerNotification(NSWorkspaceDidWakeNotification);
        _wrapper->registerNotification(NSWorkspaceWillSleepNotification);
        _wrapper->registerNotification(NSWorkspaceWillPowerOffNotification);

        _wrapper->registerNotification(NSWorkspaceSessionDidBecomeActiveNotification);
        _wrapper->registerNotification(NSWorkspaceSessionDidResignActiveNotification);

        _wrapper->registerNotification(NSWorkspaceScreensDidSleepNotification);
        _wrapper->registerNotification(NSWorkspaceScreensDidWakeNotification);
    }

    MacOSEventFilter::~MacOSEventFilter()
    {
        delete _wrapper;
        _wrapper = nullptr;
    }

    #if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool MacOSEventFilter::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result)
    #else
    bool MacOSEventFilter::nativeEventFilter(const QByteArray& eventType, void* message, long* result)
    #endif
    {
        Q_UNUSED(result);
        if (eventType != NSNotificationEventType)
            return false;

        NSNotification* notif = (NSNotification*)message;

        if (notif.name == NSWorkspaceDidWakeNotification)
        {
            DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Session: machine wakes from sleep";
            emit sessionChangeDetected(Resuming);
        }
        else if (notif.name == NSWorkspaceWillSleepNotification)
        {
            DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Session: machine goes to sleep";
            emit sessionChangeDetected(Sleeping);
        }
        else if (notif.name == NSWorkspaceWillPowerOffNotification)
        {
            DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Session: user has requested a logout or that the machine be powered off";
            emit sessionChangeDetected(Ending);
        }
        else if (notif.name == NSWorkspaceSessionDidBecomeActiveNotification)
        {
            DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Session: user session is switched in";
            emit sessionChangeDetected(Unlocking);
        }
        else if (notif.name == NSWorkspaceSessionDidResignActiveNotification)
        {
            DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Session: user session is switched out";
            emit sessionChangeDetected(Locking);
        }
        else if (notif.name == NSWorkspaceScreensDidSleepNotification)
        {
            DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Session: screen goes to sleep";
            emit sessionChangeDetected(DisplayOff);
        }
        else if (notif.name == NSWorkspaceScreensDidWakeNotification)
        {
            DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Session: screens wake";
            emit sessionChangeDetected(DisplayOn);
        }
        return false;
    }
}
