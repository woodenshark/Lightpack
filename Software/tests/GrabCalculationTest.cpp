#include "GrabCalculationTest.hpp"

void GrabCalculationTest::testCase1()
{
    QRgb result;
    unsigned char buf[16];
    memset(buf, 0xfa, 16);
    QVERIFY2(Grab::Calculations::calculateAvgColor(&result, buf, BufferFormatArgb, 16, QRect(0,0,4,1)) == QColor(0xfa, 0xfa, 0xfa).rgb(), qPrintable(QString("Failure. calculateAvgColor returned wrong errorcode %1").arg(result, 1, 16)));
}
