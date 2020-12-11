/*
 * GrabWidget.hpp
 *
 *	Created on: 29.01.2011
 *		Author: Mike Shatohin (brunql)
 *		Project: Lightpack
 *
 *	Lightpack is very simple implementation of the backlight for a laptop
 *
 *	Copyright (c) 2011 Mike Shatohin, mikeshatohin [at] gmail.com
 *
 *	Lightpack is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	Lightpack is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "GrabConfigWidget.hpp"
#include "types.h"
#include <functional>

namespace Ui {
	class GrabWidget;
}

enum GrabWidgetFeature : int {
	SyncSettings = 1 << 0,
	AllowCoefConfig = 1 << 1,
	AllowEnableConfig = 1 << 2,
	AllowColorCycle = 1 << 3,
	DimUntilInteractedWith = 1 << 4,
	AllowMove = 1 << 5,
	AllowResize = 1 << 6
};

class GrabWidget : public QWidget
{
	Q_OBJECT
public:
	GrabWidget(int id, int features, QList<GrabWidget*> *fellows = NULL, QWidget *parent = 0);
	virtual ~GrabWidget();

	void saveSizeAndPosition();

	int getId() const;
	double getCoefRed() const;
	double getCoefGreen() const;
	double getCoefBlue() const;
	WBAdjustment getCoefs() const;
	void setCoefRed(const double);
	void setCoefGreen(const double);
	void setCoefBlue(const double);
	void setCoefs(const WBAdjustment&);
	void setId(const int id);
	void setFellows(QList<GrabWidget*>* const fellows);
	bool isAreaEnabled() const;
	void setAreaEnabled(const bool);
	void fillBackgroundWhite();
	void fillBackgroundColored();

private:
	void fillBackground(int index);

signals:
	void resizeOrMoveStarted(int id);
	void resizeOrMoveCompleted(int id);
	void mouseRightButtonClicked(int selfId);
	void sizeAndPositionChanged(int w, int h, int x, int y);

public slots:
	void settingsProfileChanged();

private slots:
	void onIsAreaEnabled_Toggled(bool state);
	void onOpenConfigButton_Clicked();
	void onRedCoef_ValueChanged(double value);
	void onGreenCoef_ValueChanged(double value);
	void onBlueCoef_ValueChanged(double value);

private:
	virtual void closeEvent(QCloseEvent *event);
	void setCursorOnAll(Qt::CursorShape cursor);
	void checkAndSetCursors(QMouseEvent *pe);
	void setBackgroundColor(const QColor& color);
	void setTextColor(const QColor& color);
	QColor getBackgroundColor();
	QColor getTextColor();
	void setOpenConfigButtonBackground(const QColor &color);

	QRect resizeAccordingly(QMouseEvent *pe);
	bool snapEdgeToScreenOrClosestFellow(
		QRect& newRect,
		const QRect& screen,
		std::function<void(QRect&,int)> setter,
		std::function<int(const QRect&)> getter,
		std::function<int(const QRect&)> oppositeGetter);

private:
	enum {
		NOP,
		MOVE,
		RESIZE_HOR_RIGHT,
		RESIZE_HOR_LEFT,
		RESIZE_VER_UP,
		RESIZE_VER_DOWN,
		RESIZE_RIGHT_DOWN,
		RESIZE_RIGHT_UP,
		RESIZE_LEFT_DOWN,
		RESIZE_LEFT_UP
	} cmd;

	QPoint mousePressPosition;
	QPoint mousePressGlobalPosition;
	QSize mousePressDiffFromBorder;

	static const int MinimumWidth = 50;
	static const int MinimumHeight = 50;
	static const int BorderWidth = 10;
	static const int StickyCloserPixels = 10; // Sticky to screen when closer N pixels

	static const int ColorsCount = 12;

	static const QColor m_colors[ColorsCount][2]; // background and text colors
	int m_colorIndex; // index of color which using now

	int m_selfId; // ID of this object

	WBAdjustment m_coefs;

	Ui::GrabWidget *ui;

	GrabConfigWidget *m_configWidget;

	QColor m_textColor;
	QColor m_backgroundColor;
	QString m_selfIdString;
	QString m_widthHeight;

	int m_features;
	QList<GrabWidget*> *m_fellows;

protected:
	virtual void mousePressEvent(QMouseEvent *pe);
	virtual void mouseMoveEvent(QMouseEvent *pe);
	virtual void mouseReleaseEvent(QMouseEvent *pe);
	virtual void wheelEvent(QWheelEvent *);
	virtual void resizeEvent(QResizeEvent *);
	virtual void paintEvent(QPaintEvent *);
};

