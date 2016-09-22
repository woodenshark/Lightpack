/*
 * CustomDistributor.cpp
 *
 *  Created on: 08.06.2016
 *     Project: Prismatik
 *
 *  Copyright (c) 2015 Patrick Siegler
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

#include "CustomDistributor.hpp"
#include "PrismatikMath.hpp"

int roundDown(int n) {
	int rounded = (n % 2 == 0) ? n : (n - 1);
	return rounded ? rounded : 2;
}


CustomDistributor::~CustomDistributor() {
    if (_currentArea)
        cleanCurrentArea();
}

void CustomDistributor::startBottomRight() {
	if (_bottomLeds) {
		_dx = 1;
		_width = _screen.width() * (1.0 - _standWidth) / _bottomLeds;
		_height = _screen.height() * _thickness;
		_x = _screen.left() + _screen.width() - (roundDown(_bottomLeds) / 2 * _width);
		_y = _screen.top() + _screen.height() - _height;
	} else startRightUp();
}

void CustomDistributor::startRightUp() {
	if (_sideLeds) {
		cleanCurrentArea();
		_dx = 0;
		_dy = -1;
		_width = _screen.width() *  _thickness;
		_height = _screen.height() / _sideLeds;
		_x = _screen.left() + _screen.width() - _width;
		_y = _screen.top() + _screen.height() - _height;
	} else startTopLeft();
}

void CustomDistributor::startTopLeft() {
	if (_topLeds) {
		cleanCurrentArea();
		_dx = -1;
		_dy = 0;
		_width = _screen.width() / _topLeds;
		_height = _screen.height() * _thickness;
		_x = _screen.left() + _screen.width() - _width;
		_y = _screen.top();
	} else startLeftDown();
}

void CustomDistributor::startLeftDown() {
	if (_sideLeds) {
		cleanCurrentArea();
		_dx = 0;
		_dy = 1;
		_width = _screen.width() * _thickness;
		_height = _screen.height() / _sideLeds;
		_x = _screen.left();
		_y = _screen.top();
	} else startBottomRight2();
}

void CustomDistributor::startBottomRight2() {
	if (_bottomLeds) {
		cleanCurrentArea();
		_dx = 1;
		_dy = 0;
		_width = _screen.width() * (1.0 - _standWidth) / _bottomLeds;
		_height = _screen.height() * _thickness;
		_x = _screen.left();
		_y = _screen.top() + _screen.height() - _height;
	}
}


ScreenArea * CustomDistributor::next() {
    if (_dx == 0 && _dy == 0) {
		startBottomRight();
	} else if (_dx > 0 && cmp(_currentArea->hScanEnd(), _screen.left() + _screen.width(), .01) >= 0) {
		startRightUp();
    } else if (_dy < 0 && cmp(_currentArea->vScanStart(), _screen.top(), .01) <= 0) {
		startTopLeft();
	} else if (_dx < 0 && cmp(_currentArea->hScanStart(), _screen.left(), .01) <= 0) {
		startLeftDown();
	} else if (_dy > 0 && cmp(_currentArea->vScanEnd(), _screen.top() + _screen.height(), .01) >= 0) {
		startBottomRight2();
    }

    if (!_currentArea) {
		_currentArea = new ScreenArea(_x, _x + _width, _y, _y + _height);
    } else {
        int dx = _width * _dx;
        int dy = _height * _dy;

		ScreenArea *result = new ScreenArea(_currentArea->hScanStart() + dx, _currentArea->hScanEnd() + dx,
										_currentArea->vScanStart() + dy, _currentArea->vScanEnd() + dy);
		cleanCurrentArea();
		_currentArea = result;
    }

	// compensate for sum of all widgets != screen
	if (_dy < 0 && _screen.height() % _sideLeds != 0) {
		int screenMid = _screen.height() / 2;
		if (_currentArea->vScanStart() < screenMid && _currentArea->vScanEnd() > screenMid) {
			int dy = _screen.height() % _sideLeds;
			ScreenArea *cf = _currentArea;
			ScreenArea *result = new ScreenArea(cf->hScanStart(), cf->hScanEnd(),
				cf->vScanStart() - dy, cf->vScanEnd() - dy);
			cleanCurrentArea();
			_currentArea = result;

			return new ScreenArea(_currentArea->hScanStart(), _currentArea->hScanEnd(),
				_currentArea->vScanStart(), _currentArea->vScanEnd() + dy);
		}
	}
	if (_dx < 0 && _screen.width() % _topLeds != 0) {
		int screenMid = _screen.width() / 2;
		if (_currentArea->hScanStart() < screenMid && _currentArea->hScanEnd() > screenMid) {
			int dx = _screen.width() % _topLeds;
			ScreenArea *cf = _currentArea;
			ScreenArea *result = new ScreenArea(cf->hScanStart() - dx, cf->hScanEnd() - dx,
				cf->vScanStart(), cf->vScanEnd());
			cleanCurrentArea();
			_currentArea = result;

			return new ScreenArea(_currentArea->hScanStart(), _currentArea->hScanEnd() + dx,
				_currentArea->vScanStart(), _currentArea->vScanEnd());
		}
	}
	if (_dy > 0 && _screen.height() % _sideLeds != 0) {
		int screenMid = _screen.height() / 2;
		if (_currentArea->vScanStart() < screenMid && _currentArea->vScanEnd() > screenMid) {
			int dy = _screen.height() % _sideLeds;
			ScreenArea *cf = _currentArea;
			ScreenArea *result = new ScreenArea(cf->hScanStart(), cf->hScanEnd(),
				cf->vScanStart() + dy, cf->vScanEnd() + dy);
			cleanCurrentArea();
			_currentArea = result;

			return new ScreenArea(_currentArea->hScanStart(), _currentArea->hScanEnd(),
				_currentArea->vScanStart() - dy, _currentArea->vScanEnd());
		}
	}
	if (_dx > 0 && _screen.width() % _bottomLeds != 0) {
		int screenMid = _screen.width() / 2;
		if (_currentArea->hScanStart() < screenMid && _currentArea->hScanEnd() > screenMid) {
			int dx = _screen.width() % _bottomLeds;
			ScreenArea *cf = _currentArea;
			ScreenArea *result = new ScreenArea(cf->hScanStart() + dx, cf->hScanEnd() + dx,
				cf->vScanStart(), cf->vScanEnd());
			cleanCurrentArea();
			_currentArea = result;

			return new ScreenArea(_currentArea->hScanStart() - dx, _currentArea->hScanEnd(),
				_currentArea->vScanStart(), _currentArea->vScanEnd());
		}

	}

	return new ScreenArea(*_currentArea);

}

int CustomDistributor::areaCountOnTopEdge() const
{
    return _topLeds;
}

int CustomDistributor::areaCountOnBottomEdge() const
{
	return _bottomLeds;
}

int CustomDistributor::areaCountOnSideEdge() const
{
	return _sideLeds;
}
