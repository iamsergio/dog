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

#include "kernel.h"
#include "PluginInterface.h"
#include "logger.h"

#include <QSystemTrayIcon>
#include <QtWidgets>

Kernel::Kernel()
    : QObject()
    , m_systrayIcon(new QSystemTrayIcon(QIcon(), this))
    , m_logger(new Logger())
{
    m_configPath = qgetenv("DOG_CONFIG_PATH");
    if (m_configPath.isEmpty() || !QFile::exists(m_configPath)) {
        qWarning() << "DOG_CONFIG_PATH needs to be set and point to a valid path";
        m_valid = false;
        return;
    }

    loadPlugins();
    startPlugins();
    setupTrayIcon();

    m_logger->resize(600, 600);
}

Kernel::~Kernel()
{
    delete m_logger;
}

void Kernel::log(const QString &text)
{
    auto plugin = qobject_cast<PluginInterface*>(sender());
    Q_ASSERT(plugin);
    m_logger->log(plugin->shortName() + " " + text);
}

void Kernel::setupTrayIcon()
{
    auto menu = new QMenu();

    menu->addSeparator();
    for (auto p : m_plugins) {
        auto m = new QMenu(p->name());
        menu->addMenu(m);
    }

    QAction * action = menu->addAction("Show Log");
    connect(action, &QAction::triggered, [this] {
        m_logger->show();
    });

    menu->addSeparator();
    action = menu->addAction("Quit");
    connect(action, &QAction::triggered, [] {
        qApp->quit();
    });

    m_systrayIcon->setContextMenu(menu);
    m_systrayIcon->show();
}

void Kernel::loadPlugins()
{
    QDir pluginsDir = qApp->applicationDirPath() + "/plugins/";
    foreach (const QString &fileName, pluginsDir.entryList(QDir::Files)) {
        qDebug() << "trying " << pluginsDir.absoluteFilePath(fileName);
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        if (QObject *pluginObject = loader.instance()) {
            if (auto p = qobject_cast<PluginInterface*>(pluginObject)) {
                m_plugins << p;
                connect(p, &PluginInterface::log, this, &Kernel::log);
            }
        }
    }
}

void Kernel::startPlugins()
{
    for (auto p : m_plugins) {
        p->start();
        m_logger->log(QString("%1 started").arg(p->shortName()));
    }
}
