/*
 * SettingsAwareTrait.hpp
 *
 *  Created on: 10/23/2013
 *     Project: Prismatik
 *
 *  Copyright (c) 2013 Tim
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

#ifndef SETTINGSAWARETRAIT_HPP
#define SETTINGSAWARETRAIT_HPP

#include <QSharedPointer>
#include <QMap>

namespace SettingsScope {
class Settings;
}

class AbstractLedDevice;

struct TransientSettings {
    QSharedPointer<AbstractLedDevice>ledDevice;
	QMap<int, QPoint> zonePositions;
	QMap<int, QSize> zoneSizes;
};

class SettingsAwareTrait {
public:
    SettingsAwareTrait(bool isInitFromSettings, TransientSettings *transSettings)
        : _isInitFromSettings(isInitFromSettings)
        , _transSettings(transSettings)
    {}
protected:
    bool _isInitFromSettings;
    TransientSettings *_transSettings;

};

#endif // SETTINGSAWARETRAIT_HPP
