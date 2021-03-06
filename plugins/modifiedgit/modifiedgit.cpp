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

#include "modifiedgit.h"

#include <QDebug>
#include <QThread>
#include <QDir>
#include <QDateTime>
#include <QProcess>

#include <chrono>

using namespace std;

ModifiedGitPlugin::ModifiedGitPlugin()
    : PluginInterface("modifiedgit", chrono::hours(1))
{
}

QString ModifiedGitPlugin::name() const
{
    return "Modified Git";
}

QString ModifiedGitPlugin::shortName() const
{
    return "ModifiedGit";
}

void ModifiedGitPlugin::start_impl()
{
    m_timer.start();
    work();
}

void ModifiedGitPlugin::work_impl()
{
    //auto worker = new BuildDirCleaner(m_jobs, this);
    //startInWorkerThread(worker, &BuildDirCleaner::cleanAll);
}
