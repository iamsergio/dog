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
        if (!QFile::exists(m_config_path)) {
            qWarning() << "DOG_CONFIG_PATH needs to be set and point to a valid path";
            qApp->quit();
        }
    }

    const QString m_config_path;
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
    : d(new Private(qgetenv("DOG_CONFIG_PATH")))
{
}

QString PluginInterface::qrcPath() const
{
    return QString(":/%1/").arg(shortName());
}

QString PluginInterface::configFile() const
{
    return d->m_config_path + "conf.json";
}

QVariantMap PluginInterface::readConfig() const
{
    return readJson(configFile());
}
