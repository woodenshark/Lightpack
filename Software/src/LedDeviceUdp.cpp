/*
 * LightpackVirtual.cpp
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

#include "LedDeviceUdp.hpp"
#include "PrismatikMath.hpp"
#include "Settings.hpp"
#include "enums.hpp"
#include "debug.h"

using namespace SettingsScope;

LedDeviceUdp::LedDeviceUdp(QObject * parent) : AbstractLedDevice(parent)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	m_gamma = Settings::getDeviceGamma();
	m_brightness = Settings::getDeviceBrightness();

	m_MoteDevice = NULL;
	m_writeBufferHeader.append((char)255);

	// Open here otherwise config wizard doesn't work
	open();
}

LedDeviceUdp::~LedDeviceUdp()
{
	close();
}

void LedDeviceUdp::close()
{
	if (m_MoteDevice != NULL) {
		m_MoteDevice->close();

		delete m_MoteDevice;
		m_MoteDevice = NULL;
	}
}

void LedDeviceUdp::setColors(const QList<QRgb> & colors)
{
	if(colors.size()> 0)
	{
		m_colorsSaved = colors;

		QList<QRgb> callbackColors;

		resizeColorsBuffer(colors.count());

		applyColorModifications(colors, m_colorsBuffer);

		for (int i = 0; i < m_colorsBuffer.count(); i++)
		{
			callbackColors.append(qRgb(m_colorsBuffer[i].r>>4, m_colorsBuffer[i].g>>4, m_colorsBuffer[i].b>>4));
		
			// Mote
			m_colorsBuffer[i].r = m_colorsBuffer[i].r >> 4;
			m_colorsBuffer[i].g = m_colorsBuffer[i].g >> 4;
			m_colorsBuffer[i].b = m_colorsBuffer[i].b >> 4;
			PrismatikMath::maxCorrection(254, m_colorsBuffer[i]);
		}

		m_writeBuffer.clear();
		m_writeBuffer.append(m_writeBufferHeader);

		for (int i = 0; i < m_colorsBuffer.count(); i++)
		{
			StructRgb color = m_colorsBuffer[i];

			if (m_colorSequence == "RBG")
			{
				m_writeBuffer.append(color.r);
				m_writeBuffer.append(color.b);
				m_writeBuffer.append(color.g);
			}
			else if (m_colorSequence == "BRG")
			{
				m_writeBuffer.append(color.b);
				m_writeBuffer.append(color.r);
				m_writeBuffer.append(color.g);
			}
			else if (m_colorSequence == "BGR")
			{
				m_writeBuffer.append(color.b);
				m_writeBuffer.append(color.g);
				m_writeBuffer.append(color.r);
			}
			else if (m_colorSequence == "GRB")
			{
				m_writeBuffer.append(color.g);
				m_writeBuffer.append(color.r);
				m_writeBuffer.append(color.b);
			}
			else if (m_colorSequence == "GBR")
			{
				m_writeBuffer.append(color.g);
				m_writeBuffer.append(color.b);
				m_writeBuffer.append(color.r);
			}
			else
			{
				m_writeBuffer.append(color.r);
				m_writeBuffer.append(color.g);
				m_writeBuffer.append(color.b);
			}
		}

		emit colorsUpdated(callbackColors);
	}

	bool ok = writeBuffer(m_writeBuffer);

	emit commandCompleted(ok);
}

void LedDeviceUdp::switchOffLeds()
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

void LedDeviceUdp::setRefreshDelay(int /*value*/)
{
	emit commandCompleted(true);
}

void LedDeviceUdp::setColorDepth(int /*value*/)
{
	emit commandCompleted(true);
}

void LedDeviceUdp::setSmoothSlowdown(int /*value*/)
{
	emit commandCompleted(true);
}

void LedDeviceUdp::setColorSequence(QString value)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;

	m_colorSequence = value;
	setColors(m_colorsSaved);

	emit commandCompleted(true);
}

void LedDeviceUdp::setGamma(double value)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;

	m_gamma = value;
	setColors(m_colorsSaved);
}

void LedDeviceUdp::setBrightness(int percent)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << percent;

	m_brightness = percent;
	setColors(m_colorsSaved);
}

void LedDeviceUdp::requestFirmwareVersion()
{
	emit firmwareVersion("1.0 (virtual device)");
	emit commandCompleted(true);
}

void LedDeviceUdp::open()
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	emit openDeviceSuccess(true);

	if (m_MoteDevice != NULL)
		m_MoteDevice->close();
	else
		m_MoteDevice = new QUdpSocket();
}

bool LedDeviceUdp::writeBuffer(const QByteArray& buff)
{
	DEBUG_MID_LEVEL << Q_FUNC_INFO << "Hex:" << buff.toHex();

	if (m_MoteDevice == NULL)
		return false;

	int bytesWritten = m_MoteDevice->writeDatagram(buff, QHostAddress::LocalHost, port);

	if (bytesWritten != buff.count())
	{
		qWarning() << Q_FUNC_INFO << "bytesWritten != buff.count():" << bytesWritten << buff.count() << " " << m_MoteDevice->errorString();
		return false;
	}

	return true;
}

void LedDeviceUdp::resizeColorsBuffer(int buffSize)
{
	if (m_colorsBuffer.count() == buffSize)
		return;

	m_colorsBuffer.clear();

	if (buffSize > MaximumNumberOfLeds::Udp)
	{
		qCritical() << Q_FUNC_INFO << "buffSize > MaximumNumberOfLeds::Udp" << buffSize << ">" << MaximumNumberOfLeds::Udp;

		buffSize = MaximumNumberOfLeds::Udp;
	}

	for (int i = 0; i < buffSize; i++)
	{
		m_colorsBuffer << StructRgb();
	}

	reinitBufferHeader(buffSize);
}

void LedDeviceUdp::reinitBufferHeader(int ledsCount)
{
	m_writeBufferHeader.clear();

	// Initialize buffer header
	int ledsCountHi = ((ledsCount - 1) >> 8) & 0xff;
	int ledsCountLo = (ledsCount - 1) & 0xff;

	m_writeBufferHeader.append((char)'U');
	m_writeBufferHeader.append((char)'d');
	m_writeBufferHeader.append((char)'p');
	m_writeBufferHeader.append((char)ledsCountHi);
	m_writeBufferHeader.append((char)ledsCountLo);
	m_writeBufferHeader.append((char)(ledsCountHi ^ ledsCountLo ^ 0x55));
}
