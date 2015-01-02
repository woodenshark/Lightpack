/*
 * LibraryInjector.c
 *
 *  Created on: 05.06.2012
 *     Project: Lightpack
 *
 *  Copyright (c) 2012 Timur Sattarov
 *
 *  Lightpack a USB content-driving ambient lighting system
 *
 *  Lightpack is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Lightpack is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "LibraryInjector.h"
#ifdef _WIN64
#include "../common/D3D10GrabberDefs.hpp"
#endif

#include <stdarg.h>
#include <olectl.h>

/*! A count of how many objects of our DLL has been created
*/
static volatile LONG comObjectsCount = 0;

/*! A count of how many apps have locked our DLL via calling our
 IClassFactory object's LockServer()
*/
static volatile LONG locksCount = 0;

#define REPORT_LOG_BUF_SIZE 2048
static HANDLE hEventSrc = NULL;

typedef struct {
    const ILibraryInjectorVtbl *lpVtbl;
    volatile LONG refCount;
} LibraryInjector;

typedef struct {
    const IClassFactoryVtbl *lpVtbl;
    volatile LONG refCount;
} ClassFactory;

void freeLibraryInjector(LibraryInjector * injector);
void freeClassFactory(ClassFactory * injector);

int libraryInjectorInit(void) {
    hEventSrc = RegisterEventSourceW(NULL, L"Prismatik-libraryinjector");
    comObjectsCount = locksCount = 0;

    return 1;
}

int isLibraryInjectorActive(void) {
    return (comObjectsCount | locksCount) ? 1 : 0;
}

void libraryInjectorShutdown(void) {
    DeregisterEventSource(hEventSrc);
}

void reportLog(WORD logType, LPCWSTR message, ...) {
    va_list ap;
    int sprintfResult;

    WCHAR *reportLogBuf = malloc(REPORT_LOG_BUF_SIZE);
    memset(reportLogBuf, 0, REPORT_LOG_BUF_SIZE);

    va_start( ap, message );
    sprintfResult = wvsprintfW(reportLogBuf, message, ap);
    va_end( ap );
    if (sprintfResult > -1) {
        LPCWSTR reportStrings[] = { reportLogBuf };
        ReportEventW(hEventSrc, logType, 0, 0x100, NULL, 1, 0, reportStrings, NULL);
    }

    free(reportLogBuf);
}

static BOOL SetPrivilege(HANDLE hToken, LPCTSTR szPrivName, BOOL fEnable) {

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    LookupPrivilegeValue(NULL, szPrivName, &tp.Privileges[0].Luid);
    tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
    AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof (tp), NULL, NULL);
    return ((GetLastError() == ERROR_SUCCESS));

}

static BOOL AcquirePrivilege(void) {
    HANDLE hCurrentProc = GetCurrentProcess();
    HANDLE hCurrentProcToken;

    if (!OpenProcessToken(hCurrentProc, TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hCurrentProcToken)) {
        reportLog(EVENTLOG_ERROR_TYPE, L"OpenProcessToken Error 0x%x", GetLastError());
    } else {
        if (!SetPrivilege(hCurrentProcToken, SE_DEBUG_NAME, TRUE)) {
            reportLog(EVENTLOG_ERROR_TYPE, L"SetPrivleges SE_DEBUG_NAME Error 0x%x", GetLastError());
        } else {
            return TRUE;
        }
    }
    return FALSE;
}

static HRESULT STDMETHODCALLTYPE LibraryInjector_QueryInterface(ILibraryInjector * this, REFIID interfaceGuid, void **ppv) {
    if (!IsEqualIID(interfaceGuid, &IID_IUnknown) && !IsEqualIID(interfaceGuid, &IID_ILibraryInjector)) {
      *ppv = 0;
      return(E_NOINTERFACE);
    }

    // Fill in the caller's handle
    *ppv = this;

    // Increment the count of callers who have an outstanding pointer to this object
    this->lpVtbl->AddRef(this);

    return(NOERROR);
}

static ULONG STDMETHODCALLTYPE LibraryInjector_AddRef(ILibraryInjector * this) {
    return InterlockedIncrement(&(((LibraryInjector*)this)->refCount));
}

static ULONG STDMETHODCALLTYPE LibraryInjector_Release(ILibraryInjector * this) {
    LibraryInjector * _this = (LibraryInjector*)this;
    InterlockedDecrement(&(_this->refCount));
    if ((_this->refCount) == 0) {
        freeLibraryInjector(_this);
        return(0);
    }
    return (_this->refCount);
}

static HRESULT STDMETHODCALLTYPE LibraryInjector_Inject(ILibraryInjector * this, DWORD ProcessId, LPWSTR ModulePath) {
    char CodePage[2048];
    WCHAR modulePath[300];

    wcscpy(modulePath, ModulePath);
    UNREFERENCED_PARAMETER(this);
    reportLog(EVENTLOG_INFORMATION_TYPE, L"injecting library...");
    if(AcquirePrivilege()) {
        size_t sizeofCP;
        LPVOID Memory;
        HANDLE hThread;
        HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");

        DWORD_PTR LoadLibAddr = (DWORD_PTR)GetProcAddress(hKernel32, "LoadLibraryW");

        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);

        if (hProcess == NULL) {
            reportLog(EVENTLOG_ERROR_TYPE, L"couldn't open process");
            return S_FALSE;
        }

#ifdef _WIN64
        BOOL isWow64Process = FALSE;
        IsWow64Process(hProcess, &isWow64Process);
        if (isWow64Process) {

            HANDLE mapping = OpenFileMapping(FILE_MAP_READ, FALSE, HOOKSGRABBER_SHARED_MEM_NAME);
            if (NULL == mapping) {
                return 4;
            }

            char *memory = (char *)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, HOOKSGRABBER_SHARED_MEM_SIZE);
            if (memory == NULL) {
                return 5;
            }

            size_t len = wcslen(modulePath);
            wcscpy(modulePath + len - 4, L"32.dll");

            struct HOOKSGRABBER_SHARED_MEM_DESC *desc = (struct HOOKSGRABBER_SHARED_MEM_DESC*)memory;

            LoadLibAddr = desc->loadLibraryWAddress32;

            UnmapViewOfFile(memory);
            CloseHandle(mapping);
        }
#endif

        sizeofCP = wcslen(modulePath)*2 + 1;
        Memory = VirtualAllocEx(hProcess, 0, sizeofCP, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

        if (!Memory) {
            reportLog(EVENTLOG_ERROR_TYPE, L"couldn't allocate memory");
            return S_FALSE;
        }

        wcscpy((wchar_t*)CodePage, modulePath);

        if (!WriteProcessMemory(hProcess, Memory, CodePage, sizeofCP, 0)) {
            reportLog(EVENTLOG_ERROR_TYPE, L"couldn't write loading library code");
            return S_FALSE;
        }

        hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibAddr, Memory, 0, 0);
        if (!hThread) {
            reportLog(EVENTLOG_ERROR_TYPE, L"couldn't create remote thread");
            return S_FALSE;
        }
        WaitForSingleObject(hThread, INFINITE);

        DWORD exitCode = -1;
        GetExitCodeThread(hThread, &exitCode);

        if (!exitCode) { // LoadLibrary returns 0 on failure
            reportLog(EVENTLOG_ERROR_TYPE, L"Injection into process failed: %d", exitCode);
            return S_FALSE;
        }
        CloseHandle(hThread);
        reportLog(EVENTLOG_INFORMATION_TYPE, L"library injected successfully");

        return S_OK;
    } else {
        reportLog(EVENTLOG_ERROR_TYPE, L"couldn't acquire privileges to inject");
        return S_FALSE;
    }
}

static ULONG STDMETHODCALLTYPE ClassFactory_AddRef(IClassFactory * this) {
    return InterlockedIncrement(&(((ClassFactory*)this)->refCount));
}

// IClassFactory's QueryInterface()
static HRESULT STDMETHODCALLTYPE ClassFactory_QueryInterface(IClassFactory * this, REFIID factoryGuid, void **ppv) {
    if (IsEqualIID(factoryGuid, &IID_IUnknown) || IsEqualIID(factoryGuid, &IID_IClassFactory))
    {
        this->lpVtbl->AddRef(this);
        *ppv = this;

        return(NOERROR);
    }
    *ppv = 0;
    return(E_NOINTERFACE);
}

static ULONG STDMETHODCALLTYPE ClassFactory_Release(IClassFactory * this) {
    ClassFactory * _this = (ClassFactory*)this;
    InterlockedDecrement(&(_this->refCount));
    if ((_this->refCount) == 0) {
        freeClassFactory(_this);
        return (0);
    }
    return (_this->refCount);
}

static HRESULT STDMETHODCALLTYPE ClassFactory_CreateInstance(IClassFactory * this, IUnknown *punkOuter, REFIID vTableGuid, void **objHandle) {
    HRESULT hr;
    register ILibraryInjector * thisobj;

    UNREFERENCED_PARAMETER(this);

    // Assume an error by clearing caller's handle
    *objHandle = 0;

    // We don't support aggregation in this example
    if (punkOuter)
        hr = CLASS_E_NOAGGREGATION;
    else
    {
        if (!(thisobj = allocLibraryInjector()))
            hr = E_OUTOFMEMORY;
        else
        {
            thisobj->lpVtbl->AddRef(thisobj);
            // Fill in the caller's handle with a pointer to the LibraryInjector we just
            // allocated above. We'll let LibraryInjector's QueryInterface do that, because
            // it also checks the GUID the caller passed, and also increments the
            // reference count (to 2) if all goes well
            hr = thisobj->lpVtbl->QueryInterface(thisobj, vTableGuid, objHandle);

            // Decrement reference count. NOTE: If there was an error in QueryInterface()
            // then Release() will be decrementing the count back to 0 and will free the
            // LibraryInjector for us. One error that may occur is that the caller is asking for
            // some sort of object that we don't support (ie, it's a GUID we don't recognize)
            thisobj->lpVtbl->Release(thisobj);

        }
    }

    return(hr);
}

// IClassFactory's LockServer(). It is called by someone
// who wants to lock this DLL in memory
static HRESULT STDMETHODCALLTYPE ClassFactory_LockServer(IClassFactory * this, BOOL flock) {
    UNREFERENCED_PARAMETER(this);

    if (flock) InterlockedIncrement(&locksCount);
    else InterlockedDecrement(&locksCount);

    return (NOERROR);
}

static const ILibraryInjectorVtbl libraryInjectorVtbl = {
    LibraryInjector_QueryInterface,
    LibraryInjector_AddRef,
    LibraryInjector_Release,
    LibraryInjector_Inject
};

static const IClassFactoryVtbl classFactoryVtbl = {
    ClassFactory_QueryInterface,
    ClassFactory_AddRef,
    ClassFactory_Release,
    ClassFactory_CreateInstance,
    ClassFactory_LockServer
};

ILibraryInjector * allocLibraryInjector(void) {
    LibraryInjector * thisobj = GlobalAlloc(GMEM_FIXED, sizeof(LibraryInjector));
    if (!thisobj)
        return NULL;

    InterlockedIncrement(&comObjectsCount);
    thisobj->refCount = 0;
    thisobj->lpVtbl = &libraryInjectorVtbl;

    return (ILibraryInjector *)thisobj;
}

void freeLibraryInjector(LibraryInjector * injector) {
    if (injector) {
        InterlockedDecrement(&comObjectsCount);
        GlobalFree(injector);
    }
}

IClassFactory * allocClassFactory(void) {
    ClassFactory * thisobj = GlobalAlloc(GMEM_FIXED, sizeof(ClassFactory));
    if (!thisobj)
        return NULL;

    InterlockedIncrement(&comObjectsCount);
    thisobj->refCount = 0;
    thisobj->lpVtbl = &classFactoryVtbl;
    return (IClassFactory *)thisobj;
}

void freeClassFactory(ClassFactory * factory) {
    if (factory) {
        InterlockedDecrement(&comObjectsCount);
        GlobalFree(factory);
    }
}
