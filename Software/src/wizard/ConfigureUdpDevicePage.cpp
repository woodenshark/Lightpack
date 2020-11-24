/*
 * ConfigureUdpDevicePage.cpp
 *
 *	Created on: 15/02/2020
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

#include <QMessageBox>

#include "ConfigureUdpDevicePage.hpp"
#include "ui_ConfigureUdpDevicePage.h"
#include "Settings.hpp"
#include "LedDeviceDrgb.hpp"
#include "LedDeviceDnrgb.hpp"
#include "LedDeviceWarls.hpp"
#include "Wizard.hpp"

using namespace SettingsScope;

ConfigureUdpDevicePage::ConfigureUdpDevicePage(bool isInitFromSettings, TransientSettings *ts, QWidget *parent):
	QWizardPage(parent),
	SettingsAwareTrait(isInitFromSettings, ts),
	ui(new Ui::ConfigureUdpDevicePage)
{
	ui->setupUi(this);
}

void ConfigureUdpDevicePage::initializePage()
{
	QString currentAddress = NULL;
	QString currentPort = NULL;
	int currentTimeout = 0;

	if (field("isDrgb").toBool()) {
		currentAddress = Settings::getDrgbAddress();
		currentPort = Settings::getDrgbPort();
		currentTimeout = Settings::getDrgbTimeout();
	}
	else if (field("isDnrgb").toBool()) {
		currentAddress = Settings::getDnrgbAddress();
		currentPort = Settings::getDnrgbPort();
		currentTimeout = Settings::getDnrgbTimeout();
	}
	else if (field("isWarls").toBool()) {
		currentAddress = Settings::getWarlsAddress();
		currentPort = Settings::getWarlsPort();
		currentTimeout = Settings::getWarlsTimeout();
	}

	if (currentAddress != NULL && currentAddress.isEmpty() == false)
		ui->leAddress->setText(currentAddress);
	if (currentPort != NULL && currentPort.isEmpty() == false)
		ui->lePort->setText(currentPort);
	if (currentTimeout != 0)
		ui->sbTimeout->setValue(currentTimeout);

	registerField("address", ui->leAddress);
	registerField("port", ui->lePort);
	registerField("timeout", ui->sbTimeout);
}

void ConfigureUdpDevicePage::cleanupPage()
{
	setField("address", "");
	setField("port", "");
	setField("timeout", "");
}

bool ConfigureUdpDevicePage::validatePage()
{
	QString address = field("address").toString();
	QString port = field("port").toString();
	int timeout = field("timeout").toInt();

	if (field("isDrgb").toBool()) {
		_transSettings->ledDevice.reset(new LedDeviceDrgb(address, port, timeout));
	}
	else if (field("isDnrgb").toBool()) {
		_transSettings->ledDevice.reset(new LedDeviceDnrgb(address, port, timeout));
	}
	else if (field("isWarls").toBool()) {
		_transSettings->ledDevice.reset(new LedDeviceWarls(address, port, timeout));
	}
	else {
		QMessageBox::information(NULL, "Wrong device", "Try to restart the wizard");
		qCritical() << "couldn't create LedDevice, unexpected state, device is not selected or device is not configurable";
		return false;
	}
	_transSettings->ledDevice->open();

	return true;
}

int ConfigureUdpDevicePage::nextId() const {
	if (QGuiApplication::screens().count() == 1) {
		return reinterpret_cast<Wizard *>(wizard())->skipMonitorConfigurationPage();
	} else {
		return Page_MonitorConfiguration;
	}
}

ConfigureUdpDevicePage::~ConfigureUdpDevicePage()
{
	delete ui;
}
