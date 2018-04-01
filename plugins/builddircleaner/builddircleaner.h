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

#ifndef BUILDDIR_CLEANER_PLUGIN_H
#define BUILDDIR_CLEANER_PLUGIN_H

#include "PluginInterface.h"

#include <QObject>
#include <QVector>

class QFileInfo;
class BuildDirCleanerPlugin;

struct JobDescriptor
{
    typedef QVector<JobDescriptor> List;
    QString path;
    QString pattern;
};

class BuildDirCleaner : public QObject
{
   Q_OBJECT
public:
    enum Action {
        Action_None = 0,
        Action_Compress,
        Action_Delete
    };
    BuildDirCleaner(const JobDescriptor::List &jobs, BuildDirCleanerPlugin *q)
        : QObject()
        , m_jobs(jobs)
        , q(q) {}
    void cleanAll();
    void cleanOne(const JobDescriptor &);

signals:
   void finished();

private:
   Q_ENUM(Action)
  /* bool isCompressed(const QFileInfo &file) const;
   void compressFile(const QString &file);
   void removeFile(const QString &file);
   Action actionForFile(const QFileInfo &file) const;*/
   const JobDescriptor::List m_jobs;
   BuildDirCleanerPlugin *const q;
};


class BuildDirCleanerPlugin : public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "smartins.dog.PluginInterface/v1.0.0")
    Q_INTERFACES(PluginInterface)

public:
    BuildDirCleanerPlugin();
    QString name() const override;
    QString shortName() const override;
    QString identifier() const override { return "builddircleaner"; }
    void start() override;

protected:
    void work_impl() override;

private:
    JobDescriptor::List loadJson() const;
    const JobDescriptor::List m_jobs;
};

#endif
