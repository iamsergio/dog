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

#include "builddircleaner.h"

#include <QDebug>
#include <QThread>
#include <QDir>
#include <QDateTime>
#include <QProcess>

#include <chrono>

//#define DRY_RUN

using namespace std;

void BuildDirCleaner::cleanAll()
{
    for (const JobDescriptor &job : m_jobs) {
        cleanOne(job);
    }
}

void BuildDirCleaner::cleanOne(const JobDescriptor &job)
{
    QDir dir(job.path);
    if (!job.pattern.isEmpty()) {
        dir.setNameFilters(QStringList() << job.pattern);
    }

    QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    const QDateTime now = QDateTime::currentDateTime();
    for (const QString &dirname : dirs) {
        QFileInfo info (dir.absolutePath());
        QDateTime date = info.birthTime();
        if (!date.isValid()) {
            q->log(QString("Unable to get creation date from %1").arg(dir.absolutePath()));
            continue;
        }

        QDir dirToDelete(job.path);
        dirToDelete.cd(dirname);
        int age = date.daysTo(now);

        if (age > 2) {
            const bool success =
#ifdef DRY_RUN
            true;
#else
            dirToDelete.removeRecursively();
#endif
            if (success) {
                q->log(QString("Removed %1").arg(dirToDelete.absolutePath()));
            } else {
                q->log(QString("Unable to remove %1").arg(dirToDelete.absolutePath()));
            }
        }
    }

    deleteLater();
    emit q->log("Finished");
}

BuildDirCleanerPlugin::BuildDirCleanerPlugin()
    : m_jobs(loadJson())
{
    m_timer.setInterval(chrono::hours(1));
}

QString BuildDirCleanerPlugin::name() const
{
    return "BuildDir Cleaner";
}

QString BuildDirCleanerPlugin::shortName() const
{
    return "BuildDirCleaner";
}

void BuildDirCleanerPlugin::start()
{
    m_timer.start();
    work();
}

void BuildDirCleanerPlugin::work_impl()
{
    auto thread = new QThread();
    auto worker = new BuildDirCleaner(m_jobs, this);
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &BuildDirCleaner::cleanAll);
    connect(worker, &QObject::destroyed, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

JobDescriptor::List BuildDirCleanerPlugin::loadJson() const
{
    JobDescriptor::List jobs;
    QVariantMap json = readConfig();
    const QVariantList dirs = json.value("dirs").toList();
    for (const QVariant &dirV : dirs) {
        QVariantMap dirMap = dirV.toMap();
        QString dir = dirMap.value("dir").toString();
        QString pattern = dirMap.value("pattern").toString();
        jobs << JobDescriptor { dir, pattern };
    }

    return jobs;
}
