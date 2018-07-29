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
#include <QEventLoop>

#include <chrono>

using namespace std;

static bool isValidMethod(const QString &method)
{
    static QStringList methods = { "rm", "git-clean" };
    return methods.contains(method);
}

void BuildDirCleaner::cleanAll()
{
    static bool alreadyRunning = false;

    if (alreadyRunning) {
        qCWarning(q->category) << "cleanAll already running, bailing out";
        return;
    }

    alreadyRunning = true;
    for (const JobDescriptor &job : m_jobs) {
        cleanOne(job);
    }
    deleteLater();
    alreadyRunning = false;
}

void BuildDirCleaner::cleanOne(const JobDescriptor &job)
{
    if (job.method == QLatin1String("rm")) {
        runRm(job);
    } else if (job.method == QLatin1String("git-clean")) {
        runGitClean(job);
    }

    qCDebug(q->category) << "Finished on" << job.path;
}

void BuildDirCleaner::runGitClean(const JobDescriptor &job)
{
    QProcess p;
    QEventLoop loop;
    p.setWorkingDirectory(job.path);
    p.connect(&p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [&loop, this, &job] (int exitCode, QProcess::ExitStatus status) {
        if (exitCode == 0) {
            qCDebug(q->category) << QString("git cleaned %1").arg(job.path);
        } else {
            qCWarning(q->category) << QString("Unable to git clean %1").arg(job.path);
        }
        loop.quit();
    });

    qCDebug(q->category) << "Starting git clean -fdx on " << job.path;
    p.start("git", {"clean", "-fdx"});
    loop.exec();
    qCDebug(q->category) << "Git clean finished";
}

void BuildDirCleaner::runRm(const JobDescriptor &job)
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
            qCWarning(q->category) << QString("Unable to get creation date from %1").arg(dir.absolutePath());
            continue;
        }

        const int age = date.daysTo(now);

        if (age > 2) {
            QDir dirToDelete(job.path);
            dirToDelete.cd(dirname);
            if (dirToDelete.removeRecursively()) {
                qCDebug(q->category) << QString("Removed %1").arg(dirToDelete.absolutePath());
            } else {
                qCWarning(q->category) << QString("Unable to remove %1").arg(dirToDelete.absolutePath());
            }
        }
    }
}

BuildDirCleanerPlugin::BuildDirCleanerPlugin()
    : PluginInterface("builddircleaner", chrono::hours(1))
    , m_jobs(loadJson())
{
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
    auto worker = new BuildDirCleaner(m_jobs, this);
    startInWorkerThread(worker, &BuildDirCleaner::cleanAll);
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
        QString method = dirMap.value("method", "rm").toString();

        if (isValidMethod(method)) {
            jobs << JobDescriptor { dir, pattern, method };
        } else {
           qCWarning(category) << "Invalid method" << method;
        }
    }

    return jobs;
}
