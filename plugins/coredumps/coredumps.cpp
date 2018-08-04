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
#include "fileservice.h"

#include <QDebug>
#include <QThread>
#include <QDir>
#include <QDateTime>
#include <QProcess>

#include <chrono>

using namespace std;

#define INTERVAL 3600 // 1 hour

const char *const s_coredump_folder = "/data/cores/";

void CoreDumpCleanerWorker::work()
{
    QDir coredumpDir(s_coredump_folder);
    if (!coredumpDir.exists())
        return;

    QString outfile;
    QFileInfoList fileInfos = coredumpDir.entryInfoList(QDir::Files);
    for (const QFileInfo &file : fileInfos) {
        Action action = actionForFile(file);
        if (action == Action_Delete)
            m_plugin->m_fileService->removeFile(file.absoluteFilePath());
        else if (action == Action_Compress)
            m_plugin->m_fileService->compressFile(file.absoluteFilePath(), outfile);
    }

    // We're done, delete the worker
    deleteLater();
}

void CoreDumpCleanerWorker::loadJobDescriptors()
{
    m_jobDescriptors.clear();
    const auto descriptors = m_plugin->jobDescriptorsVariant();
    if (descriptors.isEmpty())
        return;

    QString path = descriptors.first().toMap().value(QStringLiteral("path")).toString();
    if (!path.isEmpty()) {
        m_jobDescriptors << CoreDumpsPlugin::JobDescriptor { path };
    }
}

CoreDumpCleanerWorker::CoreDumpCleanerWorker(CoreDumpsPlugin *plugin)
    : WorkerObject(plugin)
{

}

CoreDumpCleanerWorker::Action CoreDumpCleanerWorker::actionForFile(const QFileInfo &file) const
{
    // Returns what we should do with the file.
    QDateTime creationTime = file.birthTime();
    chrono::seconds ageInSeconds(creationTime.secsTo(QDateTime::currentDateTimeUtc()));
    const int ageInHours = ageInSeconds.count() / 3600.0;
    const int ageInDays = ageInHours / 24.0;

    if (ageInDays > 7)
        return Action_Delete;

    if (ageInHours > 5 && !m_plugin->m_fileService->isCompressed(file))
        return Action_Compress;

    return Action_None;
}

CoreDumpsPlugin::CoreDumpsPlugin()
    : PluginInterface("coredumps", chrono::hours(1))
{
}

QString CoreDumpsPlugin::name() const
{
    return "Core Dumps";
}

QString CoreDumpsPlugin::shortName() const
{
    return "CoreDumps";
}

void CoreDumpsPlugin::start_impl()
{
    m_timer.start();
    work();
}

void CoreDumpsPlugin::work_impl()
{
    auto worker = new CoreDumpCleanerWorker(this);
    worker->loadJobDescriptors();
    startInWorkerThread(worker, &CoreDumpCleanerWorker::work);
}
