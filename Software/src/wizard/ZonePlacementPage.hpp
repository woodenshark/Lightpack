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

#include <QTimer>
#include "WizardPageUsingDevice.hpp"
#include "SettingsAwareTrait.hpp"

namespace Ui {
class ZonePlacementPage;
}

class ZoneWidget;
class AbstractLedDevice;
class AreaDistributor;
class GrabWidget;
class QScreen;
class MonitorIdForm;

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
	void onNumberOfLeds_timeout();
	void onTopLeds_valueChanged(int arg1);
	void onSideLeds_valueChanged(int arg1);
	void onMonitor_currentIndexChanged(int idx);
	void onClearDisplay_clicked();

private:
	void addGrabArea(QList<GrabWidget*>& list, int id, const QRect &rect, const bool enabled = true);
	void removeLastGrabArea();
	void cleanupGrabAreas(int idx = -1);
	void cleanupMonitors();
	void distributeAreas(AreaDistributor *distributor, bool invertIds = false, int idOffset = 0);
	void resetNewAreaRect();
	bool checkZoneIssues();
	QRect screenRect() const;

	Ui::ZonePlacementPage *_ui;
	QRect _newAreaRect;
	QList<MonitorIdForm*> _monitorForms;

	struct MonitorSettings {
		int topLeds{ 0 };
		int sideLeds{ 0 };

		int offset{ 0 };
		int standWidth{ 0 };
		int thickness{ 0 };

		double topMargin{ 0.0 };
		double sideMargin{ 0.0 };
		double bottomMargin{ 0.0 };

		bool skipCorners{ false };
		bool invertOrder{ false };

		QList<GrabWidget*> grabAreas;
		const QScreen* screen;

		int startingLed{ 0 };
	};
	void saveMonitorSettings(MonitorSettings& settings);

	QMap<int, MonitorSettings> _screens;
	QList<GrabWidget*> _zonePool;
	QTimer _ledNumberUpdate;
};

#endif // ZONECONFIGURATION_HPP
