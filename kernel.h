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

#ifndef DOG_KERNEL_H
#define DOG_KERNEL_H

#include "PluginInterface.h"

#include <QObject>
class Logger;
class QSystemTrayIcon;

class Kernel : public QObject
{
    Q_OBJECT
public:
    Kernel();
    ~Kernel();
    bool isValid() const { return m_valid; }
private Q_SLOTS:
    void log(const QString &);
private:
    void loadPlugins();
    void startPlugins();
    void setupTrayIcon();
    QSystemTrayIcon *const m_systrayIcon;
    PluginInterface::List m_plugins;
    Logger *const m_logger;
    QString m_configPath;
    bool m_valid = true;
};

#endif
