/*
 * ConfigureDevicePage.cpp
 *
 *	Created on: 11/1/2013
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

#include "ConfigureDevicePage.hpp"
#include "ui_ConfigureDevicePage.h"
#include "Settings.hpp"
#include "LedDeviceArdulight.hpp"
#include "LedDeviceAdalight.hpp"
#include "LedDeviceVirtual.hpp"
#include "Wizard.hpp"

using namespace SettingsScope;

ConfigureDevicePage::ConfigureDevicePage(bool isInitFromSettings, TransientSettings *ts, QWidget *parent):
	QWizardPage(parent),
	SettingsAwareTrait(isInitFromSettings, ts),
	ui(new Ui::ConfigureDevicePage)
{
	ui->setupUi(this);
}

void ConfigureDevicePage::initializePage()
{
	ui->cbBaudRate->clear();
	// NOTE: This line emit's signal currentIndex_Changed()
	ui->cbBaudRate->addItems(Settings::getSupportedSerialPortBaudRates());
	int currentBaudRate = 0;
	QString currentSerialPort;
	QString currentColorSequence;
	if (field(QStringLiteral("isAdalight")).toBool()) {
		currentBaudRate = Settings::getAdalightSerialPortBaudRate();
		currentSerialPort = Settings::getAdalightSerialPortName();
		currentColorSequence = Settings::getColorSequence(SupportedDevices::DeviceTypeAdalight);
	}
	else if (field(QStringLiteral("isArdulight")).toBool()) {
		currentBaudRate = Settings::getArdulightSerialPortBaudRate();
		currentSerialPort = Settings::getArdulightSerialPortName();
		currentColorSequence = Settings::getColorSequence(SupportedDevices::DeviceTypeArdulight);
	}
	else
		currentSerialPort = QStringLiteral(SERIAL_PORT_DEFAULT);

	if (currentBaudRate > 0)
		ui->cbBaudRate->setCurrentText(QString::number(currentBaudRate));
	if (!currentSerialPort.isEmpty())
		ui->leSerialPortName->setText(currentSerialPort);
	if (!currentColorSequence.isEmpty())
		ui->cbColorFormat->setCurrentText(currentColorSequence);

	registerField(QStringLiteral("serialPort"), ui->leSerialPortName);
	registerField(QStringLiteral("baudRate"), ui->cbBaudRate, "currentText");
	registerField(QStringLiteral("colorFormat"), ui->cbColorFormat, "currentText");

}

void ConfigureDevicePage::cleanupPage()
{
	setField(QStringLiteral("serialPort"), "");
	setField(QStringLiteral("baudRate"), 0);
	setField(QStringLiteral("colorFormat"), "RGB");
}

bool ConfigureDevicePage::validatePage()
{
	QString portName = field(QStringLiteral("serialPort")).toString();
	int baudRate = field(QStringLiteral("baudRate")).toInt();
	if (field(QStringLiteral("isAdalight")).toBool()) {
		_transSettings->ledDevice.reset(new LedDeviceAdalight(portName, baudRate));
	} else if (field(QStringLiteral("isArdulight")).toBool()){
		_transSettings->ledDevice.reset(new LedDeviceArdulight(portName, baudRate));
	} else {
		QMessageBox::information(NULL, QStringLiteral("Wrong device"), QStringLiteral("Try to restart the wizard"));
		qCritical() << "couldn't create LedDevice, unexpected state, device is not selected or device is not configurable";
		return false;
	}
	_transSettings->ledDevice->setColorSequence(field(QStringLiteral("colorFormat")).toString());
	_transSettings->ledDevice->open();

	return true;
}

ConfigureDevicePage::~ConfigureDevicePage()
{
	delete ui;
}

int ConfigureDevicePage::nextId() const
{
	// skip UDP device page
	return Page_ChooseProfile;
}
