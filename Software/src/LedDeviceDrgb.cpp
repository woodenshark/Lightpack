/*
 * LedDeviceDrgb.cpp
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

#include "LedDeviceDrgb.hpp"
#include "PrismatikMath.hpp"
#include "Settings.hpp"
#include "enums.hpp"
#include "debug.h"

using namespace SettingsScope;

LedDeviceDrgb::LedDeviceDrgb(const QString& address, const QString& port, QObject * parent) : AbstractLedDevice(parent)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	m_address = address;
	m_port = port.toInt();

	m_gamma = Settings::getDeviceGamma();
	m_brightness = Settings::getDeviceBrightness();

	m_Socket = NULL;
	m_writeBufferHeader.append((char)255);
}

LedDeviceDrgb::~LedDeviceDrgb()
{
	close();
}

void LedDeviceDrgb::close()
{
	if (m_Socket != NULL) {
		m_Socket->close();

		delete m_Socket;
		m_Socket = NULL;
	}
}

void LedDeviceDrgb::setColors(const QList<QRgb> & colors)
{
	if(colors.size()> 0)
	{
		m_colorsSaved = colors;

		QList<QRgb> callbackColors;

		resizeColorsBuffer(colors.count());

		applyColorModifications(colors, m_colorsBuffer);

		m_writeBuffer.clear();
		m_writeBuffer.append(m_writeBufferHeader);

		for (int i = 0; i < m_colorsBuffer.count(); i++)
		{
			callbackColors.append(qRgb(m_colorsBuffer[i].r>>4, m_colorsBuffer[i].g>>4, m_colorsBuffer[i].b>>4));
		
			StructRgb color = m_colorsBuffer[i];

			// Reduce 12-bit colour information
			color.r = color.r >> 4;
			color.g = color.g >> 4;
			color.b = color.b >> 4;

			m_writeBuffer.append(color.r);
			m_writeBuffer.append(color.g);
			m_writeBuffer.append(color.b);
		}

		emit colorsUpdated(callbackColors);
	}

	bool ok = writeBuffer(m_writeBuffer);

	emit commandCompleted(ok);
}

void LedDeviceDrgb::switchOffLeds()
{
	int count = m_colorsSaved.count();
	m_colorsSaved.clear();

	for (int i = 0; i < count; i++) {
		m_colorsSaved << 0;
	}

	m_writeBuffer.clear();
	m_writeBuffer.append(m_writeBufferHeader);

	for (int i = 0; i < count; i++) {
		m_writeBuffer.append((char)0)
			.append((char)0)
			.append((char)0);
	}

	bool ok = writeBuffer(m_writeBuffer);

	emit colorsUpdated(m_colorsSaved);
	emit commandCompleted(ok);
}

void LedDeviceDrgb::setRefreshDelay(int /*value*/)
{
	emit commandCompleted(true);
}

void LedDeviceDrgb::setColorDepth(int /*value*/)
{
	emit commandCompleted(true);
}

void LedDeviceDrgb::setSmoothSlowdown(int /*value*/)
{
	emit commandCompleted(true);
}

void LedDeviceDrgb::setColorSequence(QString value)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;

	m_colorSequence = value;
	setColors(m_colorsSaved);

	emit commandCompleted(true);
}

void LedDeviceDrgb::setGamma(double value)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;

	m_gamma = value;
	setColors(m_colorsSaved);
}

void LedDeviceDrgb::setBrightness(int percent)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << percent;

	m_brightness = percent;
	setColors(m_colorsSaved);
}

void LedDeviceDrgb::requestFirmwareVersion()
{
	emit firmwareVersion("1.0 (drgb device)");
	emit commandCompleted(true);
}

void LedDeviceDrgb::open()
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	emit openDeviceSuccess(true);

	if (m_Socket != NULL)
		m_Socket->close();
	else
		m_Socket = new QUdpSocket();
}

bool LedDeviceDrgb::writeBuffer(const QByteArray& buff)
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

void LedDeviceDrgb::resizeColorsBuffer(int buffSize)
{
	if (m_colorsBuffer.count() == buffSize)
		return;

	m_colorsBuffer.clear();

	if (buffSize > MaximumNumberOfLeds::Drgb)
	{
		qCritical() << Q_FUNC_INFO << "buffSize > MaximumNumberOfLeds::Drgb" << buffSize << ">" << MaximumNumberOfLeds::Drgb;

		buffSize = MaximumNumberOfLeds::Drgb;
	}

	for (int i = 0; i < buffSize; i++)
	{
		m_colorsBuffer << StructRgb();
	}

	reinitBufferHeader();
}

void LedDeviceDrgb::reinitBufferHeader()
{
	m_writeBufferHeader.clear();

	// Initialize buffer header
	m_writeBufferHeader.append((char)2);    // DRGB protocol
	m_writeBufferHeader.append((char)255);  // Max-timeout
}
