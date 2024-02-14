//
//  MacUtils.mm
//  grab
//
//  Created by zomfg on 20/12/2018.
//

#include "MacUtils.h"
#include "PrismatikMath.hpp"
#include "../src/debug.h"
#include <Foundation/Foundation.h>

#pragma mark - NightShift private API
@interface CBBlueLightClient : NSObject

typedef struct {
    int hour;
    int minute;
} Time;

typedef struct {
    Time from;
    Time to;
} Schedule;

typedef struct {
    BOOL active;
    BOOL enabled;
    BOOL sunSchedulePermitted;
    int mode;
    Schedule schedule;
    unsigned long long disableFlags;
} Status;

//- (BOOL)setStrength:(float)strength commit:(BOOL)commit;
//- (BOOL)setEnabled:(BOOL)enabled;
//- (BOOL)setMode:(int)mode;
//- (BOOL)setSchedule:(Schedule *)schedule;
- (BOOL)getStrength:(float *)strength;
- (BOOL)getBlueLightStatus:(Status *)status;
- (void)setStatusNotificationBlock:(void (^)(void))block;
+ (BOOL)supportsBlueLightReduction;
@end

namespace MacUtils
{
#pragma mark - NightShift
    class NightShift::NightShiftImpl
    {
    public:
        NightShiftImpl() : _client([[CBBlueLightClient alloc] init])
        {
            void (^loadStatus)(void) = ^{
                [_client getBlueLightStatus:&_status];
                [_client getStrength:&_strength];
            };
            [_client setStatusNotificationBlock:loadStatus];
            loadStatus();
        }
        ~NightShiftImpl()
        {
            [_client setStatusNotificationBlock:nil];
            [_client release];
            _client = nullptr;
        }
        
        static bool isSupported(const bool checkEnabled = false)
        {
            if ((NSClassFromString(@"CBBlueLightClient") == nil || ![CBBlueLightClient supportsBlueLightReduction]))
                return false;
            if (!checkEnabled)
                return true;
            Status currentStatus;
            CBBlueLightClient* client = [[CBBlueLightClient alloc] init];
            BOOL ok = [client getBlueLightStatus:&currentStatus];
            [client release];
            return (ok && currentStatus.mode != Mode::Off);
        }
        
        uint16_t getColorTemperature() const
        {
            // TODO: on/off transition
            if (!_status.enabled)
                return kMaxTemperature;
            return static_cast<uint16_t>(kMaxTemperature + (kMinTemperature - kMaxTemperature) * _strength);
        }
    private:
        enum Mode {
            Off = 0,
            SunSchedule = 1,
            ManualSchedule = 2
        };
        const uint16_t kMaxTemperature = 6500;
        const uint16_t kMinTemperature = 2500;
        CBBlueLightClient* _client;
        Status _status;
        float _strength;
    };
    
    NightShift::NightShift() : _impl(new NightShiftImpl())
    {}
    
    NightShift::~NightShift()
    {
        delete _impl;
        _impl = nullptr;
    }
    bool NightShift::isSupported()
    {
        return NightShiftImpl::isSupported(true);
    }

    void NightShift::apply(QList<QRgb> &colors, const double gamma)
    {
        PrismatikMath::applyColorTemperature(colors, _impl->getColorTemperature(), gamma);
    }
    
    
#pragma mark - GammaRamp
    GammaRamp::GammaRamp() : _gammaR(NULL), _gammaG(NULL), _gammaB(NULL), _rampCapacity(0) {};
    GammaRamp::~GammaRamp()
    {
        freeGammaTable(&_gammaR);
        freeGammaTable(&_gammaG);
        freeGammaTable(&_gammaB);
    }
    
    bool GammaRamp::isSupported()
    {
        uint32_t capacity = 0;
        uint32_t sampleCount = 0;
        CGGammaValue* gammaR = NULL;
        CGGammaValue* gammaG = NULL;
        CGGammaValue* gammaB = NULL;
        if (loadGammaTables(&gammaR, &gammaG, &gammaB, &capacity, &sampleCount))
        {
            freeGammaTable(&gammaR);
            freeGammaTable(&gammaG);
            freeGammaTable(&gammaB);
            return true;
        }
        return false;
    }
    
    void GammaRamp::getValidGammaTable(CGGammaValue** gamma, const uint32_t currentCapacity, const uint32_t newCapacity)
    {
        if (*gamma != NULL && currentCapacity != newCapacity)
            freeGammaTable(gamma);
        if (*gamma == NULL)
            *gamma = (CGGammaValue*)malloc(newCapacity * sizeof(CGGammaValue));
    }
    
    void GammaRamp::freeGammaTable(CGGammaValue** gamma)
    {
        if (*gamma != NULL)
            free(*gamma);
        *gamma = NULL;
    }
    
    bool GammaRamp::loadGammaTables(CGGammaValue **gammaR, CGGammaValue **gammaG, CGGammaValue **gammaB, uint32_t *capacity, uint32_t *sampleCount)
    {
        CGDirectDisplayID display = CGMainDisplayID();
        const uint32_t newCapacity = CGDisplayGammaTableCapacity(display);
        getValidGammaTable(gammaR, *capacity, newCapacity);
        getValidGammaTable(gammaG, *capacity, newCapacity);
        getValidGammaTable(gammaB, *capacity, newCapacity);
        *capacity = newCapacity;
        *sampleCount = 0;
        const CGError error = CGGetDisplayTransferByTable(display, *capacity, *gammaR, *gammaG, *gammaB, sampleCount);
        if (error != kCGErrorSuccess || *sampleCount == 0)
        {
            qWarning() << Q_FUNC_INFO << "Unable to get gamma tables";
            freeGammaTable(gammaR);
            freeGammaTable(gammaG);
            freeGammaTable(gammaB);
            return false;
        }
        return true;
    }
    
    void GammaRamp::apply(QList<QRgb> &colors, const double /*gamma*/)
    {
        uint32_t sampleCount = 0;
        if (loadGammaTables(&_gammaR, &_gammaG, &_gammaB, &_rampCapacity, &sampleCount))
            for (QRgb& color : colors) {
                color = qRgb(static_cast<int>(_gammaR[qRed(color) * (sampleCount - 1) / UINT8_MAX] * UINT8_MAX),
                             static_cast<int>(_gammaG[qGreen(color) * (sampleCount - 1) / UINT8_MAX] * UINT8_MAX),
                             static_cast<int>(_gammaB[qBlue(color) * (sampleCount - 1) / UINT8_MAX] * UINT8_MAX));
            }
    }
}
