#include "task.h"
#include "ui_task.h"
#include <QDebug>

Task::Task(int id, QString text, bool local, QFont font, QColor localColor, QColor onlineColor, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Task)
{
    m_id = id;
    ui->setupUi(this);
    if(local)
        this->setStyleSheet("background-color: " + localColor.name() + ";");
    else
        this->setStyleSheet("background-color: " + onlineColor.name() + ";");

    this->ui->plainTextEdit->document()->setDefaultFont(font);

    this->ui->plainTextEdit->document()->setPlainText(text);
    m_local = local;
}


int Task::getId(){
    return m_id;
}

QString Task::getText()
{
    return this->ui->plainTextEdit->document()->toPlainText();
}

bool Task::isLocal()
{
    return m_local;
}

void Task::setText(QString text)
{
    this->ui->plainTextEdit->document()->setPlainText(text);
}



Task::~Task()
{
    this->ui->plainTextEdit->clearFocus();
    delete ui;
}

void Task::on_plainTextEdit_textChanged()
{
    emit focusChanged(this);
}
