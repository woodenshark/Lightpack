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
    AbstractLedDeviceUdp(const QString& address, const QString& port, const int timeout, QObject * parent = 0);
    virtual ~AbstractLedDeviceUdp();

public slots:
	void open();
    void close();
	void setRefreshDelay(int /*value*/);
	void setColorDepth(int /*value*/);
	void setSmoothSlowdown(int /*value*/);
	void setColorSequence(QString /*value*/);
	void setGamma(double value);
	void setBrightness(int value);
	int defaultLedsCount() { return 10; }

protected:
    QByteArray m_writeBufferHeader;
    QByteArray m_writeBuffer;

    virtual void resizeColorsBuffer(int buffSize) = 0;
    virtual void reinitBufferHeader() = 0;
    bool writeBuffer(const QByteArray& buff);

    int m_timeout;
    constexpr static const char InfiniteTimeout = 255;

private:
    QUdpSocket* m_Socket;

    QString m_address;
    int m_port;
};
