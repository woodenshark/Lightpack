/*
 * SelectDevicePage.cpp
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

#include "SelectDevicePage.hpp"
#include "ui_SelectDevicePage.h"
#include "Wizard.hpp"
#include "LedDeviceVirtual.hpp"
#include "LedDeviceDrgb.hpp"
#include "Settings.hpp"

SelectDevicePage::SelectDevicePage(bool isInitFromSettings, TransientSettings *ts, QWidget *parent):
	QWizardPage(parent),
	SettingsAwareTrait(isInitFromSettings, ts),
	ui(new Ui::SelectDevicePage)
{
	ui->setupUi(this);
}

SelectDevicePage::~SelectDevicePage()
{
	delete ui;
}

void SelectDevicePage::initializePage()
{
	const SupportedDevices::DeviceType deviceType = SettingsScope::Settings::getConnectedDevice();
	registerField(QStringLiteral("isAdalight"), ui->rbAdalight);
	if (deviceType == SupportedDevices::DeviceTypeAdalight)
		ui->rbAdalight->setChecked(true);
	registerField(QStringLiteral("isArdulight"), ui->rbArdulight);
	if (deviceType == SupportedDevices::DeviceTypeArdulight)
		ui->rbArdulight->setChecked(true);
	registerField(QStringLiteral("isVirtual"), ui->rbVirtual);
	if (deviceType == SupportedDevices::DeviceTypeVirtual)
		ui->rbVirtual->setChecked(true);
    registerField(QStringLiteral("isDrgb"), ui->rbDrgb);
	if (deviceType == SupportedDevices::DeviceTypeDrgb)
		ui->rbDrgb->setChecked(true);
    registerField(QStringLiteral("isDnrgb"), ui->rbDnrgb);
	if (deviceType == SupportedDevices::DeviceTypeDnrgb)
		ui->rbDnrgb->setChecked(true);
    registerField(QStringLiteral("isWarls"), ui->rbWarls);
	if (deviceType == SupportedDevices::DeviceTypeWarls)
		ui->rbWarls->setChecked(true);
}

void SelectDevicePage::cleanupPage()
{
	setField(QStringLiteral("isAdalight"), false);
	setField(QStringLiteral("isArdulight"), false);
	setField(QStringLiteral("isVirtual"), false);
    setField(QStringLiteral("isDrgb"), false);
    setField(QStringLiteral("isDnrgb"), false);
    setField(QStringLiteral("isWarls"), false);
}

bool SelectDevicePage::validatePage()
{
	if (field(QStringLiteral("isVirtual")).toBool()) {
		_transSettings->ledDevice.reset(new LedDeviceVirtual());
	}
	return true;
}

int SelectDevicePage::nextId() const
{
	if (ui->rbVirtual->isChecked())
		return Page_ChooseProfile;
    else if (ui->rbDrgb->isChecked() || ui->rbDnrgb->isChecked() || ui->rbWarls->isChecked())
        return Page_ConfigureUdpDevice;
	else
		return Page_ConfigureDevice;
}
