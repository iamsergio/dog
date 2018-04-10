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
#include <QDebug>
#include <QDir>

FileService::FileService(PluginInterface *parent)
    : QObject(parent)
    , q(parent)
    , category(parent->category)
{
}

bool FileService::isCompressed(const QFileInfo &file) const
{
    // Returns true if the file is compressed. Which for us means having a zsdt extension
    return file.suffix() == "zst";
}

bool FileService::removeFile(const QString &file)
{
    qCDebug(category) << "FileService::removeFile: removing " + file;
    if (!QFile::remove(file)) {
        qCWarning(category) << "FileService::removeFile: error removing file";
        return false;
    }

    return true;
}

bool FileService::tarDirectory(const QString &path, QString &outputFileName)
{
    outputFileName.clear();

    qCDebug(category) << "FileService::tarDirectory: tarring" + path;
    QFileInfo info(path);
    if (!info.exists()) {
        qCWarning(category) << "FileService::tarDirectory: directory doesn't exist";
        return false;
    }

    if (!info.isDir()) {
        qCWarning(category) << "FileService::tarDirectory: path isn't a directory";
        return false;
    }

    QDir dir = info.dir();
    dir.cdUp();

    const QString tar_filename = dir.path() + "/" + info.dir().dirName() + ".tar";

    bool success = QProcess::execute(QString("tar cvzf %1 %2").arg(tar_filename, path)) == 0;
    if (!success) {
        qCWarning(category) << "FileService::tarDirectory: error executing tar";
        return false;
    }

    if (!compressFile(tar_filename, outputFileName))
        return false;

    return true;
}

bool FileService::encryptFile(const QString &file, QString &outFile, bool remove_original)
{
    outFile.clear();
    qCDebug(category) << "FileService::encryptFile: " + file;
    static QString encryptionCommand = qgetenv("DOG_ENCRYPT_COMMAND");
    if (encryptionCommand.isEmpty())
        return false;

    const QString gpgFilename = file + ".gpg";
    if (QFile::exists(gpgFilename)) {
        if (!QFile::remove(gpgFilename)) {
            qCWarning(category) << QString("FileService::encryptFile: failed to remove file %1").arg(gpgFilename);
            return false;
        }
    }

    const QString command = encryptionCommand.arg(file);

    bool success = QProcess::execute(command) == 0;
    if (!success) {
        qCWarning(category) << QString("FileService::encryptFile: error executing %1").arg(command);
        return false;
    }

    success = !remove_original || removeFile(file);
    if (!success)
        return false;

    outFile = gpgFilename;
    return true;
}

bool FileService::uploadFile(const QString &file, const QString &destiny, bool remove_original)
{
    qCDebug(category) << "FileService::uploadFile: " + file + " to " + destiny;

    if (!QFile::exists(file)) {
        qCWarning(category) << "FileService::uploadFile: File doesn't exist" + file;
        return false;
    }

    const QString command = QString("scp %1 %2").arg(file, destiny);
    qDebug() << "command: " << command;
    bool success = QProcess::execute(command) == 0;
    if (!success) {
        qCWarning(category) << "FileService::uploadFile: Error running" + command;
        return false;
    }

    success = !remove_original || removeFile(file);
    return true;
}

bool FileService::compressFile(const QString &file, QString &outFile, bool remove_original)
{
    outFile.clear();
    // Methods will already be running on a separate thread, no need for async.
    qCDebug(category) << "FileService::compressFile: compressing file " + file + "...";

    const QString zstFile = file + ".zst";
    if (QFile::exists(zstFile)) {
        if (!QFile::remove(zstFile)) {
            qCWarning(category) << QString("FileService::compressFile: failed to remove file %1").arg(zstFile);
            return false;
        }
    }

    bool success = QProcess::execute(QString("zstd %1").arg(file)) == 0;
    if (!success) {
        qCWarning(category) << "FileService::compressFile: error compressing file";
        return false;
    }

    success = !remove_original || removeFile(file);
    if (!success)
        return false;

    outFile = zstFile;
    return true;
}
