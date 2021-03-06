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
#include <QDir>

class QFileInfo;

class BuildDirCleanerPlugin : public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "smartins.dog.PluginInterface/v1.0.0")
    Q_INTERFACES(PluginInterface)

public:

    struct JobDescriptor
    {
        typedef QVector<JobDescriptor> List;

        enum Method {
            Method_None = 0,
            Method_Rm,
            Method_RmChilds,
            Method_GitClean
        };

        QString path;
        QString pattern;
        Method method;

        static Method methodFromString(const QString &s) {
            if (s == "rm-childs")
                return Method_RmChilds;
            if (s == "rm")
                return Method_Rm;
            if (s == "git-clean")
                return Method_GitClean;

            return Method_None;
        }
    };

    BuildDirCleanerPlugin();
    QString name() const override;
    QString shortName() const override;
    void start_impl() override;

protected:
    void work_impl() override;
};


class BuildDirCleanerWorker : public WorkerObject<BuildDirCleanerPlugin::JobDescriptor>
{
   Q_OBJECT
public:
    enum Action {
        Action_None = 0,
        Action_Compress,
        Action_Delete
    };
    explicit BuildDirCleanerWorker(BuildDirCleanerPlugin *plugin);
    void work() override;
    void loadJobDescriptors() override;
    void cleanOne(const BuildDirCleanerPlugin::JobDescriptor &);
    void runGitClean(const BuildDirCleanerPlugin::JobDescriptor &);
    void runRmChilds(const BuildDirCleanerPlugin::JobDescriptor &);
    void runRm(QDir &dir);

signals:
   void finished();

private:
   Q_ENUM(Action)
  /* bool isCompressed(const QFileInfo &file) const;
   void compressFile(const QString &file);
   void removeFile(const QString &file);
   Action actionForFile(const QFileInfo &file) const;*/
};

#endif
