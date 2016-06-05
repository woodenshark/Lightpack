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

class SoundManager : public QObject
{
    Q_OBJECT
public:
	explicit SoundManager(int hWnd, QObject *parent = 0);

signals:
    void updateLedsColors(const QList<QRgb> & colors);

public:
    void start(bool isMoodLampEnabled);

    // Common options
    void setSendDataOnlyIfColorsChanged(bool state);
    void reset();

public slots:
    void initFromSettings();
    void settingsProfileChanged(const QString &profileName);
	void setNumberOfLeds(int value);
	void setMinColor(QColor color);
	void setMaxColor(QColor color);

private slots:
    void updateColors();

private:
	void initColors(int numberOfLeds);
	void update();

private:
	QList<QRgb> m_colors;
	QList<int> m_peaks;
	int m_frames;

	QTimer m_timer;
	int m_hWnd;

	bool   m_isMoodLampEnabled;
	QColor m_minColor;
	QColor m_maxColor;
    bool   m_isSendDataOnlyIfColorsChanged;
};
