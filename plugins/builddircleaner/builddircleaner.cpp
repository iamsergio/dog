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

BuildDirCleanerWorker::BuildDirCleanerWorker(BuildDirCleanerPlugin *plugin)
    : WorkerObject(plugin)
{
}

void BuildDirCleanerWorker::work()
{
    static bool alreadyRunning = false;

    if (alreadyRunning) {
        qCWarning(m_plugin->category) << "work() already running, bailing out";
        return;
    }

    alreadyRunning = true;
    for (const BuildDirCleanerPlugin::JobDescriptor &job : qAsConst(m_jobDescriptors)) {
        cleanOne(job);
    }
    deleteLater();
    alreadyRunning = false;
}

void BuildDirCleanerWorker::loadJobDescriptors()
{
    m_jobDescriptors.clear();
    const QVariantList dirs = m_plugin->jobDescriptorsVariant();
    for (const QVariant &dirV : dirs) {
        QVariantMap dirMap = dirV.toMap();
        QString dir = dirMap.value("dir").toString();
        QString pattern = dirMap.value("pattern").toString();
        QString methodStr = dirMap.value("method").toString();
        BuildDirCleanerPlugin::JobDescriptor::Method method = BuildDirCleanerPlugin::JobDescriptor::methodFromString(methodStr);

        if (method == BuildDirCleanerPlugin::JobDescriptor::Method_None) {
            qCWarning(m_plugin->category) << "Invalid method" << method;
        } else {
            m_jobDescriptors << BuildDirCleanerPlugin::JobDescriptor { dir, pattern, method };
        }
    }
}

void BuildDirCleanerWorker::cleanOne(const BuildDirCleanerPlugin::JobDescriptor &job)
{
    if (job.method == BuildDirCleanerPlugin::JobDescriptor::Method_RmChilds) {
        runRmChilds(job);
    } else if (job.method == BuildDirCleanerPlugin::JobDescriptor::Method_GitClean) {
        runGitClean(job);
    } else if (job.method == BuildDirCleanerPlugin::JobDescriptor::Method_Rm) {
        QDir dirToDelete(job.path);
        runRm(dirToDelete);
    }

    qCDebug(m_plugin->category) << "Finished on" << job.path;
}

void BuildDirCleanerWorker::runGitClean(const BuildDirCleanerPlugin::JobDescriptor &job)
{
    QProcess p;
    QEventLoop loop;
    p.setWorkingDirectory(job.path);
    p.connect(&p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), m_plugin, [&loop, this, &job] (int exitCode, QProcess::ExitStatus) {
        if (exitCode == 0) {
            qCDebug(m_plugin->category) << QString("git cleaned %1").arg(job.path);
        } else {
            qCWarning(m_plugin->category) << QString("Unable to git clean %1").arg(job.path);
        }
        loop.quit();
    });

    qCDebug(m_plugin->category) << "Starting git clean -fdx on " << job.path;
    p.start("git", {"clean", "-fdx"});
    loop.exec();
    qCDebug(m_plugin->category) << "Git clean finished";
}

void BuildDirCleanerWorker::runRmChilds(const BuildDirCleanerPlugin::JobDescriptor &job)
{
    QDir dir(job.path);
    if (!job.pattern.isEmpty()) {
        dir.setNameFilters(QStringList() << job.pattern);
    }

    QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &dirname : dirs) {
        QDir dirToDelete(job.path);
        dirToDelete.cd(dirname);
        runRm(dirToDelete);
    }
}

void BuildDirCleanerWorker::runRm(QDir &dirToDelete)
{
    const QDateTime now = QDateTime::currentDateTime();
    QFileInfo info(dirToDelete.absolutePath());
    if (!info.exists())
        return;

    QDateTime date = info.birthTime();

    if (!date.isValid()) {
        qCWarning(m_plugin->category) << QString("Unable to get creation date from %1").arg(dirToDelete.absolutePath());
        return;
    }

    const int age = date.daysTo(now);

    if (age > 2) {
        if (dirToDelete.removeRecursively()) {
            qCDebug(m_plugin->category) << QString("Removed %1").arg(dirToDelete.absolutePath());
        } else {
            qCWarning(m_plugin->category) << QString("Unable to remove %1").arg(dirToDelete.absolutePath());
        }
    }
}

BuildDirCleanerPlugin::BuildDirCleanerPlugin()
    : PluginInterface("builddircleaner", chrono::hours(1))
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

void BuildDirCleanerPlugin::start_impl()
{
    m_timer.start();
    work();
}

void BuildDirCleanerPlugin::work_impl()
{
    auto worker = new BuildDirCleanerWorker(this);
    worker->loadJobDescriptors();
    startInWorkerThread(worker, &BuildDirCleanerWorker::work);
}
