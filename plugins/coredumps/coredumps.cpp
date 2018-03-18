/*
  This file is part of Dog.

  Copyright (C) 2018 Sérgio Martins <smartins@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "coredumps.h"

#include <QDebug>
#include <QThread>
#include <QDir>
#include <QDateTime>
#include <QProcess>

#include <chrono>

using namespace std;

#define INTERVAL 3600 // 1 hour

const char *const s_coredump_folder = "/data/cores/";

void CoreDumpCleaner::clean()
{
    QDir coredumpDir(s_coredump_folder);
    if (!coredumpDir.exists())
        return;

    QFileInfoList fileInfos = coredumpDir.entryInfoList(QDir::Files);
    for (const QFileInfo &file : fileInfos) {
        Action action = actionForFile(file);
        if (action == Action_Delete)
            removeFile(file.absoluteFilePath());
        else if (action == Action_Compress)
            compressFile(file.absoluteFilePath());
    }

    // We're done, delete the worker
    deleteLater();
}

bool CoreDumpCleaner::isCompressed(const QFileInfo &file) const
{
    // Returns true if the coredump is compressed. Which for us means having a zsdt extension
    return file.suffix() == "zst";
}

void CoreDumpCleaner::removeFile(const QString &file)
{
    emit q->log("CoreDumpCleaner::removeFile: removing " + file);
    QFile::remove(file);
}

void CoreDumpCleaner::compressFile(const QString &file)
{
    emit q->log("CoreDumpCleaner::compressFile: compressing file " + file + "...");
    QProcess::execute(QString("zstd %1").arg(file));
    QFile::remove(file);
}

CoreDumpCleaner::Action CoreDumpCleaner::actionForFile(const QFileInfo &file) const
{
    // Returns what we should do with the file.
    QDateTime creationTime = file.birthTime();
    chrono::seconds ageInSeconds(creationTime.secsTo(QDateTime::currentDateTimeUtc()));
    const int ageInHours = ageInSeconds.count() / 3600.0;
    const int ageInDays = ageInHours / 24.0;

    if (ageInDays > 7)
        return Action_Delete;

    if (ageInHours > 5 && !isCompressed(file))
        return Action_Compress;

    return Action_None;
}

CoreDumpsPlugin::CoreDumpsPlugin()
{
    m_timer.setInterval(chrono::hours(1));
    connect(&m_timer, &QTimer::timeout, this, &CoreDumpsPlugin::work);
}

QString CoreDumpsPlugin::name() const
{
    return "Core Dumps";
}

QString CoreDumpsPlugin::shortName() const
{
    return "CoreDumps";
}

void CoreDumpsPlugin::start()
{
    m_timer.start();
    work();
}

void CoreDumpsPlugin::work()
{
    m_working = true;
    auto thread = new QThread();
    auto worker = new CoreDumpCleaner(this);
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &CoreDumpCleaner::clean);
    connect(worker, &QObject::destroyed, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}
