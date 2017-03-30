/*
 * AbstractLedDevice.cpp
 *
 *  Created on: 05.02.2013
 *     Project: Prismatik
 *
 *  Copyright (c) 2013 Timur Sattarov, tim.helloworld [at] gmail.com
 *
 *  Lightpack is an open-source, USB content-driving ambient lighting
 *  hardware.
 *
 *  Prismatik is a free, open-source software: you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License as published 
 *  by the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Prismatik and Lightpack files is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "AbstractLedDevice.hpp"
#include "colorspace_types.h"
#include "PrismatikMath.hpp"
#include "Settings.hpp"

void AbstractLedDevice::setGamma(double value) {
    m_gamma = value;
    setColors(m_colorsSaved);
}

void AbstractLedDevice::setBrightness(int value) {
    m_brightness = value;
    setColors(m_colorsSaved);
}

void AbstractLedDevice::setLuminosityThreshold(int value) {
    m_luminosityThreshold = value;
    setColors(m_colorsSaved);
}

void AbstractLedDevice::setMinimumLuminosityThresholdEnabled(bool value) {
    m_isMinimumLuminosityEnabled = value;
    setColors(m_colorsSaved);
}

void AbstractLedDevice::updateWBAdjustments(const QList<WBAdjustment> &coefs) {
    m_wbAdjustments.clear();
    m_wbAdjustments.append(coefs);
    setColors(m_colorsSaved);
}

void AbstractLedDevice::updateDeviceSettings()
{
    using namespace SettingsScope;
    setGamma(Settings::getDeviceGamma());
    setBrightness(Settings::getDeviceBrightness());
    setLuminosityThreshold(Settings::getLuminosityThreshold());
    setMinimumLuminosityThresholdEnabled(Settings::isMinimumLuminosityEnabled());
    updateWBAdjustments(Settings::getLedCoefs());
}

/*!
  Modifies colors according to gamma, luminosity threshold, white balance and brightness settings
  All modifications are made over extended 12bit RGB, so \code outColors \endcode will contain 12bit
  RGB instead of 8bit.
*/
void AbstractLedDevice::applyColorModifications(const QList<QRgb> &inColors, QList<StructRgb> &outColors) {

    bool isApplyWBAdjustments = m_wbAdjustments.count() == inColors.count();

    for(int i = 0; i < inColors.count(); i++) {

        //renormalize to 12bit
        double k = 4095/255.0;
        outColors[i].r = qRed(inColors[i])   * k;
        outColors[i].g = qGreen(inColors[i]) * k;
        outColors[i].b = qBlue(inColors[i])  * k;

        PrismatikMath::gammaCorrection(m_gamma, outColors[i]);
    }

    StructLab avgColor = PrismatikMath::toLab(PrismatikMath::avgColor(outColors));

    for (int i = 0; i < outColors.count(); ++i) {
        StructLab lab = PrismatikMath::toLab(outColors[i]);
        int dl = m_luminosityThreshold - lab.l;
        if (dl > 0) {
            if (m_isMinimumLuminosityEnabled) { // apply minimum luminosity or dead-zone
                // Cross-fade a and b channels to avarage value within kFadingRange, fadingFactor = (dL - fadingRange)^2 / (fadingRange^2)
                const int kFadingRange = 5;
                double fadingCoeff = dl < kFadingRange ? (dl - kFadingRange)*(dl - kFadingRange)/(kFadingRange*kFadingRange): 1;
                char da = avgColor.a - lab.a;
                char db = avgColor.b - lab.b;
                lab.l = m_luminosityThreshold;
                lab.a += PrismatikMath::round(da * fadingCoeff);
                lab.b += PrismatikMath::round(db * fadingCoeff);
                StructRgb rgb = PrismatikMath::toRgb(lab);
                outColors[i] = rgb;
            } else {
                outColors[i].r = 0;
                outColors[i].g = 0;
                outColors[i].b = 0;
            }
        }

        PrismatikMath::brightnessCorrection(m_brightness, outColors[i]);

		if (isApplyWBAdjustments) {
			outColors[i].r *= m_wbAdjustments[i].red;
			outColors[i].g *= m_wbAdjustments[i].green;
			outColors[i].b *= m_wbAdjustments[i].blue;
		}
    }

}
