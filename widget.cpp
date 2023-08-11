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

    connect(ui->selectFileButton, &QPushButton::clicked, this, &Widget::on_selectFileButton_pressed );
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
    qDebug() << "File selected" ;

    server = ui->serverEdit->text();
    if(server.isEmpty())
    {
        qDebug() << "Server label is empty.";
        QMessageBox::warning(nullptr, "Empty Label", "The server label is empty.");
    }
}

void Widget::replyFinished()
{
    QMessageBox result;

    qDebug() << "Reply received, status:" << reply->error();
    if (reply->error() == QNetworkReply::NoError)
    {
        returnedUrl = QString(reply->readAll());

        qDebug() << "returnedUrl= " << returnedUrl;
        result.setText("Success!\nHere is paste link:\n" + returnedUrl);
        result.exec();

    } else {
        qDebug() << "Error:" << reply->errorString();
        result.setText("Error!\nThe server communication is not connected.");
        result.exec();
    }
    ui->uploadProgressBar->setValue(0);
}

void Widget::on_sendFileButton_pressed()
{
    serverUrl.setUrl(server);
    QNetworkRequest request(serverUrl);

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(file).fileName() + "\""));
    filePart.setBodyDevice(&file);
    file.setParent(multiPart);
    multiPart->append(filePart);

    manager = new QNetworkAccessManager;
    reply = manager->post(request, multiPart);
    multiPart->setParent(reply);
    qDebug() << "sendFileButton after";

    ui->uploadProgressBar->setRange(0, file.size());

    connect(reply, &QNetworkReply::uploadProgress, this, [=](qint64 bytesSent, qint64 bytesTotal){
        if (bytesTotal > 0) {
            ui->uploadProgressBar->setValue(bytesSent);
        }
    });

    connect(manager, &QNetworkAccessManager::finished, this, &Widget::replyFinished);
}

