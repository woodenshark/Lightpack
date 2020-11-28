#include "Plugin.hpp"
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QImageReader>
#include "Settings.hpp"
#include "../common/defs.h"

using namespace SettingsScope;

#if defined(Q_OS_WIN)
	const QString kOsSpecificExecuteKey = "ExecuteOnWindows";
#elif defined(MAC_OS)
	const QString kOsSpecificExecuteKey = "ExecuteOnOSX";
#elif defined(Q_OS_UNIX)
	const QString kOsSpecificExecuteKey = "ExecuteOnNix";
#endif

Plugin::Plugin(QString name, QString path, QObject *parent) :
	QObject(parent)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << name << path;
	_pathPlugin = path;
	QDir pluginPath(_pathPlugin);

	const QString fileName(path+"/"+name+".ini");
	if (!QFile::exists(fileName)) {
		qWarning() << Q_FUNC_INFO << name << fileName << "does not exist, generating defaults, make sure to edit the file!";
		if (!QFile::copy(":/plugin-template.ini", fileName))
			qWarning() << Q_FUNC_INFO << name << "failed to generate" << fileName;
		else if (!QFile::setPermissions(fileName, QFile::permissions(fileName) | QFileDevice::WriteOwner))
			qWarning() << Q_FUNC_INFO << name << "could not set write permissions to" << fileName;
	}
	QSettings settings(fileName, QSettings::IniFormat);
	settings.beginGroup("Main");
	_name = settings.value("Name", "").toString();
	if (_name.isEmpty())
		_name = name;

	if (settings.contains(kOsSpecificExecuteKey))
		_arguments = settings.value(kOsSpecificExecuteKey, "").toString().split(' ');
	else
		_arguments = settings.value("Execute", "").toString().split(' ');

	if (!_arguments.isEmpty())
		_exec = _arguments.takeFirst();

	if (_exec.isEmpty())
		qWarning() << Q_FUNC_INFO << name << "no executable, check" << fileName;

	_guid = settings.value("Guid", "").toString();
	_author = settings.value("Author", "").toString();
	_description = settings.value("Description", "").toString();
	_version = settings.value("Version", "").toString();

	const QString iconName = settings.value("Icon", "").toString();
	const QString iconPath = pluginPath.absoluteFilePath(iconName);
	if (!iconName.isEmpty() && QFile::exists(iconPath) && !QImageReader::imageFormat(iconPath).isEmpty())
		_icon = QIcon(iconPath);
	if (_icon.isNull())
		_icon = QIcon(":/icons/plugins.png");

	settings.endGroup();

	process = new QProcess(this);
}

Plugin::~Plugin()
{
	Stop();
}

QString Plugin::Name() const {
	return _name;
}

QString Plugin::Guid() const {
	return _guid;
}

QString Plugin::Author() const {
	return _author;
}

QString Plugin::Description() const	{
	return _description;
}

QString Plugin::Version() const	{
	return _version;
}


QIcon Plugin::Icon() const {
	return _icon;
}


int Plugin::getPriority() const {
	const QString key = _name+"/Priority";
	return Settings::valueMain(key).toInt();
}

void Plugin::setPriority(int priority) {
	const QString key = _name+"/Priority";
	Settings::setValueMain(key,priority);
}

bool Plugin::isEnabled() const {
	const QString key = _name+"/Enable";
	return Settings::valueMain(key).toBool();
}

void Plugin::setEnabled(bool enable) {
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << _name << enable;
	const QString key = _name+"/Enable";
	Settings::setValueMain(key,enable);
	if (!enable) Stop();
	if (enable) Start();
}


void Plugin::Start()
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << _name << "starting" << _exec << _arguments;

	QDir dir(_pathPlugin);
	QDir::setCurrent(dir.absolutePath());

	process->disconnect();

	connect(process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChanged(QProcess::ProcessState)));
	connect(process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(errorOccurred(QProcess::ProcessError)));

	connect(process, SIGNAL(started()), this, SLOT(started()));
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));

	connect(process, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
	connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));

	process->setEnvironment(QProcess::systemEnvironment());
	process->setProgram(_exec);
	process->setArguments(_arguments);
	process->start();
}

void Plugin::Stop()
{
	process->kill();
}

QProcess::ProcessState Plugin::state() const
{
	return process->state();
}

// QProcess slots
void Plugin::stateChanged(QProcess::ProcessState newState)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << _name << newState;
	emit pluginStateChanged(newState);
}

void Plugin::errorOccurred(QProcess::ProcessError error)
{
	qWarning() << Q_FUNC_INFO << _name << error << _exec << _arguments;
}

void Plugin::started()
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << _name;
}

void Plugin::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << _name << "exitCode=" << exitCode << exitStatus;
}

void Plugin::readyReadStandardError()
{
	qWarning() << Q_FUNC_INFO << _name << process->readAllStandardError();
}

void Plugin::readyReadStandardOutput()
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << _name << process->readAllStandardOutput();
}
