#include "demo.h"

std::mutex mtx;
TcpSocketWorker::TcpSocketWorker(const QString& ip, quint16 port, QObject* parent)
	: QObject(parent), m_ip(ip), m_port(port), m_socket(nullptr) {}

QTcpSocket* TcpSocketWorker::getSocket() {
	return m_socket; 
}

void TcpSocketWorker::startWork() {
	m_socket = new QTcpSocket();
	connect(m_socket, &QTcpSocket::connected, this, &TcpSocketWorker::onConnected);
	connect(m_socket, &QTcpSocket::readyRead, this, &TcpSocketWorker::onReadyRead);
	connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpSocketWorker::onErrorOccurred);
	connect(m_socket, &QAbstractSocket::errorOccurred, [](QAbstractSocket::SocketError error) {
		qDebug() << "Socket error:" << error;
		});


	m_socket->connectToHost(QHostAddress(m_ip), m_port);
}

//实现向服务区发送请求的逻辑
void TcpSocketWorker::onConnected() {
	qDebug() << "Socket connected! ip:" << m_socket->localAddress() << "port:" << m_socket->localPort();

	// 确保 Socket 已连接
	if (getSocket()->state() != QAbstractSocket::ConnectedState) {
		qDebug() << "Socket not connected!";
		return;
	}

	// 用户名和密码
	std::string userName = "user" + std::to_string(m_socket->localPort());
	std::string password = "123";

	// 注册请求
	PDU* registerPdu = mkPDU(0);
	registerPdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
	strncpy(registerPdu->caData, userName.c_str(), 32);
	strncpy(registerPdu->caData + 32, password.c_str(), 32);
	getSocket()->write((char*)registerPdu, registerPdu->uiPDULen);
	getSocket()->flush();
	free(registerPdu);

	QString curPath = QString("./users/%1").arg(QString::fromStdString(userName));


	// 延迟1秒后发送登录请求
	QTimer::singleShot(1000, this, [=]() {
		// 登录请求
		PDU* loginPdu = mkPDU(0);
		loginPdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
		strncpy(loginPdu->caData, userName.c_str(), 32);
		strncpy(loginPdu->caData + 32, password.c_str(), 32);
		getSocket()->write((char*)loginPdu, loginPdu->uiPDULen);
		getSocket()->flush();
		free(loginPdu);


		// 延迟3秒后完成
		QTimer::singleShot(1000, this, [=]() {
			QString strCurPath = curPath;
			qDebug() << "curPath:" << strCurPath;
			QString m_strUploadFilePath = "C:/Users/陈鑫源/Desktop/11111.rar";
			if (m_strUploadFilePath.isEmpty()) {
				return;
			}

			int idx = m_strUploadFilePath.lastIndexOf("/");
			QString fileName = m_strUploadFilePath.right(m_strUploadFilePath.size() - idx - 1);
			QFile* file = new QFile(m_strUploadFilePath); // 动态分配 QFile

			if (!file->exists()) {
				qDebug() << "File does not exist!";
				delete file; // 释放资源
				return;
			}

			qint64 fileSize = file->size();
			if (!file->open(QIODevice::ReadOnly)) {
				qDebug() << "Failed to open the file!";
				delete file; // 释放资源
				return;
			}

			PDU* pdu = mkPDU(strCurPath.size() + 1 + userName.size() + 1);
			pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;

			// 将路径写入到 caMsg 中
			memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
			((char*)pdu->caMsg)[strCurPath.size()] = '\0';

			memcpy(((char*)pdu->caMsg + strCurPath.size() + 1), userName.c_str(), userName.size());
			pdu->caMsg[strCurPath.size() + 1 + userName.size()] = '\0';

			sprintf(pdu->caData, "%s %lld", fileName.toStdString().c_str(), fileSize);
			getSocket()->write((char*)pdu, pdu->uiPDULen);
			getSocket()->flush();
			free(pdu);



			QTimer::singleShot(1000, this, [=]() {

				int flag = getSocket()->localPort()%2;
				switch (flag)
				{
					//每隔500ms发送
				case 0: {
					cout1 << "port" << getSocket()->localPort() << " delayed delivery";
					char* pBuffer = new char[4096];
					qint64 fileSize = file->size();
					qint64 bytesSent = 0;

					// 记录开始时间
					qint64 startTime = QDateTime::currentMSecsSinceEpoch();

					QTimer* sendTimer = new QTimer(this); // 定时器，用于控制发送速度
					connect(sendTimer, &QTimer::timeout, this, [=]() mutable {
						static qint64 lastPrintTime = startTime; // 上次打印时间
						static qint64 lastBytesSent = 0;         // 上次统计时发送的字节数

						qint64 ret = file->read(pBuffer, 4096);
						if (ret > 0) {
							getSocket()->write(pBuffer, ret);
							bytesSent += ret;

							// 控制缓冲区大小
							if (getSocket()->bytesToWrite() > 50 * 1024 * 1024) {
								getSocket()->flush();
							}

							// 当前时间
							qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

							// 每秒打印上传速度
							if (currentTime - lastPrintTime >= 1000) { // 每秒打印一次
								qint64 bytesInLastSecond = bytesSent - lastBytesSent; // 计算1秒内发送的字节数
								lastBytesSent = bytesSent;
								lastPrintTime = currentTime;

								double speedKBps = static_cast<double>(bytesInLastSecond) / 1024.0; // 转换为 KB/s
								if (speedKBps > 1000.0) {
									double speedMBps = speedKBps / 1024.0; // 转换为 MB/s
									qDebug() << "port:" << m_socket->localPort() << "upload speed:"
										<< QString::number(speedMBps, 'f', 2) << "MB/s";
								}
								else {
									qDebug() << "port:" << m_socket->localPort() << "upload speed:"
										<< QString::number(speedKBps, 'f', 2) << "KB/s";
								}
							}

							// 每5秒打印上传进度
							//if ((currentTime - startTime) % 5000 < 1000) { // 每5秒打印一次
							//	double percentage = (static_cast<double>(bytesSent) / fileSize) * 100;
							//	qDebug() << "port:" << m_socket->localPort()
							//		<< "upload progress:" << percentage << "%";
							//}
						}
						else {
							// 文件读取完毕
							sendTimer->stop();
							sendTimer->deleteLater();
							delete[] pBuffer;
							file->close();
							delete file; // 释放文件对象

							// 计算总用时
							qint64 endTime = QDateTime::currentMSecsSinceEpoch();
							qint64 totalTime = (endTime - startTime) / 1000; // 总用时（秒）

							// 转换为时:分:秒格式
							qint64 hours = totalTime / 3600;
							qint64 minutes = (totalTime % 3600) / 60;
							qint64 seconds = totalTime % 60;

							qDebug() << "File upload completed for user:" << QString::fromStdString(userName);
							qDebug() << QString("Total time taken: %1:%2:%3")
								.arg(hours, 2, 10, QChar('0'))
								.arg(minutes, 2, 10, QChar('0'))
								.arg(seconds, 2, 10, QChar('0'));
						}
						});

					sendTimer->start(1); // 立即开始发送数据

					break;
				}
					  //立马发送
				case 1: {
					cout1 << "port" << getSocket()->localPort() << " immediate delivery";

					char* pBuffer = new char[4096];
					qint64 fileSize = file->size();
					qint64 bytesSent = 0;

					// 记录开始时间
					qint64 startTime = QDateTime::currentMSecsSinceEpoch();
					qint64 lastPrintTime = startTime; // 上次打印时间
					qint64 lastBytesSent = 0;         // 上次统计时发送的字节数

					while (true) {
						qint64 ret = file->read(pBuffer, 4096);
						if (ret > 0) {
							getSocket()->write(pBuffer, ret);
							bytesSent += ret;

							// 控制缓冲区大小
							if (getSocket()->bytesToWrite() > 50 * 1024 * 1024) {
								getSocket()->flush();
							}

							// 当前时间
							qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

							// 每隔 1 秒打印上传速度
							if (currentTime - lastPrintTime >= 1000) { // 每秒打印一次
								qint64 bytesInLastSecond = bytesSent - lastBytesSent; // 计算1秒内发送的字节数
								lastBytesSent = bytesSent;
								lastPrintTime = currentTime;

								double speedKBps = static_cast<double>(bytesInLastSecond) / 1024.0; // 转换为 KB/s
								if (speedKBps > 1000.0) {
									double speedMBps = speedKBps / 1024.0; // 转换为 MB/s
									qDebug() << "port:" << m_socket->localPort() << "upload speed:" << QString::number(speedMBps, 'f', 2) << "MB/s";
								}
								else {
									qDebug() << "port:" << m_socket->localPort() << "upload speed:" << QString::number(speedKBps, 'f', 2) << "KB/s";
								}
							}

							// 每5秒打印上传进度
							//if (currentTime - startTime >= 5000 && (currentTime - startTime) % 5000 < 1000) { // 每5秒打印一次
							//	double percentage = (static_cast<double>(bytesSent) / fileSize) * 100;
							//	qDebug() << "port:" << m_socket->localPort() << "upload progress:" << percentage << "%";
							//}
						}
						else {
							// 文件读取完毕
							qint64 endTime = QDateTime::currentMSecsSinceEpoch();
							qint64 totalTime = (endTime - startTime) / 1000; // 总用时，单位为秒

							// 转换为时:分:秒格式
							qint64 hours = totalTime / 3600;
							qint64 minutes = (totalTime % 3600) / 60;
							qint64 seconds = totalTime % 60;

							delete[] pBuffer;
							file->close();
							delete file; // 释放文件对象

							qDebug() << "File upload completed for user:" << QString::fromStdString(userName) << QString(" Total time taken: %1:%2:%3")
								.arg(hours, 2, 10, QChar('0'))
								.arg(minutes, 2, 10, QChar('0'))
								.arg(seconds, 2, 10, QChar('0'));
							break;
						}
					}


					break;
				}

				default:
					break;
				}


				});

			});

		});

	return;


	// 文件上传逻辑
	QString strCurPath = curPath;
	qDebug() << "curPath:" << strCurPath;
	QString m_strUploadFilePath = "C:/Users/陈鑫源/Desktop/1.cpp";
	if (m_strUploadFilePath.isEmpty()) {
		//QMessageBox::warning(this, "上传文件", "上传文件名称不能为空");
		return;
	}

	int idx = m_strUploadFilePath.lastIndexOf("/");
	QString fileName = m_strUploadFilePath.right(m_strUploadFilePath.size() - idx - 1);
	QFile file(m_strUploadFilePath);
	if (!file.exists()) {
		//QMessageBox::warning(this, "上传文件", "文件不存在");
		return;
	}

	qint64 fileSize = file.size();
	if (!file.open(QIODevice::ReadOnly)) {
		//QMessageBox::warning(this, "上传文件", "打开文件失败");
		return;
	}

	PDU* pdu = mkPDU(strCurPath.size() + 1 + userName.size() + 1);
	pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;

	// 将路径写入到 caMsg 中
	memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
	// 在路径后面追加 null 字符
	((char*)pdu->caMsg)[strCurPath.size()] = '\0';



	//*((char*)pdu->caMsg + strCurPath.size()) = '\0';
	//pdu->caMsg[strCurPath.size()] = '\0';
	//cout1 << "username size:" << userName.size();
	// 将用户名写入到 caMsg 路径后面
	//std::string username = "your_username"; // 替换为实际的用户名变量
	memcpy(((char*)pdu->caMsg + strCurPath.size() + 1), userName.c_str(), userName.size());


	// 在用户名后面追加 null 字符
	pdu->caMsg[strCurPath.size() + 1 + userName.size()] = '\0';



	/*PDU* pdu = mkPDU(strCurPath.size() + 1);
	pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
	memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());*/

	sprintf(pdu->caData, "%s %lld", fileName.toStdString().c_str(), fileSize);
	getSocket()->write((char*)pdu, pdu->uiPDULen);
	getSocket()->flush();
	free(pdu);

	QThread::sleep(3);


	//文件开始上传
	char* pBuffer = new char[4096];
	qint64 ret = 0;
	while ((ret = file.read(pBuffer, 4096)) > 0) {
		getSocket()->write(pBuffer, ret);
		if (getSocket()->bytesToWrite() > 50 * 1024 * 1024) {
			getSocket()->flush();
			//QThread::msleep(10); // 稍微等待缓冲区清空
		}
	}
	delete[] pBuffer;
	file.close();
	qDebug() << "File upload completed for user:" << QString::fromStdString(userName);

	/*std::string userName = "user" + std::to_string(m_socket->localPort());
	std::string password = "123";*/


	emit connectionSuccess();
}

void TcpSocketWorker::onReadyRead() {
	// Handle data read from the socket
	uint uiPDULen = 0;
	// 先获取通信对象的总大小
	m_socket->read((char*)&uiPDULen, sizeof(int));
	// 消息数据大小
	uint uiMsgLen = uiPDULen - sizeof(PDU);
	// 创建通信对象空间
	PDU* pdu = mkPDU(uiMsgLen);
	// 更新通信对象总大小
	pdu->uiPDULen = uiPDULen;
	// 读取剩余数据内容到通信对象中
	m_socket->read((char*)pdu + sizeof(int), uiPDULen - sizeof(int));

	// 获取当前端口号
	quint16 port = m_socket->localPort();

	switch (pdu->uiMsgType)
	{
	case ENUM_MSG_TYPE_REGIST_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_REGIST_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_LOGIN_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_LOGIN_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
	{
		OpeWidget::getInstance().getFriend()->showAllOnlineUser(pdu);
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_ALL_ONLINE_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_SEARCH_USER_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_SEARCH_USER_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_ADD_FRIEND_REQUEST";
		break;
	}
	case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_ADD_FRIEND_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_ADD_FRIEND_AGREE";
		break;
	}
	case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_ADD_FRIEND_REFUSE";
		break;
	}
	case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST";
		break;
	}
	case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST";
		break;
	}
	case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_GROUP_CHAT_REQUEST";
		break;
	}
	case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_CREATE_DIR_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_FLUSH_DIR_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_FLUSH_DIR_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_DEL_DIR_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_DEL_DIR_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_RENAME_DIR_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_RENAME_DIR_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_ENTER_DIR_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_DEL_FILE_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_DEL_FILE_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_SHARE_FILE_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_SHARE_FILE_NOTE:
	{
		qDebug() << "Port:" << port << "receive msg type: ENUM_MSG_TYPE_SHARE_FILE_NOTE";
		break;
	}
	default:
	{
		qDebug() << "Port:" << port << "receive msg type: UNKNOWN";
		break;
	}
	}

	// 释放内存
	free(pdu);
	pdu = NULL;
	emit dataReceived();
}


void TcpSocketWorker::onErrorOccurred(QAbstractSocket::SocketError error) {
	qDebug() << m_socket->localPort() << "Socket error: " << error;
	emit errorOccurred(error);
}
