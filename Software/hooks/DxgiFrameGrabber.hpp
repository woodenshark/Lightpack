#ifndef DXGIFRAMEGRABBER_HPP
#define DXGIFRAMEGRABBER_HPP

#include "GAPIProxyFrameGrabber.hpp"
#include "LoggableTrait.hpp"

class IPCContext;
class ProxyFuncVFTable;
class Logger;
struct ID3D10Texture2D;
struct ID3D11Texture2D;
struct IDXGISwapChain;
struct ID3D10Device;
struct ID3D11Device;

class DxgiFrameGrabber : public GAPIProxyFrameGrabber, LoggableTrait
{
public:
    static DxgiFrameGrabber *m_this;
    static DxgiFrameGrabber *getInstance() {
        if (!m_this)
            m_this = new DxgiFrameGrabber(Logger::getInstance());
        return m_this;
    }
    static bool hasInstance() {
        return m_this != NULL;
    }
    ~DxgiFrameGrabber() {
        m_this = NULL;
    }

    virtual bool init();
    virtual bool isGAPILoaded();
    virtual bool isHooksInstalled();
    virtual bool installHooks();
    virtual bool removeHooks();
    virtual void free();

    friend void D3D10Grab(ID3D10Texture2D* pBackBuffer);
    friend void D3D11Grab(ID3D11Texture2D *pBackBuffer);
    friend HRESULT WINAPI DXGIPresent(IDXGISwapChain * sc, UINT b, UINT c);

protected:
    DxgiFrameGrabber(Logger *logger);
    ProxyFuncVFTable *m_dxgiPresentProxyFuncVFTable;
    ID3D10Texture2D *m_mapTexture10A;
    ID3D10Device *m_mapDevice10;
    ID3D10Texture2D *m_mapTexture10B;
    ID3D11Texture2D *m_mapTexture11A;
    ID3D11Texture2D *m_mapTexture11B;
    ID3D11Device *m_mapDevice11;
    UINT m_mapWidth;
    UINT m_mapHeight;
    UINT m_frameCount;
private:
    void ** calcDxgiPresentPointer();
};

#endif // DXGIFRAMEGRABBER_HPP
