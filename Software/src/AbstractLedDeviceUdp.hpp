/*
 * AbstractLedDeviceUdp.hpp
 *
 *	Created on: 15.02.2020
 *		Author: Tom Archer
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
#include "colorspace_types.h"
#include <QUdpSocket>

class AbstractLedDeviceUdp : public AbstractLedDevice
{
	Q_OBJECT
public:
	AbstractLedDeviceUdp(const QString& address, const QString& port, const uint8_t timeout, QObject * parent = 0);
	virtual ~AbstractLedDeviceUdp();
	int defaultLedsCount() { return 10; }

public slots:
	void open();
	void close();
	virtual void setRefreshDelay(int value);
	virtual void setSmoothSlowdown(int value);
	virtual void setColorSequence(const QString& value);
	virtual void setColorDepth(int value);
	virtual void requestFirmwareVersion();
	void switchOffLeds();
	virtual void setColors(const QList<QRgb> & colors, const bool rawColors) = 0;
	void setColors(const QList<QRgb> & colors) {setColors(colors, false);};

protected:
	QByteArray m_writeBufferHeader;
	QByteArray m_writeBuffer;

	void resizeColorsBuffer(int buffSize);
	virtual void reinitBufferHeader() = 0;
	bool writeBuffer(const QByteArray& buff);

	uint8_t m_timeout;
	constexpr static const uint8_t InfiniteTimeout = (uint8_t)255;

private:
	QUdpSocket* m_Socket;

	QString m_address;
	uint16_t m_port {21324};
};
