#include "LightpackCommandLineParserTest.hpp"
#include "LightpackCommandLineParser.hpp"
#include <QtTest/QtTest>

LightpackCommandLineParserTest::LightpackCommandLineParserTest(QObject *parent)
    : QObject(parent)
{
}

void LightpackCommandLineParserTest::initTestCase() {}
void LightpackCommandLineParserTest::init() {}
void LightpackCommandLineParserTest::cleanup() {}

void LightpackCommandLineParserTest::testCase_parseVersion()
{
    LightpackCommandLineParser parser;
    QString error;
    const QStringList arguments = QStringList() << "app.binary" << "--version";

    QVERIFY(parser.parse(&error, arguments));
    QVERIFY(parser.isSetVersion());

    const QStringList arguments2 = QStringList() << "app.binary" << "-v";
    QVERIFY(parser.parse(&error, arguments2));
    QVERIFY(parser.isSetVersion());
}

void LightpackCommandLineParserTest::testCase_parseHelp()
{
    LightpackCommandLineParser parser;
    QString error;
    const QStringList arguments = QStringList() << "app.binary" << "--help";

    QVERIFY(parser.parse(&error, arguments));
    QVERIFY(parser.isSetHelp());

    const QStringList arguments2 = QStringList() << "app.binary" << "-h";
    QVERIFY(parser.parse(&error, arguments2));
    QVERIFY(parser.isSetHelp());
}

void LightpackCommandLineParserTest::testCase_parseWizard()
{
    LightpackCommandLineParser parser;
    QString error;
    const QStringList arguments = QStringList() << "app.binary" << "--wizard";

    QVERIFY(parser.parse(&error, arguments));
    QVERIFY(parser.isSetWizard());
}

void LightpackCommandLineParserTest::testCase_parseBacklightOff()
{
    LightpackCommandLineParser parser;
    QString error;
    const QStringList arguments = QStringList() << "app.binary" << "--off";

    QVERIFY(parser.parse(&error, arguments));
    QVERIFY(parser.isSetBacklightOff());
}

void LightpackCommandLineParserTest::testCase_parseBacklightOn()
{
    LightpackCommandLineParser parser;
    QString error;
    const QStringList arguments = QStringList() << "app.binary" << "--on";

    QVERIFY(parser.parse(&error, arguments));
    QVERIFY(parser.isSetBacklightOn());
}

void LightpackCommandLineParserTest::testCase_parseBacklightOnAndOff()
{
    LightpackCommandLineParser parser;
    QString error;
    const QStringList arguments = QStringList() << "app.binary" << "--on" << "--off";

    QVERIFY(!parser.parse(&error, arguments));
    QVERIFY(parser.isSetBacklightOn());
    QVERIFY(parser.isSetBacklightOff());
}

void LightpackCommandLineParserTest::testCase_parseDebuglevel()
{
    QString error;
    const QString levelNames[] = {
        QString("high"), QString("mid"), QString("low"), QString("zero")
    };
    const Debug::DebugLevels levelValues[] = {
        Debug::HighLevel, Debug::MidLevel, Debug::LowLevel, Debug::ZeroLevel
    };

    QCOMPARE(sizeof(levelNames)/sizeof(levelNames[0]), sizeof(levelValues)/sizeof(levelValues[0]));
    for (size_t i = 0; i < sizeof(levelNames)/sizeof(levelNames[0]); ++i)
    {
        LightpackCommandLineParser parser;
        QStringList arguments = QStringList() << "app.binary" << QString("--debug=") + levelNames[i];
        QVERIFY(parser.parse(&error, arguments));
        QVERIFY(parser.isSetDebuglevel());
        QCOMPARE(parser.debugLevel(), levelValues[i]);
    }

    for (size_t i = 0; i < sizeof(levelNames)/sizeof(levelNames[0]); ++i)
    {
        LightpackCommandLineParser parser;
        QStringList arguments = QStringList() << "app.binary" << QString("--debug-") + levelNames[i];
        QVERIFY(parser.parse(&error, arguments));
        QVERIFY(parser.isSetDebuglevel());
        QCOMPARE(parser.debugLevel(), levelValues[i]);
    }
}
