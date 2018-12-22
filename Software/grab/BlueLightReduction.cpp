#include "BlueLightReduction.hpp"
#if defined(Q_OS_WIN)
#include "WinUtils.hpp"
#elif defined(Q_OS_MACOS)
#include "MacUtils.h"
#endif

namespace BlueLightReduction
{
	Client* create()
	{
#if defined(Q_OS_WIN)
#ifdef NIGHTLIGHT_SUPPORT
		if (WinUtils::NightLight::isSupported())
			return new WinUtils::NightLight();
#endif // NIGHTLIGHT_SUPPORT
		if (WinUtils::GammaRamp::isSupported())
			return new WinUtils::GammaRamp();
#elif defined(Q_OS_MACOS)
        if (MacUtils::GammaRamp::isSupported())
            return new MacUtils::GammaRamp();
#endif
		return nullptr;
	}
}
