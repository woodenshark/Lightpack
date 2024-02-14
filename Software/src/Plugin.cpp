#include "Plugin.hpp"
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QImageReader>
#include "Settings.hpp"
#include "../common/defs.h"

using namespace SettingsScope;

#if defined(Q_OS_WIN)
	const QString kOsSpecificExecuteKey = QStringLiteral("ExecuteOnWindows");
#elif defined(MAC_OS)
	const QString kOsSpecificExecuteKey = QStringLiteral("ExecuteOnOSX");
#elif defined(Q_OS_UNIX)
	const QString kOsSpecificExecuteKey = QStringLiteral("ExecuteOnNix");
#endif

Plugin::Plugin(const QString& name, const QString& path, QObject *parent) :
	QObject(parent)
{
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << name << path;
	_pathPlugin = path;
	QDir pluginPath(_pathPlugin);

	const QString fileName(QStringLiteral("%1/%2.ini").arg(path, name));
	if (!QFile::exists(fileName)) {
		qWarning() << Q_FUNC_INFO << name << fileName << "does not exist, generating defaults, make sure to edit the file!";
		if (!QFile::copy(QStringLiteral(":/plugin-template.ini"), fileName))
			qWarning() << Q_FUNC_INFO << name << "failed to generate" << fileName;
		else if (!QFile::setPermissions(fileName, QFile::permissions(fileName) | QFileDevice::WriteOwner))
			qWarning() << Q_FUNC_INFO << name << "could not set write permissions to" << fileName;
	}
	QSettings settings(fileName, QSettings::IniFormat);
	settings.beginGroup(QStringLiteral("Main"));
	_name = settings.value(QStringLiteral("Name"), QLatin1String("")).toString();
	if (_name.isEmpty())
		_name = name;

	if (settings.contains(kOsSpecificExecuteKey))
		_arguments = settings.value(kOsSpecificExecuteKey, QLatin1String("")).toString().split(' ');
	else
		_arguments = settings.value(QStringLiteral("Execute"), QLatin1String("")).toString().split(' ');

	if (!_arguments.isEmpty())
		_exec = _arguments.takeFirst();

	if (_exec.isEmpty())
		qWarning() << Q_FUNC_INFO << name << "no executable, check" << fileName;

	_guid = settings.value(QStringLiteral("Guid"), QLatin1String("")).toString();
	_author = settings.value(QStringLiteral("Author"), QLatin1String("")).toString();
	_description = settings.value(QStringLiteral("Description"), QLatin1String("")).toString();
	_version = settings.value(QStringLiteral("Version"), QLatin1String("")).toString();

	const QString iconName = settings.value(QStringLiteral("Icon"), QLatin1String("")).toString();
	const QString iconPath = pluginPath.absoluteFilePath(iconName);
	if (!iconName.isEmpty() && QFile::exists(iconPath) && !QImageReader::imageFormat(iconPath).isEmpty())
		_icon = QIcon(iconPath);
	if (_icon.isNull())
		_icon = QIcon(QStringLiteral(":/icons/plugins.png"));

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
	const QString key = QStringLiteral("%1/Priority").arg(_name);
	return Settings::valueMain(key).toInt();
}

void Plugin::setPriority(int priority) {
	const QString key = QStringLiteral("%1/Priority").arg(_name);
	Settings::setValueMain(key,priority);
}

bool Plugin::isEnabled() const {
	const QString key = QStringLiteral("%1/Enable").arg(_name);
	return Settings::valueMain(key).toBool();
}

void Plugin::setEnabled(bool enable) {
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << _name << enable;
	const QString key = QStringLiteral("%1/Enable").arg(_name);
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

	connect(process, &QProcess::stateChanged, this, &Plugin::stateChanged);
	connect(process, &QProcess::errorOccurred, this, &Plugin::errorOccurred);

	connect(process, &QProcess::started, this, &Plugin::started);
	connect(process, qOverload<int,QProcess::ExitStatus>(&QProcess::finished), this, &Plugin::finished);

	connect(process, &QProcess::readyReadStandardError, this, &Plugin::readyReadStandardError);
	connect(process, &QProcess::readyReadStandardOutput, this, &Plugin::readyReadStandardOutput);

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
