/*
 * CustomDistributor.hpp
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

#ifndef CUSTOMDISTRIBUTOR_HPP
#define CUSTOMDISTRIBUTOR_HPP
#include "AreaDistributor.hpp"

class CustomDistributor : public AreaDistributor
{
public:
	CustomDistributor(QRect screen, int top, int side, int bottom, double thickness = 0.15, double standWidth = 0.0) :
		AreaDistributor(screen, top + 2 * side + bottom),
		_topLeds(top), _sideLeds(side), _bottomLeds(bottom), _thickness(thickness), _standWidth(bottom % 2 == 0 ? standWidth : 0.0),
		_dx(0), _dy(0)
	{}
	virtual ~CustomDistributor();

	virtual ScreenArea * next();

protected:
	char _dx, _dy;
	int _width, _height;
	int _x, _y;
	int _topLeds, _sideLeds, _bottomLeds;
	double _thickness;
	double _standWidth;

	void startBottomRight();
	void startRightUp();
	void startTopLeft();
	void startLeftDown();
	void startBottomRight2();

	virtual int areaCountOnSideEdge() const;
	virtual int areaCountOnTopEdge() const;
	virtual int areaCountOnBottomEdge() const;

private:
	void cleanCurrentArea() {
		if (_currentArea)
			delete _currentArea;
		_currentArea = NULL;
	}
};

#endif // CUSTOMDISTRIBUTOR_HPP
