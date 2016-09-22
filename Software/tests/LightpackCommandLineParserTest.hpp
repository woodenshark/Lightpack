/*
 * LightpackCommandLineParserTest.hpp
 *
 *  Created on: 08.08.2014
 *     Project: Lightpack
 *
 *  Lightpack is very simple implementation of the backlight for a laptop
 *
 *  Copyright (c) 2014 Woodenshark LLC
 *
 *  Lightpack is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Lightpack is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LIGHTPACKCOMMANDLINEPARSERTEST_H
#define LIGHTPACKCOMMANDLINEPARSERTEST_H

#include <QtTest/QtTest>

class LightpackCommandLineParserTest : public QObject
{
    Q_OBJECT
public:
    explicit LightpackCommandLineParserTest(QObject *parent = 0);

private slots:
    void initTestCase();

    void init();
    void cleanup();

    void testCase_parseVersion();
    void testCase_parseHelp();
    void testCase_parseWizard();
    void testCase_parseBacklightOff();
    void testCase_parseBacklightOn();
    void testCase_parseBacklightOnAndOff();
    void testCase_parseDebuglevel();
};

#endif // LIGHTPACKCOMMANDLINEPARSERTEST_H
