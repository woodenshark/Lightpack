#include "ProxyFuncJmpToVFTable.hpp"
#include "ProxyFuncJmp.hpp"
#include "ProxyFuncVFTable.hpp"
#include "hooksutils.h"

ProxyFuncJmpToVFTable::ProxyFuncJmpToVFTable(void * pTarget, void *pSubstFunc, Logger *logger) : LoggableTrait(logger)
{
	m_funcJmp = new ProxyFuncJmp(pTarget, pSubstFunc, logger);
	m_funcVFTable = NULL;
}

ProxyFuncJmpToVFTable::~ProxyFuncJmpToVFTable() {
	if (this->isHookInstalled())
		this->removeHook();

	if (m_funcJmp)
		delete m_funcJmp;

	if (m_funcVFTable)
		delete m_funcVFTable;
}

bool ProxyFuncJmpToVFTable::init() {
	return m_funcJmp->init();
}

bool ProxyFuncJmpToVFTable::isHookInstalled() {
	if (m_funcVFTable)
		return m_funcVFTable->isHookInstalled();
	else
		return m_funcJmp->isHookInstalled();
}

bool ProxyFuncJmpToVFTable::isSwitched() {
	return m_funcVFTable != NULL;
}

bool ProxyFuncJmpToVFTable::installHook() {
	if (m_funcVFTable)
		return m_funcVFTable->installHook();
	else
		return m_funcJmp->installHook();
}
bool ProxyFuncJmpToVFTable::switchToVFTHook(void * pNewTarget, void * pNewSubstFunc) {
	if (m_funcVFTable)
		return m_funcVFTable->installHook();

	if (m_funcJmp->isHookInstalled())
		if (!m_funcJmp->removeHook())
			return FALSE;

	delete m_funcJmp;
	m_funcJmp = NULL;

	m_funcVFTable = new ProxyFuncVFTable(pNewTarget, pNewSubstFunc, m_logger);
	return m_funcVFTable->init() && m_funcVFTable->installHook();
}

bool ProxyFuncJmpToVFTable::removeHook() {
	if (m_funcVFTable)
		return m_funcVFTable->removeHook();
	else
		return m_funcJmp->removeHook();

}

void *ProxyFuncJmpToVFTable::getOriginalFunc() {
	if (m_funcVFTable)
		return m_funcVFTable->getOriginalFunc();
	else
		return NULL;
}