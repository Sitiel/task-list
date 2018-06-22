#ifndef TASK_H
#define TASK_H

#include <QWidget>

namespace Ui {
class Task;
}

class Task : public QWidget
{
    Q_OBJECT

public:
    explicit Task(int id, QString text, bool local, QWidget *parent = 0);
    int getId();
    QString getText();
    bool isLocal();
    ~Task();

signals:
    void focusChanged(Task* t);

private slots:
    void on_plainTextEdit_textChanged();

private:
    int m_id;
    Ui::Task *ui;
    bool m_local;
};


#endif // TASK_H
