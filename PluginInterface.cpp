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

using namespace std;

class PluginInterface::Private
{
public:
    Private(const QString &config_path, const QByteArray &id)
        : config_path(config_path)
        , id(id)
        , loggingCategoryName(QByteArray("dog.plugins." + id))
    {
    }

    const QString config_path;
    const QString id;
    const QByteArray loggingCategoryName;
    bool valid = true;
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

PluginInterface::PluginInterface(const QByteArray &id)
    : m_fileService(new FileService(this))
    , d(new Private(qgetenv("DOG_CONFIG_PATH"), id))
    , category(d->loggingCategoryName.constData())
{
    connect(&m_timer, &QTimer::timeout, this, &PluginInterface::work_impl);
}

PluginInterface::PluginInterface(const QByteArray &id, chrono::milliseconds timerInterval)
    : m_fileService(new FileService(this))
    , d(new Private(qgetenv("DOG_CONFIG_PATH"), id))
    , category(d->loggingCategoryName.constData())
{
    m_timer.setInterval(timerInterval);
}

QString PluginInterface::identifier() const
{
    return d->id;
}

bool PluginInterface::isValid() const
{
    return d->valid;
}

bool PluginInterface::isWorking() const
{
    return d->working;
}

void PluginInterface::start()
{
    qCDebug(category) << "Started";
    start_impl();
}

void PluginInterface::emitVisualWarning(const QString &text)
{
    qCWarning(category) << text;
    emit visualWarning(text);
}

QString PluginInterface::qrcPath() const
{
    return QString(":/%1/").arg(shortName());
}

QString PluginInterface::configFile() const
{
    return QString("%1/%2/conf.json").arg(d->config_path, identifier());
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
