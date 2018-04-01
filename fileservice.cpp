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
    return QFile::remove(file);
}

bool FileService::compressFile(const QString &file)
{
    // Methods will already be running on a separate thread, no need for async.
    emit q->log("FileService::compressFile: compressing file " + file + "...");
    bool success = QProcess::execute(QString("zstd %1").arg(file)) == 0;
    return success && QFile::remove(file);
}
