#pragma once

#ifndef TCPSOCKETWORKER_H
#define TCPSOCKETWORKER_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDebug>
#include"my.h"

class TcpSocketWorker : public QObject {
    Q_OBJECT

public:
    TcpSocketWorker(const QString& ip, quint16 port, QObject* parent = nullptr);
    QTcpSocket* getSocket();

public slots:
    void startWork();
    void onConnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError error);

signals:
    void connectionSuccess();
    void dataReceived();
    void errorOccurred(QAbstractSocket::SocketError error);
    void finished();
    void sendReq();

private:
    QString m_ip;
    quint16 m_port;
    QTcpSocket* m_socket;
};

#endif // TCPSOCKETWORKER_H
