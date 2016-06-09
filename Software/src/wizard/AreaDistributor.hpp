/*
 * ZoneDistributor.hpp
 *
 *  Created on: 10/28/2013
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

#ifndef AREADISTRIBUTOR_HPP
#define AREADISTRIBUTOR_HPP
#include <QList>
#include <QApplication>
#include <QDesktopWidget>
#include <math.h>

class ScreenArea {
public:
    ScreenArea(double hScanStart, double hScanEnd,
                   double vScanStart, double vScanEnd):
        _hScanStart(hScanStart),
        _hScanEnd(hScanEnd),
        _vScanStart(vScanStart),
        _vScanEnd(vScanEnd){}

    int hScanStart() { return _hScanStart; }
	int hScanEnd() { return _hScanEnd; }
	int vScanStart() { return _vScanStart; }
	int vScanEnd() { return _vScanEnd; }
protected:
	int _hScanStart;
	int _hScanEnd;
	int _vScanStart;
	int _vScanEnd;
};


template <class T>
inline int cmp(T x, T y, double e) {
    T d = x - y;
    if (fabs((double)d) < e) return 0;
    if (d < 0) return -1;
    return 1;
}

class AreaDistributor {
public:
	AreaDistributor(QRect screen, int areaCount) :
        _areaCount(areaCount),
		_screen(screen),
        _currentArea(NULL)
    {}
    virtual ~AreaDistributor(){}

    virtual ScreenArea * next() = 0;

    int areaCount() const { return _areaCount; }

protected:
    int _areaCount;
	QRect _screen;
    ScreenArea * _currentArea;

    double aspect() const {
       return (double)_screen.width() / _screen.height();
    }

    virtual int areaCountOnSideEdge() const = 0;
    virtual int areaCountOnTopEdge() const = 0;
    virtual int areaCountOnBottomEdge() const = 0;
};

#endif // AREADISTRIBUTOR_HPP
