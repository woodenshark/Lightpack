/*
 * AbstractLedDeviceUdp.cpp
 *
 *	Created on: 06.09.2011
 *		Author: Mike Shatohin (brunql), Timur Sattarov
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

#include "AbstractLedDeviceUdp.hpp"
#include "PrismatikMath.hpp"
#include "Settings.hpp"
#include "enums.hpp"
#include "debug.h"

using namespace SettingsScope;

AbstractLedDeviceUdp::AbstractLedDeviceUdp(const QString& address, const QString& port, const int timeout, QObject * parent) : AbstractLedDevice(parent)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	m_address = address;
	m_port = port.toInt();
	m_timeout = timeout;

	m_gamma = Settings::getDeviceGamma();
	m_brightness = Settings::getDeviceBrightness();

	m_Socket = NULL;
	m_writeBufferHeader.append((char)255);
}

AbstractLedDeviceUdp::~AbstractLedDeviceUdp()
{
	close();
}

void AbstractLedDeviceUdp::close()
{
	if (m_Socket != NULL) {
		m_Socket->close();

		delete m_Socket;
		m_Socket = NULL;
	}
}

void AbstractLedDeviceUdp::setRefreshDelay(int /*value*/)
{
	emit commandCompleted(true);
}

void AbstractLedDeviceUdp::setColorDepth(int /*value*/)
{
	emit commandCompleted(true);
}

void AbstractLedDeviceUdp::setSmoothSlowdown(int /*value*/)
{
	emit commandCompleted(true);
}

void AbstractLedDeviceUdp::setColorSequence(QString value)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;

	m_colorSequence = value;
	setColors(m_colorsSaved);

	emit commandCompleted(true);
}

void AbstractLedDeviceUdp::setGamma(double value)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;

	m_gamma = value;
	setColors(m_colorsSaved);
}

void AbstractLedDeviceUdp::setBrightness(int percent)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << percent;

	m_brightness = percent;
	setColors(m_colorsSaved);
}

void AbstractLedDeviceUdp::open()
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	emit openDeviceSuccess(true);

	if (m_Socket != NULL)
		m_Socket->close();
	else
		m_Socket = new QUdpSocket();
}

bool AbstractLedDeviceUdp::writeBuffer(const QByteArray& buff)
{
	DEBUG_MID_LEVEL << Q_FUNC_INFO << "Hex:" << buff.toHex();

	if (m_Socket == NULL)
		return false;

	int bytesWritten = m_Socket->writeDatagram(buff, QHostAddress(m_address), m_port);

	if (bytesWritten != buff.count())
	{
		qWarning() << Q_FUNC_INFO << "bytesWritten != buff.count():" << bytesWritten << buff.count() << " " << m_Socket->errorString();
		return false;
	}

	return true;
}
