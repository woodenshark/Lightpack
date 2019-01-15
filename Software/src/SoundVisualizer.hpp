/*
 * MoodLampManager.hpp
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

#pragma once

#include <QColor>
#include "LiquidColorGenerator.hpp"

struct SoundManagerVisualizerInfo {
	SoundManagerVisualizerInfo() { this->name = ""; this->id = -1; }
	SoundManagerVisualizerInfo(QString name, int id) { this->name = name; this->id = id; }
	QString name;
	int id;
};
Q_DECLARE_METATYPE(SoundManagerVisualizerInfo);

class SoundVisualizerBase;

typedef SoundVisualizerBase* (*VisualizerFactory)();

class SoundVisualizerBase
{
public:
	static const char* const name() { return "NO_NAME"; };
	static SoundVisualizerBase* create() { Q_ASSERT_X(false, "SoundVisualizerBase::create()", "not implemented"); return nullptr; };
	static void populateFactoryList(QList<VisualizerFactory>&);
	static void populateNameList(QList<SoundManagerVisualizerInfo>&, int& recommended);

	SoundVisualizerBase() = default;
	virtual ~SoundVisualizerBase() {
		stop();
	};

	void setMinColor(const QColor& color) {
		m_minColor = color;
	}

	void setMaxColor(const QColor& color) {
		m_maxColor = color;
	}

	void setLiquidMode(const bool liquid) {
		m_isLiquidMode = liquid;

		if (m_isLiquidMode && m_isRunning)
			m_generator.start();
		else
			m_generator.stop();
	}

	void setSpeed(const int speed) {
		m_generator.setSpeed(speed);
	}

	const bool isRunning() const {
		return m_isRunning;
	}

	virtual void start() {
		m_isRunning = true;

		if (m_isLiquidMode)
			m_generator.start();
		else
			m_generator.stop();
	}

	virtual void stop() {
		m_isRunning = false;
		m_generator.stop();
	}

	virtual void reset() {
		m_generator.reset();
	}

	virtual void clear(const int /*numberOfLeds*/) {};

	virtual const bool visualize(const float* const fftData, const size_t fftSize, QList<QRgb>& colors) = 0;

protected:
	QColor 	m_minColor;
	QColor 	m_maxColor;
	LiquidColorGenerator m_generator;
	bool	m_isLiquidMode{ false };
	bool	m_isRunning{ false };
	size_t  m_frames{ 0 };
};

#define DECLARE_VISUALIZER(_OBJ_NAME_,_LABEL_) \
public:\
_OBJ_NAME_##SoundVisualizer() = default;\
~##_OBJ_NAME_##SoundVisualizer() = default;\
\
static const char* const name() { return _LABEL_; };\
static SoundVisualizerBase* create() { return new _OBJ_NAME_##SoundVisualizer(); };\
\
const bool visualize(const float* const fftData, const size_t fftSize, QList<QRgb>& colors);

class PrismatikSoundVisualizer : public SoundVisualizerBase
{
	DECLARE_VISUALIZER(Prismatik, "Prismatik (default)");
	void clear(const int numberOfLeds);
private:
	QList<int> m_peaks;
	const int SpecHeight = 1000;
};

class TwinPeaksSoundVisualizer : public SoundVisualizerBase
{
	DECLARE_VISUALIZER(TwinPeaks, "Twin Peaks");
private:
	float m_previousPeak{ 0.0f };
};
