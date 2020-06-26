/*
 * ZoneConfiguration.cpp
 *
 *	Created on: 10/25/2013
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

#include "ZonePlacementPage.hpp"
#include "ui_ZonePlacementPage.h"
#include "QDesktopWidget"
#include "AbstractLedDevice.hpp"
#include "Settings.hpp"
#include "CustomDistributor.hpp"
#include "GrabWidget.hpp"
#include "LedDeviceLightpack.hpp"

ZonePlacementPage::ZonePlacementPage(bool isInitFromSettings, TransientSettings *ts, QWidget *parent):
	WizardPageUsingDevice(isInitFromSettings, ts, parent),
	_ui(new Ui::ZonePlacementPage)
{
	_ui->setupUi(this);

	QRect screen = QGuiApplication::screens().value(_screenId, QGuiApplication::primaryScreen())->geometry();

	_x0 = screen.left() + 150;
	_y0 = screen.top() + 150;

	resetNewAreaRect();
}

ZonePlacementPage::~ZonePlacementPage()
{
	delete _ui;
}

void ZonePlacementPage::initializePage()
{
	using namespace SettingsScope;

	_screenId = field("screenId").toInt();
	registerField("numberOfLeds", _ui->sbNumberOfLeds);

	device()->setSmoothSlowdown(70);

	_ui->sbNumberOfLeds->setMaximum(device()->maxLedsCount());

	if (_isInitFromSettings) {
		int ledCount = Settings::getNumberOfLeds(Settings::getConnectedDevice());
		_ui->sbNumberOfLeds->setValue(ledCount);
		_transSettings->ledCount = ledCount;

		for (int i = 0; i < ledCount; i++) {
			QPoint topLeft = Settings::getLedPosition(i);
			QSize size = Settings::getLedSize(i);
			QRect r(topLeft, size);
			addGrabArea(i, r);
		}
	} else {
		_ui->sbNumberOfLeds->setValue(device()->defaultLedsCount());
		_transSettings->ledCount = device()->defaultLedsCount();
		on_pbAndromeda_clicked();
	}
	connect(_ui->sbNumberOfLeds, SIGNAL(valueChanged(int)), this, SLOT(on_numberOfLeds_valueChanged(int)));

	resetDeviceSettings();
	turnLightsOff();
}

void ZonePlacementPage::cleanupPage()
{
	cleanupGrabAreas();
	_ui->sbNumberOfLeds->disconnect();
}

void ZonePlacementPage::cleanupGrabAreas()
{
	for(int i = 0; i < _grabAreas.size(); i++) {
		delete _grabAreas[i];
	}
	_grabAreas.clear();
}

bool ZonePlacementPage::validatePage()
{
	_transSettings->zonePositions.clear();
	_transSettings->zoneSizes.clear();
	for (int i = 0; i < _grabAreas.size(); i++) {
		_transSettings->zonePositions.insert(_grabAreas[i]->getId(), _grabAreas[i]->geometry().topLeft());
		_transSettings->zoneSizes.insert(_grabAreas[i]->getId(), _grabAreas[i]->geometry().size());
	}

	cleanupGrabAreas();
	return true;
}

void ZonePlacementPage::resetNewAreaRect()
{
	_newAreaRect.setX(_x0);
	_newAreaRect.setY(_y0);
	_newAreaRect.setWidth(100);
	_newAreaRect.setHeight(100);
}

void ZonePlacementPage::distributeAreas(AreaDistributor *distributor, bool invertIds, int idOffset) {

	cleanupGrabAreas();
	for(int i = 0; i < distributor->areaCount(); i++) {
		ScreenArea *sf = distributor->next();
		qDebug() << sf->hScanStart() << sf->vScanStart();

		QRect r(sf->hScanStart(),
				sf->vScanStart(),
				(sf->hScanEnd() - sf->hScanStart()),
				(sf->vScanEnd() - sf->vScanStart()));
		int id = ((invertIds ? distributor->areaCount() - (i + 1) : i) + idOffset) % distributor->areaCount();
		id = (id + distributor->areaCount()) % distributor->areaCount();
		addGrabArea(id, r);

		delete sf;
	}
	resetNewAreaRect();
}

void ZonePlacementPage::addGrabArea(int id, const QRect &r)
{
	GrabWidget *zone = new GrabWidget(id, DimUntilInteractedWith, &_grabAreas);

	zone->move(r.topLeft());
	zone->resize(r.size());
	connect(zone, SIGNAL(resizeOrMoveStarted(int)), this, SLOT(turnLightOn(int)));
	connect(zone, SIGNAL(resizeOrMoveCompleted(int)), this, SLOT(turnLightsOff()));
	zone->show();
	_grabAreas.append(zone);
}

void ZonePlacementPage::removeLastGrabArea()
{
	delete _grabAreas.last();
	_grabAreas.removeLast();
}

void ZonePlacementPage::on_pbAndromeda_clicked()
{
	QRect screen = QGuiApplication::screens().value(_screenId, QGuiApplication::primaryScreen())->geometry();
	const int bottomWidth = screen.width() * (1.0 - _ui->sbStandWidth->value() / 100.0);
	const int perimeter = screen.width() + screen.height() * 2 + bottomWidth;
	const int ledSize = perimeter / _ui->sbNumberOfLeds->value();

	const int bottomLeds = ((bottomWidth / ledSize) + 1) & ~1;//round up / down to next even number
	const int sideLeds = screen.height() / ledSize;
	const int topLeds = _ui->sbNumberOfLeds->value() - bottomLeds - sideLeds * 2;
	CustomDistributor *custom = new CustomDistributor(
		screen,
		topLeds,
		sideLeds,
		bottomLeds,
		_ui->sbThickness->value() / 100.0,
		_ui->sbStandWidth->value() / 100.0);

	distributeAreas(custom, _ui->cbInvertOrder->isChecked(), _ui->sbNumberingOffset->value());
	_ui->sbTopLeds->setValue(topLeds);
	_ui->sbSideLeds->setValue(sideLeds);
	_ui->sbBottomLeds->setValue(bottomLeds);
	delete custom;
}

void ZonePlacementPage::on_pbCassiopeia_clicked()
{
	QRect screen = QGuiApplication::screens().value(_screenId, QGuiApplication::primaryScreen())->geometry();
	const int perimeter = screen.width() + screen.height() * 2;
	const int ledSize = perimeter / _ui->sbNumberOfLeds->value();
	const int sideLeds = screen.height() / ledSize;
	const int topLeds = _ui->sbNumberOfLeds->value() - sideLeds * 2;
	CustomDistributor *custom = new CustomDistributor(
		screen,
		topLeds,
		sideLeds,
		0,
		_ui->sbThickness->value() / 100.0,
		_ui->sbStandWidth->value() / 100.0);

	distributeAreas(custom, _ui->cbInvertOrder->isChecked(), _ui->sbNumberingOffset->value());
	_ui->sbTopLeds->setValue(topLeds);
	_ui->sbSideLeds->setValue(sideLeds);
	_ui->sbBottomLeds->setValue(0);
	delete custom;
}

void ZonePlacementPage::on_pbPegasus_clicked()
{
	QRect screen = QGuiApplication::screens().value(_screenId, QGuiApplication::primaryScreen())->geometry();
	const int sideLeds = _ui->sbNumberOfLeds->value() / 2;
	CustomDistributor *custom = new CustomDistributor(
		screen,
		0,
		sideLeds,
		0,
		_ui->sbThickness->value() / 100.0,
		_ui->sbStandWidth->value() / 100.0);

	distributeAreas(custom, _ui->cbInvertOrder->isChecked(), _ui->sbNumberingOffset->value());
	_ui->sbTopLeds->setValue(0);
	_ui->sbSideLeds->setValue(sideLeds);
	_ui->sbBottomLeds->setValue(0);
	delete custom;
}


void ZonePlacementPage::on_pbApply_clicked()
{
	QRect screen = QGuiApplication::screens().value(_screenId, QGuiApplication::primaryScreen())->geometry();
	CustomDistributor *custom = new CustomDistributor(
		screen,
		_ui->sbTopLeds->value(),
		_ui->sbSideLeds->value(),
		_ui->sbBottomLeds->value(),
		_ui->sbThickness->value() / 100.0,
		_ui->sbStandWidth->value() / 100.0);

	distributeAreas(custom, _ui->cbInvertOrder->isChecked(), _ui->sbNumberingOffset->value());

	_ui->sbNumberOfLeds->setValue(_grabAreas.size());

	delete custom;
}


void ZonePlacementPage::on_numberOfLeds_valueChanged(int numOfLed)
{
	while (numOfLed < _grabAreas.size()) {
		removeLastGrabArea();
	}
	QRect screen = QGuiApplication::screens().value(_screenId, QGuiApplication::primaryScreen())->geometry();

	const int dx = 10;
	const int dy = 10;

	while (numOfLed > _grabAreas.size()) {
		addGrabArea(_grabAreas.size(), _newAreaRect);
		if (_newAreaRect.right() + dx < screen.right()) {
			_newAreaRect.moveTo(_newAreaRect.x() + dx, _newAreaRect.y());
		} else if (_newAreaRect.bottom() + dy < screen.bottom()) {
			_newAreaRect.moveTo(_x0, _newAreaRect.y() + dy);
		} else {
			_newAreaRect.moveTo(_x0,_y0);
		}
	}

	_transSettings->ledCount = numOfLed;
}
