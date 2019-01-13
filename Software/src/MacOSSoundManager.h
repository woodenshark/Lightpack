//
//  MacOSSoundManager.h
//  Prismatik
//
//  Created by zomfg on 07/01/2019.
//

#pragma once

#include "SoundManagerBase.hpp"

#include <objc/objc-runtime.h>
#ifdef __OBJC__
@class MacOSNativeSoundCapture;
#else
typedef struct objc_object MacOSNativeSoundCapture;
#endif

class MacOSSoundManager : public SoundManagerBase
{
	Q_OBJECT

public:
	explicit MacOSSoundManager(QObject *parent = 0);
	~MacOSSoundManager();
	virtual void start(bool isEnabled);
	
private:
	virtual bool init();
	virtual void populateDeviceList(QList<SoundManagerDeviceInfo>& devices, int& recommended);
	
	bool m_isAuthorized{false};
	bool m_awaitingAuthorization{false};
	MacOSNativeSoundCapture* _capture{nullptr};
};
