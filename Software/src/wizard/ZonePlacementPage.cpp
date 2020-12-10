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
#include "AbstractLedDevice.hpp"
#include "Settings.hpp"
#include "CustomDistributor.hpp"
#include "GrabWidget.hpp"
#include "LedDeviceLightpack.hpp"
#include "MonitorIdForm.hpp"


ZonePlacementPage::ZonePlacementPage(bool isInitFromSettings, TransientSettings *ts, QWidget *parent):
	WizardPageUsingDevice(isInitFromSettings, ts, parent),
	_ui(new Ui::ZonePlacementPage)
{
	_ui->setupUi(this);
}

ZonePlacementPage::~ZonePlacementPage()
{
	delete _ui;
}

void ZonePlacementPage::initializePage()
{
	using namespace SettingsScope;

	int i = 0;
	for (const QScreen* const screen : QGuiApplication::screens()) {
		const QString& displayName = QStringLiteral("Display %1").arg(QString::number(i + 1));
		MonitorIdForm* const monitorIdForm = new MonitorIdForm(displayName, screen->geometry());
		monitorIdForm->show();
		_monitorForms.append(monitorIdForm);

		_ui->cbMonitorSelect->addItem(displayName, i);
		MonitorSettings settings;
		settings.screen = screen;
		saveMonitorSettings(settings);
		_screens.insert(i, settings);
		++i;
	}
	_monitorForms[_ui->cbMonitorSelect->currentIndex()]->setActive(true);
	resetNewAreaRect();

	device()->setSmoothSlowdown(70);

	connect(_ui->pbAndromeda, &QPushButton::clicked, this, &ZonePlacementPage::onAndromeda_clicked);
	connect(_ui->pbCassiopeia, &QPushButton::clicked, this, &ZonePlacementPage::onCassiopeia_clicked);
	connect(_ui->pbPegasus, &QPushButton::clicked, this, &ZonePlacementPage::onPegasus_clicked);
	connect(_ui->pbApply, &QPushButton::clicked, this, &ZonePlacementPage::onApply_clicked);
	connect(_ui->pbClearDisplay, &QPushButton::clicked, this, &ZonePlacementPage::onClearDisplay_clicked);
	connect(_ui->cbMonitorSelect, qOverload<int>(&QComboBox::currentIndexChanged), this, &ZonePlacementPage::onMonitor_currentIndexChanged);

	connect(_ui->sbNumberOfLeds, qOverload<int>(&QSpinBox::valueChanged), this, &ZonePlacementPage::onNumberOfLeds_valueChanged);
	connect(_ui->sbTopLeds, qOverload<int>(&QSpinBox::valueChanged), this, &ZonePlacementPage::onTopLeds_valueChanged);
	connect(_ui->sbSideLeds, qOverload<int>(&QSpinBox::valueChanged), this, &ZonePlacementPage::onSideLeds_valueChanged);

	_ui->sbNumberOfLeds->setMaximum(device()->maxLedsCount());
	_ui->sbStartingLed->setMaximum(device()->maxLedsCount() - 1);
	_ui->sbNumberOfLeds->blockSignals(true);

	int monitorLedCount = 0;
	if (_isInitFromSettings) {
		const int ledCount = Settings::getNumberOfLeds(Settings::getConnectedDevice());
		_transSettings->ledCount = ledCount;
		_zonePool.reserve(_transSettings->ledCount);
		QMap<int, MonitorSettings>::iterator it = _screens.begin();
		while (it != _screens.end()) {
			int startingLed = -1;
			for (int i = 0; i < ledCount; i++) {
				const QRect areaGeometry(Settings::getLedPosition(i), Settings::getLedSize(i));
				if (it.value().screen->geometry().contains(areaGeometry.center())) {
					addGrabArea(it.value().grabAreas, i, areaGeometry, Settings::isLedEnabled(i));
					startingLed = startingLed < 0 ? i : std::min(i, startingLed);
				}
			}
			it.value().startingLed = startingLed;
			++it;
		}
		monitorLedCount = _screens[_ui->cbMonitorSelect->currentIndex()].grabAreas.count();
		if (monitorLedCount == 0)
			monitorLedCount = device()->defaultLedsCount();
		_ui->sbStartingLed->setValue(_screens[_ui->cbMonitorSelect->currentIndex()].startingLed + 1);
	} else {
		monitorLedCount = device()->defaultLedsCount();
		_transSettings->ledCount = device()->defaultLedsCount();
		onAndromeda_clicked();
	}
	_ui->sbNumberOfLeds->setValue(monitorLedCount);
	_ui->sbNumberOfLeds->blockSignals(false);
	resetDeviceSettings();
	turnLightsOff();
	checkZoneIssues();
}

void ZonePlacementPage::cleanupPage()
{
	qDeleteAll(_zonePool);
	cleanupGrabAreas();
	cleanupMonitors();
}

void ZonePlacementPage::cleanupMonitors()
{
	qDeleteAll(_monitorForms);
	_monitorForms.clear();
}

void ZonePlacementPage::cleanupGrabAreas(int idx)
{
	QMap<int, MonitorSettings>::iterator it = _screens.begin();
	while (it != _screens.end()) {
		if (idx < 0 || idx == it.key()) {
			qDeleteAll(it.value().grabAreas);
			it.value().grabAreas.clear();
		}
		++it;
	}
}

void ZonePlacementPage::onClearDisplay_clicked()
{
	cleanupGrabAreas(_ui->cbMonitorSelect->currentIndex());
	checkZoneIssues();
}

bool ZonePlacementPage::checkZoneIssues()
{
	QMultiMap<int, std::nullptr_t> ids;
	QMap<int, MonitorSettings>::const_iterator it = _screens.cbegin();
	// get all IDs (even repeating ones)
	while (it != _screens.cend()) {
		for (const GrabWidget* const widget : it.value().grabAreas)
			ids.insert(widget->getId(), nullptr);
		++it;
	}

	// build gap string list and gather overlaping IDs (no repeats)
	QStringList gapStrs;
	int prevId = -1;
	QList<int> overlapIds;
	QMultiMap<int, std::nullptr_t>::const_iterator idIt = ids.constBegin();
	while (idIt != ids.constEnd()) {
		const int id = idIt.key();
		const int delta = id - prevId;
		if (delta == 2)
			gapStrs << QString::number(id);
		else if (delta > 2)
			gapStrs << QStringLiteral("%1-%2").arg(QString::number(id - delta + 2), QString::number(id));
		else if (delta == 0)
			overlapIds << id;
		prevId = id;
		++idIt;
	}

	// condense overlapping IDs into "X-Y" ranges when possible and build the string list
	QStringList overlapStrs;
	prevId = -1;
	int overlapStart = -1;
	auto addOverlap = [&overlapStrs](const int overlapStart, const int prevId) {
		if (overlapStart == prevId)
			overlapStrs << QString::number(prevId + 1);
		else
			overlapStrs << QStringLiteral("%1-%2").arg(QString::number(overlapStart + 1), QString::number(prevId + 1));
	};
	for (const int id : overlapIds) {
		if (prevId > -1) {
			if (id - prevId > 1) {
				addOverlap(overlapStart, prevId);
				overlapStart = id;
			}
		} else
			overlapStart = id;
		prevId = id;
	}
	if (prevId > -1 && overlapStart > -1)
		addOverlap(overlapStart, prevId);

	QStringList errors;
	if (!gapStrs.isEmpty())
		errors << QStringLiteral("Missing LEDs: %1").arg(gapStrs.join(QStringLiteral(", ")));
	if (!overlapStrs.isEmpty())
		errors << QStringLiteral("LEDs on multiple displays: %1").arg(overlapStrs.join(QStringLiteral(", ")));
	if (!errors.isEmpty())
		errors << QStringLiteral("Adjust the number of LEDs and/or the starting LED for concerned displays");
	_ui->labelZoneIssues->setText(errors.join('\n'));
	return errors.isEmpty();
}

bool ZonePlacementPage::validatePage()
{
	if (!checkZoneIssues())
		return false;

	_transSettings->zonePositions.clear();
	_transSettings->zoneSizes.clear();
	_transSettings->zoneEnabled.clear();
	QMap<int, MonitorSettings>::const_iterator it = _screens.constBegin();
	_transSettings->ledCount = 0;
	while (it != _screens.constEnd()) {
		for (const GrabWidget* const grabArea : it.value().grabAreas) {
			_transSettings->zonePositions.insert(grabArea->getId(), grabArea->geometry().topLeft());
			_transSettings->zoneSizes.insert(grabArea->getId(), grabArea->geometry().size());
			_transSettings->zoneEnabled.insert(grabArea->getId(), grabArea->isAreaEnabled());
			_transSettings->ledCount = std::max(grabArea->getId() + 1, _transSettings->ledCount);
		}
		++it;
	}

	cleanupPage();
	return true;
}

void ZonePlacementPage::resetNewAreaRect()
{
	const QRect screen = screenRect();
	_newAreaRect.setX(screen.left() + 150);
	_newAreaRect.setY(screen.top() + 150);
	_newAreaRect.setWidth(100);
	_newAreaRect.setHeight(100);
}

void ZonePlacementPage::distributeAreas(AreaDistributor *distributor, bool invertIds, int idOffset)
{
	QList<GrabWidget*>& grabAreas = _screens[_ui->cbMonitorSelect->currentIndex()].grabAreas;
	const int startId = _ui->sbStartingLed->value() - 1;

	for (GrabWidget* const widget : grabAreas)
		widget->hide();
	_zonePool.append(grabAreas);

	grabAreas.clear();
	grabAreas.reserve(distributor->areaCount());
	for(int i = 0; i < distributor->areaCount(); i++) {
		const ScreenArea * const sf = distributor->next();
		qDebug() << sf->hScanStart() << sf->vScanStart();

		const QRect r(sf->hScanStart(),
				sf->vScanStart(),
				(sf->hScanEnd() - sf->hScanStart()),
				(sf->vScanEnd() - sf->vScanStart()));
		int id = ((invertIds ? distributor->areaCount() - (i + 1) : i) + idOffset) % distributor->areaCount();
		id = (id + distributor->areaCount()) % distributor->areaCount();
		addGrabArea(grabAreas, id + startId, r);
		delete sf;
	}
	resetNewAreaRect();

	_transSettings->ledCount = 0;
	QMap<int, MonitorSettings>::const_iterator it = _screens.constBegin();
	for (; it != _screens.constEnd(); ++it) {
		for (const GrabWidget* const widget : it.value().grabAreas)
			_transSettings->ledCount = std::max(widget->getId() + 1, _transSettings->ledCount);
	}
	checkZoneIssues();
}

void ZonePlacementPage::addGrabArea(QList<GrabWidget*>& list, int id, const QRect &r, const bool enabled)
{
	const bool reuse = !_zonePool.isEmpty();
	GrabWidget * const zone = reuse ? _zonePool.takeLast() : new GrabWidget(id, DimUntilInteractedWith | AllowEnableConfig | AllowMove | AllowResize, &list);

	zone->move(r.topLeft());
	zone->resize(r.size());
	zone->setAreaEnabled(enabled);
	if (reuse) {
		zone->setId(id);
		zone->setFellows(&list);
	} else {
		connect(zone, &GrabWidget::resizeOrMoveStarted, this, &ZonePlacementPage::turnLightOn);
		connect(zone, &GrabWidget::resizeOrMoveCompleted, this, qOverload<>(&ZonePlacementPage::turnLightsOff));
	}
	zone->show();
	list.append(zone);
}

void ZonePlacementPage::removeLastGrabArea()
{
	QList<GrabWidget*>& grabAreas = _screens[_ui->cbMonitorSelect->currentIndex()].grabAreas;
	_zonePool.append(grabAreas.takeLast());
	_zonePool.last()->hide();
}

QRect ZonePlacementPage::screenRect() const
{
	QRect screen = QGuiApplication::screens().value(_ui->cbMonitorSelect->currentIndex(), QGuiApplication::primaryScreen())->geometry();
	const int topMargin = std::floor(screen.height() * _ui->doubleSpinBox_topMargin->value() / 100.0);
	const int sideMargin = std::floor(screen.width() * _ui->doubleSpinBox_sideMargin->value() / 100.0);
	const int bottomMargin = std::floor(screen.height() * _ui->doubleSpinBox_bottomMargin->value() / 100.0);
	screen.setTopLeft(QPoint(screen.left() + sideMargin, screen.top() + topMargin));
	screen.setBottomRight(QPoint(screen.right() - sideMargin, screen.bottom() - bottomMargin));
	return screen;
}

void ZonePlacementPage::onAndromeda_clicked()
{
	const QRect screen = screenRect();
	const int bottomWidth = screen.width() * (1.0 - _ui->sbStandWidth->value() / 100.0);
	const int perimeter = screen.width() + screen.height() * 2 + bottomWidth;
	const int ledSize = perimeter / _ui->sbNumberOfLeds->value();

	const int bottomLeds = ((bottomWidth / ledSize) + 1) & ~1;//round up / down to next even number
	const int sideLeds = screen.height() / ledSize;
	const int topLeds = _ui->sbNumberOfLeds->value() - bottomLeds - sideLeds * 2;
	CustomDistributor custom(
		screen,
		topLeds,
		sideLeds,
		bottomLeds,
		_ui->sbThickness->value() / 100.0,
		_ui->sbStandWidth->value() / 100.0,
		_ui->checkBox_skipCorners->isChecked()
	);

	distributeAreas(&custom, _ui->cbInvertOrder->isChecked(), _ui->sbNumberingOffset->value());
	_ui->sbTopLeds->setValue(topLeds);
	_ui->sbSideLeds->setValue(sideLeds);
	_ui->sbBottomLeds->setValue(bottomLeds);

	MonitorSettings& settings = _screens[_ui->cbMonitorSelect->currentIndex()];
	saveMonitorSettings(settings);
}

void ZonePlacementPage::onCassiopeia_clicked()
{
	const QRect screen = screenRect();
	const int perimeter = screen.width() + screen.height() * 2;
	const int ledSize = perimeter / _ui->sbNumberOfLeds->value();
	const int sideLeds = screen.height() / ledSize;
	const int topLeds = _ui->sbNumberOfLeds->value() - sideLeds * 2;
	CustomDistributor custom(
		screen,
		topLeds,
		sideLeds,
		0,
		_ui->sbThickness->value() / 100.0,
		_ui->sbStandWidth->value() / 100.0,
		_ui->checkBox_skipCorners->isChecked()
	);

	distributeAreas(&custom, _ui->cbInvertOrder->isChecked(), _ui->sbNumberingOffset->value());
	_ui->sbTopLeds->setValue(topLeds);
	_ui->sbSideLeds->setValue(sideLeds);
	_ui->sbBottomLeds->setValue(0);

	MonitorSettings& settings = _screens[_ui->cbMonitorSelect->currentIndex()];
	saveMonitorSettings(settings);
}

void ZonePlacementPage::onPegasus_clicked()
{
	const QRect screen = screenRect();
	const int sideLeds = _ui->sbNumberOfLeds->value() / 2;
	CustomDistributor custom(
		screen,
		0,
		sideLeds,
		0,
		_ui->sbThickness->value() / 100.0,
		_ui->sbStandWidth->value() / 100.0,
		_ui->checkBox_skipCorners->isChecked()
	);

	distributeAreas(&custom, _ui->cbInvertOrder->isChecked(), _ui->sbNumberingOffset->value());
	_ui->sbTopLeds->setValue(0);
	_ui->sbSideLeds->setValue(sideLeds);
	_ui->sbBottomLeds->setValue(0);

	MonitorSettings& settings = _screens[_ui->cbMonitorSelect->currentIndex()];
	saveMonitorSettings(settings);
}


void ZonePlacementPage::onApply_clicked()
{
	const QRect screen = screenRect();
	CustomDistributor custom(
		screen,
		_ui->sbTopLeds->value(),
		_ui->sbSideLeds->value(),
		_ui->sbBottomLeds->value(),
		_ui->sbThickness->value() / 100.0,
		_ui->sbStandWidth->value() / 100.0,
		_ui->checkBox_skipCorners->isChecked()
	);

	distributeAreas(&custom, _ui->cbInvertOrder->isChecked(), _ui->sbNumberingOffset->value());

	MonitorSettings& settings = _screens[_ui->cbMonitorSelect->currentIndex()];
	_ui->sbNumberOfLeds->setValue(settings.grabAreas.size());
	saveMonitorSettings(settings);
}


void ZonePlacementPage::onNumberOfLeds_valueChanged(int numOfLed)
{
	QList<GrabWidget*>& grabAreas = _screens[_ui->cbMonitorSelect->currentIndex()].grabAreas;
	while (numOfLed < grabAreas.size())
		removeLastGrabArea();

	int maxId = 0;
	for (const GrabWidget* const widget : grabAreas)
		maxId = std::max(maxId, widget->getId());

	const QRect screen = screenRect();

	const int dx = 10;
	const int dy = 10;
	grabAreas.reserve(numOfLed);
	while (numOfLed > grabAreas.size()) {
		addGrabArea(grabAreas, ++maxId, _newAreaRect);
		if (_newAreaRect.right() + dx < screen.right()) {
			_newAreaRect.moveTo(_newAreaRect.x() + dx, _newAreaRect.y());
		} else if (_newAreaRect.bottom() + dy < screen.bottom()) {
			_newAreaRect.moveTo(screen.left() + 150, _newAreaRect.y() + dy);
		} else {
			_newAreaRect.moveTo(screen.left() + 150, screen.top() + 150);
		}
	}
	checkZoneIssues();
}

void ZonePlacementPage::onTopLeds_valueChanged(int numOfLed)
{
	_ui->sbBottomLeds->setValue(_ui->sbNumberOfLeds->value() - numOfLed - _ui->sbSideLeds->value() * 2);
	_screens[_ui->cbMonitorSelect->currentIndex()].topLeds = numOfLed;
}

void ZonePlacementPage::onSideLeds_valueChanged(int numOfLed)
{
	_ui->sbBottomLeds->setValue(_ui->sbNumberOfLeds->value() - _ui->sbTopLeds->value() - numOfLed * 2);
	_screens[_ui->cbMonitorSelect->currentIndex()].sideLeds = numOfLed;
}

void ZonePlacementPage::onMonitor_currentIndexChanged(int idx)
{
	int i = 0;
	for (MonitorIdForm* const monitorId : _monitorForms)
		monitorId->setActive(idx == i++);

	resetNewAreaRect();

	const MonitorSettings& settings = _screens[idx];
	_ui->sbStartingLed->setValue(settings.startingLed + 1);
	_ui->sbNumberOfLeds->blockSignals(true);
	const int ledCount = settings.grabAreas.count();
	_ui->sbNumberOfLeds->setValue(ledCount > 0 ? ledCount : device()->defaultLedsCount());
	_ui->sbNumberOfLeds->blockSignals(false);
	_ui->sbTopLeds->setValue(settings.topLeds);
	_ui->sbSideLeds->setValue(settings.sideLeds);
	_ui->sbNumberingOffset->setValue(settings.offset);
	_ui->sbThickness->setValue(settings.thickness);
	_ui->sbStandWidth->setValue(settings.standWidth);
	_ui->doubleSpinBox_topMargin->setValue(settings.topMargin);
	_ui->doubleSpinBox_sideMargin->setValue(settings.sideMargin);
	_ui->doubleSpinBox_bottomMargin->setValue(settings.bottomMargin);
	_ui->cbInvertOrder->setChecked(settings.invertOrder);
	_ui->checkBox_skipCorners->setChecked(settings.skipCorners);
}

void ZonePlacementPage::saveMonitorSettings(MonitorSettings& settings)
{
	settings.startingLed = _ui->sbStartingLed->value() - 1;
	settings.topLeds = _ui->sbTopLeds->value();
	settings.sideLeds = _ui->sbSideLeds->value();
	settings.offset = _ui->sbNumberingOffset->value();
	settings.thickness = _ui->sbThickness->value();
	settings.standWidth = _ui->sbStandWidth->value();
	settings.topMargin = _ui->doubleSpinBox_topMargin->value();
	settings.sideMargin = _ui->doubleSpinBox_sideMargin->value();
	settings.bottomMargin = _ui->doubleSpinBox_bottomMargin->value();
	settings.invertOrder = _ui->cbInvertOrder->isChecked();
	settings.skipCorners = _ui->checkBox_skipCorners->isChecked();
}
