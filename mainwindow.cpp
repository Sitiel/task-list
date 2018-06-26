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
#include <QSettings>
#include <QFontDialog>
#include <QColorDialog>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QWidget::setWindowIcon(QIcon("kang.png"));
    readSettings();
    updateTaskList();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTaskList()));
    timer->start(10000);

    QFile jsonFile("save.task");
    jsonFile.open(QFile::ReadOnly);
    QJsonArray array = QJsonDocument().fromJson(jsonFile.readAll()).array();
    for(int i = 0 ; i < array.size() ; i++){
        addLocalTask(array[i].toString());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::saveSettings(){
    QSettings settings("Kang", "tasklist");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("mainColor", mainColor);
    settings.setValue("onlineColor", onlineTaskColor);
    settings.setValue("localeColor", localeTaskColor);
    settings.setValue("font", font);
}

void MainWindow::readSettings()
{
    QSettings settings("Kang", "tasklist");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    mainColor = settings.value("mainColor").value<QColor>();
    onlineTaskColor = settings.value("onlineColor").value<QColor>();
    localeTaskColor = settings.value("localeColor").value<QColor>();
    font = settings.value("font").value<QFont>();

    if(!mainColor.isValid()){
        mainColor = QColor(Qt::GlobalColor::yellow);
    }
    if(!onlineTaskColor.isValid()){
        onlineTaskColor = QColor(200, 200, 0);
    }
    if(!localeTaskColor.isValid()){
        localeTaskColor = QColor(120, 120, 0);
    }

    this->setStyleSheet("background-color: " + mainColor.name() + ";");
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
            this->save();
        }
    }

    if(k->key() == Qt::Key_W)
    {
        if(QApplication::keyboardModifiers() && Qt::ControlModifier)
        {
            QWidget * w = qApp->focusWidget();
            QPlainTextEdit* t = dynamic_cast<QPlainTextEdit*>(w);
            if(t == NULL){
                return;
            }

            Task *task = static_cast<Task*>(t->parent());
            if(task->isLocal()){
                this->ui->scrollAreaWidgetContents->layout()->removeWidget(task);
                t->setVisible(false);
                t->document()->setPlainText("");
                this->save();
            }else{
                this->removeTask(task->getId());
            }
        }
    }

    if(k->key() == Qt::Key_I)
    {
        if(QApplication::keyboardModifiers() && Qt::ControlModifier)
        {
            mainColor = QColorDialog::getColor(Qt::yellow, this);
            this->setStyleSheet("background-color: " + mainColor.name() + ";");
            saveSettings();
        }
    }
    if(k->key() == Qt::Key_O)
    {
        if(QApplication::keyboardModifiers() && Qt::ControlModifier)
        {
            onlineTaskColor = QColorDialog::getColor(Qt::yellow, this);
            saveSettings();
        }
    }
    if(k->key() == Qt::Key_P)
    {
        if(QApplication::keyboardModifiers() && Qt::ControlModifier)
        {
            localeTaskColor = QColorDialog::getColor(Qt::yellow, this);
            saveSettings();
        }
    }
    if(k->key() == Qt::Key_G)
    {
        if(QApplication::keyboardModifiers() && Qt::ControlModifier)
        {
            bool ok;
            QFont tmpFont = QFontDialog::getFont(
                           &ok, QFont("Helvetica [Cronyx]", 10), this);
            if(ok){
                font = tmpFont;
            }
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
        QString text = obj.value("task_txt").toString();
        bool found = false;
        for(int j = 0 ; j < this->ui->scrollAreaWidgetContents->layout()->count() ; j++){
            Task* taskWidget = static_cast<Task*>(this->ui->scrollAreaWidgetContents->layout()->itemAt(j)->widget());
            if(taskWidget->getId() == id){
                found = true;
                taskWidget->setText(text);
            }
        }
        if(!found)
        {
            Task* t = new Task(id, obj.value("task_txt").toString(), false, font, localeTaskColor, onlineTaskColor);
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
    this->modifyTask(taskWidget);
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
    Task* t = new Task(-1, text, true, font, localeTaskColor, onlineTaskColor);
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
        this->save();
    }
}


void MainWindow::save(){

    QFile file("save.task");
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::information(this, tr("Impossible de sauvegarder les tâches locales"),
                                 file.errorString());
        return;
    }

    QJsonArray jsonArray;

    for(int j = 0 ; j < this->ui->scrollAreaWidgetContents->layout()->count() ; j++){
        Task* taskWidget = static_cast<Task*>(this->ui->scrollAreaWidgetContents->layout()->itemAt(j)->widget());
        if(taskWidget->isLocal()){
            jsonArray.push_back(taskWidget->getText());
        }
    }

    QJsonDocument jsonDoc(jsonArray);

    file.write(jsonDoc.toJson());
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
    if(mpos.x() > this->width()-25 && mpos.y() > this->height() - 25)
        resizing = true;
    else
        resizing = false;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    if (event->buttons() & Qt::LeftButton) {
        QPoint diff = event->pos() - mpos;
        QPoint newpos = this->pos() + diff;

        if(resizing){
            this->resize(this->width() + diff.x(), this->height() + diff.y());
            mpos = event->pos();
        }
        else{
             this->move(newpos);
        }

    }
}
