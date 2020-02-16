/*
 * LedDeviceWarls.cpp
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

#include "LedDeviceWarls.hpp"
#include "enums.hpp"

LedDeviceWarls::LedDeviceWarls(const QString& address, const QString& port, QObject * parent) : AbstractLedDeviceUdp(address, port, parent)
{
}

const QString LedDeviceWarls::name() const
{ 
	return "warls"; 
}

int LedDeviceWarls::maxLedsCount()
{
	return 255;
}

void LedDeviceWarls::setColors(const QList<QRgb> & colors)
{
	if(colors.size()> 0)
	{
		m_colorsSaved = colors;

		resizeColorsBuffer(colors.count());

		applyColorModifications(colors, m_colorsBuffer);

		m_writeBuffer.clear();
		m_writeBuffer.append(m_writeBufferHeader);

		for (int i = 0; i < m_colorsBuffer.count(); i++)
		{
			StructRgb color = m_colorsBuffer[i];

			// Reduce 12-bit colour information
			color.r = color.r >> 4;
			color.g = color.g >> 4;
			color.b = color.b >> 4;

			m_writeBuffer.append(i);
			m_writeBuffer.append(color.r);
			m_writeBuffer.append(color.g);
			m_writeBuffer.append(color.b);
		}
	}

	bool ok = writeBuffer(m_writeBuffer);
	emit commandCompleted(ok);
}

void LedDeviceWarls::switchOffLeds()
{
	int count = m_colorsSaved.count();
	m_colorsSaved.clear();

	for (int i = 0; i < count; i++) {
		m_colorsSaved << 0;
	}

	m_writeBuffer.clear();
	m_writeBuffer.append(m_writeBufferHeader);

	for (int i = 0; i < count; i++) {
		m_writeBuffer.append((char)i)
			.append((char)0)
			.append((char)0)
			.append((char)0);
	}

	bool ok = writeBuffer(m_writeBuffer);
	emit commandCompleted(ok);
}

void LedDeviceWarls::requestFirmwareVersion()
{
	emit firmwareVersion("1.0 (warls device)");
	emit commandCompleted(true);
}

void LedDeviceWarls::resizeColorsBuffer(int buffSize)
{
	if (m_colorsBuffer.count() == buffSize)
		return;

	m_colorsBuffer.clear();

	if (buffSize > MaximumNumberOfLeds::Warls)
	{
		qCritical() << Q_FUNC_INFO << "buffSize > MaximumNumberOfLeds::Warls" << buffSize << ">" << MaximumNumberOfLeds::Warls;

		buffSize = MaximumNumberOfLeds::Warls;
	}

	for (int i = 0; i < buffSize; i++)
	{
		m_colorsBuffer << StructRgb();
	}

	reinitBufferHeader();
}

void LedDeviceWarls::reinitBufferHeader()
{
	m_writeBufferHeader.clear();

	// Initialize buffer header
	m_writeBufferHeader.append((char)1);    // WARLS protocol
	m_writeBufferHeader.append((char)255);  // Max-timeout
}
