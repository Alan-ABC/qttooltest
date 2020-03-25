#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

struct AssetData
{
    QString name;
    QString md5;
    QString size;
    int state;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void InitConstrols();
    void LoadConfig(int index, QString fileName);
    QString GetFilePath(QString fileName);
    void AddTabPage(int pageIdx, QString fileName);
    QList<AssetData*>* DiffVersionByPreview(int pre, int next);

private slots:
    void on_pushButton_clicked();
    void onTabchanged(int index);

private:
    Ui::MainWindow *ui;
    QList<QList<AssetData*>*> List;
    QString prefix;
    short startPage;
    short endPage;
    QList<bool> *bInst;
    QList<QWidget*> widget;
    QList<QString> fileNames;
};

#endif // MAINWINDOW_H