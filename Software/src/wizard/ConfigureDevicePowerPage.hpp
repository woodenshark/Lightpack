/*
 * ConfigureDevicePowerPage.hpp
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

#ifndef CONFIGUREDEVICEPOWERPAGE_HPP
#define CONFIGUREDEVICEPOWERPAGE_HPP

#include <QWizardPage>
#include "SettingsAwareTrait.hpp"

namespace Ui {
class ConfigureDevicePowerPage;
}

class ConfigureDevicePowerPage : public QWizardPage, SettingsAwareTrait
{
	Q_OBJECT

public:
	explicit ConfigureDevicePowerPage(bool isInitFromSettings, TransientSettings *ts, QWidget *parent = 0);
	~ConfigureDevicePowerPage();

protected:
	void initializePage();
	bool validatePage();

private:
	Ui::ConfigureDevicePowerPage*ui;
};

#endif // CONFIGUREDEVICEPOWERPAGE_HPP
