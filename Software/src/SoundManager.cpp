/*
 * SoundManager.cpp
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


#include "SoundManager.hpp"
#include "PrismatikMath.hpp"
#include "Settings.hpp"
#include <QTime>

#if defined(Q_OS_MACOS)
#include "MacOSSoundManager.h"
#elif defined(Q_OS_WIN) && defined(BASS_SOUND_SUPPORT)
#include "WindowsSoundManager.hpp"
#endif

using namespace SettingsScope;

SoundManager* SoundManager::create(int hWnd, QObject* parent)
{
#if defined(Q_OS_MACOS)
	Q_UNUSED(hWnd);
	return new MacOSSoundManager(parent);
#elif defined(Q_OS_WIN) && defined(BASS_SOUND_SUPPORT)
	return new WindowsSoundManager(hWnd, parent);
#endif
	return nullptr;
}

SoundManager::SoundManager(QObject *parent) : QObject(parent)
{
	initFromSettings();
	m_fft = (float *)calloc(fftSize(), sizeof(*m_fft));
}

SoundManager::~SoundManager()
{
	if (m_isEnabled) start(false);
	if (m_fft)
		free((void *)m_fft);
}

size_t SoundManager::fftSize() const
{
	// has to be a power of 2
	return 1024;
}

float* SoundManager::fft() const
{
	return m_fft;
}

void SoundManager::requestDeviceList()
{
	if (!m_isInited)
	{
		if (!init()) {
			m_isEnabled = false;
			return;
		}
	}

	QList<SoundManagerDeviceInfo> devices;
	int recommended = -1;
	
	populateDeviceList(devices, recommended);

	emit deviceList(devices, recommended);
}

void SoundManager::setDevice(int value)
{
	DEBUG_MID_LEVEL << Q_FUNC_INFO << value;

	bool enabled = m_isEnabled;
	if (enabled) start(false);
	m_device = value;
	if (enabled) start(true);
}

void SoundManager::setMinColor(QColor color)
{
	DEBUG_MID_LEVEL << Q_FUNC_INFO << color;

	m_minColor = color;
}

void SoundManager::setMaxColor(QColor color)
{
	DEBUG_MID_LEVEL << Q_FUNC_INFO << color;

	m_maxColor = color;
}

void SoundManager::setLiquidMode(bool state)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << state;
	m_isLiquidMode = state;
	if (m_isLiquidMode && m_isEnabled)
		m_generator.start();
	else {
		m_generator.stop();
		if (m_isEnabled)
			updateColors();
	}
}

void SoundManager::setLiquidModeSpeed(int value)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;
	m_generator.setSpeed(value);
}

void SoundManager::setSendDataOnlyIfColorsChanged(bool state)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << state;
	m_isSendDataOnlyIfColorsChanged = state;
}

void SoundManager::setNumberOfLeds(int numberOfLeds)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << numberOfLeds;

	initColors(numberOfLeds);
}

void SoundManager::settingsProfileChanged(const QString &profileName)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	Q_UNUSED(profileName)
	initFromSettings();
}

void SoundManager::initFromSettings()
{
	m_device = Settings::getSoundVisualizerDevice();
	m_minColor = Settings::getSoundVisualizerMinColor();
	m_maxColor = Settings::getSoundVisualizerMaxColor();
	m_isLiquidMode = Settings::isSoundVisualizerLiquidMode();
	m_generator.setSpeed(Settings::getSoundVisualizerLiquidSpeed());
	m_isSendDataOnlyIfColorsChanged = Settings::isSendDataOnlyIfColorsChanges();

	initColors(Settings::getNumberOfLeds(Settings::getConnectedDevice()));
}

void SoundManager::reset()
{
	initColors(m_colors.size());
	m_generator.reset();
}

void SoundManager::updateColors()
{
	DEBUG_HIGH_LEVEL << Q_FUNC_INFO;
	updateFft();
	bool colorsChanged = applyFft();
	if (colorsChanged || !m_isSendDataOnlyIfColorsChanged)
		emit updateLedsColors(m_colors);
}

bool SoundManager::applyFft()
{
	m_frames++;
#define SPECHEIGHT 1000
	size_t b0 = 0;
	bool changed = false;
	for (int i = 0; i < m_colors.size(); i++)
	{
		float peak = 0;
		size_t b1 = pow(2, i*9.0 / (m_colors.size() - 1)); // 9 was 10, but the last bucket rarely saw any action
		if (b1>fftSize() - 1) b1 = fftSize() - 1;
		if (b1 <= b0) b1 = b0 + 1; // make sure it uses at least 1 FFT bin
		for (; b0<b1; b0++)
			if (peak<m_fft[1 + b0]) peak = m_fft[1 + b0];
		int val = sqrt(peak) * /* 3 * */ SPECHEIGHT - 4; // scale it (sqrt to make low values more visible)
		if (val>SPECHEIGHT) val = SPECHEIGHT; // cap it
		if (val<0) val = 0; // cap it

		if (m_frames % 5 == 0) m_peaks[i] -= 1;
		if (m_peaks[i] < 0) m_peaks[i] = 0;
		if (val > m_peaks[i]) m_peaks[i] = val;
		if (val < m_peaks[i] - 5) 
			val = (val * SPECHEIGHT) / m_peaks[i]; // scale val according to peak

		if (Settings::isLedEnabled(i)) {
			QColor from = m_isLiquidMode ? QColor(0, 0, 0) : m_minColor;
			QColor to = m_isLiquidMode ? m_generator.current() : m_maxColor;
			QColor rgb;
			rgb.setRed(from.red() + (to.red() - from.red()) * (val / (double)SPECHEIGHT));
			rgb.setGreen(from.green() + (to.green() - from.green()) * (val / (double)SPECHEIGHT));
			rgb.setBlue(from.blue() + (to.blue() - from.blue()) * (val / (double)SPECHEIGHT));
			if (m_colors[i] != rgb.rgb()) changed = true;

			m_colors[i] = rgb.rgb();
		} else
			m_colors[i] = 0;
	}
	return changed;
}

void SoundManager::initColors(int numberOfLeds)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << numberOfLeds;

	m_colors.clear();
	m_peaks.clear();

	for (int i = 0; i < numberOfLeds; i++) {
		m_colors << 0;
		m_peaks << 0;
	}
}
