/*
* DDGrabber.cpp
*
*  Created on: 21.12.2018
*     Project: Lightpack
*
*  Copyright (c) 2014 Patrick Siegler
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

#include "DDuplGrabber.hpp"

#ifdef DDUPL_GRAB_SUPPORT


DDuplGrabber::DDuplGrabber(QObject * parent, GrabberContext *context)
	: GrabberBase(parent, context) 
{
}

DDuplGrabber::~DDuplGrabber()
{
}


GrabResult DDuplGrabber::grabScreens()
{
	//_screensWithWidgets.
	return GrabResultError;
}

bool DDuplGrabber::reallocate(const QList< ScreenInfo > &grabScreens)
{
	freeScreens();

	for (const ScreenInfo& screen : grabScreens)
	{
		GrabbedScreen grabScreen;
		grabScreen.imgData = (unsigned char *)nullptr;
		grabScreen.imgFormat = BufferFormatArgb;
		grabScreen.screenInfo = screen;
		grabScreen.associatedData = nullptr;
		_screensWithWidgets.append(grabScreen);
	}
	return true;
}

QList< ScreenInfo > * DDuplGrabber::screensWithWidgets(QList< ScreenInfo > * result, const QList<GrabWidget *> &grabWidgets)
{
	result->clear();

	ScreenInfo screenInfo;
	screenInfo.rect = QRect(0, 0, 6000, 1080);
	screenInfo.handle = nullptr;

	result->append(screenInfo);
	return result;
}


void DDuplGrabber::freeScreens()
{
	for (GrabbedScreen& screen : _screensWithWidgets)
	{
		screen.associatedData = nullptr;

		if (screen.imgData != NULL) {
			free(screen.imgData);
			screen.imgData = NULL;
			screen.imgDataSize = 0;
		}
	}
}

#endif