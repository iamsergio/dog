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

#ifndef COREDUMPS_PLUGIN_H
#define COREDUMPS_PLUGIN_H

#include "PluginInterface.h"
#include <QObject>

class QFileInfo;
class CoreDumpsPlugin;
class CoreDumpCleaner : public QObject
{
   Q_OBJECT
public:
    enum Action {
        Action_None = 0,
        Action_Compress,
        Action_Delete
    };
    CoreDumpCleaner(CoreDumpsPlugin *q)
        : QObject()
        , q(q) {}
    void clean();

signals:
   void finished();

private:
   Q_ENUM(Action)
   Action actionForFile(const QFileInfo &file) const;
   CoreDumpsPlugin *const q;
};

class CoreDumpsPlugin : public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "smartins.dog.PluginInterface/v1.0.0")
    Q_INTERFACES(PluginInterface)

public:
    CoreDumpsPlugin();
    QString name() const override;
    QString shortName() const override;
    void start() override;

protected:
    void work_impl() override;

private:
    friend class CoreDumpCleaner;
};

#endif
