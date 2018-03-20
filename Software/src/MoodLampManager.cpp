/*
 * MoodLampManager.cpp
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


#include "MoodLampManager.hpp"
#include "PrismatikMath.hpp"
#include "Settings.hpp"
#include <QTime>

using namespace SettingsScope;

MoodLampManager::MoodLampManager(QObject *parent) : QObject(parent)
{
	qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

	m_isMoodLampEnabled = false;
	m_rgbSaved = 0;

	initFromSettings();

	connect(&m_generator, SIGNAL(updateColor(QColor)), this, SLOT(updateColors()));
}

void MoodLampManager::start(bool isEnabled)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << isEnabled;

	m_isMoodLampEnabled = isEnabled;

	if (m_isMoodLampEnabled)
	{
		// Clear saved color for force emit signal after check m_rgbSaved != rgb in updateColors()
		// This is usable if start is called after API unlock, and we should force set current color
		m_rgbSaved = 0;
		updateColors();

	}
	
	if (m_isMoodLampEnabled && m_isLiquidMode)
		m_generator.start();
	else
		m_generator.stop();

}

void MoodLampManager::setCurrentColor(QColor color)
{
	DEBUG_MID_LEVEL << Q_FUNC_INFO << color;

	m_currentColor = color;

	if (m_isMoodLampEnabled && !m_isLiquidMode)
	{
		fillColors(color.rgb());
		emit updateLedsColors(m_colors);
	}
}

void MoodLampManager::setLiquidMode(bool state)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << state;
	m_isLiquidMode = state;
	if (m_isLiquidMode && m_isMoodLampEnabled)
		m_generator.start();
	else {
		m_generator.stop();
		if (m_isMoodLampEnabled)
			updateColors();
	}
}

void MoodLampManager::setLiquidModeSpeed(int value)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;
	m_generator.setSpeed(value);
}

void MoodLampManager::setSendDataOnlyIfColorsChanged(bool state)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << state;
	m_isSendDataOnlyIfColorsChanged = state;
}

void MoodLampManager::setNumberOfLeds(int numberOfLeds)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << numberOfLeds;

	initColors(numberOfLeds);
	setCurrentColor(m_rgbSaved);
}

void MoodLampManager::reset()
{
	m_rgbSaved = 0;
	m_generator.reset();
}

void MoodLampManager::settingsProfileChanged(const QString &profileName)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	Q_UNUSED(profileName)
	initFromSettings();
}

void MoodLampManager::initFromSettings()
{
	m_generator.setSpeed(Settings::getMoodLampSpeed());
	m_currentColor = Settings::getMoodLampColor();
	setLiquidMode(Settings::isMoodLampLiquidMode());
	m_isSendDataOnlyIfColorsChanged = Settings::isSendDataOnlyIfColorsChanges();

	initColors(Settings::getNumberOfLeds(Settings::getConnectedDevice()));
}

void MoodLampManager::updateColors()
{
	DEBUG_HIGH_LEVEL << Q_FUNC_INFO << m_isLiquidMode;

	QRgb rgb;

	if (m_isLiquidMode)
	{
		rgb = m_generator.current().rgb();
	}
	else
	{
		rgb = m_currentColor.rgb();
	}

	if (m_rgbSaved != rgb || m_isSendDataOnlyIfColorsChanged == false)
	{
		fillColors(rgb);
		emit updateLedsColors(m_colors);
	}

	m_rgbSaved = rgb;
}

void MoodLampManager::initColors(int numberOfLeds)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << numberOfLeds;

	m_colors.clear();

	for (int i = 0; i < numberOfLeds; i++)
		m_colors << 0;
}

void MoodLampManager::fillColors(QRgb rgb)
{
	DEBUG_MID_LEVEL << Q_FUNC_INFO << rgb;

	for (int i = 0; i < m_colors.size(); i++)
	{
		if (Settings::isLedEnabled(i))
			m_colors[i] = rgb;
		else
			m_colors[i] = 0;
	}
}
