/*
 * MonitorsConfigurationPage.cpp
 *
 *	Created on: 15.2.2017
 *		Project: Prismatik
 *
 *	Copyright (c) 2017 Patrick Siegler
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

#include <QDesktopWidget>
#include <QRadioButton>
#include "GlobalColorCoefPage.hpp"
#include "ui_GlobalColorCoefPage.h"
#include "MonitorIdForm.hpp"
#include "AbstractLedDevice.hpp"
#include "Settings.hpp"
#include "debug.h"
#include "PrismatikMath.hpp"
#include "GrabWidget.hpp"

GlobalColorCoefPage::GlobalColorCoefPage(bool isInitFromSettings, TransientSettings *ts, QWidget *parent) :
	WizardPageUsingDevice(isInitFromSettings, ts, parent),
	_ui(new Ui::GlobalColorCoefPage)
{
	_ui->setupUi(this);
}

GlobalColorCoefPage::~GlobalColorCoefPage()
{
	delete _ui;
}

void GlobalColorCoefPage::initializePage()
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	_screenId = field(QStringLiteral("screenId")).toInt();

	QList<QScreen*> screenList = QGuiApplication::screens();
	int i = 0;
	foreach(QScreen* screen, screenList) {
		QRect geom = screen->geometry();
		MonitorIdForm *monitorIdForm = new MonitorIdForm();

		monitorIdForm->setWindowFlags(Qt::FramelessWindowHint);
		monitorIdForm->setStyleSheet(QStringLiteral("background: #fff"));

		monitorIdForm->move(geom.topLeft());
		monitorIdForm->resize(geom.width(), geom.height());

		monitorIdForm->setId(i+1);

		monitorIdForm->show();

		_monitorForms.append(monitorIdForm);
		i++;
	}
	this->activateWindow();

	// RESETS WBA
	resetDeviceSettings();

	if (_isInitFromSettings) {
		_grabAreas.reserve(_transSettings->ledCount);
		for (int i = 0; i < _transSettings->ledCount; i++)
			addGrabArea(i);
		const GrabWidget* const firstWidget = _grabAreas.first();
		_ui->sbRed->setValue(firstWidget->getCoefRed() * _ui->sbRed->maximum());
		_ui->sbGreen->setValue(firstWidget->getCoefGreen() * _ui->sbGreen->maximum());
		_ui->sbBlue->setValue(firstWidget->getCoefBlue() * _ui->sbBlue->maximum());
	}
	connect(_ui->sbRed, SIGNAL(valueChanged(int)), this, SLOT(onCoefValueChanged(int)));
	connect(_ui->sbGreen, SIGNAL(valueChanged(int)), this, SLOT(onCoefValueChanged(int)));
	connect(_ui->sbBlue, SIGNAL(valueChanged(int)), this, SLOT(onCoefValueChanged(int)));
	connect(_ui->hsColorTemperature, &QSlider::valueChanged, this, &GlobalColorCoefPage::onColorTemperatureValueChanged);

	turnLightsOn(qRgb(255, 255, 255));

	// prevent some firmwares/devices from timing out during this phase
	// also avoids tracking widget coef signals
	connect(&_keepAlive, &QTimer::timeout, this, &GlobalColorCoefPage::updateDevice);
	_keepAlive.start(200);
}

bool GlobalColorCoefPage::validatePage()
{
	using namespace SettingsScope;
	const QString deviceName = device()->name();
	SupportedDevices::DeviceType devType;
	if (deviceName.compare(QStringLiteral("lightpack"), Qt::CaseInsensitive) == 0) {
		devType = SupportedDevices::DeviceTypeLightpack;
	}
	else if (deviceName.compare(QStringLiteral("adalight"), Qt::CaseInsensitive) == 0) {
		devType = SupportedDevices::DeviceTypeAdalight;
		Settings::setAdalightSerialPortName(field(QStringLiteral("serialPort")).toString());
		Settings::setAdalightSerialPortBaudRate(field(QStringLiteral("baudRate")).toString());
		Settings::setColorSequence(devType, field(QStringLiteral("colorFormat")).toString());

	}
	else if (deviceName.compare(QStringLiteral("ardulight"), Qt::CaseInsensitive) == 0) {
		devType = SupportedDevices::DeviceTypeArdulight;
		Settings::setArdulightSerialPortName(field(QStringLiteral("serialPort")).toString());
		Settings::setArdulightSerialPortBaudRate(field(QStringLiteral("baudRate")).toString());
		Settings::setColorSequence(devType, field(QStringLiteral("colorFormat")).toString());

	}
	else if (deviceName.compare(QStringLiteral("drgb"), Qt::CaseInsensitive) == 0) {
		devType = SupportedDevices::DeviceTypeDrgb;
		Settings::setDrgbAddress(field(QStringLiteral("address")).toString());
		Settings::setDrgbPort(field(QStringLiteral("port")).toString());
		Settings::setDrgbTimeout(field(QStringLiteral("timeout")).toInt());
	}
	else if (deviceName.compare(QStringLiteral("dnrgb"), Qt::CaseInsensitive) == 0) {
		devType = SupportedDevices::DeviceTypeDnrgb;
		Settings::setDnrgbAddress(field(QStringLiteral("address")).toString());
		Settings::setDnrgbPort(field(QStringLiteral("port")).toString());
		Settings::setDnrgbTimeout(field(QStringLiteral("timeout")).toInt());
	}
	else if (deviceName.compare(QStringLiteral("warls"), Qt::CaseInsensitive) == 0) {
		devType = SupportedDevices::DeviceTypeWarls;
		Settings::setWarlsAddress(field(QStringLiteral("address")).toString());
		Settings::setWarlsPort(field(QStringLiteral("port")).toString());
		Settings::setWarlsTimeout(field(QStringLiteral("timeout")).toInt());
	}
	else {
		devType = SupportedDevices::DeviceTypeVirtual;
	}
	Settings::setConnectedDevice(devType);
	Settings::setNumberOfLeds(devType, _transSettings->zonePositions.size());

	for (const int id : _transSettings->zonePositions.keys()) {
		Settings::setLedPosition(id, _transSettings->zonePositions[id]);
		Settings::setLedSize(id, _transSettings->zoneSizes[id]);
		Settings::setLedEnabled(id, _transSettings->zoneEnabled[id]);
		Settings::setLedCoefRed(id, _grabAreas[id]->getCoefRed());
		Settings::setLedCoefGreen(id, _grabAreas[id]->getCoefGreen());
		Settings::setLedCoefBlue(id, _grabAreas[id]->getCoefBlue());
	}

	cleanupMonitors();
	cleanupGrabAreas();
	return true;
}

void GlobalColorCoefPage::cleanupPage()
{
	cleanupMonitors();
	cleanupGrabAreas();
}

void GlobalColorCoefPage::updateDevice()
{
	QList<WBAdjustment> wblist;
	wblist.reserve(_grabAreas.size());

	for (const GrabWidget * const widget : _grabAreas)
		wblist.append(widget->getCoefs());

	device()->updateWBAdjustments(wblist, true);
}

void GlobalColorCoefPage::onCoefValueChanged(int value)
{
	Q_UNUSED(value);

	WBAdjustment wba;
	wba.red = _ui->sbRed->value() / (double)_ui->sbRed->maximum();
	wba.green = _ui->sbGreen->value() / (double)_ui->sbGreen->maximum();
	wba.blue = _ui->sbBlue->value() / (double)_ui->sbBlue->maximum();

	for (GrabWidget* const widget : _grabAreas)
		widget->setCoefs(wba);
}

void GlobalColorCoefPage::onColorTemperatureValueChanged(int value)
{
	const StructRgb whitePoint = PrismatikMath::whitePoint(value);
	_ui->sbRed->setValue(whitePoint.r / 2.55);
	_ui->sbGreen->setValue(whitePoint.g / 2.55);
	_ui->sbBlue->setValue(whitePoint.b / 2.55);
}

void GlobalColorCoefPage::cleanupMonitors()
{
	foreach (MonitorIdForm *monitorIdForm, _monitorForms)
		delete monitorIdForm;
	_monitorForms.clear();
}

void GlobalColorCoefPage::addGrabArea(const int id)
{
	GrabWidget* const zone = new GrabWidget(id, AllowCoefConfig, &_grabAreas);

	zone->move(_transSettings->zonePositions[id]);
	zone->resize(_transSettings->zoneSizes[id]);
	zone->setCoefRed(SettingsScope::Settings::getLedCoefRed(id));
	zone->setCoefGreen(SettingsScope::Settings::getLedCoefGreen(id));
	zone->setCoefBlue(SettingsScope::Settings::getLedCoefBlue(id));
	zone->setAreaEanled(_transSettings->zoneEnabled[id]);
	zone->fillBackgroundWhite();
	zone->show();
	_grabAreas.append(zone);
}

void GlobalColorCoefPage::cleanupGrabAreas()
{
	for (int i = 0; i < _grabAreas.size(); i++)
		delete _grabAreas[i];
	_grabAreas.clear();
}
