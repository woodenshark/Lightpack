#ifndef D3D9FRAMEGRABBER_HPP
#define D3D9FRAMEGRABBER_HPP

#include "GAPIProxyFrameGrabber.hpp"
#include "LoggableTrait.hpp"

class Logger;
class ProxyFuncJmp;
class ProxyFuncJmpToVFTable;
struct IDirect3DDevice9;
struct IDirect3DSwapChain9;
struct IDirect3DSurface9;
typedef enum _D3DFORMAT D3DFORMAT;
#include "d3d9.h" //PRESENT_PARAMETERS

class D3D9FrameGrabber : public GAPIProxyFrameGrabber, LoggableTrait
{
public:
    static D3D9FrameGrabber *m_this;
    static D3D9FrameGrabber *getInstance(HANDLE syncRunMutex) {
        if (!m_this)
            m_this = new D3D9FrameGrabber(syncRunMutex, Logger::getInstance());
        return m_this;
    }
    static D3D9FrameGrabber *getInstance() {
        return m_this;
    }
    static bool hasInstance() {
        return m_this != NULL;
    }
    ~D3D9FrameGrabber() {
        m_this = NULL;
    }

    virtual bool init();
    virtual bool isGAPILoaded();
    virtual bool isHooksInstalled();
    virtual bool installHooks();
    virtual bool removeHooks();
    virtual void free();
    void freeDeviceData();
    friend HRESULT WINAPI D3D9Present(IDirect3DDevice9 *, CONST RECT*,CONST RECT*,HWND,CONST RGNDATA*);
    friend HRESULT WINAPI D3D9SCPresent(IDirect3DSwapChain9*, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*, DWORD);
	friend HRESULT WINAPI D3D9Reset(IDirect3DDevice9* pDev, D3DPRESENT_PARAMETERS* pPresentationParameters);

protected:
	ProxyFuncJmpToVFTable *m_d3d9PresentProxyFunc;
	ProxyFuncJmpToVFTable *m_d3d9SCPresentProxyFunc;
	ProxyFuncJmpToVFTable *m_d3d9ResetProxyFunc;
    D3D9FrameGrabber(HANDLE syncRunMutex, Logger *logger);
    void ** calcD3d9PresentPointer();
    void ** calcD3d9SCPresentPointer();
	void ** calcD3d9ResetPointer();

    IDirect3DSurface9 *m_pDemultisampledSurf;
    IDirect3DSurface9 *m_pOffscreenSurf;
    IDirect3DDevice9 *m_pDev;
    D3DFORMAT m_surfFormat;
    UINT m_surfWidth;
    UINT m_surfHeight;
    UINT m_frameCount;
    bool m_mapPending;
    UINT m_lastGrab;
};

#endif // D3D9FRAMEGRABBER_HPP
