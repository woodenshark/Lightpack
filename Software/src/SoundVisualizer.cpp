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

#include <QTime>
#include "Settings.hpp"
#include "SoundVisualizer.hpp"
#include <cmath>

using namespace SettingsScope;

void SoundVisualizerBase::populateFactoryList(QList<VisualizerFactory>& list)
{
	list.append(PrismatikSoundVisualizer::create);
	list.append(TwinPeaksSoundVisualizer::create);
}

void SoundVisualizerBase::populateNameList(QList<SoundManagerVisualizerInfo>& list, int& recommended)
{
	list.append(SoundManagerVisualizerInfo(PrismatikSoundVisualizer::name(), 0));
	list.append(SoundManagerVisualizerInfo(TwinPeaksSoundVisualizer::name(), 1));

	recommended = 0;
}

#pragma region Prismatik
const bool PrismatikSoundVisualizer::visualize(const float* const fftData, const size_t fftSize, QList<QRgb>& colors)
{
	size_t b0 = 0;
	bool changed = false;
	for (int i = 0; i < colors.size(); i++)
	{
		float peak = 0;
		size_t b1 = pow(2, i*9.0 / (colors.size() - 1)); // 9 was 10, but the last bucket rarely saw any action
		if (b1 > fftSize - 1) b1 = fftSize - 1;
		if (b1 <= b0) b1 = b0 + 1; // make sure it uses at least 1 FFT bin
		for (; b0 < b1; b0++)
			if (peak < fftData[1 + b0]) peak = fftData[1 + b0];
		int val = sqrt(peak) * /* 3 * */ SpecHeight - 4; // scale it (sqrt to make low values more visible)
		if (val > SpecHeight) val = SpecHeight; // cap it
		if (val < 0) val = 0; // cap it

		if (m_frames % 5 == 0) m_peaks[i] -= 1;
		if (m_peaks[i] < 0) m_peaks[i] = 0;
		if (val > m_peaks[i]) m_peaks[i] = val;
		if (val < m_peaks[i] - 5)
			val = (val * SpecHeight) / m_peaks[i]; // scale val according to peak

		QRgb color = 0;
		if (Settings::isLedEnabled(i)) {
			QColor from = m_isLiquidMode ? QColor(0, 0, 0) : m_minColor;
			QColor to = m_isLiquidMode ? m_generator.current() : m_maxColor;
			QColor rgb;
			rgb.setRed(from.red() + (to.red() - from.red()) * (val / (double)SpecHeight));
			rgb.setGreen(from.green() + (to.green() - from.green()) * (val / (double)SpecHeight));
			rgb.setBlue(from.blue() + (to.blue() - from.blue()) * (val / (double)SpecHeight));

			color = rgb.rgb();
		}

		changed = changed || (colors[i] != color);
		colors[i] = color;
	}
	m_frames++;
	return changed;
}

void PrismatikSoundVisualizer::clear(const int numberOfLeds) {
	m_peaks.clear();
	for (int i = 0; i < numberOfLeds; ++i)
		m_peaks << 0;
}
#pragma endregion Prismatik



#pragma region TwinPeaks
const bool TwinPeaksSoundVisualizer::visualize(const float* const fftData, const size_t fftSize, QList<QRgb>& colors)
{
	bool changed = false;
	const size_t middleLed = std::floor(colors.size() / 2);

	float currentPeak = 0.0f;
	for (size_t i = 0; i < fftSize; ++i)
		currentPeak += fftData[i];

	if (m_previousPeak < currentPeak)
		m_previousPeak = currentPeak;

	const size_t thresholdLed = m_previousPeak != 0.0f ? middleLed * (currentPeak / m_previousPeak) : 0;
	for (size_t i = 0; i < middleLed; ++i) {
		QRgb color = 0;
		if (i < thresholdLed) {
			QColor from = m_isLiquidMode ? m_generator.current() : m_minColor;
			QColor to = m_isLiquidMode ? from : m_maxColor;
			if (m_isLiquidMode) {
				from.setHsl(from.hue(), from.saturation(), 120);
				to.setHsl(to.hue() + 180, to.saturation(), 120);
			}

			QColor rgb;
			rgb.setRed(from.red() + (to.red() - from.red()) * (i / (double)middleLed));
			rgb.setGreen(from.green() + (to.green() - from.green()) * (i / (double)middleLed));
			rgb.setBlue(from.blue() + (to.blue() - from.blue()) * (i / (double)middleLed));

			color = rgb.rgb();
		}
			
		// peak A
		QRgb colorA = Settings::isLedEnabled(i) ? color : 0;
		changed = changed || (colors[i] != colorA);
		colors[i] = colorA;

		// peak B
		QRgb colorB = Settings::isLedEnabled(colors.size() - i - 1) ? color : 0;
		changed = changed || (colors[colors.size() - i - 1] != colorB);
		colors[colors.size() - i - 1] = colorB;
	}
	if (currentPeak == 0.0f)
		m_previousPeak = currentPeak;
	return changed;
}
#pragma endregion TwinPeaks
