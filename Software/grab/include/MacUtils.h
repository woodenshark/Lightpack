//
//  MacUtils.h
//  grab
//
//  Created by zomfg on 20/12/2018.
//
#pragma once
#ifndef MACUTILS_H
#define MACUTILS_H
#include <CoreGraphics/CoreGraphics.h>
#include "BlueLightReduction.hpp"

namespace MacUtils {
    class NightShift : public BlueLightReduction::Client
    {
    public:
        NightShift();
        ~NightShift();
        void apply(QList<QRgb>& colors, const double gamma);
        static bool isSupported();
    private:
        class NightShiftImpl;
        NightShiftImpl* _impl;
    };

    class GammaRamp : public BlueLightReduction::Client
    {
    public:
        GammaRamp();
        ~GammaRamp();
        void apply(QList<QRgb>& colors, const double /*gamma*/);
        static bool isSupported();
    private:
        CGGammaValue* _gammaR;
        CGGammaValue* _gammaG;
        CGGammaValue* _gammaB;
        uint32_t _rampCapacity;
        
        static void getValidGammaTable(CGGammaValue** gamma, const uint32_t currentCapacity, const uint32_t newCapacity);
        static void freeGammaTable(CGGammaValue** gamma);
        static bool loadGammaTables(CGGammaValue** gammaR, CGGammaValue** gammaG, CGGammaValue** gammaB, uint32_t* capacity, uint32_t* sampleCount);
    };
}

#endif /* MACUTILS_H */

