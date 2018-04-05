/*
 * LightpackCommandLineParser.hpp
 *
 *	Created on: 08.08.2014
 *		Project: Lightpack
 *
 *	Lightpack is very simple implementation of the backlight for a laptop
 *
 *	Copyright (c) 2014 Woodenshark LLC
 *
 *	Lightpack is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	Lightpack is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LIGHTPACKCOMMANDLINEPARSER_H
#define LIGHTPACKCOMMANDLINEPARSER_H

#include "debug.h"
#include <QCoreApplication>
#include <QCommandLineParser>

class LightpackCommandLineParser {
public:
	LightpackCommandLineParser();

	bool isSetVersion() const;
	bool isSetHelp() const;
	bool isSetNoGUI() const;
	bool isSetWizard() const;
	bool isSetBacklightOff() const;
	bool isSetBacklightOn() const;
	bool isSetDebuglevel() const;
	bool isSetProfile() const;
	bool isSetStartAfterUpdate() const; //TODO: remove for next release
	// Valid only if isSetDebuglevel() is true.
	Debug::DebugLevels debugLevel() const;

	// Valid only if isSetProfile() is true.
	QString profileName() const;

	QString helpText() const;
	QString errorText() const;

	bool parse(const QStringList& arguments = QCoreApplication::arguments());

private:
	static Debug::DebugLevels parseDebugLevel(const QString& levelValue);

	QCommandLineParser m_parser;
	// Options
	// --nogui
	const QCommandLineOption m_noGUIOption;
	// --wizard
	const QCommandLineOption m_wizardOption;
	// --off
	const QCommandLineOption m_backlightOffOption;
	// --on
	const QCommandLineOption m_backlightOnOption;
	// --debug=[high | mid | low | zero]
	const QCommandLineOption m_debugLevelOption;
	// --startafterupdate
	QCommandLineOption m_startAfterUpdateOption; //TODO: remove for next release

	// Keep this for a while for backward compatibility
	// TODO: remove those options.
	const QCommandLineOption m_debugLevelHighOption;
	const QCommandLineOption m_debugLevelMidOption;
	const QCommandLineOption m_debugLevelLowOption;
	const QCommandLineOption m_debugLevelZeroOption;

	const QCommandLineOption m_versionOption;
	const QCommandLineOption m_helpOption;
	const QCommandLineOption m_optionSetProfile;

	// Values from command line.
	Debug::DebugLevels m_debugLevel;
	QString m_profileName;
};

#endif // LIGHTPACKCOMMANDLINEPARSER_H
