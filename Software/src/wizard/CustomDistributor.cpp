/*
 * CustomDistributor.cpp
 *
 *	Created on: 08.06.2016
 *		Project: Prismatik
 *
 *	Copyright (c) 2015 Patrick Siegler
 *
 *	Lightpack is an open-source, USB content-driving ambient lighting
 *	hardware.
 *
 *	Prismatik is a free, open-source software: you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as published
 *	by the Free Software Foundation, either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	Prismatik and Lightpack files is distributed in the hope that it will be
 *	useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
 *	General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "CustomDistributor.hpp"
#include "PrismatikMath.hpp"

int roundDown(int n) {
	const int rounded = (n % 2 == 0) ? n : (n - 1);
	return rounded ? rounded : 2;
}


CustomDistributor::~CustomDistributor() {
	if (_currentArea)
		cleanCurrentArea();
}

void CustomDistributor::startBottomMiddleToRight() {
	_sizeBudget = 0;
	_ledCount = 0;
	if (_bottomLeds) {
		_dx = 1;
		_dy = 0;
		_ledCount = roundDown(_bottomLeds) / 2;
		const int bLeds = _bottomLeds + _skipCorners * 2;
		const int hLeds = _ledCount + _skipCorners;
		if (_standWidth == 0.0) { // without stand, use top side logic
			_width = _screen.width() / bLeds;
			_sizeBudget = std::floor((_screen.width() % bLeds) / 2.0);
		}
		else {
			// use left side led count as they'll have 1 more with an odd total number
			// this will give the smaller width that'll fit on both sides
			const int halfWidth = _screen.width() * (1.0 - _standWidth) / 2;
			_width = halfWidth / (bLeds - hLeds);
		}
		_height = _screen.height() * _thickness;
		_x = _screen.left() + _screen.width() - (hLeds * _width) - _sizeBudget;
		_y = _screen.top() + _screen.height() - _height;
	} else startBottomRightToTop();
}

void CustomDistributor::startBottomRightToTop() {
	_sizeBudget = 0;
	_ledCount = 0;
	if (_sideLeds) {
		cleanCurrentArea();
		_ledCount = _sideLeds;
		const int leds = _sideLeds + (_skipCorners * 2);
		_dx = 0;
		_dy = -1;
		_width = _screen.width() * _thickness;
		_height = _screen.height() / leds;
		_sizeBudget = _screen.height() % leds;
		_x = _screen.left() + _screen.width() - _width;
		_y = _screen.top() + _screen.height() - _height - _height * _skipCorners;
	} else startTopRightToLeft();
}

void CustomDistributor::startTopRightToLeft() {
	_sizeBudget = 0;
	_ledCount = 0;
	if (_topLeds) {
		cleanCurrentArea();
		_ledCount = _topLeds;
		const int leds = _topLeds + (_skipCorners * 2);
		_dx = -1;
		_dy = 0;
		_width = _screen.width() / leds;
		_height = _screen.height() * _thickness;
		_sizeBudget = _screen.width() % leds;
		_x = _screen.left() + _screen.width() - _width - _width * _skipCorners;
		_y = _screen.top();
	} else startTopLeftToBottom();
}

void CustomDistributor::startTopLeftToBottom() {
	_sizeBudget = 0;
	_ledCount = 0;
	if (_sideLeds) {
		cleanCurrentArea();
		_ledCount = _sideLeds;
		const int leds = _sideLeds + (_skipCorners * 2);
		_dx = 0;
		_dy = 1;
		_width = _screen.width() * _thickness;
		_height = _screen.height() / leds;
		_sizeBudget = _screen.height() % leds;
		_x = _screen.left();
		_y = _screen.top() + _height * _skipCorners;
	} else startBottomRightToMiddle();
}

void CustomDistributor::startBottomRightToMiddle() {
	_sizeBudget = 0;
	_ledCount = 0;
	if (_bottomLeds) {
		cleanCurrentArea();
		_dx = 1;
		_dy = 0;
		_ledCount = _bottomLeds - roundDown(_bottomLeds) / 2;
		const int bLeds = _bottomLeds + _skipCorners * 2;
		const int hLeds = _ledCount + _skipCorners;
		if (_standWidth == 0.0) {
			_width = _screen.width() / bLeds;
			_sizeBudget = std::ceil((_screen.width() % bLeds) / 2.0);
		}
		else {
			const int halfWidth = _screen.width() * (1.0 - _standWidth) / 2;
			_width = halfWidth / hLeds;
		}
		_height = _screen.height() * _thickness;
		_x = _screen.left() + _skipCorners * _width;
		_y = _screen.top() + _screen.height() - _height;
	}
}


ScreenArea * CustomDistributor::next() {
	if (_dx == 0 && _dy == 0) {
		startBottomMiddleToRight();
	} else if (_dx > 0 && _ledCount == 0) {
		startBottomRightToTop();
	} else if (_dy < 0 && _ledCount == 0) {
		startTopRightToLeft();
	} else if (_dx < 0 && _ledCount == 0) {
		startTopLeftToBottom();
	} else if (_dy > 0 && _ledCount == 0) {
		startBottomRightToMiddle();
	}

	int wAdjust = 0;
	int hAdjust = 0;

	if (_sizeBudget > 0) {
		if (_dx < 0 && _dy == 0) {//top left
			// distribute around the middle
			const int leds = _topLeds + _skipCorners * 2;
			const int startTopSide = (leds - ((leds - _sizeBudget) / 2)) * _width + _sizeBudget;
			if (_currentArea && _currentArea->hScanStart() <= startTopSide) {
				wAdjust = 1;
				_sizeBudget--;
			}
		}
		else if (_dx > 0 && _dy == 0) {//bottom
			wAdjust = 1;
			_sizeBudget--;
		}
		else if (_dx == 0 && _dy < 0) {//right up
			// distribute around the middle
			const int leds = _sideLeds + _skipCorners * 2;
			const int startRightSide = (leds - ((leds - _sizeBudget) / 2)) * _height + _sizeBudget;
			if (_currentArea && _currentArea->vScanStart() <= startRightSide) {
				hAdjust = 1;
				_sizeBudget--;
			}
		}
		else if (_dx == 0 && _dy > 0) {//left down
			// distribute around the middle
			const int leds = _sideLeds + _skipCorners * 2;
			const int startLeftSide = ((leds - _sizeBudget) / 2) * _height;
			if (_currentArea && _currentArea->vScanEnd() >= startLeftSide) {
				hAdjust = 1;
				_sizeBudget--;
			}
		}
	}

	if (!_currentArea) {
		_currentArea = new ScreenArea(_x, _x + _width + wAdjust, _y, _y + _height + hAdjust);
	} else {
		const int dx = (_width + wAdjust) * _dx;
		const int dy = (_height + hAdjust) * _dy;

		ScreenArea* result = new ScreenArea(
			(_dx > 0 && _dy == 0) ? _currentArea->hScanEnd() : _currentArea->hScanStart() + dx,
			(_dx < 0 && _dy == 0) ? _currentArea->hScanStart() : _currentArea->hScanEnd() + dx,
			(_dx == 0 && _dy > 0) ? _currentArea->vScanEnd() : _currentArea->vScanStart() + dy,
			(_dx == 0 && _dy < 0) ? _currentArea->vScanStart() : _currentArea->vScanEnd() + dy
		);

		cleanCurrentArea();
		_currentArea = result;
	}
	_ledCount--;
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
