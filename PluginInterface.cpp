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
#include "kernel.h"

#include <QFile>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QDebug>
#include <QAction>

using namespace std;

class PluginInterface::Private
{
public:
    Private(const QString &config_path, const QByteArray &id, PluginInterface *q)
        : q(q)
        , config_path(config_path)
        , id(id)
        , loggingCategoryName(QByteArray("dog.plugins." + id))
        , startAction(new QAction(tr("Start")))
        , stopAction(new QAction(tr("Stop")))
    {
        connect(startAction, &QAction::triggered, q, &PluginInterface::start);
        connect(q, &PluginInterface::started, startAction, [this] {
            startAction->setEnabled(false);
            stopAction->setEnabled(true);
        });

        connect(q, &PluginInterface::stopped, startAction, [this] {
            startAction->setEnabled(true);
            stopAction->setEnabled(false);
        });

        stopAction->setEnabled(false);
    }

    PluginInterface *const q;
    const QString config_path;
    const QString id;
    const QByteArray loggingCategoryName;
    QVariantList jobDescriptors;
    QAction *startAction;
    QAction *stopAction;
    bool valid = true;
    bool working = false;
    bool autoStarts = false;
};

void PluginInterface::readJson(const QString &filename)
{
    if (!QFile::exists(filename))
        return;

    QFile f(filename);
    f.open(QFile::ReadOnly);
    QByteArray fileContents = f.readAll();
    QJsonParseError jsonError; // TODO handle errors
    QJsonDocument document = QJsonDocument::fromJson(fileContents, &jsonError);
    const auto map = document.toVariant().toMap();
    d->autoStarts = map.value("autoStarts", false).toBool();

    // The custom/per-plugin format is under "jobs"
    d->jobDescriptors = map.value("jobs").toList();
    d->jobDescriptors += map.value(QStringLiteral("jobs_%1").arg(Kernel::osStr())).toList();
    d->jobDescriptors += map.value(QStringLiteral("jobs_%1").arg(Kernel::osTypeStr())).toList();
}

PluginInterface::PluginInterface(const QByteArray &id)
    : m_fileService(new FileService(this))
    , d(new Private(qgetenv("DOG_CONFIG_PATH"), id, this))
    , category(d->loggingCategoryName.constData())
{
    connect(&m_timer, &QTimer::timeout, this, &PluginInterface::work_impl);
    readJson(configFile());
}

PluginInterface::PluginInterface(const QByteArray &id, chrono::milliseconds timerInterval)
    : m_fileService(new FileService(this))
    , d(new Private(qgetenv("DOG_CONFIG_PATH"), id, this))
    , category(d->loggingCategoryName.constData())
{
    m_timer.setInterval(timerInterval);
    readJson(configFile());
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

QAction *PluginInterface::startAction() const
{
    return d->startAction;
}

QAction *PluginInterface::stopAction() const
{
    return d->stopAction;
}

void PluginInterface::start()
{
    qCDebug(category) << "Started";
    start_impl();

    emit started();
}

void PluginInterface::stop()
{
    // TODO
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

QVariantList PluginInterface::jobDescriptors() const
{
    return d->jobDescriptors;
}

bool PluginInterface::autoStarts() const
{
    return d->autoStarts;
}

void PluginInterface::work()
{
    Q_ASSERT(!isWorking());
    qCDebug(category) << "Work started";
    d->working = true;
    work_impl();
    d->working = false;
}
