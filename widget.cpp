#include "widget.h"
#include "./ui_widget.h"

#include <QFileDialog>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    manager = new QNetworkAccessManager(this);
    connect(ui->selectFileButton, &QPushButton::clicked, this, &Widget::on_selectFileButton_pressed );
    connect(this, &Widget::fileSelectedFunc, this, &Widget::on_selectFileButton_pressed);
//    connect(manager, &QNetworkAccessManager::finished, this, &Widget::replyFinished);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_selectFileButton_pressed()
{
    selectedFileName = QFileDialog::getOpenFileName(this, "All Files (*)");

    if (selectedFileName.isEmpty())
    {
        qDebug() << "No file selected.";
        return;
    }

    file.setFileName(selectedFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Unable to open file";
        return;
    }

    ui->selectedFileLine->setText(selectedFileName);
    emit fileSelectedFunc();
}

void Widget::fileSelectedFunc()
{
    qDebug() << "File selected" ;

    server = ui->serverEdit->text();
    if(server.isEmpty())
    {
        qDebug() << "Server label is empty.";
        QMessageBox::warning(nullptr, "Empty Label", "The server label is empty.");
    }
    //    emit prepareRequest();
}

void Widget::replyFinished()
{
    QMessageBox result;

    qDebug() << "Reply received, status:" << reply->error();
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "Success:" << reply->readAll();
        returnedUrl = QString::fromUtf8(reply->readAll());

        qDebug() << returnedUrl;
        result.setText("Success!\nHere is paste link:\n" + returnedUrl);

    } else {
        qDebug() << "Error:" << reply->errorString();
        result.setText("Error!\nThe server communication is not connected.");
    }

    reply->deleteLater();
    delete[] reply;
}

void Widget::on_sendFileButton_pressed()
{
    QUrl serverUrl(server);
    QNetworkRequest request(serverUrl);

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(file).fileName() + "\""));
    filePart.setBodyDevice(&file);
    file.setParent(multiPart);
    multiPart->append(filePart);


    reply = manager->post(request, multiPart);
    multiPart->setParent(reply);
    connect(manager, &QNetworkAccessManager::finished, this, &Widget::replyFinished);
}

