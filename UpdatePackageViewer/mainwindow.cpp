#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDomDocument>
#include <qdebug.h>
#include <QXmlStreamReader>
#include <QFile>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableView>
#include <qtwidgetsglobal.h>
#include <qsizepolicy.h>
#include <QHeaderView>
#include <QTime>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    prefix("")
{
    bInst = new QVector<bool>();
    ui->setupUi(this);

    InitConstrols();

    ui->tabsVersion->clear();
}

MainWindow::~MainWindow()
{
    for (int i = List.count() - 1; i >= 0; --i)
    {
        //delete List->at(i);
        List.removeAt(i);
    }
    delete ui;
    delete bInst;

    for (int i = widget.count() - 1; i >= 0; --i)
    {
        delete widget.at(i);
    }

    widget.clear();
}

void MainWindow::InitConstrols()
{
    connect(ui->tabsVersion, SIGNAL(currentChanged(int)), this, SLOT(onTabchanged(int)));

    GetAllEdition();
}

QString MainWindow::GetFilePath(const QString& fileName)
{
    QString filePath = QApplication::applicationDirPath() + "/" + fileName;
    return filePath;
}

void MainWindow::AddTabPage(int pageIdx, const QString& fileName)
{
    if (pageIdx < 0 && pageIdx >= bInst->count())
    {
        return;
    }

    if (bInst->at(pageIdx) == true)
    {
        return;
    }

    bInst->replace(pageIdx, true);

    LoadConfig(pageIdx, fileName);

    //qDebug()<< QTime::currentTime().msecsSinceStartOfDay();
    //ui->tabsVersion->
    QGridLayout* rgid = new QGridLayout(widget.at(pageIdx));
    QTableView* tView = new QTableView(widget.at(pageIdx));
    rgid->addWidget(tView, 0, 0, 1, 1);
    QStandardItemModel* model = new QStandardItemModel(tView);
    tView->setGeometry(0, 0, 1000, 800);
    //tView->setSizePolicy(QSizePolicy::Expanding);
    tView->setModel(model);
    model->setColumnCount(4);
    model->setHeaderData(0, Qt::Horizontal, QString::fromUtf8("%1").arg("             名称         　"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromUtf8("%1").arg("              Md5         　"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromUtf8("%1").arg("大小　"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromUtf8("%1").arg("状态　"));
    //tView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tView->setColumnWidth(0, 400);
    tView->setColumnWidth(1, 400);
    tView->setColumnWidth(2, 50);
    tView->setColumnWidth(3, 50);
    tView->setAlternatingRowColors(true);

    QVector<AssetData*>* temp = nullptr;

    if (pageIdx > 0)
    {
        temp = DiffVersionByPreview(pageIdx - 1, pageIdx);
    }
    else
    {
        temp = List.at(pageIdx);
    }


    for (int i = 0; i < temp->count(); ++i)
    {
        model->setItem(i, 0, new QStandardItem(temp->at(i)->name));
        model->setItem(i, 1, new QStandardItem(temp->at(i)->md5));
        model->setItem(i, 2, new QStandardItem(temp->at(i)->size));

        int state = temp->at(i)->state;
        if (state == 1)
        {
            model->setItem(i, 3, new QStandardItem(QString::fromUtf8("增加")));
        }
        else if (state == 2)
        {
            model->setItem(i, 3, new QStandardItem(QString::fromUtf8("更新")));
        }
        else
        {
            model->setItem(i, 3, new QStandardItem(QString::fromUtf8("增加")));
        }

    }

    if (pageIdx > 0)
    {
        delete temp;
    }

    tView->resizeColumnsToContents();
    tView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    //tView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //tView->setMaximumSize(QSize(10000, 10000));
    tView->show();
    //qDebug()<<QTime::currentTime().msecsSinceStartOfDay();
}

QVector<AssetData*>* MainWindow::DiffVersionByPreview(int pre, int next)
{
    QVector<AssetData*>* updateItems = new QVector<AssetData*>();

    QVector<AssetData*>* newList = List.at(next);
    QVector<AssetData*>* oldList = List.at(pre);
    for (int i = 0, count = newList->count(); i < count; ++i)
    {
        bool hasItem = false;
        AssetData* itemNew = newList->at(i);

        for (int k = 0, count2 = oldList->count(); k < count2; ++k)
        {
            AssetData* itemOld = oldList->at(k);

            if (itemOld->name == itemNew->name)
            {
                hasItem = true;
                if (itemOld->md5 != itemNew->md5)
                {
                    itemNew->state = 2;
                    updateItems->append(itemNew);
                    break;
                }
                else
                {
                    break;
                }
            }
        }

        if (!hasItem)
        {
            itemNew->state = 3;
            updateItems->append(itemNew);
        }
    }

    return updateItems;
}

void MainWindow::GetAllEdition()
{
    QString path = QApplication::applicationDirPath();

    QDir dir(path);

    if (dir.exists())
    {
        dir.setFilter(QDir::Dirs | QDir::Files);
        dir.setSorting(QDir::DirsFirst);
        QFileInfoList list = dir.entryInfoList();

        for (int i = 0; i < list.count(); ++i)
        {
            QFileInfo info = list.at(i);

            if (info.isFile())
            {
                QString orgfileName = info.fileName();

                if (orgfileName.indexOf("ResVersion") == -1)
                {
                    continue;
                }

                QString fileName = GetEditionFromFileName(orgfileName);

                if (!editions.contains(fileName))
                {
                    editions.append(fileName);
                }

                allFiles.append(GetFileNameNoExtra(orgfileName));
            }
        }
    }

    ui->AEdition->addItems(editions);
}

QString MainWindow::GetFileNameNoExtra(const QString& fileName)
{
    int idx = fileName.lastIndexOf('.');
    QString nameNoExtra = fileName.left(idx);
    return nameNoExtra;
}

QString MainWindow::GetEditionFromFileName(const QString& fileName)
{
    int idx = fileName.lastIndexOf('_');
    QString nameNoExtra = fileName.left(idx);
    return nameNoExtra;
}


void MainWindow::LoadConfig(int index, const QString& fileName)
{
    //qDebug()<<QApplication::applicationDirPath() + "/ResVersion1.0.2_1.0.48.xml";
    QString filePath = GetFilePath(fileName);
    QFile file(filePath);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::critical(this, tr("Error"), tr("Cannot read file %1").arg(filePath));
        return;
    }

    QDomDocument document;
    QString strErr;
    int errLin = 0;
    int errCol = 0;

    if (!document.setContent(&file, false, &strErr, &errLin, &errCol))
    {
        return;
    }

    if (document.isNull())
    {
        return;
    }

    QDomElement root = document.documentElement();
    QDomElement catalogs = root.firstChildElement();

    if (catalogs.isNull())
    {
        return;
    }

    while(!catalogs.isNull())
    {
        AssetData* data = new AssetData();
        data->name = catalogs.attributeNode("n").value();
        data->md5 = catalogs.attributeNode("m").value();
        data->size = catalogs.attributeNode("s").value();
        data->state = 1;
        //qDebug()<<data->name<<data->md5<<data->size;
        if (List.at(index) == nullptr)
        {
            List.replace(index, new QVector<AssetData*>());
        }
        List.at(index)->append(data);
        catalogs = catalogs.nextSiblingElement();
    }
}

void MainWindow::on_pushButton_clicked()
{
    if (ui->fromVersion->text().count() == 0 || ui->toVersion->text().count() == 0)
    {
        return;
    }

    QString fileName = ui->fromVersion->text();
    int idx = fileName.lastIndexOf('.');
    prefix = fileName.left(idx);
    QString start = fileName.right(fileName.count() - idx - 1);
    startPage = start.toShort();

    if (ui->bAllCheck->isChecked())
    {
        QString endFileName = ui->toVersion->text();
        idx = endFileName.lastIndexOf('.');
        QString end = endFileName.right(endFileName.count() - idx - 1);
        endPage = end.toShort();

        for (short i = startPage; i <= endPage; ++i)
        {
            bInst->append(false);
            List.append(nullptr);

            QWidget* tab = new QWidget(ui->tabsVersion);
            QIcon icon;
            QString str;
            str.setNum(i);
            QString fileName = prefix + "." + str + ".xml";
            fileNames.append(fileName);
            widget.append(tab);
            ui->tabsVersion->addTab(tab, icon, fileName);
        }
    }
    else
    {
        for (short i = startPage; i <= startPage + 1; ++i)
        {
            bInst->append(false);
            List.append(nullptr);

            QWidget* tab = new QWidget(ui->tabsVersion);
            QIcon icon;
            QString fileName;
            if (i == startPage)
            {
                fileName = ui->fromVersion->text() + ".xml";
            }
            else
            {
                fileName = ui->toVersion->text() + ".xml";
            }
            fileNames.append(fileName);
            widget.append(tab);
            ui->tabsVersion->addTab(tab, icon, fileName);
        }
    }


    AddTabPage(0, fileNames[0]);
}

void MainWindow::onTabchanged(int index)
{
    if (index > 0)
    {
        AddTabPage(index, fileNames[index]);
    }
}

void MainWindow::on_clearBtn_clicked()
{
    bInst->clear();
    for (int i = 0; i < widget.count(); ++i)
    {
        ui->tabsVersion->removeTab(i);
        delete widget.at(i);
    }

    widget.clear();

    for (int i = List.count() - 1; i >= 0; --i)
    {
        QVector<AssetData*>* child = List.at(i);

        if (child == nullptr)
        {
            List.removeAt(i);
            continue;
        }

        for (int k = child->count() - 1; k >= 0; --k)
        {
            AssetData *data =  child->at(k);
            if (data != nullptr)
            {
                delete data;
            }

            child->removeAt(k);
        }

        delete List.at(i);
        List.removeAt(i);
        fileNames.clear();
    }
}

void MainWindow::on_fromNumer_currentTextChanged(const QString &arg1)
{
    ui->fromVersion->setText(arg1);
}

void MainWindow::on_AEdition_currentTextChanged(const QString &arg1)
{
    qDebug()<<arg1;

    QStringList list;
    for (int i = 0; i < allFiles.count(); ++i)
    {
        QString fileName = allFiles.at(i);
        if (fileName.indexOf(arg1) > -1)
        {
            list.append(fileName);
        }
    }

    ui->fromNumer->clear();
    ui->fromNumer->addItems(list);

    ui->toNumber->clear();
    ui->toNumber->addItems(list);
}

void MainWindow::on_toNumber_currentTextChanged(const QString &arg1)
{
    ui->toVersion->setText(arg1);
}
