#ifndef LIGHTPACKAPITEST_HPP
#define LIGHTPACKAPITEST_HPP

#include <QtNetwork>
// get
// getstatus - on off
// getstatusapi - busy idle
// getprofiles - list name profiles
// getprofile - current name profile

// commands
// lock - begin work with api (disable capture,backlight)
// unlock - end work with api (enable capture,backlight)
// setcolor:1-r,g,b;5-r,g,b;	numbering starts with 1
// setgamma:2.00 - set gamma for setcolor
// setsmooth:100 - set smooth in device
// setprofile:<name> - set profile
// setstatus:on - set status (on, off)

class SettingsWindowMockup;
class ApiServer;
class LightpackPluginInterface;

class LightpackApiTest : public QObject
{
	Q_OBJECT

public:
	LightpackApiTest();

private Q_SLOTS:
	void initTestCase();

	void init();
	void cleanup();

	void testCase_ApiVersion();

	void testCase_GetStatus();
	void testCase_GetStatusAPI();
	void testCase_GetProfiles();
	void testCase_GetProfile();

	void testCase_Lock();
	void testCase_Unlock();

	void testCase_SetColor();
	void testCase_SetColorValid();
	void testCase_SetColorValid_data();
	void testCase_SetColorValid2();
	void testCase_SetColorValid2_data();
	void testCase_SetColorInvalid();
	void testCase_SetColorInvalid_data();

	void testCase_SetGammaValid();
	void testCase_SetGammaValid_data();
	void testCase_SetGammaInvalid();
	void testCase_SetGammaInvalid_data();

	void testCase_SetBrightnessValid();
	void testCase_SetBrightnessValid_data();
	void testCase_SetBrightnessInvalid();
	void testCase_SetBrightnessInvalid_data();

	void testCase_SetSmoothValid();
	void testCase_SetSmoothValid_data();
	void testCase_SetSmoothInvalid();
	void testCase_SetSmoothInvalid_data();

	void testCase_SetProfile();
	void testCase_SetStatus();

	void testCase_ApiAuthorization();

private:
	QByteArray readResult(QTcpSocket * socket);
	void writeCommand(QTcpSocket * socket, const char * cmd);
	bool writeCommandWithCheck(QTcpSocket * socket, const QByteArray & command, const QByteArray & result);

	QString getProfilesResultString();
	void processEventsFromLittle();

	bool checkVersion(QTcpSocket * socket);
	bool lock(QTcpSocket * socket);
	bool unlock(QTcpSocket * socket);
	bool setGamma(QTcpSocket * socket, QString gammaStr);

private:
	ApiServer *m_apiServer;
	QThread *m_apiServerThread;
	LightpackPluginInterface *m_interfaceApi;
	SettingsWindowMockup *m_little;

	QTcpSocket * m_socket;
	bool m_sockReadLineOk;
};
#endif // LIGHTPACKAPITEST_HPP
