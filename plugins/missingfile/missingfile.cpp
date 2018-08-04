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

#include "missingfile.h"

#include <QDebug>
#include <QThread>
#include <QDir>
#include <QDateTime>
#include <QProcess>

#include <chrono>

using namespace std;

MissingFilePlugin::MissingFilePlugin()
    : PluginInterface("missingfile", chrono::hours(24))
{
}

QString MissingFilePlugin::name() const
{
    return "Missing File";
}

QString MissingFilePlugin::shortName() const
{
    return "MissingFile";
}

void MissingFilePlugin::start_impl()
{
    m_timer.start();
    work();
}

void MissingFilePlugin::work_impl()
{
    auto worker = new MissingFileWorker(this);
    worker->loadJobDescriptors();
    startInWorkerThread(worker, &MissingFileWorker::work);
}

MissingFileWorker::MissingFileWorker(PluginInterface *plugin)
    : WorkerObject(plugin)
{
}

void MissingFileWorker::work()
{
    for (auto &descriptor : m_jobDescriptors) {
        if (!QFile::exists(descriptor.path)) {
            m_plugin->emitVisualWarning(QStringLiteral("File does not exist %1").arg(descriptor.path));
        }
    }
}

void MissingFileWorker::loadJobDescriptors()
{
    const QVariantList dirs = m_plugin->jobDescriptors();
    for (const QVariant &fileV : dirs) {
        QVariantMap fileMap = fileV.toMap();
        QString path = fileMap.value("path").toString();
        if (!path.isEmpty()) {
            m_jobDescriptors << MissingFile::JobDescriptor{ path };
        }
    }
}
