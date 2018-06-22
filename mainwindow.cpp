#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>
#include <QPlainTextEdit>
#include <QNetworkReply>
#include "task.h"
#include <QJsonArray>
#include <QTimer>
#include <QMessageBox>
#include <QDataStream>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setStyleSheet("background-color: yellow;");
    updateTaskList();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTaskList()));
    timer->start(10000);

    QFile inputFile("save.task");
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            addLocalTask(line);
        }
        inputFile.close();
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent( QKeyEvent *k )
{
    if(k->key() == Qt::Key_N)
    {
        if(QApplication::keyboardModifiers() && Qt::ControlModifier)
        {
            addOnlineTask("-");
        }
    }
    if(k->key() == Qt::Key_L)
    {
        if(QApplication::keyboardModifiers() && Qt::ControlModifier)
        {
            addLocalTask("-");
        }
    }
}


void MainWindow::updateTaskList()
{
    QNetworkAccessManager networkManager;

    QUrl url("http://tech_todo_list.ms.kang.lan/task/all");
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setUrl(url);

    QNetworkReply* currentReply = networkManager.get(request);
    while(!currentReply->isFinished()) {
        qApp->processEvents();
    }
    QByteArray response_data = currentReply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response_data);
    QJsonArray tasks = jsonDoc.array();

    for(int i = 0 ; i < tasks.size() ; i++){
        QJsonObject obj = tasks.at(i).toObject();
        int id = obj.value("id").toInt();
        bool found = false;
        for(int j = 0 ; j < this->ui->scrollAreaWidgetContents->layout()->count() ; j++){
            Task* taskWidget = static_cast<Task*>(this->ui->scrollAreaWidgetContents->layout()->itemAt(j)->widget());
            if(taskWidget->getId() == id){
                found = true;
                break;
            }
        }
        if(!found)
        {
            Task* t = new Task(id, obj.value("task_txt").toString(), false);
            this->ui->scrollAreaWidgetContents->layout()->addWidget(t);
            connect(t, SIGNAL(focusChanged(Task*)), this, SLOT(focusChange(Task*)));
        }
    }

    for(int j = 0 ; j < this->ui->scrollAreaWidgetContents->layout()->count() ; j++){
        Task* taskWidget = static_cast<Task*>(this->ui->scrollAreaWidgetContents->layout()->itemAt(j)->widget());
        if(taskWidget->isLocal())
            continue;
        bool found = false;
        for(int i = 0 ; i < tasks.size() ; i++){
            QJsonObject obj = tasks.at(i).toObject();
            int id = obj.value("id").toInt();
            if(taskWidget->getId() == id){
                found = true;
                break;
            }
        }
        if(!found){
            taskWidget->clearFocus();
            this->ui->scrollAreaWidgetContents->layout()->removeWidget(taskWidget);
            taskWidget->setVisible(false);
            j=0;
        }
    }

    currentReply->deleteLater();

}

void MainWindow::focusChange(Task *taskWidget)
{

    if(taskWidget->getText() == ""){
        this->removeTask(taskWidget->getId());
    }
    else{
        this->modifyTask(taskWidget);
    }
}

void MainWindow::addOnlineTask(QString text)
{
    QNetworkAccessManager networkManager;

    QUrl url("http://tech_todo_list.ms.kang.lan/task");
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setUrl(url);

    QJsonObject json;
    json.insert("task_txt", text);
    json.insert("author", "valerian");
    json.insert("tasklist_id", 1);

    QNetworkReply* currentReply = networkManager.post(request, QJsonDocument(json).toJson());
    while(!currentReply->isFinished()) {
        qApp->processEvents();
    }

    QByteArray response_data = currentReply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response_data);
    currentReply->deleteLater();
    updateTaskList();
}

void MainWindow::addLocalTask(QString text)
{
    Task* t = new Task(-1, text, true);
    this->ui->scrollAreaWidgetContents->layout()->addWidget(t);
    connect(t, SIGNAL(focusChanged(Task*)), this, SLOT(focusChange(Task*)));
}

void MainWindow::modifyTask(Task* t)
{
    if(!t->isLocal()){
        QNetworkAccessManager networkManager;

        QUrl url("http://tech_todo_list.ms.kang.lan/task/edit");
        QNetworkRequest request;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setUrl(url);

        QJsonObject json;
        json.insert("task_txt", t->getText());
        json.insert("id", t->getId());

        QNetworkReply* currentReply = networkManager.post(request, QJsonDocument(json).toJson());
        while(!currentReply->isFinished()) {
            qApp->processEvents();
        }
        QByteArray response_data = currentReply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(response_data);
        currentReply->deleteLater();
    }
    else{

        QFile file("save.task");
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Impossible de sauvegarder les tâches locales"),
                                     file.errorString());
            return;
        }
        QTextStream out(&file);
        for(int j = 0 ; j < this->ui->scrollAreaWidgetContents->layout()->count() ; j++){
            Task* taskWidget = static_cast<Task*>(this->ui->scrollAreaWidgetContents->layout()->itemAt(j)->widget());
            if(taskWidget->isLocal()){
                if(taskWidget->getText() == ""){
                    this->ui->scrollAreaWidgetContents->layout()->removeWidget(taskWidget);
                    taskWidget->setVisible(false);
                    continue;
                }
                out << taskWidget->getText() << "\r\n";
            }
        }
    }
}



void MainWindow::removeTask(int id)
{
    QNetworkAccessManager networkManager;

    QUrl url("http://tech_todo_list.ms.kang.lan/task/remove");
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setUrl(url);

    QJsonObject json;
    json.insert("id", id);

    QNetworkReply* currentReply = networkManager.post(request, QJsonDocument(json).toJson());
    while(!currentReply->isFinished()) {
        qApp->processEvents();
    }
    QByteArray response_data = currentReply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response_data);
    currentReply->deleteLater();
    updateTaskList();
}




void MainWindow::mousePressEvent(QMouseEvent *event){
    mpos = event->pos();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    if (event->buttons() & Qt::LeftButton) {
        QPoint diff = event->pos() - mpos;
        QPoint newpos = this->pos() + diff;

        this->move(newpos);
    }
}