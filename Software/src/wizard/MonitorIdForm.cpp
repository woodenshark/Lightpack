/*
 * MonitorIdForm.cpp
 *
 *	Created on: 10/23/2013
 *		Project: Prismatik
 *
 *	Copyright (c) 2013 Tim
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

#include "MonitorIdForm.hpp"
#include "ui_MonitorIdForm.h"
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif

MonitorIdForm::MonitorIdForm(const QString& idStr, const QRect geometry, const QColor& fgColor, const QColor& bgColor, QWidget *parent) :
	QWidget(parent),
	_ui(new Ui::MonitorIdForm),
	_fgColor(fgColor)
{
	_ui->setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	QPalette palette = _ui->label->palette();
	palette.setColor(QPalette::Window, bgColor);
	palette.setBrush(QPalette::WindowText, QBrush(fgColor));
	_ui->label->setPalette(palette);
	move(geometry.topLeft());
	resize(geometry.width(), geometry.height());

	// limit text width to X percent of the window
	constexpr const double strWidthGeometryPercent = 0.5;
	const QRect strBox = _ui->label->fontMetrics().boundingRect(idStr);
	if (strBox.width() > geometry.width() * strWidthGeometryPercent) {
		QFont font = _ui->label->font();
		font.setPointSizeF(font.pointSizeF() * (geometry.width() * strWidthGeometryPercent / strBox.width()));
		_ui->label->setFont(font);
	}
	_ui->label->setText(idStr);
}

MonitorIdForm::~MonitorIdForm()
{
	delete _ui;
}

void MonitorIdForm::setActive(const bool active)
{
	QLinearGradient linearGrad(QPointF(0, 0), QPointF(_ui->label->size().width(), _ui->label->size().height()));
	QColor col(Qt::red);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
	col.setHsl((col.hslHue() + QRandomGenerator::global()->bounded(360)) % 360, col.hslSaturation(), col.lightness());
#endif
	constexpr const double gradStep = 0.05;
	for (qreal pos = 0.0; pos <= 1.0; pos += gradStep) {
		col.setHsl((col.hslHue() + static_cast<int>(360 * gradStep)) % 360, col.hslSaturation(), col.lightness());
		linearGrad.setColorAt(pos, col);
	}

	QPalette palette = _ui->label->palette();
	palette.setBrush(QPalette::WindowText, active ? QBrush(linearGrad) : QBrush(_fgColor));
	_ui->label->setPalette(palette);
}
