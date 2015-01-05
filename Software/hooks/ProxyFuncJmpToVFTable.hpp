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
    bool init();
	bool isHookInstalled();
	bool isSwitched();
	bool switchToVFTHook(void * pNewTarget, void * pNewSubstFunc);
    bool installHook();
    bool removeHook();
	
	void *getOriginalFunc();
private:
	ProxyFuncJmp *m_funcJmp;
	ProxyFuncVFTable *m_funcVFTable;
};

#endif // PROXYFUNCJMPTOVFTABLE_H
