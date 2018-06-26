#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QColor>
#include <QFont>
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
    void closeEvent(QCloseEvent *event);
    void readSettings();
    void saveSettings();

    void removeTask(int id);
    void addLocalTask(QString text);
    void addOnlineTask(QString text);
    void modifyTask(Task* t);
    void save();
    ~MainWindow();

public slots:
        void updateTaskList();
        void focusChange(Task*);

private:
    Ui::MainWindow *ui;
    bool resizing;
    QPoint mpos;
    QColor mainColor, onlineTaskColor, localeTaskColor;
    QFont font;
};

#endif // MAINWINDOW_H
