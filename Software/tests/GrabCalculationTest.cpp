#include "GrabCalculationTest.hpp"

void GrabCalculationTest::testCase1()
{
	unsigned char buf[16];
	memset(buf, 0xfa, 16);
	QRgb result = Grab::Calculations::calculateAvgColor(buf, BufferFormatArgb, 16, QRect(0,0,4,1));
	QVERIFY2(result == QColor(0xfa, 0xfa, 0xfa).rgb(), qPrintable(QString("Failure. calculateAvgColor returned wrong errorcode %1").arg(result, 1, 16)));
}
