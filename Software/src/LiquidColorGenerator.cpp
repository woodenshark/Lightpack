/*
 * MoodLampManager.cpp
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


#include "LiquidColorGenerator.hpp"
#include "PrismatikMath.hpp"
#include "Settings.hpp"
#include <QTime>

using namespace SettingsScope;

const QColor LiquidColorGenerator::AvailableColors[LiquidColorGenerator::ColorsMoodLampCount] =
{
    Qt::white, Qt::red, Qt::yellow, Qt::green, Qt::blue, Qt::magenta, Qt::cyan,
    Qt::darkRed, Qt::darkGreen, Qt::darkBlue, Qt::darkYellow,
    qRgb(255,128,0), qRgb(128,255,255), qRgb(128,0,255)
};

LiquidColorGenerator::LiquidColorGenerator(QObject *parent) : QObject(parent)
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

	m_isEnabled = false;

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateColor()));
}

void LiquidColorGenerator::start()
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	m_isEnabled = true;

	reset();
	updateColor();
}

void LiquidColorGenerator::stop()
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	m_isEnabled = false;

	m_timer.stop();
}

QColor LiquidColorGenerator::current()
{
	return QColor(m_red, m_green, m_blue);
}

void LiquidColorGenerator::setSpeed(int value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;
    m_speed = value;
    m_delay = generateDelay();
}

void LiquidColorGenerator::reset()
{
	m_red = 0;
	m_green = 0;
	m_blue = 0;
}

void LiquidColorGenerator::updateColor()
{
    DEBUG_HIGH_LEVEL << Q_FUNC_INFO << m_speed;

	if (m_red == m_redNew && m_green == m_greenNew && m_blue == m_blueNew)
    {
        m_delay = generateDelay();
        QColor colorNew = generateColor();

		m_redNew = colorNew.red();
		m_greenNew = colorNew.green();
		m_blueNew = colorNew.blue();

        DEBUG_HIGH_LEVEL << Q_FUNC_INFO << colorNew;
    }

	if (m_redNew != m_red)  { if (m_red   > m_redNew)   --m_red;   else ++m_red; }
	if (m_greenNew != m_green){ if (m_green > m_greenNew) --m_green; else ++m_green; }
	if (m_blueNew != m_blue) { if (m_blue  > m_blueNew)  --m_blue;  else ++m_blue; }

	emit updateColor(QColor(m_red, m_green, m_blue));

    if (m_isEnabled)
    {
        m_timer.start(m_delay);
    }
}

int LiquidColorGenerator::generateDelay()
{
    return 1000 / (m_speed + PrismatikMath::rand(25) + 1);
}

QColor LiquidColorGenerator::generateColor()
{
    static QList<QColor> unselectedColors;

    if (unselectedColors.empty())
    {
        for (int i = 0; i < ColorsMoodLampCount; i++)
            unselectedColors << AvailableColors[i];
    }

    int randIndex = PrismatikMath::rand(unselectedColors.size());

    return unselectedColors.takeAt(randIndex);
}