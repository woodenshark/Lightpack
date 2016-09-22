/*
 * MoodLampManager.hpp
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

#pragma once

#include <QObject>
#include <QColor>
#include <QTimer>
#include "LiquidColorGenerator.hpp"


struct SoundManagerDeviceInfo {
	SoundManagerDeviceInfo(){ this->name = ""; this->id = -1; }
	SoundManagerDeviceInfo(QString name, int id){ this->name = name; this->id = id; }
	QString name;
	int id;
};
Q_DECLARE_METATYPE(SoundManagerDeviceInfo);

class SoundManager : public QObject
{
    Q_OBJECT

public:
	explicit SoundManager(int hWnd, QObject *parent = 0);
	~SoundManager();

signals:
    void updateLedsColors(const QList<QRgb> & colors);
	void deviceList(const QList<SoundManagerDeviceInfo> & devices, int recommended);

public:
    void start(bool isMoodLampEnabled);

    // Common options
    void setSendDataOnlyIfColorsChanged(bool state);
    void reset();

public slots:
    void initFromSettings();
    void settingsProfileChanged(const QString &profileName);
	void setNumberOfLeds(int value);
	void setDevice(int value);
	void setMinColor(QColor color);
	void setMaxColor(QColor color);
	void setLiquidMode(bool isEnabled);
	void setLiquidModeSpeed(int value);
	void requestDeviceList();

private slots:
    void updateColors();

private:
	bool init();
	void initColors(int numberOfLeds);

private:
	LiquidColorGenerator m_generator;

	QList<QRgb> m_colors;
	QList<int> m_peaks;
	int m_frames;

	QTimer m_timer;
	int m_hWnd;


	bool   m_isEnabled;
	bool   m_isInited;
	bool   m_isLiquidMode;
	QColor m_minColor;
	QColor m_maxColor;
	int    m_device;
    bool   m_isSendDataOnlyIfColorsChanged;
};
