/*
 * LedDeviceVirtual.cpp
 *
 *	Created on: 17.04.2011
 *		Author: Timur Sattarov && Mike Shatohin
 *		Project: Lightpack
 *
 *	Lightpack is very simple implementation of the backlight for a laptop
 *
 *	Copyright (c) 2011 Mike Shatohin, mikeshatohin [at] gmail.com
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

#include "AbstractLedDevice.hpp"

class LedDeviceVirtual : public AbstractLedDevice
{
	Q_OBJECT
public:
	LedDeviceVirtual(QObject * parent = 0);
	virtual ~LedDeviceVirtual() {}
	QString name() const { return QStringLiteral("virtual"); }
	int maxLedsCount();
	int defaultLedsCount() { return 10; }

public slots:
	void open();
	void close(){}
	void setColors(const QList<QRgb> & colors);
	void switchOffLeds();
	void setRefreshDelay(int /*value*/);
	void setColorDepth(int /*value*/);
	void setSmoothSlowdown(int /*value*/);
	void setColorSequence(const QString& /*value*/);
	void requestFirmwareVersion();

private:
	void resizeColorsBuffer(int buffSize);
};
