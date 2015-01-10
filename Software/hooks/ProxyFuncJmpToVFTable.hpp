#ifndef PROXYFUNCJMPTOVFTABLE_H
#define PROXYFUNCJMPTOVFTABLE_H

#include <stdint.h>
#include "LoggableTrait.hpp"

class ProxyFuncJmp;
class ProxyFuncVFTable;

class ProxyFuncJmpToVFTable : public LoggableTrait
{
public:
    ProxyFuncJmpToVFTable(void * pTarget, void *pSubstFunc, Logger *logger);
    virtual ~ProxyFuncJmpToVFTable();
    virtual bool init();
    virtual bool isHookInstalled();
    virtual bool isSwitched();
    virtual bool switchToVFTHook(void * pNewTarget, void * pNewSubstFunc);
    virtual bool installHook();
    virtual bool removeHook();
    
    void *getOriginalFunc();
private:
    ProxyFuncJmp *m_funcJmp;
    ProxyFuncVFTable *m_funcVFTable;
};

#endif // PROXYFUNCJMPTOVFTABLE_H
