#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include "task.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

    void removeTask(int id);
    void addLocalTask(QString text);
    void addOnlineTask(QString text);
    void modifyTask(Task* t);
    ~MainWindow();

public slots:
        void updateTaskList();
        void focusChange(Task*);

private:
    Ui::MainWindow *ui;
    QPoint mpos;
};

#endif // MAINWINDOW_H
