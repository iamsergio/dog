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
#include <QLoggingCategory>

class FileService;

class PluginInterface : public QObject
{
    Q_OBJECT
public:
    typedef QList<PluginInterface*> List;

    PluginInterface(const QByteArray &id);
    PluginInterface(const QByteArray &id, std::chrono::milliseconds timerInterval);
    virtual ~PluginInterface() { }

    QString identifier() const;

    bool isValid() const;
    bool isWorking() const;

    //virtual bool enabled() const = 0;
    //virtual void setEnabled(bool enabled) = 0;
    virtual QString name() const = 0;
    virtual QString shortName() const = 0;
    virtual void start() = 0;

    FileService *fileService() const { return m_fileService; }

protected:
    QString qrcPath() const;
    QString configFile() const;
    QVariantMap readConfig() const;

    template <typename Obj, typename Slot>
    QThread *startInWorkerThread(Obj *worker, Slot slot)
    {
        auto thread = new QThread();
        worker->moveToThread(thread);
        connect(thread, &QThread::started, worker, slot);
        connect(worker, &QObject::destroyed, thread, &QThread::quit);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        thread->start();
        return thread;
    }

    void work();
    virtual void work_impl() = 0;

    FileService *const m_fileService;
    QTimer m_timer;
private:
    Q_DISABLE_COPY(PluginInterface);
    class Private;
    Private *const d;
public:
    QLoggingCategory category;
};

Q_DECLARE_INTERFACE(PluginInterface, "smartins.dog.PluginInterface/v1.0")

#endif
