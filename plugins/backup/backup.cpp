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

#include <QDebug>
#include <QThread>
#include <QDir>
#include <QDateTime>
#include <QProcess>

#include <chrono>

using namespace std;

Backuper::Backuper(const BackupPlugin::BackupItem::List &items, QObject *parent)
    : QObject(parent)
    , m_backupItems(items)
{
}

void Backuper::backup()
{
    for (const auto &item : m_backupItems) {

    }
}

BackupPlugin::BackupPlugin()
    : PluginInterface(chrono::hours(24 * 2))
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

void BackupPlugin::start()
{
    loadJson();
    m_timer.start();
    work();
}

void BackupPlugin::work_impl()
{
    auto worker = new Backuper(m_backupItems, this);
    startInWorkerThread(worker, &Backuper::backup);
}

void BackupPlugin::loadJson()
{
    QVariantMap json = readConfig();
    auto items = json.value("items").toList();
    for (const QVariant &i : items) {
        QVariantMap item = i.toMap();
        auto name = item.value("name").toString();
        auto destination = item.value("destination").toString();
        auto encrypt = item.value("encrypt").toBool();

        m_backupItems.append({name, destination, encrypt });
    }

    emit log(QString("Loaded %1 backup items").arg(m_backupItems.size()));
}
