/*
 * LogWriter.hpp
 *
 *	Created on: 11.07.2014
 *		Project: Lightpack
 *
 *	Lightpack is very simple implementation of the backlight for a laptop
 *
 *	Copyright (c) 2014 Woodenshark LLC
 *
 *	Lightpack is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	Lightpack is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

class LogWriter
{
public:
	enum Level { Debug, Warn, Critical, Fatal, LevelCount };

	LogWriter();
	~LogWriter();

	int initWith(const QString& logsDirPath);
	void initDisabled();
	void writeMessage(const QString& msg, Level level = Debug);
	QDir logsDir() const;
	static QDir getLogsDir();

	struct ScopedMessageHandler
	{
		QtMessageHandler m_oldHandler;

		ScopedMessageHandler(LogWriter* logWriter)
			: m_oldHandler(qInstallMessageHandler(&LogWriter::messageHandler))
		{
			LogWriter::g_logWriter = logWriter;
		}

		~ScopedMessageHandler()
		{
			qInstallMessageHandler(m_oldHandler);
			LogWriter::g_logWriter = NULL;
		}
	};

private:
	static const int StoreLogsLaunches = 5;
	static LogWriter* g_logWriter;

	static void messageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);
	static bool rotateLogFiles(const QDir& logsDir);

	QTextStream m_logStream;
	QMutex m_mutex;
	QString m_startupLogStore;
	QDir m_logsDir;
	bool m_disabled;
};
