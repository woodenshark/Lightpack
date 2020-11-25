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

#include <QDesktopWidget>
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
	QString currentAddress;
	QString currentPort;
	int currentTimeout = 0;

	if (field(QStringLiteral("isDrgb")).toBool()) {
		currentAddress = Settings::getDrgbAddress();
		currentPort = Settings::getDrgbPort();
		currentTimeout = Settings::getDrgbTimeout();
	}
	else if (field(QStringLiteral("isDnrgb")).toBool()) {
		currentAddress = Settings::getDnrgbAddress();
		currentPort = Settings::getDnrgbPort();
		currentTimeout = Settings::getDnrgbTimeout();
	}
	else if (field(QStringLiteral("isWarls")).toBool()) {
		currentAddress = Settings::getWarlsAddress();
		currentPort = Settings::getWarlsPort();
		currentTimeout = Settings::getWarlsTimeout();
	}

	if (!currentAddress.isEmpty())
		ui->leAddress->setText(currentAddress);
	if (!currentPort.isEmpty())
		ui->lePort->setText(currentPort);
	if (currentTimeout != 0)
		ui->sbTimeout->setValue(currentTimeout);

	registerField(QStringLiteral("address"), ui->leAddress);
	registerField(QStringLiteral("port"), ui->lePort);
	registerField(QStringLiteral("timeout"), ui->sbTimeout);
}

void ConfigureUdpDevicePage::cleanupPage()
{
	setField(QStringLiteral("address"), "");
	setField(QStringLiteral("port"), "");
	setField(QStringLiteral("timeout"), "");
}

bool ConfigureUdpDevicePage::validatePage()
{
	QString address = field(QStringLiteral("address")).toString();
	QString port = field(QStringLiteral("port")).toString();
	int timeout = field(QStringLiteral("timeout")).toInt();

	if (field(QStringLiteral("isDrgb")).toBool()) {
		_transSettings->ledDevice.reset(new LedDeviceDrgb(address, port, timeout));
	} 
	else if (field(QStringLiteral("isDnrgb")).toBool()) {
		_transSettings->ledDevice.reset(new LedDeviceDnrgb(address, port, timeout));
	}
	else if (field(QStringLiteral("isWarls")).toBool()) {
		_transSettings->ledDevice.reset(new LedDeviceWarls(address, port, timeout));
	}
	else {
		QMessageBox::information(NULL, QStringLiteral("Wrong device"), QStringLiteral("Try to restart the wizard"));
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
