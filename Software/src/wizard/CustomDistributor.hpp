/*
 * CustomDistributor.hpp
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

#ifndef CUSTOMDISTRIBUTOR_HPP
#define CUSTOMDISTRIBUTOR_HPP
#include "AreaDistributor.hpp"

class CustomDistributor : public AreaDistributor
{
public:
    CustomDistributor(int screenId, size_t top, size_t side, size_t bottom):
		AreaDistributor(screenId, top + 2*side + bottom),
		_top(top), _side(side), _bottom(bottom),
        _dx(0), _dy(0)
    {}
    virtual ~CustomDistributor();

    virtual ScreenArea * next();

protected:
    char _dx, _dy;
    double _width, _height;
	size_t _top, _side, _bottom;

    virtual size_t areaCountOnSideEdge() const;
    virtual size_t areaCountOnTopEdge() const;
    virtual size_t areaCountOnBottomEdge() const;

private:
    void cleanCurrentArea() {
        delete _currentArea;
        _currentArea = NULL;
    }
};

#endif // CUSTOMDISTRIBUTOR_HPP
