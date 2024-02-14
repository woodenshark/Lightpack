#ifndef GAPIPROXY_HPP
#define GAPIPROXY_HPP

#if !defined NOMINMAX
#define NOMINMAX
#endif

#if !defined WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "ProxyFunc.hpp"

class IPCContext;

class GAPIProxyFrameGrabber {
public:
	GAPIProxyFrameGrabber(HANDLE syncRunMutex);
	~GAPIProxyFrameGrabber();
	virtual bool init() = 0;
	virtual bool isGAPILoaded() = 0;
	virtual bool isHooksInstalled() = 0;
	virtual bool installHooks() = 0;
	virtual bool removeHooks() = 0;
	virtual void free() = 0;

	void setIPCContext(IPCContext *ipcContext) { m_ipcContext = ipcContext; }

protected:
	LARGE_INTEGER m_pcOverall;
	LARGE_INTEGER m_pcOriginal;
	LARGE_INTEGER m_pcStart;
	LARGE_INTEGER m_pcIntermediate;
	LARGE_INTEGER m_pcStop;
	LARGE_INTEGER m_liFreq;
	IPCContext *m_ipcContext;
	HANDLE m_syncRunMutex;
	bool m_isInited;

	bool startOverallPerfCount();
	bool startOriginalPerfCount();
	bool stopPerfCount();
};

#endif // GAPIPROXY_HPP
