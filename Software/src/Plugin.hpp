#pragma once

#include <QObject>
#include <QProcess>
#include <QIcon>
#include "debug.h"

class Plugin : public QObject
{
	Q_OBJECT
public:
	Plugin(const QString& name, const QString& path, QObject *parent = 0);
	~Plugin();

	void Start();
	void Stop();

	QString Name() const;
	QString Guid() const;
	QString Description() const;
	QString Author() const;
	QString Version() const;
	QIcon Icon() const;


	QProcess::ProcessState state() const;
	bool isEnabled() const;
	void setEnabled(bool enable);
	void setPriority(int priority);
	int getPriority() const;


signals:
	void pluginStateChanged(QProcess::ProcessState newState);

public slots:
	void stateChanged(QProcess::ProcessState newState);
	void errorOccurred(QProcess::ProcessError error);
	void started();
	void finished(int exitCode, QProcess::ExitStatus exitStatus);
	void readyReadStandardError();
	void readyReadStandardOutput();

private:

	QString _guid;
	QString _name;
	QString _description;
	QString _author;
	QString _version;
	QIcon	_icon;
	QString _exec;
	QStringList _arguments;
	QString _pathPlugin;
	QProcess *process;

};
