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

#ifndef FILESERVICE_H
#define FILESERVICE_H

#include "dog_export.h"

#include <QObject>
#include <QLoggingCategory>

class QFileInfo;
class PluginInterface;

class DOG_EXPORT FileService : public QObject
{
    Q_OBJECT
public:
    explicit FileService(PluginInterface *parent);
    bool isCompressed(const QFileInfo &file) const;
    bool compressFile(const QString &file, QString &outFile, bool remove_original = true);
    bool removeFile(const QString &file);
    bool tarDirectory(const QString &path, QString &outputFileName);
    bool encryptFile(const QString &file, QString &outFilename, bool remove_original = true);
    bool uploadFile(const QString &file, const QString &destiny, bool remove_original = true);
private:
    PluginInterface *const q;
    const QLoggingCategory &category;
};

#endif
