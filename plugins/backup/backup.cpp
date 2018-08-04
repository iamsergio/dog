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

#include "backup.h"
#include "fileservice.h"

#include <QDebug>
#include <QThread>
#include <QDir>
#include <QDateTime>
#include <QProcess>

#include <chrono>

using namespace std;

BackupWorker::BackupWorker(PluginInterface *q)
    : WorkerObject(q)
    , m_encriptionCommand(qgetenv("DOG_ENCRYPT_COMMAND"))
{
}

void BackupWorker::work()
{
    if (m_encriptionCommand.isEmpty())
        return;

    for (const auto &item : m_jobDescriptors) {
        QString tarFilename;
        if (!fileService()->tarDirectory(item.source, tarFilename)) {
            deleteLater();
            return;
        }

        QString gpgFilename;
        if (item.encrypt) {
            if (!fileService()->encryptFile(tarFilename, gpgFilename)) {
                deleteLater();
                return;
            }
        } else {
           gpgFilename = tarFilename;
        }

        if (!fileService()->uploadFile(gpgFilename, item.destination)) {
            deleteLater();
            return;
        }
    }

    deleteLater();
}

BackupPlugin::BackupPlugin()
    : PluginInterface("backup", chrono::hours(24 * 2))
{
}

QString BackupPlugin::name() const
{
    return "Backup Plugin";
}

QString BackupPlugin::shortName() const
{
    return "BackupPlugin";
}

void BackupPlugin::start_impl()
{
    m_timer.start();
    work();
}

void BackupPlugin::work_impl()
{
    auto worker = new BackupWorker(this);
    startInWorkerThread(worker, &BackupWorker::work);
}

void BackupWorker::loadJobDescriptors()
{
    auto items = m_plugin->jobDescriptors();
    for (const QVariant &i : items) {
        QVariantMap item = i.toMap();
        auto name = item.value("name").toString();
        auto source = item.value("source").toString();
        auto destination = item.value("destination").toString();
        auto encrypt = item.value("encrypt").toBool();

        m_jobDescriptors.append({name, source, destination, encrypt });
    }

     qCDebug(m_plugin->category) << QString("Loaded %1 backup items").arg(m_jobDescriptors.size());
}
