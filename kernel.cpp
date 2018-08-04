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
#include <QPointer>
#include <QTimer>

static QtMessageHandler s_originalMessageHandler;
static QPointer<Kernel> s_kernel;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    s_originalMessageHandler(type, context, msg); // Goes to stderr

    const QString typeStr = [type] {
        switch (type) {
        case QtDebugMsg:
            return "DEBUG: ";
        case QtWarningMsg:
            return "WARNING: ";
        case QtCriticalMsg:
            return "CRITICAL: ";
        case QtFatalMsg:
            return "FATAL: ";
        case QtInfoMsg:
            return "INFO: ";
        }
        return "";
    }();

    const QString endmsg = typeStr + msg;
    const QString cat = context.category;

    // And also to our widget logger. Use queued connection so it logs in main thread
    QTimer::singleShot(0, s_kernel->logger(), [endmsg, cat] {
        s_kernel->logger()->log(endmsg, cat);
    });

}

Kernel::Kernel()
    : QObject()
    , m_systrayIcon(new QSystemTrayIcon(QIcon(), this))
    , m_logger(new Logger())
{
    s_kernel = this;
    s_originalMessageHandler = qInstallMessageHandler(myMessageOutput);
    m_configPath = qgetenv("DOG_CONFIG_PATH");
    if (m_configPath.isEmpty() || !QFile::exists(m_configPath)) {
        qWarning() << "DOG_CONFIG_PATH needs to be set and point to a valid path";
        m_valid = false;
        return;
    }

    loadPlugins();


    m_logger->resize(600, 600);
}

Kernel::~Kernel()
{
    delete m_logger;
}

void Kernel::start()
{
    startPlugins();
    setupTrayIcon();
}

void Kernel::listPlugins()
{
    qDebug() << "Available plugins:";
    for (auto p : m_plugins)
        qDebug() << "    " << p->identifier();
}

QWidget *Kernel::window() const
{
    const auto widgets = qApp->topLevelWidgets();
    return widgets.isEmpty() ? nullptr : widgets.first();
}

QString Kernel::osStr()
{
    switch (Kernel::os()) {
    case DogOS_Linux:
        return "linux";
    case DogOS_Windows:
        return "windows";
    case DogOS_macOS:
        return "macOS";
    default:
        return QString();
    }
}

QString Kernel::osTypeStr()
{
    switch (Kernel::osType()) {
    case DogOSType_Posix:
        return "posix";
    case DogOSType_NT:
        return "nt";
    default:
        return QString();
    }
}

Kernel::DogOS Kernel::os()
{
#if defined(Q_OS_LINUX)
    return DogOS_Linux;
#elif define(Q_OS_MACOS)
    return DogOS_macOS;
#elif defined(Q_OS_WIN)
    return DogOS_Windows;
#else
    return DogOS_None;
#endif
}

Kernel::DogOSType Kernel::osType()
{
    auto os = Kernel::os();
    if (os == DogOS_Linux || os == DogOS_macOS)
        return DogOSType_Posix;
    else if (os == DogOS_Windows)
        return DogOSType_NT;
    else
        return DogOSType_None;
}

void Kernel::onVisualWarning(const QString &text)
{
    // For now a message box
    QMessageBox::warning(window(), "warning", text);
}

void Kernel::setupTrayIcon()
{
    auto menu = new QMenu();

    menu->addSeparator();
    for (auto p : m_plugins) {
        auto m = new QMenu(p->name());
        m->addAction(p->startAction());
        m->addAction(p->stopAction());
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

QStringList Kernel::userSpeciciedPluginNames() const
{
    QStringList names;
    for (auto p : m_plugins) {
        if (qApp->arguments().contains(p->identifier()))
            names << p->identifier();
    }

    return names;
}

void Kernel::loadPlugins()
{
    QStringList pluginPaths;
    for (const QString &path : qApp->libraryPaths()) {
        pluginPaths += QStringLiteral("%1/../lib/dog/plugins/").arg(path);
        pluginPaths += QStringLiteral("%1/../lib64/dog/plugins/").arg(path);
        pluginPaths += QStringLiteral("%1/../lib32/dog/plugins/").arg(path);
    }

    pluginPaths += QStringLiteral("%1/plugins/").arg(qApp->applicationDirPath());

    for (const QString &pluginPath : pluginPaths) {
        QDir pluginsDir = pluginPath;
        qDebug() << "Looking in" << qApp->libraryPaths();
        foreach (const QString &fileName, pluginsDir.entryList(QDir::Files)) {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            if (QObject *pluginObject = loader.instance()) {
                if (auto p = qobject_cast<PluginInterface*>(pluginObject)) {
                    m_plugins << p;
                    connect(p, &PluginInterface::visualWarning, this, &Kernel::onVisualWarning);
                    qDebug() << "Loadded " << fileName;
                } else {
                    qWarning() << "Failed to load " << pluginsDir.absoluteFilePath(fileName);
                }
            } else {
                qWarning() << "Failed to load" << pluginsDir.absoluteFilePath(fileName);
            }
        }
    }
}

// Starts the plugins which have autoStart=true
void Kernel::startPlugins()
{
    const QStringList userSpecifiedPlugins = userSpeciciedPluginNames();
    for (auto p : m_plugins) {
        if (userSpecifiedPlugins.isEmpty() || userSpecifiedPlugins.contains(p->identifier())) {
            if (p->autoStarts()) {
                p->start();
                qDebug() << QString("%1 started").arg(p->shortName());
            }
        }
    }
}
