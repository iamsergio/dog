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

#include "PluginInterface.h"
#include "fileservice.h"

#include <QFile>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QDebug>

class PluginInterface::Private
{
public:
    Private(const QString &config_path)
        : m_config_path(config_path)
    {
    }

    const QString m_config_path;
    bool m_valid = true;
    bool working = false;
};

static QVariantMap readJson(const QString &filename)
{
    QFile f(filename);
    f.open(QFile::ReadOnly);
    QByteArray fileContents = f.readAll();
    QJsonParseError jsonError; // TODO handle errors
    QJsonDocument document = QJsonDocument::fromJson(fileContents, &jsonError);
    return document.toVariant().toMap();
}

PluginInterface::PluginInterface()
    : m_fileService(new FileService(this))
    , d(new Private(qgetenv("DOG_CONFIG_PATH")))
{
    connect(&m_timer, &QTimer::timeout, this, &PluginInterface::work_impl);
}

bool PluginInterface::isValid() const
{
    return d->m_valid;
}

bool PluginInterface::isWorking() const
{
    return d->working;
}

QString PluginInterface::qrcPath() const
{
    return QString(":/%1/").arg(shortName());
}

QString PluginInterface::configFile() const
{
    return QString("%1/%2/conf.json").arg(d->m_config_path, identifier());
}

QVariantMap PluginInterface::readConfig() const
{
    return readJson(configFile());
}

void PluginInterface::work()
{
    Q_ASSERT(!isWorking());
    d->working = true;
    work_impl();
    d->working = false;
}

template<typename Slot>
QThread *PluginInterface::startInWorkerThread(QObject *worker, Slot slot)
{
    auto thread = new QThread();
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, slot);
    connect(worker, &QObject::destroyed, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
    return thread;
}
