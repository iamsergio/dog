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

#include "logger.h"

#include <QtWidgets>
#include <QDateTime>

Logger::Logger(QWidget *parent)
    : QWidget(parent)
    , m_textEdit(new QTextEdit(this))
{
    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_textEdit);
    m_textEdit->setReadOnly(true);
}

void Logger::log(const QString &text)
{
    QString now = QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm::ss");
    m_textEdit->setText(m_textEdit->toPlainText() + "[" + now + "] " +  text + "\n");
}
