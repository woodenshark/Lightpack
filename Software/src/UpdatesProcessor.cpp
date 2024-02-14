/*
 * UpdatesProcessor.cpp
 *
 *	Created on: 4/11/2014
 *		Project: Prismatik
 *
 *	Copyright (c) 2014 tim
 *
 *	Lightpack is an open-source, USB content-driving ambient lighting
 *	hardware.
 *
 *	Prismatik is a free, open-source software: you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as published
 *	by the Free Software Foundation, either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	Prismatik and Lightpack files is distributed in the hope that it will be
 *	useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
 *	General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QCoreApplication>
#include <QXmlStreamReader>
#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include <QProcess>
#include "version.h"
#include "debug.h"
#include "UpdatesProcessor.hpp"
#include "Settings.hpp"

//#define UPDATE_CHECK_URL "https://psieg.de/lightpack/update.xml"
#define UPDATE_CHECK_URL "https://psieg.github.io/Lightpack/update.xml"

const AppVersion kCurVersion(VERSION_STR);

UpdatesProcessor::UpdatesProcessor(QObject * parent)
	: QObject(parent)
	, _reply(NULL)
{
}

void UpdatesProcessor::error(QNetworkReply::NetworkError code)
{
	qWarning() << Q_FUNC_INFO << "Updatecheck/updateload failed: " << code << ": " << _reply->errorString();
}

void UpdatesProcessor::requestUpdates()
{
	DEBUG_MID_LEVEL << Q_FUNC_INFO << "checking" << UPDATE_CHECK_URL;
	if(_reply != NULL) {
		_reply->disconnect();
		delete _reply;
		_reply = NULL;
	}

	QNetworkRequest request(QUrl(UPDATE_CHECK_URL));
	request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
	_reply = _networkMan.get(request);
	connect(_reply, &QNetworkReply::finished, this, &UpdatesProcessor::readyRead);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	connect(_reply, &QNetworkReply::errorOccurred, this, &UpdatesProcessor::error);
#else
	connect(_reply, qOverload<QNetworkReply::NetworkError>(&QNetworkReply::error), this, &UpdatesProcessor::error);
#endif
}

QList<UpdateInfo> UpdatesProcessor::readUpdates()
{
	QList<UpdateInfo> updates;

	QXmlStreamReader xmlReader(_reply->readAll());

	if (xmlReader.readNextStartElement()) {
		if (xmlReader.name() == QStringLiteral("updates")) {
			readUpdates(&updates, &xmlReader);
			QList<UpdateInfo>::iterator it = updates.begin();
			while (it != updates.end()) {
				UpdateInfo updateInfo = *it;
				if (!updateInfo.softwareVersion.isEmpty() && !isVersionMatches(updateInfo.softwareVersion, kCurVersion)) {
					updates.removeAll(updateInfo);
				} else {
					it++;
				}
			}
		}
	}
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << updates.size() << "updates available";
	return updates;
}

void UpdatesProcessor::loadUpdate(UpdateInfo& info)
{
#ifdef Q_OS_WIN
	DEBUG_MID_LEVEL << Q_FUNC_INFO << "fetching" << info.pkgUrl;
	if (info.pkgUrl.isEmpty() || info.sigUrl.isEmpty()) {
		qCritical() << Q_FUNC_INFO << "UpdateInfo is missing required links";
		return;
	}

	_sigUrl = info.sigUrl;
	if (_reply != NULL) {
		_reply->disconnect();
		delete _reply;
		_reply = NULL;
	}

	QNetworkRequest request(QUrl(info.pkgUrl));
	request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
#else
#	if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#	endif
#endif

	_reply = _networkMan.get(request);
	connect(_reply, &QNetworkReply::finished, this, &UpdatesProcessor::updatePgkLoaded);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	connect(_reply, &QNetworkReply::errorOccurred, this, &UpdatesProcessor::error);
#else
	connect(_reply, qOverload<QNetworkReply::NetworkError>(&QNetworkReply::error), this, &UpdatesProcessor::error);
#endif

#else
	qWarning() << Q_FUNC_INFO << "Trying to load update on non-windows platform -- ignored";
#endif
}

#ifdef Q_OS_WIN
void UpdatesProcessor::updatePgkLoaded()
{
	if (!(_reply->error() == QNetworkReply::NetworkError::NoError))
		return;

	DEBUG_MID_LEVEL << Q_FUNC_INFO << "fetching " << _sigUrl;

	QFile f(QDir::tempPath() + "\\PsiegUpdateElevate_Prismatik.exe");
	if (!f.open(QIODevice::WriteOnly)) {
		qCritical() << Q_FUNC_INFO << "Failed to write update package";
	}
	f.write(_reply->readAll());
	f.close();

	_reply->deleteLater();
	_reply = NULL;

	QNetworkRequest request = QNetworkRequest(QUrl(_sigUrl));
	request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
#else
#	if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#	endif
#endif

	_reply = _networkMan.get(request);
	connect(_reply, &QNetworkReply::finished, this, &UpdatesProcessor::updateSigLoaded);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	connect(_reply, &QNetworkReply::errorOccurred, this, &UpdatesProcessor::error);
#else
	connect(_reply, qOverload<QNetworkReply::NetworkError>(&QNetworkReply::error), this, &UpdatesProcessor::error);
#endif
}

void UpdatesProcessor::updateSigLoaded()
{
	if (!(_reply->error() == QNetworkReply::NetworkError::NoError))
		return;
	DEBUG_MID_LEVEL << Q_FUNC_INFO;

	QFile f(QDir::tempPath() + "\\PsiegUpdateElevate_Prismatik.exe.sig");
	if (!f.open(QIODevice::WriteOnly)) {
		qCritical() << Q_FUNC_INFO << "Failed to write update signature";
	}
	f.write(_reply->readAll());
	f.close();

	_reply->deleteLater();
	_reply = NULL;

	// TODO: ensure user is not using the program
	DEBUG_LOW_LEVEL << Q_FUNC_INFO << "triggering update process";
	SettingsScope::Settings::setAutoUpdatingVersion(VERSION_STR);

	QStringList args;
	args.append("request");
	args.append(QDir::tempPath());
	args.append(QCoreApplication::applicationFilePath());
	if (QProcess::startDetached(QCoreApplication::applicationDirPath() + "\\UpdateElevate.exe", args)) {
		QCoreApplication::quit();
	} else {
		qCritical() << Q_FUNC_INFO << "Failed to start UpdateElevate.exe";
	}
}
#endif

bool UpdatesProcessor::isVersionMatches(const QString &predicate, const AppVersion &version)
{
	enum Condition {
		LT,
		GT,
		EQ,
		LE,
		GE
	};

	Condition cond;
	QString predicateRef(predicate);

	QString predicateVerStr = predicate.mid(2).trimmed();

	QString condStr = predicateRef.left(2);

	if (condStr == QStringLiteral("lt")) {
		cond = LT;
	} else if (condStr == QStringLiteral("gt")) {
		cond = GT;
	} else if (condStr == QStringLiteral("eq")) {
		cond = EQ;
	} else if (condStr == QStringLiteral("le")) {
		cond = LE;
	} else if (condStr == QStringLiteral("ge")) {
		cond = GE;
	} else {
		cond = EQ;
		predicateVerStr = predicateRef.trimmed();
	}

	AppVersion predicateVer(predicateVerStr);

	if (!version.isValid() || !predicateVer.isValid())
		return false;

	switch (cond) {
	case LT:
		return version < predicateVer;
		break;
	case GT:
		return version > predicateVer;
		break;
	case EQ:
		return version == predicateVer;
		break;
	case LE:
		return version <= predicateVer;
		break;
	case GE:
		return version >= predicateVer;
		break;
	}

	Q_ASSERT(false);
	return false;
}

QList<UpdateInfo> * UpdatesProcessor::readUpdates(QList<UpdateInfo> *updates, QXmlStreamReader *xmlReader)
{
	if (xmlReader->readNextStartElement()) {
		while (xmlReader->name() == QStringLiteral("update")) {
			UpdateInfo updateInfo;
			while(xmlReader->readNextStartElement()) {
				if (xmlReader->name() == QStringLiteral("id")) {
					xmlReader->readNext();
					QString id = xmlReader->text().toString();
					updateInfo.id = id.toUInt();
				} else if (xmlReader->name() == QStringLiteral("url")) {
					xmlReader->readNext();
					updateInfo.url = xmlReader->text().toString();
				} else if (xmlReader->name() == QStringLiteral("pkgUrl")) {
					xmlReader->readNext();
					updateInfo.pkgUrl = xmlReader->text().toString();
				} else if (xmlReader->name() == QStringLiteral("sigUrl")) {
					xmlReader->readNext();
					updateInfo.sigUrl = xmlReader->text().toString();
				} else if (xmlReader->name() == QStringLiteral("title")) {
					xmlReader->readNext();
					updateInfo.title = xmlReader->text().toString();
				} else if (xmlReader->name() == QStringLiteral("text")) {
					xmlReader->readNext();
					updateInfo.text = xmlReader->text().toString();
				} else if (xmlReader->name() == QStringLiteral("softwareVersion")) {
					xmlReader->readNext();
					updateInfo.softwareVersion = xmlReader->text().toString();
				} else if (xmlReader->name() == QStringLiteral("firmwareVersion")) {
					xmlReader->readNext();
					updateInfo.firmwareVersion = xmlReader->text().toString();
				} else if (xmlReader->name() == QStringLiteral("update")) {
					break;
				}
				xmlReader->skipCurrentElement();
			}
			// add updates with defined id only
			if(updateInfo.id > 0)
				updates->append(updateInfo);
		}
	}
	return updates;
}
