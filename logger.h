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

#ifndef DOG_LOGGER_H
#define DOG_LOGGER_H

#include <QWidget>

class QTextEdit;
class Logger : public QWidget
{
    Q_OBJECT
public:
    explicit Logger(QWidget *parent = nullptr);

public slots:
    void log(const QString &text);
private:
    QTextEdit *const m_textEdit;
};

#endif
