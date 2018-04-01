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

#ifndef BACKUP_PLUGIN_H
#define BACKUP_PLUGIN_H

#include "PluginInterface.h"

#include <QObject>
#include <QVector>

class QFileInfo;

class BackupPlugin : public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "smartins.dog.PluginInterface/v1.0.0")
    Q_INTERFACES(PluginInterface)

public:

    struct BackupItem {
        typedef QVector<BackupItem> List;
        QString name;
        QString source;
        QString destination;
        bool encrypt;
    };

    BackupPlugin();
    QString name() const override;
    QString shortName() const override;
    QString identifier() const override { return "backup"; }
    void start() override;

protected:
    void work_impl() override;

private:
    void loadJson();
    BackupItem::List m_backupItems;
};

class Backuper : public QObject {
    Q_OBJECT
public:
    explicit Backuper(BackupPlugin *q, const BackupPlugin::BackupItem::List &items);

public Q_SLOTS:
    void backup();

private:
     BackupPlugin::BackupItem::List m_backupItems;
     BackupPlugin *const q;
};

#endif
