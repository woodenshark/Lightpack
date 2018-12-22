//
//  MacUtils.mm
//  grab
//
//  Created by zomfg on 20/12/2018.
//

#include "MacUtils.h"
#include "PrismatikMath.hpp"
#include "../src/debug.h"

namespace MacUtils
{
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
                color = qRgb(static_cast<int>(_gammaR[qRed(color) * sampleCount / UINT8_MAX] * UINT8_MAX),
                             static_cast<int>(_gammaG[qGreen(color) * sampleCount / UINT8_MAX] * UINT8_MAX),
                             static_cast<int>(_gammaB[qBlue(color) * sampleCount / UINT8_MAX] * UINT8_MAX));
            }
    }
}
