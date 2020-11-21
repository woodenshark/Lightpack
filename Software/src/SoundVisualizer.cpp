/*
 * SoundVisualizer.cpp
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

/*
	_OBJ_NAME_	: class name prefix
	_LABEL_		: name string to be displayed
	_BODY_		: class declaration body
*/
#define DECLARE_VISUALIZER(_OBJ_NAME_,_LABEL_,_BODY_) \
class _OBJ_NAME_ ## SoundVisualizer : public SoundVisualizerBase \
{\
public:\
_OBJ_NAME_ ## SoundVisualizer() = default;\
~_OBJ_NAME_ ## SoundVisualizer() = default;\
\
static const char* name() { return _LABEL_; };\
static SoundVisualizerBase* create() { return new _OBJ_NAME_ ## SoundVisualizer(); };\
\
bool visualize(const float* const fftData, const size_t fftSize, QList<QRgb>& colors);\
_BODY_\
};\
struct _OBJ_NAME_ ## Register {\
_OBJ_NAME_ ## Register(){\
	visualizerMap.insert(vizID, SoundManagerVisualizerInfo(_OBJ_NAME_ ## SoundVisualizer::name(), _OBJ_NAME_ ## SoundVisualizer::create, vizID));\
	++vizID;\
}\
};\
_OBJ_NAME_ ## Register _OBJ_NAME_ ## Reg;

using namespace SettingsScope;

namespace {
	static QMap<int, SoundManagerVisualizerInfo> visualizerMap;
	static int vizID = 0;
}

SoundVisualizerBase* SoundVisualizerBase::createWithID(const int id)
{
	QMap<int, SoundManagerVisualizerInfo>::const_iterator i = visualizerMap.find(id);
	if (i != visualizerMap.end())
		return i.value().factory();
	qWarning() << Q_FUNC_INFO << "failed to find visualizer ID: " << id;
	return nullptr;
}

void SoundVisualizerBase::populateNameList(QList<SoundManagerVisualizerInfo>& list, int& recommended)
{
	list = visualizerMap.values();

	if (list.size() > 0)
		recommended = list[0].id;
}

#pragma region Prismatik
DECLARE_VISUALIZER(Prismatik, "Prismatik (default)",
public:
	void clear(const int numberOfLeds);
private:
	QList<int> m_peaks;
	const int SpecHeight = 1000;
);

bool PrismatikSoundVisualizer::visualize(const float* const fftData, const size_t fftSize, QList<QRgb>& colors)
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
			interpolateColor(color, from, to, val, SpecHeight);
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
DECLARE_VISUALIZER(TwinPeaks, "Twin Peaks",
public:
private:
	float m_previousPeak{ 0.0f };
	size_t m_prevThresholdLed{ 0 };
	double m_speedCoef = 1.0;
	const uint8_t FadeOutSpeed = 12;
);

bool TwinPeaksSoundVisualizer::visualize(const float* const fftData, const size_t fftSize, QList<QRgb>& colors)
{
	bool changed = false;
	const size_t middleLed = std::floor(colors.size() / 2);

	// most sensitive Hz range for humans
	// this assumes 44100Hz sample rate
	const size_t optimalHzMin = 1950 / (44100 / 2 / fftSize); // 2kHz
	const size_t optimalHzMax = 5050 / (44100 / 2 / fftSize); // 5kHz

	float currentPeak = 0.0f;
	for (size_t i = 0; i < fftSize; ++i) {
		float mag = fftData[i];
		if (i > optimalHzMin && i < optimalHzMax)// amplify sensitive range
			mag *= 6.0f;
		currentPeak += mag;
	}

	if (m_previousPeak < currentPeak)
		m_previousPeak = currentPeak;
	else
		m_previousPeak = std::max(0.0f, m_previousPeak - (fftSize * 0.000005f)); // lower the stale peak so it doesn't persist forever

	const size_t thresholdLed = m_previousPeak != 0.0f ? middleLed * (currentPeak / m_previousPeak) : 0;

	// if leds move a lot with x% amplitude, increase fade speed
	if (std::abs((int)(thresholdLed - m_prevThresholdLed)) > middleLed * 0.2)
		m_speedCoef = std::min(10.0, m_speedCoef + 2.0);
	else // reset on slow down
		m_speedCoef = 1.0;// std::max(1.0, speedCoef - 2.0);

	for (int idxA = 0; idxA < middleLed; ++idxA) {
		const int idxB = colors.size() - 1 - idxA;
		QRgb color = 0;
		if (idxA < thresholdLed) {
			QColor from = m_isLiquidMode ? m_generator.current() : m_minColor;
			QColor to = m_isLiquidMode ? from : m_maxColor;
			if (m_isLiquidMode) {
				from.setHsl(from.hue(), from.saturation(), 120);
				to.setHsl(to.hue() + 180, to.saturation(), 120);
			}
			interpolateColor(color, from, to, idxA, middleLed);
		}
		else if (FadeOutSpeed > 0 && (colors[idxA] > 0 || colors[idxB] > 0)) { // fade out old peaks
			QColor oldColor(std::max(colors[idxA], colors[idxB])); // both colors are either the same or one is 0, so max() is good enough here
			const int luminosity = std::min(std::max(oldColor.lightness() - FadeOutSpeed * ((thresholdLed > 0 ? thresholdLed : 1) / (double)idxA)* m_speedCoef, 255.0), 0.0);
			oldColor.setHsl(oldColor.hue(), oldColor.saturation(), luminosity);
			color = oldColor.rgb();
		}

		// peak A
		QRgb colorA = Settings::isLedEnabled(idxA) ? color : 0;
		changed = changed || (colors[idxA] != colorA);
		colors[idxA] = colorA;

		// peak B
		QRgb colorB = Settings::isLedEnabled(idxB) ? color : 0;
		changed = changed || (colors[idxB] != colorB);
		colors[idxB] = colorB;
	}
	if (currentPeak == 0.0f) // reset peak on silence
		m_previousPeak = currentPeak;

	m_prevThresholdLed = thresholdLed;
	return changed;
}
#pragma endregion TwinPeaks
