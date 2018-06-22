#include "task.h"
#include "ui_task.h"
#include <QDebug>

Task::Task(int id, QString text, bool local, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Task)
{
    m_id = id;
    ui->setupUi(this);
    if(local)
        this->setStyleSheet("background-color: #e5e500;");
    else
        this->setStyleSheet("background-color: yellow;");
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



Task::~Task()
{
    this->ui->plainTextEdit->clearFocus();
    delete ui;
}

void Task::on_plainTextEdit_textChanged()
{
    emit focusChanged(this);
}
