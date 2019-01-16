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


#include "SoundManagerBase.hpp"
#include "PrismatikMath.hpp"
#include "Settings.hpp"
#include <QTime>

#if defined(Q_OS_MACOS)
#include "MacOSSoundManager.h"
#elif defined(Q_OS_WIN) && defined(BASS_SOUND_SUPPORT)
#include "WindowsSoundManager.hpp"
#endif

using namespace SettingsScope;

SoundManagerBase* SoundManagerBase::create(int hWnd, QObject* parent)
{
#if defined(Q_OS_MACOS)
	Q_UNUSED(hWnd);
	return new MacOSSoundManager(parent);
#elif defined(Q_OS_WIN) && defined(BASS_SOUND_SUPPORT)
	return new WindowsSoundManager(hWnd, parent);
#endif
	return nullptr;
}

SoundManagerBase::SoundManagerBase(QObject *parent) : QObject(parent)
{
	SoundVisualizerBase::populateFactoryList(m_visualizerList);
	m_fft = (float *)calloc(fftSize(), sizeof(*m_fft));
	initFromSettings();
}

SoundManagerBase::~SoundManagerBase()
{
	if (m_isEnabled) start(false);
	if (m_fft)
		free((void *)m_fft);
	if (m_visualizer)
		delete m_visualizer;
}

size_t SoundManagerBase::fftSize() const
{
	const size_t size = 1024;
	static_assert(size && !(size & (size - 1)), "FFT size has to be a power of 2");
	return size;
}

float* SoundManagerBase::fft() const
{
	return m_fft;
}

void SoundManagerBase::requestDeviceList()
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

void SoundManagerBase::setDevice(int value)
{
	DEBUG_MID_LEVEL << Q_FUNC_INFO << value;

	bool enabled = m_isEnabled;
	if (enabled) start(false);
	m_device = value;
	if (enabled) start(true);
}

void SoundManagerBase::setMinColor(QColor color)
{
	DEBUG_MID_LEVEL << Q_FUNC_INFO << color;

	if (m_visualizer)
		m_visualizer->setMinColor(color);
}

void SoundManagerBase::setMaxColor(QColor color)
{
	DEBUG_MID_LEVEL << Q_FUNC_INFO << color;

	if (m_visualizer)
		m_visualizer->setMaxColor(color);
}

void SoundManagerBase::setLiquidMode(bool state)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << state;

	if (m_visualizer)
		m_visualizer->setLiquidMode(state);

	if (!state && m_isEnabled)
		updateColors();
}

void SoundManagerBase::setLiquidModeSpeed(int value)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;
	if (m_visualizer)
		m_visualizer->setSpeed(value);
}

void SoundManagerBase::setSendDataOnlyIfColorsChanged(bool state)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << state;
	m_isSendDataOnlyIfColorsChanged = state;
}

void SoundManagerBase::setNumberOfLeds(int numberOfLeds)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << numberOfLeds;

	initColors(numberOfLeds);
}

void SoundManagerBase::settingsProfileChanged(const QString &profileName)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;
	Q_UNUSED(profileName)
	initFromSettings();
}

void SoundManagerBase::initFromSettings()
{
	m_device = Settings::getSoundVisualizerDevice();

	setVisualizer(Settings::getSoundVisualizerVisualizer());

	m_isSendDataOnlyIfColorsChanged = Settings::isSendDataOnlyIfColorsChanges();

	initColors(Settings::getNumberOfLeds(Settings::getConnectedDevice()));
}

void SoundManagerBase::reset()
{
	initColors(m_colors.size());
	if (m_visualizer)
		m_visualizer->reset();
}

void SoundManagerBase::updateColors()
{
	DEBUG_HIGH_LEVEL << Q_FUNC_INFO;

	updateFft();
	bool colorsChanged = (m_visualizer ? m_visualizer->visualize(m_fft, fftSize(), m_colors) : false);
	if (colorsChanged || !m_isSendDataOnlyIfColorsChanged)
		emit updateLedsColors(m_colors);
}

void SoundManagerBase::initColors(int numberOfLeds)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << numberOfLeds;

	m_colors.clear();
	if (m_visualizer)
		m_visualizer->clear(numberOfLeds);

	for (int i = 0; i < numberOfLeds; i++)
		m_colors << 0;
}

void SoundManagerBase::setVisualizer(int value)
{
	if (value >= m_visualizerList.size() || value < 0)
		return;

	bool running = false;
	if (m_visualizer) {
		running = m_visualizer->isRunning();
		delete m_visualizer;
		m_visualizer = nullptr;
	}
	m_visualizer = m_visualizerList[value]();
	if (m_visualizer) {
		m_visualizer->setMinColor(Settings::getSoundVisualizerMinColor());
		m_visualizer->setMaxColor(Settings::getSoundVisualizerMaxColor());
		m_visualizer->setLiquidMode(Settings::isSoundVisualizerLiquidMode());
		m_visualizer->setSpeed(Settings::getSoundVisualizerLiquidSpeed());
		m_visualizer->clear(m_colors.size());
		if (running)
			m_visualizer->start();
	}
}

void SoundManagerBase::requestVisualizerList()
{
	QList<SoundManagerVisualizerInfo> visualizers;
	int recommended = -1;

	SoundVisualizerBase::populateNameList(visualizers, recommended);

	emit visualizerList(visualizers, recommended);
}
