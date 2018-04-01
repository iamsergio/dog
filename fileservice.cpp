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

#include "fileservice.h"
#include "PluginInterface.h"

#include <QFileInfo>
#include <QFile>
#include <QProcess>

FileService::FileService(PluginInterface *parent)
    : QObject(parent)
    , q(parent)
{
}

bool FileService::isCompressed(const QFileInfo &file) const
{
    // Returns true if the file is compressed. Which for us means having a zsdt extension
    return file.suffix() == "zst";
}

bool FileService::removeFile(const QString &file)
{
    emit q->log("FileService::removeFile: removing " + file);
    if (!QFile::remove(file)) {
        emit q->log("FileService::removeFile: error removing file");
        return false;
    }

    return true;
}

bool FileService::tarDirectory(const QString &path)
{
    emit q->log("FileService::tarDirectory: tarring" + path);
    QFileInfo info(path);
    if (!info.exists()) {
        emit q->log("FileService::tarDirectory: directory doesn't exist");
        return false;
    }

    if (!info.isDir()) {
        emit q->log("FileService::tarDirectory: path isn't a directory");
        return false;
    }

    const QString tar_filename = info.path() + "/" + info.baseName() + ".tar";

    bool success = QProcess::execute(QString("tar cvzf %1 %2").arg(path, tar_filename)) == 0;
    if (!success) {
        emit q->log("FileService::tarDirectory: error executing tar");
        return false;
    }

    if (!compressFile(tar_filename))
        return false;

    return true;
}

bool FileService::compressFile(const QString &file, bool remove_original)
{
    // Methods will already be running on a separate thread, no need for async.
    emit q->log("FileService::compressFile: compressing file " + file + "...");
    bool success = QProcess::execute(QString("zstd %1").arg(file)) == 0;

    if (!success) {
        emit q->log("FileService::compressFile: error compressing file");
        return false;
    }

    return !remove_original || removeFile(file);
}
