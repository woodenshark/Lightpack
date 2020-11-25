/*
 * ZoneConfiguration.hpp
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

#ifndef ZONECONFIGURATION_HPP
#define ZONECONFIGURATION_HPP

#include "WizardPageUsingDevice.hpp"
#include "SettingsAwareTrait.hpp"

namespace Ui {
class ZonePlacementPage;
}

class ZoneWidget;
class AbstractLedDevice;
class AreaDistributor;
class GrabWidget;

class ZonePlacementPage : public WizardPageUsingDevice
{
	Q_OBJECT

public:
	explicit ZonePlacementPage(bool isInitFromSettings, TransientSettings *ts, QWidget *parent = 0);
	~ZonePlacementPage();

protected:
	void initializePage();
	void cleanupPage();
	bool validatePage();

private slots:
	void onAndromeda_clicked();
	void onCassiopeia_clicked();
	void onPegasus_clicked();
	void onApply_clicked();
	void onNumberOfLeds_valueChanged(int arg1);
	void onTopLeds_valueChanged(int arg1);
	void onSideLeds_valueChanged(int arg1);

private:
	void addGrabArea(int id, const QRect &rect);
	void removeLastGrabArea();
	void cleanupGrabAreas();
	void distributeAreas(AreaDistributor *distributor, bool invertIds = false, int idOffset = 0);
	void resetNewAreaRect();
	QRect screenRect() const;

	Ui::ZonePlacementPage *_ui;
	int _screenId;
	QList<GrabWidget*> _grabAreas;
	QRect _newAreaRect;
	int _x0;
	int _y0;

};

#endif // ZONECONFIGURATION_HPP
