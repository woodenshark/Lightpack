/*
 * WindowsSoundManager.hpp
 *
 *	Created on: 11.12.2011
 *		Project: Lightpack
 *
 *	Copyright (c) 2011 Mike Shatohin, mikeshatohin [at] gmail.com
 *
 *	Lightpack a USB content-driving ambient lighting system
 *
 *	Lightpack is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	Lightpack is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <QTime>
#include "SoundManagerBase.hpp"

class WindowsSoundManager : public SoundManagerBase
{
	Q_OBJECT
public:
	WindowsSoundManager(int hWnd, QObject *parent = 0);
	virtual ~WindowsSoundManager();

public:
	virtual void start(bool isMoodLampEnabled);

protected:
	virtual bool init();
	virtual void populateDeviceList(QList<SoundManagerDeviceInfo>& devices, int& recommended);
	virtual void updateFft();

private:
	QTimer m_timer;
	int m_hWnd;
};
