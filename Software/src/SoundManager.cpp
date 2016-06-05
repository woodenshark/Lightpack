/*
 * SoundManager.cpp
 *
 *  Created on: 11.12.2011
 *     Project: Lightpack
 *
 *  Copyright (c) 2011 Mike Shatohin, mikeshatohin [at] gmail.com
 *
 *  Lightpack a USB content-driving ambient lighting system
 *
 *  Lightpack is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Lightpack is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "SoundManager.hpp"
#include "PrismatikMath.hpp"
#include "Settings.hpp"
#include <QTime>
#include "bass.h"
#include "basswasapi.h"

using namespace SettingsScope;

SoundManager::SoundManager(int hWnd, QObject *parent) : QObject(parent)
{
	m_hWnd = hWnd;

    m_isMoodLampEnabled = false;

	initFromSettings();

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateColors()));
}

// WASAPI callback - not doing anything with the data
DWORD CALLBACK DuffRecording(void *buffer, DWORD length, void *user)
{
	Q_UNUSED(buffer);
	Q_UNUSED(length);
	Q_UNUSED(user);
	return TRUE; // continue recording
}

void SoundManager::start(bool isEnabled)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << isEnabled;

	if (m_isMoodLampEnabled == isEnabled)
		return;

    m_isMoodLampEnabled = isEnabled;

    if (m_isMoodLampEnabled)
	{
		// initialize "no sound" BASS device
		if (!BASS_Init(0, 44100, 0, (HWND)m_hWnd, NULL)) {
			qCritical() << Q_FUNC_INFO << "Can't initialize BASS" << BASS_ErrorGetCode();
			return;
		}

		int device = -2;
		for (int i = -5; i < 20; i++) {
			BASS_WASAPI_DEVICEINFO inf;
			if (BASS_WASAPI_GetDeviceInfo(i, &inf)) {
				if ((inf.flags & BASS_DEVICE_ENABLED) &&
					(inf.flags & BASS_DEVICE_LOOPBACK)) {
					device = i;
					break;
				}
			}
		}

		// initialize default input WASAPI device
		if (!BASS_WASAPI_Init(device, 0, 0, BASS_WASAPI_BUFFER, 1, 0.1, &DuffRecording, NULL)) {
			qCritical() << Q_FUNC_INFO << "Can't initialize WASAPI device" << BASS_ErrorGetCode();
			return;
		}
		//BASS_WASAPI_GetInfo(&info);
		BASS_WASAPI_Start();
		// setup update timer (40hz)
		//m_timer = timeSetEvent(25, 25, (LPTIMECALLBACK)&UpdateSpectrum, 0, TIME_PERIODIC);
		m_timer.start(25);
		m_frames = 0;
    }
	else
	{
		m_timer.stop();
		BASS_WASAPI_Free();
		BASS_Free();
	}
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

void SoundManager::reset()
{
}

void SoundManager::settingsProfileChanged(const QString &profileName)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    Q_UNUSED(profileName)
    initFromSettings();
}

void SoundManager::initFromSettings()
{
	m_minColor = Settings::getSoundVisualizerMinColor();
	m_maxColor = Settings::getSoundVisualizerMaxColor();
    m_isSendDataOnlyIfColorsChanged = Settings::isSendDataOnlyIfColorsChanges();

    initColors(Settings::getNumberOfLeds(Settings::getConnectedDevice()));
}

void SoundManager::updateColors()
{
	DEBUG_HIGH_LEVEL << Q_FUNC_INFO;

	float fft[1024];
	BASS_WASAPI_GetData(fft, BASS_DATA_FFT2048); // get the FFT data

	m_frames++;
#define SPECHEIGHT 255
	int b0 = 0;
	bool changed = false;
	for (int i = 0; i < m_colors.size(); i++)
	{
		float peak = 0;
		int b1 = pow(2, i*9.0 / (m_colors.size() - 1)); // 9 was 10, but the last bucket rarely saw any action
		if (b1>1023) b1 = 1023;
		if (b1 <= b0) b1 = b0 + 1; // make sure it uses at least 1 FFT bin
		for (; b0<b1; b0++)
			if (peak<fft[1 + b0]) peak = fft[1 + b0];
		int val = sqrt(peak) * 3 * SPECHEIGHT - 4; // scale it (sqrt to make low values more visible)
		if (val>SPECHEIGHT) val = SPECHEIGHT; // cap it
		if (val<0) val = 0; // cap it

		if (m_frames % 5 == 0) m_peaks[i] -= 1;
		if (m_peaks[i] < 0) m_peaks[i] = 0;
		if (val > m_peaks[i]) m_peaks[i] = val;
		if (val < m_peaks[i] - 5) 
			val = (val * SPECHEIGHT) / m_peaks[i]; // scale val according to peak

		if (Settings::isLedEnabled(i)) {
			QColor rgb;
			rgb.setRed(m_minColor.red() + (m_maxColor.red() - m_minColor.red()) * (val / 255.f));
			rgb.setGreen(m_minColor.green() + (m_maxColor.green() - m_minColor.green()) * (val / 255.f));
			rgb.setBlue(m_minColor.blue() + (m_maxColor.blue() - m_minColor.blue()) * (val / 255.f));
			if (m_colors[i] != rgb.rgb()) changed = true;

			m_colors[i] = rgb.rgb();
		} else
			m_colors[i] = 0;
	}

	if (changed || m_isSendDataOnlyIfColorsChanged)
		emit updateLedsColors(m_colors);
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