#ifndef LightpackPluginInterface_HPP
#define LightpackPluginInterface_HPP

#include <QtGui>
#include <QObject>
#include "enums.hpp"

class Plugin;

class LightpackPluginInterface : public QObject
{
	Q_OBJECT
public:
	LightpackPluginInterface(QObject *parent = 0);
	~LightpackPluginInterface();

 public slots:
// Plugin section
	QString GetSessionKey(const QString& module);
	int CheckLock(const QString& sessionKey);
	bool Lock(const QString& sessionKey);

// need LOCK
	bool UnLock(const QString& sessionKey);
	bool SetStatus(const QString& sessionKey, int status);
	bool SetColors(const QString& sessionKey, int r, int g, int b);
	bool SetFrame(const QString& sessionKey, QList<QColor> colors);
	bool SetColor(const QString& sessionKey, int ind,int r, int g, int b);
	bool SetGamma(const QString& sessionKey, double gamma);
	bool SetBrightness(const QString& sessionKey, int brightness);
	bool SetCountLeds(const QString& sessionKey, int countLeds);
	bool SetSmooth(const QString& sessionKey, int smooth);
	bool SetProfile(const QString& sessionKey, const QString& profile);
	bool SetDevice(const QString& sessionKey, const QString& device);
#ifdef SOUNDVIZ_SUPPORT
	bool SetSoundVizColors(const QString& sessionKey, QColor min, QColor max);
	bool SetSoundVizLiquidMode(const QString& sessionKey, bool enabled);
#endif
	bool SetPersistOnUnlock(const QString& sessionKey, bool enabled);

	bool SetLeds(const QString& sessionKey, QList<QRect> leds);
	bool NewProfile(const QString& sessionKey, const QString& profile);
	bool DeleteProfile(const QString& sessionKey, const QString& profile);
	bool SetBacklight(const QString& sessionKey, int backlight);

// no LOCK
	QString Version();
	int GetCountLeds();
	int GetStatus();
	bool GetStatusAPI();
	QStringList GetProfiles();
	QString GetProfile();
	QList<QRect> GetLeds();
	QList<QRgb> GetColors();
	double GetFPS();
	QRect GetScreenSize();
	int GetBacklight();
	double GetGamma();
	int GetBrightness();
	int GetSmooth();
#ifdef SOUNDVIZ_SUPPORT
	QPair<QColor, QColor> GetSoundVizColors();
	bool GetSoundVizLiquidMode();
#endif
	bool GetPersistOnUnlock();

// Settings
	QString GetPluginsDir();
	void SetSettingProfile(const QString& key, const QVariant& value);
	QVariant GetSettingProfile(const QString& key);
	void SetSettingMain(const QString& key, const QVariant& value);
	QVariant GetSettingMain(const QString& key);

	bool VerifySessionKey(const QString& sessionKey);
	void SetLockAlive(const QString& sessionKey);

signals:
	void ChangeProfile(const QString& profile);
	void ChangeStatus(int status);
	void ChangeLockStatus(bool lock);

//end Plugin section

signals:
	void requestBacklightStatus();
	void updateDeviceLockStatus(const DeviceLocked::DeviceLockStatus status, const QList<QString>& modules);
	void updateLedsColors(const QList<QRgb> & colors);
	void updateGamma(double value);
	void updateBrightness(int value);
	void updateSmooth(int value);
#ifdef SOUNDVIZ_SUPPORT
	void updateSoundVizMinColor(QColor color);
	void updateSoundVizMaxColor(QColor color);
	void updateSoundVizLiquid(bool value);
#endif
	void updateProfile(const QString& profileName);
	void updateStatus(Backlight::Status status);
	void updateBacklight(Lightpack::Mode status);
	void updateCountLeds(int value);
	void changeDevice(const QString& device);


public slots:
	void setNumberOfLeds(int numberOfLeds);
	void resultBacklightStatus(Backlight::Status status);
	void changeProfile(const QString& profile);
	void refreshAmbilightEvaluated(double updateResultMs);
	void refreshScreenRect(QRect rect);
	void updateColorsCache(const QList<QRgb> & colors);
	void updateGammaCache(double value);
	void updateBrightnessCache(int value);
	void updateSmoothCache(int value);
#ifdef SOUNDVIZ_SUPPORT
	void updateSoundVizMinColorCache(QColor color);
	void updateSoundVizMaxColorCache(QColor color);
	void updateSoundVizLiquidCache(bool value);
#endif
	void updatePlugin(const QList<Plugin*>& plugins);

private slots:
	void timeoutLock();

private:
	bool lockAlive;

	static const int SignalWaitTimeoutMs;
	QElapsedTimer m_timer;
	bool m_isRequestBacklightStatusDone;
	Backlight::Status m_backlightStatusResult;

	double hz;
	QRect screen;

	QList<QString> lockSessionKeys;
	QList<QRgb> m_setColors;
	QList<QRgb> m_curColors;
	QTimer *m_timerLock;

	double m_gamma;
	int m_brightness;
	int m_smooth;
#ifdef SOUNDVIZ_SUPPORT
	QColor m_soundVizMin;
	QColor m_soundVizMax;
	bool m_soundVizLiquid;
#endif
	bool m_persistOnUnlock;

	void initColors(int numberOfLeds);

	QList<Plugin*> _plugins;
	Plugin* findName(const QString& name);
	Plugin* findSessionKey(const QString& sessionKey);
};
#endif

