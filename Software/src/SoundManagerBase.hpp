/*
 * SoundManagerBase.hpp
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

#include <QObject>
#include <QColor>
#include <QElapsedTimer>
#include "SoundVisualizer.hpp"

struct SoundManagerDeviceInfo {
	SoundManagerDeviceInfo(){ this->name = ""; this->id = -1; }
	SoundManagerDeviceInfo(QString name, int id){ this->name = name; this->id = id; }
	QString name;
	int id;
};
Q_DECLARE_METATYPE(SoundManagerDeviceInfo);

class SoundManagerBase : public QObject
{
	Q_OBJECT

public:
	SoundManagerBase(QObject *parent = 0);
	virtual ~SoundManagerBase();
	static SoundManagerBase* create(int hWnd = 0, QObject* parent = 0);

signals:
	void updateLedsColors(const QList<QRgb> & colors);
	void deviceList(const QList<SoundManagerDeviceInfo> & devices, int recommended);
	void visualizerList(const QList<SoundManagerVisualizerInfo>& visualizers, int recommended);
	void visualizerFrametime(const double);

public:
	virtual void start(bool isEnabled) { Q_UNUSED(isEnabled); Q_ASSERT(("Not implemented", false)); };

	// Common options
	void setSendDataOnlyIfColorsChanged(bool state);
	void reset();
	virtual size_t fftSize() const;
	float* fft() const;

public slots:
	void initFromSettings();
	void settingsProfileChanged(const QString &profileName);
	void setNumberOfLeds(int value);
	void setDevice(int value);
	void setVisualizer(int value);
	void setMinColor(QColor color);
	void setMaxColor(QColor color);
	void setLiquidMode(bool isEnabled);
	void setLiquidModeSpeed(int value);
	void requestDeviceList();
	void requestVisualizerList();
	void updateColors();

protected:
	virtual bool init() = 0;
	void initColors(int numberOfLeds);
	virtual void populateDeviceList(QList<SoundManagerDeviceInfo>& devices, int& recommended) = 0;
	virtual void updateFft() {};

protected:
	SoundVisualizerBase* m_visualizer{nullptr};

	QList<QRgb> m_colors;

	bool	m_isEnabled{false};
	bool	m_isInited{false};
	int		m_device{-1};
	bool	m_isSendDataOnlyIfColorsChanged{false};
	
	float*	m_fft{nullptr};
	
	QElapsedTimer m_elapsedTimer;
};
