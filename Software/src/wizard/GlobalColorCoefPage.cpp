/*
 * MonitorsConfigurationPage.cpp
 *
 *  Created on: 15.2.2017
 *     Project: Prismatik
 *
 *  Copyright (c) 2017 Patrick Siegler
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

#include <QDesktopWidget>
#include <QRadioButton>
#include "GlobalColorCoefPage.hpp"
#include "ui_GlobalColorCoefPage.h"
#include "MonitorIdForm.hpp"
#include "AbstractLedDevice.hpp"
#include "Settings.hpp"
#include "debug.h"

GlobalColorCoefPage::GlobalColorCoefPage(bool isInitFromSettings, TransientSettings *ts, QWidget *parent) :
    WizardPageUsingDevice(isInitFromSettings, ts, parent),
	_ui(new Ui::GlobalColorCoefPage)
{
	_ui->setupUi(this);
	connect(_ui->sbRed, SIGNAL(valueChanged(int)), this, SLOT(onCoefValueChanged()));
	connect(_ui->sbGreen, SIGNAL(valueChanged(int)), this, SLOT(onCoefValueChanged()));
	connect(_ui->sbBlue, SIGNAL(valueChanged(int)), this, SLOT(onCoefValueChanged()));
}

GlobalColorCoefPage::~GlobalColorCoefPage()
{
    delete _ui;
}

void GlobalColorCoefPage::initializePage()
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO;

	_screenId = field("screenId").toInt();

	QRect s = QApplication::desktop()->screenGeometry(_screenId);

    int screenCount = QApplication::desktop()->screenCount();
    for (int i = 0; i < screenCount; i++) {
        QRect geom = QApplication::desktop()->screenGeometry(i);
        MonitorIdForm *monitorIdForm = new MonitorIdForm();

		monitorIdForm->setWindowFlags(Qt::FramelessWindowHint);
		monitorIdForm->setStyleSheet("background: #fff");

        monitorIdForm->move(geom.topLeft());
        monitorIdForm->resize(geom.width(), geom.height());

        monitorIdForm->setId(i+1);

        monitorIdForm->show();

        _monitorForms.append(monitorIdForm);
	}
	this->activateWindow();

	if (_isInitFromSettings) {
		_ui->sbRed->setValue(SettingsScope::Settings::getLedCoefRed(1) * 100);
		_ui->sbGreen->setValue(SettingsScope::Settings::getLedCoefGreen(1) * 100);
		_ui->sbBlue->setValue(SettingsScope::Settings::getLedCoefBlue(1) * 100);
	}

	resetDeviceSettings();
	onCoefValueChanged();
}

bool GlobalColorCoefPage::validatePage()
{
	using namespace SettingsScope;
	QString deviceName = device()->name();
	SupportedDevices::DeviceType devType;
	if (deviceName.compare("lightpack", Qt::CaseInsensitive) == 0) {
		devType = SupportedDevices::DeviceTypeLightpack;

	}
	else if (deviceName.compare("adalight", Qt::CaseInsensitive) == 0) {
		devType = SupportedDevices::DeviceTypeAdalight;
		Settings::setAdalightSerialPortName(field("serialPort").toString());
		Settings::setAdalightSerialPortBaudRate(field("baudRate").toString());
		Settings::setColorSequence(devType, field("colorFormat").toString());

	}
	else if (deviceName.compare("ardulight", Qt::CaseInsensitive) == 0) {
		devType = SupportedDevices::DeviceTypeArdulight;
		Settings::setArdulightSerialPortName(field("serialPort").toString());
		Settings::setArdulightSerialPortBaudRate(field("baudRate").toString());
		Settings::setColorSequence(devType, field("colorFormat").toString());

	}
	else {
		devType = SupportedDevices::DeviceTypeVirtual;
	}
	Settings::setConnectedDevice(devType);
	Settings::setNumberOfLeds(devType, _transSettings->zonePositions.size());

	for (int id : _transSettings->zonePositions.keys()) {
		Settings::setLedPosition(id, _transSettings->zonePositions[id]);
		Settings::setLedSize(id, _transSettings->zoneSizes[id]);
		Settings::setLedCoefRed(id, _ui->sbRed->value() / 100.0);
		Settings::setLedCoefGreen(id, _ui->sbGreen->value() / 100.0);
		Settings::setLedCoefBlue(id, _ui->sbBlue->value() / 100.0);
	}

    cleanupMonitors();
    return true;
}

void GlobalColorCoefPage::cleanupPage()
{
    cleanupMonitors();
}

void GlobalColorCoefPage::onCoefValueChanged()
{
	turnLightsOn(qRgb(255 * (_ui->sbRed->value() / 100.0), 255 * (_ui->sbGreen->value() / 100.0), 255 * (_ui->sbBlue->value() / 100.0)));
}

void GlobalColorCoefPage::cleanupMonitors()
{
    foreach ( MonitorIdForm *monitorIdForm, _monitorForms ) {
        delete monitorIdForm;
    }
    _monitorForms.clear();
}