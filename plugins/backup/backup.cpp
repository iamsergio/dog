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

Backuper::Backuper(BackupPlugin *q, const BackupPlugin::BackupItem::List &items)
    : QObject(nullptr)
    , m_encriptionCommand(qgetenv("DOG_ENCRYPT_COMMAND"))
    , m_backupItems(items)
    , q(q)
{
}

void Backuper::backup()
{
    if (m_encriptionCommand.isEmpty())
        return;

    for (const auto &item : m_backupItems) {
        QString tarFilename;
        if (!q->fileService()->tarDirectory(item.source, tarFilename)) {
            deleteLater();
            return;
        }

        QString gpgFilename;
        if (item.encrypt) {
            if (!q->fileService()->encryptFile(tarFilename, gpgFilename)) {
                deleteLater();
                return;
            }
        } else {
           gpgFilename = tarFilename;
        }

        if (!q->fileService()->uploadFile(gpgFilename, item.destination)) {
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
    loadJson();
    m_timer.start();
    work();
}

void BackupPlugin::work_impl()
{
    auto worker = new Backuper(this, m_backupItems);
    startInWorkerThread(worker, &Backuper::backup);
}

void BackupPlugin::loadJson()
{
    QVariantMap json = readConfig();
    auto items = json.value("items").toList();
    for (const QVariant &i : items) {
        QVariantMap item = i.toMap();
        auto name = item.value("name").toString();
        auto source = item.value("source").toString();
        auto destination = item.value("destination").toString();
        auto encrypt = item.value("encrypt").toBool();

        m_backupItems.append({name, source, destination, encrypt });
    }

     qCDebug(category) << QString("Loaded %1 backup items").arg(m_backupItems.size());
}
