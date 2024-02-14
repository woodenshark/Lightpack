#pragma once

#include <QObject>
#include <QMap>
#include <QProcess>
#include "Plugin.hpp"

class PluginsManager : public QObject
{
	Q_OBJECT
public:
	PluginsManager(QObject *parent = 0);
	virtual ~PluginsManager();

	void LoadPlugins(const QString& path);
	QList<Plugin*> getPluginList();
	Plugin* getPlugin(const QString& name_);

private:
	void dropPlugins();
	QMap<QString, Plugin*> _plugins;

signals:
	void updatePlugin(QList<Plugin*>);

public slots:
	void reloadPlugins();
	void StartPlugins();
	void StopPlugins();

private slots:
	void onPluginStateChangedHandler(QProcess::ProcessState state);

};

