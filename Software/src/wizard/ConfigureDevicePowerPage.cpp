/*
 * ConfigureDevicePowerPage.cpp
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

#include "ConfigureDevicePowerPage.hpp"
#include "ui_ConfigureDevicePowerPage.h"
#include "Settings.hpp"
#include "LedDeviceDrgb.hpp"
#include "LedDeviceDnrgb.hpp"
#include "LedDeviceWarls.hpp"
#include "Wizard.hpp"

using namespace SettingsScope;

ConfigureDevicePowerPage::ConfigureDevicePowerPage(bool isInitFromSettings, TransientSettings *ts, QWidget *parent):
	QWizardPage(parent),
	SettingsAwareTrait(isInitFromSettings, ts),
	ui(new Ui::ConfigureDevicePowerPage)
{
	ui->setupUi(this);
}

void ConfigureDevicePowerPage::initializePage()
{
	SupportedDevices::DeviceType device = SupportedDevices::DefaultDeviceType;
	if (field("isDrgb").toBool())
		device = SupportedDevices::DeviceTypeDrgb;
	else if (field("isDnrgb").toBool())
		device = SupportedDevices::DeviceTypeDnrgb;
	else if (field("isWarls").toBool())
		device = SupportedDevices::DeviceTypeWarls;
	else if (field("isLightpack").toBool())
		device = SupportedDevices::DeviceTypeLightpack;
	else if (field("isAdalight").toBool())
		device = SupportedDevices::DeviceTypeAdalight;
	else if (field("isArdulight").toBool())
		device = SupportedDevices::DeviceTypeArdulight;
	else if (field("isVirtual").toBool())
		device = SupportedDevices::DeviceTypeVirtual;
	else if (field("isAlienFx").toBool())
		device = SupportedDevices::DeviceTypeAlienFx;

	ui->sbLedMilliAmps->setValue(Settings::getDeviceLedMilliAmps(device));
	ui->sbPowerSupplyAmps->setValue(Settings::getDevicePowerSupplyAmps(device));
}


bool ConfigureDevicePowerPage::validatePage()
{
	SupportedDevices::DeviceType device = SupportedDevices::DefaultDeviceType;
	if (field("isDrgb").toBool())
		device = SupportedDevices::DeviceTypeDrgb;
	else if (field("isDnrgb").toBool())
		device = SupportedDevices::DeviceTypeDnrgb;
	else if (field("isWarls").toBool())
		device = SupportedDevices::DeviceTypeWarls;
	else if (field("isLightpack").toBool())
		device = SupportedDevices::DeviceTypeLightpack;
	else if (field("isAdalight").toBool())
		device = SupportedDevices::DeviceTypeAdalight;
	else if (field("isArdulight").toBool())
		device = SupportedDevices::DeviceTypeArdulight;
	else if (field("isVirtual").toBool())
		device = SupportedDevices::DeviceTypeVirtual;
	else if (field("isAlienFx").toBool())
		device = SupportedDevices::DeviceTypeAlienFx;

	const int ledMilliAmps = ui->sbLedMilliAmps->value();
	const double powerSupplyAmps = ui->sbPowerSupplyAmps->value();

	Settings::setDeviceLedMilliAmps(device, ledMilliAmps);
	Settings::setDevicePowerSupplyAmps(device, powerSupplyAmps);

	_transSettings->ledDevice->setLedMilliAmps(ledMilliAmps);
	_transSettings->ledDevice->setPowerSupplyAmps(powerSupplyAmps);

	return true;
}

ConfigureDevicePowerPage::~ConfigureDevicePowerPage()
{
	delete ui;
}
