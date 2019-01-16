/*
 * WindowsSoundManager.cpp
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


#include "WindowsSoundManager.hpp"
#include "debug.h"
#include "bass.h"
#include "basswasapi.h"

#define MAX_DEVICE_ID 100

WindowsSoundManager::WindowsSoundManager(int hWnd, QObject *parent) : SoundManagerBase(parent)
{
	m_hWnd = hWnd;

	m_timer.setTimerType(Qt::PreciseTimer);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateColors()));
}

WindowsSoundManager::~WindowsSoundManager()
{
	if (m_isInited) BASS_Free();
}

bool WindowsSoundManager::init() {
	BASS_SetConfig(BASS_CONFIG_UNICODE, true);
	// initialize "no sound" BASS device
	if (!BASS_Init(0, 44100, 0, (HWND)m_hWnd, NULL)) {
		qCritical() << Q_FUNC_INFO << "Can't initialize BASS" << BASS_ErrorGetCode();
		m_isInited = false;
		return false;
	}

	m_isInited = true;
	return true;
}

// WASAPI callback - not doing anything with the data
DWORD CALLBACK DuffRecording(void *buffer, DWORD length, void *user)
{
	Q_UNUSED(buffer);
	Q_UNUSED(length);
	Q_UNUSED(user);
	return TRUE; // continue recording
}

void WindowsSoundManager::populateDeviceList(QList<SoundManagerDeviceInfo>& devices, int& recommended)
{
	for (int i = 0; i < MAX_DEVICE_ID; i++) {
		BASS_WASAPI_DEVICEINFO inf;
		if (BASS_WASAPI_GetDeviceInfo(i, &inf)) {
			if (inf.flags & BASS_DEVICE_ENABLED) {
				devices.append(SoundManagerDeviceInfo(inf.name, i));

				if (inf.flags & BASS_DEVICE_LOOPBACK) {
					devices.last().name.append(tr(" (loopback)"));
					if (!recommended) recommended = i;
				}
			}
		}
	}
}

void WindowsSoundManager::start(bool isEnabled)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << isEnabled;

	if (m_isEnabled == isEnabled)
		return;

	m_isEnabled = isEnabled;

	if (m_isEnabled)
	{
		if (!m_isInited)
		{
			if (!init()) {
				m_isEnabled = false;
				return;
			}
		}

		int device = -2;
		if (m_device == -1) {
			// Fallback: first enabled loopback device (recommended)
			for (int i = 0; i < MAX_DEVICE_ID; i++) {
				BASS_WASAPI_DEVICEINFO inf;
				if (BASS_WASAPI_GetDeviceInfo(i, &inf)) {
					if ((inf.flags & BASS_DEVICE_ENABLED) &&
						(inf.flags & BASS_DEVICE_LOOPBACK)) {
						device = i;
						break;
					}
				}
			}
			if (device == -2) {
				qWarning() << Q_FUNC_INFO << "No saved device. No auto fallback device found.";
			} else {
				DEBUG_LOW_LEVEL << Q_FUNC_INFO << "No saved device. Falling back to first enabled loopback: " << device;
			}
		} else {
			BASS_WASAPI_DEVICEINFO inf;
			if (BASS_WASAPI_GetDeviceInfo(m_device, &inf)) {
				if (!(inf.flags & BASS_DEVICE_ENABLED)) {
					qCritical() << Q_FUNC_INFO << "Selected device is not enabled!";
					return;
				}
				if (!(inf.flags & BASS_DEVICE_LOOPBACK)) {
					qWarning() << Q_FUNC_INFO << "Selected device is not a loopback device";
				}
			} else {
				qCritical() << Q_FUNC_INFO << "Unable to get device info for" << m_device;
				return;
			}
			device = m_device;
		}

		// initialize input WASAPI device
		if (!BASS_WASAPI_Init(device, 0, 0, BASS_WASAPI_BUFFER, 1, 0.1f, &DuffRecording, NULL)) {
			qCritical() << Q_FUNC_INFO << "Can't initialize WASAPI device" << BASS_ErrorGetCode();
			return;
		}

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
	}

	if (m_isEnabled && m_isLiquidMode)
		m_generator.start();
	else
		m_generator.stop();
}

void WindowsSoundManager::updateFft()
{
	// get the FFT data
	if (fftSize() == 1024)
		BASS_WASAPI_GetData(m_fft, BASS_DATA_FFT2048); 
	else if (fftSize() == 2048)
		BASS_WASAPI_GetData(m_fft, BASS_DATA_FFT4096);
}
