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

#ifndef DOG_PLUGININTERFACE_H
#define DOG_PLUGININTERFACE_H

#include <QString>
#include <QList>
#include <QtPlugin>
#include <QVariantMap>
#include <QObject>
#include <QTimer>
#include <QThread>

class FileService;

class PluginInterface : public QObject
{
    Q_OBJECT
public:
    typedef QList<PluginInterface*> List;

    PluginInterface();
    virtual ~PluginInterface() { }

    bool isValid() const;
    bool isWorking() const;

    //virtual bool enabled() const = 0;
    //virtual void setEnabled(bool enabled) = 0;
    virtual QString name() const = 0;
    virtual QString shortName() const = 0;
    virtual QString identifier() const = 0;
    virtual void start() = 0;

signals:
    void log(const QString &);

protected:
    QString qrcPath() const;
    QString configFile() const;
    QVariantMap readConfig() const;

    template <typename Slot>
    QThread *startInWorkerThread(QObject *worker, Slot slot);

    void work();
    virtual void work_impl() = 0;

    FileService *const m_fileService;
    QTimer m_timer;
private:
    Q_DISABLE_COPY(PluginInterface);
    class Private;
    Private *const d;
};

Q_DECLARE_INTERFACE(PluginInterface, "smartins.dog.PluginInterface/v1.0")

#endif
