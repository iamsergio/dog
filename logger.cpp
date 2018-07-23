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
    , m_tabWidget(new QTabWidget(this))
{
    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_tabWidget);

    m_defaultTextEdit = createTab("dog");
}

QTextEdit *Logger::textEditForCategory(const QString &category)
{
    if (category.isEmpty() || category == "default")
        return m_defaultTextEdit;

    for (int i = 0, e = m_tabWidget->count(); i < e; ++i) {
        if (auto textEdit = qobject_cast<QTextEdit*>(m_tabWidget->widget(i))) {
            if (m_tabWidget->tabText(i) == category)
                return textEdit;
        }
    }

    return createTab(category);
}

QTextEdit *Logger::createTab(const QString &category)
{
    auto textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    m_tabWidget->addTab(textEdit, category);
    return textEdit;
}

void Logger::log(const QString &text, const QString &category)
{
    QString cat = category;
    cat.replace("dog.plugins.", "");

    QString now = QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm::ss");
    QMutexLocker locker(&m_mutex);
    auto textEdit = textEditForCategory(cat);
    textEdit->setText(textEdit->toPlainText() + "[" + now + "] " +  text + "\n");
}
