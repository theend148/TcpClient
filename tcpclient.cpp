#include "tcpclient.h"
#include "ui_tcpclient.h"
#include"privatechat.h"
#include<QFile>
#include"my.h"
#define QABSTRACTSOCKET_DEBUG


QString userName = nullptr;


TcpClient::TcpClient(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::TcpClient)
{
	ui->setupUi(this);
	resize(500, 300);
	// 加载配置文件
	loadConfig();
	// 定义服务器链接成功槽函数
	connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(showConnect()));
	// 服务器接受到数据槽函数
	connect(&m_tcpSocket, SIGNAL(readyRead()), this, SLOT(recvMsg()));
	// 链接服务器
	//m_tcpSocket.connectToHost(QHostAddress(m_strIp), m_usPort);

	for (int i = 0; i < 10; ++i) {
		QThread* thread = new QThread();  // 创建新的线程
		TcpSocketWorker* worker = new TcpSocketWorker(m_strIp, m_usPort);

		worker->moveToThread(thread);  // 将工作对象移到新的线程

		// 连接信号和槽
		connect(thread, &QThread::started, worker, &TcpSocketWorker::startWork);
		
		// 线程完成时，删除工作对象和线程
		connect(worker, &TcpSocketWorker::finished, worker, &QObject::deleteLater);
		connect(worker, &TcpSocketWorker::finished, thread, &QThread::quit);
		connect(thread, &QThread::finished, thread, &QObject::deleteLater);

		// 启动线程
		thread->start();
	}


	//for (int i = 0; i < 5; ++i) {
	//	sockets.append(new QTcpSocket());
	//	sockets[i]->connectToHost(QHostAddress(m_strIp), m_usPort);
	//	connect(sockets[i], &QTcpSocket::connected, this, [this, i]() {
	//		this->func(i);
	//		});
	//	/*connect(sockets[i], &QTcpSocket::readyRead, this, [this, i]() {
	//		this->func1(i);
	//		});*/
	//	connect(sockets[i], &QTcpSocket::errorOccurred, this, [this, i](QAbstractSocket::SocketError error) {
	//		qDebug() << "Socket" << i << "error:" << error;
	//		});
	//}


}



//void TcpClient::func1(int i) {
//	qDebug() << "socket" << i << ' ';
//	//通信对象的总大小
//	uint uiPDULen = 0;
//	// 先获取通信对象的总大小
//	sockets[i]->read((char*)&uiPDULen, sizeof(int));
//	// 消息数据大小
//	uint uiMsgLen = uiPDULen - sizeof(PDU);
//	// 创建通信对象空间
//	PDU* pdu = mkPDU(uiMsgLen);
//	// 更新通信对象总大小
//	pdu->uiPDULen = uiPDULen;
//	// 读取剩余数据内容到通信对象中
//	sockets[i]->read((char*)pdu + sizeof(int), uiPDULen - sizeof(int));
//	switch (pdu->uiMsgType)
//	{
//	case ENUM_MSG_TYPE_REGIST_RESPOND:
//	{
//		//regist(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_REGIST_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_LOGIN_RESPOND:
//	{
//		//login(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_LOGIN_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
//	{
//		OpeWidget::getInstance().getFriend()->showAllOnlineUser(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_ALL_ONLINE_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_SEARCH_USER_RESPOND:
//	{
//		searchUser(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_SEARCH_USER_RESPOND";
//		break;
//	}
//	// TODO-----服务器转发请求  添加好友相关
//	case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
//		addFriendRequest(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_ADD_FRIEND_REQUEST";
//		break;
//	}
//	case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND: {
//		QMessageBox::information(this, "添加好友", pdu->caData);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_ADD_FRIEND_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_ADD_FRIEND_AGREE: {
//		addFriendAgree(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_ADD_FRIEND_AGREE";
//		break;
//	}
//	case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE: {
//		addFriendFefuse(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_ADD_FRIEND_REFUSE";
//		break;
//	}
//	case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND: {
//		OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND";
//		break;
//	}
//										   // 接受转发回来的消息
//	case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: {
//		deleteFriend(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST";
//		break;
//	}
//	case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND: {
//		QMessageBox::information(this, "删除好友", pdu->caData);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND";
//		break;
//	}
//											// 转发消息
//	case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST: {
//		privateChat(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST";
//		break;
//	}
//	case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: {
//		OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_GROUP_CHAT_REQUEST";
//		break;
//	}
//	case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
//	{
//		createDir(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_CREATE_DIR_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_FLUSH_DIR_RESPOND:
//	{
//		flushDir(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_FLUSH_DIR_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_DEL_DIR_RESPOND:
//	{
//		delDir(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_DEL_DIR_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_RENAME_DIR_RESPOND:
//	{
//		renameDir(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_RENAME_DIR_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
//	{
//		enterDir(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_ENTER_DIR_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
//	{
//		QMessageBox::information(this, "上传文件", pdu->caData);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_DEL_FILE_RESPOND:
//	{
//		QMessageBox::information(this, "删除文件", pdu->caData);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_DEL_FILE_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
//	{
//		qDebug() << pdu->caData;
//		downloadFilePre(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:
//	{
//		QMessageBox::information(this, "分享文件", pdu->caData);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_SHARE_FILE_RESPOND";
//		break;
//	}
//	case ENUM_MSG_TYPE_SHARE_FILE_NOTE:
//	{
//		shareFileNote(pdu);
//		qDebug() << "receive msg type: ENUM_MSG_TYPE_SHARE_FILE_NOTE";
//		break;
//	}
//	default:
//	{
//		qDebug() << "receive msg type: UNKNOWN";
//		break;
//	}
//	}
//
//	// 释放内存
//	free(pdu);
//	pdu = NULL;
//
//
//}
/*


*/

//void TcpClient::func(TcpSocketWorker*worker) {
//	static int i = 0;
//	qDebug() << "connected:" /*<< sockets[s]->localAddress()
//		<< ' ' << sockets[s]->localPort() << ' ' << sockets.size()*/ << ++i << "\n";
//
//		// 确保 Socket 已连接
//	if (worker->getSocket()->state() != QAbstractSocket::ConnectedState) {
//		qDebug() << "Socket not connected!";
//		return;
//	}
//
//	// 用户名和密码
//	std::string userName = "user" + std::to_string(i);
//	std::string password = "123";
//
//	//注册请求
//	PDU* registerPdu = mkPDU(0);
//	registerPdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
//	strncpy(registerPdu->caData, userName.c_str(), 32);
//	strncpy(registerPdu->caData + 32, password.c_str(), 32);
//	worker->getSocket()->write((char*)registerPdu, registerPdu->uiPDULen);
//	worker->getSocket()->flush();
//	free(registerPdu);
//
//	QThread::sleep(1);
//
//
//	// 登录请求
//	PDU* loginPdu = mkPDU(0);
//	loginPdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
//	strncpy(loginPdu->caData, userName.c_str(), 32);
//	strncpy(loginPdu->caData + 32, password.c_str(), 32);
//	worker->getSocket()->write((char*)loginPdu, loginPdu->uiPDULen);
//	worker->getSocket()->flush();
//	free(loginPdu);
//	m_strCurPath = QString("./%1").arg(QString::fromStdString(userName));
//
//	QThread::sleep(1);
//
//
//	// 文件上传逻辑
//	QString strCurPath = TcpClient::getInstance().curPath();
//	qDebug() << "curPath:" << TcpClient::getInstance().curPath();
//	QString m_strUploadFilePath = "C:/Users/陈鑫源/Desktop/11111.rar";
//	if (m_strUploadFilePath.isEmpty()) {
//		QMessageBox::warning(this, "上传文件", "上传文件名称不能为空");
//		return;
//	}
//
//	int idx = m_strUploadFilePath.lastIndexOf("/");
//	QString fileName = m_strUploadFilePath.right(m_strUploadFilePath.size() - idx - 1);
//	QFile file(m_strUploadFilePath);
//	if (!file.exists()) {
//		QMessageBox::warning(this, "上传文件", "文件不存在");
//		return;
//	}
//
//	qint64 fileSize = file.size();
//	if (!file.open(QIODevice::ReadOnly)) {
//		QMessageBox::warning(this, "上传文件", "打开文件失败");
//		return;
//	}
//
//	PDU* pdu = mkPDU(strCurPath.size() + 1 + userName.size() + 1);
//	pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
//
//	// 将路径写入到 caMsg 中
//	memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
//	// 在路径后面追加 null 字符
//	((char*)pdu->caMsg)[strCurPath.size()] = '\0';
//
//
//
//	//*((char*)pdu->caMsg + strCurPath.size()) = '\0';
//	//pdu->caMsg[strCurPath.size()] = '\0';
//	//cout1 << "username size:" << userName.size();
//	// 将用户名写入到 caMsg 路径后面
//	//std::string username = "your_username"; // 替换为实际的用户名变量
//	memcpy(((char*)pdu->caMsg + strCurPath.size() + 1), userName.c_str(), userName.size());
//
//
//	// 在用户名后面追加 null 字符
//	pdu->caMsg[strCurPath.size() + 1 + userName.size()] = '\0';
//
//
//
//	/*PDU* pdu = mkPDU(strCurPath.size() + 1);
//	pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
//	memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());*/
//	sprintf(pdu->caData, "%s %lld", fileName.toStdString().c_str(), fileSize);
//	worker->getSocket()->write((char*)pdu, pdu->uiPDULen);
//	worker->getSocket()->flush();
//	free(pdu);
//
//	QThread::sleep(1);
//
//
//	// 文件开始上传
//	char* pBuffer = new char[4096];
//	qint64 ret = 0;
//	while ((ret = file.read(pBuffer, 4096)) > 0) {
//		worker->getSocket()->write(pBuffer, ret);
//		if (worker->getSocket()->bytesToWrite() > 50 * 1024 * 1024) {
//			worker->getSocket()->flush();
//			//QThread::msleep(10); // 稍微等待缓冲区清空
//		}
//	}
//	delete[] pBuffer;
//	file.close();
//	qDebug() << "File upload completed for user:" << QString::fromStdString(userName);
//
//	//static int i = 0;
//	//qDebug() << "connected:" /*<< sockets[s]->localAddress()
//	//	<< ' ' << sockets[s]->localPort() << ' ' << sockets.size()*/ << ++i << "\n";
//
//	//	// 确保 Socket 已连接
//	//if (worker->getSocket()->state() != QAbstractSocket::ConnectedState) {
//	//	qDebug() << "Socket not connected!";
//	//	return;
//	//}
//
//	//// 用户名和密码
//	//std::string userName = "user" + std::to_string(i);
//	//std::string password = "123";
//
//	////注册请求
//	//PDU* registerPdu = mkPDU(0);
//	//registerPdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
//	//strncpy(registerPdu->caData, userName.c_str(), 32);
//	//strncpy(registerPdu->caData + 32, password.c_str(), 32);
//	//sockets[s]->write((char*)registerPdu, registerPdu->uiPDULen);
//	//sockets[s]->flush();
//	//free(registerPdu);
//
//	//QThread::sleep(1);
//
//
//	//// 登录请求
//	//PDU* loginPdu = mkPDU(0);
//	//loginPdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
//	//strncpy(loginPdu->caData, userName.c_str(), 32);
//	//strncpy(loginPdu->caData + 32, password.c_str(), 32);
//	//sockets[s]->write((char*)loginPdu, loginPdu->uiPDULen);
//	//sockets[s]->flush();
//	//free(loginPdu);
//	//m_strCurPath = QString("./%1").arg(QString::fromStdString(userName));
//
//	//QThread::sleep(1);
//
//
//	//// 文件上传逻辑
//	//QString strCurPath = TcpClient::getInstance().curPath();
//	//qDebug() << "curPath:" << TcpClient::getInstance().curPath();
//	//QString m_strUploadFilePath = "C:/Users/陈鑫源/Desktop/11111.rar";
//	//if (m_strUploadFilePath.isEmpty()) {
//	//	QMessageBox::warning(this, "上传文件", "上传文件名称不能为空");
//	//	return;
//	//}
//
//	//int idx = m_strUploadFilePath.lastIndexOf("/");
//	//QString fileName = m_strUploadFilePath.right(m_strUploadFilePath.size() - idx - 1);
//	//QFile file(m_strUploadFilePath);
//	//if (!file.exists()) {
//	//	QMessageBox::warning(this, "上传文件", "文件不存在");
//	//	return;
//	//}
//
//	//qint64 fileSize = file.size();
//	//if (!file.open(QIODevice::ReadOnly)) {
//	//	QMessageBox::warning(this, "上传文件", "打开文件失败");
//	//	return;
//	//}
//
//	//PDU* pdu = mkPDU(strCurPath.size() + 1 + userName.size() + 1);
//	//pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
//
//	//// 将路径写入到 caMsg 中
//	//memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
//	//// 在路径后面追加 null 字符
//	//((char*)pdu->caMsg)[strCurPath.size()] = '\0';
//
//
//
//	////*((char*)pdu->caMsg + strCurPath.size()) = '\0';
//	////pdu->caMsg[strCurPath.size()] = '\0';
//	////cout1 << "username size:" << userName.size();
//	//// 将用户名写入到 caMsg 路径后面
//	////std::string username = "your_username"; // 替换为实际的用户名变量
//	//memcpy(((char*)pdu->caMsg + strCurPath.size() + 1), userName.c_str(), userName.size());
//
//
//	//// 在用户名后面追加 null 字符
//	//pdu->caMsg[strCurPath.size() + 1 + userName.size()] = '\0';
//
//
//
//	///*PDU* pdu = mkPDU(strCurPath.size() + 1);
//	//pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
//	//memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());*/
//	//sprintf(pdu->caData, "%s %lld", fileName.toStdString().c_str(), fileSize);
//	//sockets[s]->write((char*)pdu, pdu->uiPDULen);
//	//sockets[s]->flush();
//	//free(pdu);
//
//	//QThread::sleep(1);
//
//
//	//// 文件开始上传
//	//char* pBuffer = new char[4096];
//	//qint64 ret = 0;
//	//while ((ret = file.read(pBuffer, 4096)) > 0) {
//	//	sockets[s]->write(pBuffer, ret);
//	//	if (sockets[s]->bytesToWrite() > 50 * 1024 * 1024) {
//	//		sockets[s]->flush();
//	//		//QThread::msleep(10); // 稍微等待缓冲区清空
//	//	}
//	//}
//	//delete[] pBuffer;
//	//file.close();
//	//qDebug() << "File upload completed for user:" << QString::fromStdString(userName);
//}





TcpClient::~TcpClient()
{
	for (auto i : sockets) {
		qDebug() << i->socketDescriptor() << "will delete\n";
		delete(i);
	}
	delete ui;
}

void TcpClient::loadConfig()
{
	// : 代表读取的是资源文件
	// 设置文件路径
	QFile file(":/client.config");
	// 设置打开方式，尝试打开
	if (file.open(QIODevice::ReadOnly)) {
		// 进行文件读取
		QByteArray btData = file.readAll();
		// 转换为 char *
		QString fileStrData = btData.toStdString().c_str();
		// 将换行替换为空格
		fileStrData.replace("\r\n", " ");
		// 有些人的环境换行是 \n,所以再加一步替换，防止切分失败
		fileStrData.replace("\n", " ");
		// 按行切分数据
		QStringList strList = fileStrData.split(" ");
		// 获取ip
		//m_strIp = strList.at(0);
		m_strIp = "192.168.254.137";

		// 获取port
		m_usPort = strList.at(1).toUShort();
		// 测试输出
		qDebug() << fileStrData << "\n";
		// 如果打开失败
	}
	else {
		// 错误提示框，和information相比就是界面的图片变成了红色的感叹号
		QMessageBox::critical(this, "错误", "加载配置文件错误");
	}
	// 关闭文件
	file.close();
}

TcpClient& TcpClient::getInstance()
{
	static TcpClient instance;
	return instance;
}

QTcpSocket& TcpClient::getTcpSocket()
{
	return m_tcpSocket;
}

QString TcpClient::loginName()
{
	return m_strLoginName;
}

QString TcpClient::curPath()
{
	return m_strCurPath;
}

void TcpClient::setCurPath(QString strPath)
{
	m_strCurPath = strPath;
}

void TcpClient::showConnect()
{
	QMessageBox::information(this, "成功", "TCP链接服务端成功");
	qDebug() << TcpClient::getTcpSocket().localAddress() << ' ' << TcpClient::getTcpSocket().localPort() << "\n";
}

void TcpClient::regist(PDU* pdu) {
	if (0 == strcmp(pdu->caData, REGIST_OK))
	{
		QMessageBox::information(this, "注册", REGIST_OK);
	}
	else if (0 == strcmp(pdu->caData, REGIST_FAIL))
	{
		QMessageBox::warning(this, "注册", REGIST_FAIL);
	}
}

void TcpClient::login(PDU* pdu)
{
	if (0 == strcmp(pdu->caData, LOGIN_OK))
	{
		m_strCurPath = QString("./%1").arg(m_strLoginName);
		QMessageBox::information(this, "登录", LOGIN_OK);
		//TODO-----这里实现登录成功之后的页面跳转
		OpeWidget::getInstance().setWindowTitle("User:" + userName);

		OpeWidget::getInstance().show();
		this->hide();
	}
	else if (0 == strcmp(pdu->caData, LOGIN_FAIL))
	{
		QMessageBox::warning(this, "登录", LOGIN_FAIL);
	}
}

void TcpClient::searchUser(PDU* pdu)
{
	if (0 == strcmp(SEARCH_USER_NO, pdu->caData))
	{
		QMessageBox::information(this, "搜索", QString("%1: no exists").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
	}
	else if (0 == strcmp(SEARCH_USER_ONLINE, pdu->caData))
	{
		QMessageBox::information(this, "搜索", QString("%1: online").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
	}
	else if (0 == strcmp(SEARCH_USER_OFFLINE, pdu->caData))
	{
		QMessageBox::information(this, "搜索", QString("%1: offline").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
	}
}

void TcpClient::addFriendRequest(PDU* pdu)
{
	char caName[32] = { '\0' };//请求添加好友的名称
	strncpy(caName, pdu->caData + 32, 32);
	int ret = QMessageBox::information(this, "添加好友", QString("%1 want to add you as friend.").arg(caName), QMessageBox::Yes, QMessageBox::No);
	PDU* retPdu = mkPDU(0);
	memcpy(retPdu->caData, pdu->caData, 64);
	if (ret == QMessageBox::Yes)
	{
		retPdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
	}
	else
	{
		retPdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
	}
	m_tcpSocket.write((char*)retPdu, retPdu->uiPDULen);
	free(retPdu);
	retPdu = NULL;
}

void TcpClient::addFriendAgree(PDU* pdu)
{
	char friendName[32] = { '\0' };//好友的名称
	strncpy(friendName, pdu->caData, 32);
	QMessageBox::information(this, "添加好友", QString("添加好友成功！%1 同意了您的好友添加请求").arg(friendName));
}

void TcpClient::addFriendFefuse(PDU* pdu)
{
	char friendName[32] = { '\0' };//好友的名称
	strncpy(friendName, pdu->caData, 32);
	QMessageBox::information(this, "添加好友", QString("添加好友失败！%1 拒绝了您的好友添加请求").arg(friendName));
}

void TcpClient::deleteFriend(PDU* pdu)
{
	char loginName[32] = { '\0' };//登录者的名称
	strncpy(loginName, pdu->caData + 32, 32);
	QMessageBox::information(this, "删除好友", QString("%1 已经删除了您的好友").arg(loginName));
}

void TcpClient::privateChat(PDU* pdu)
{
	if (PrivateChat::getInstace().isHidden())
	{
		PrivateChat::getInstace().setWindowTitle("cur user:" + QString(pdu->caData));
		PrivateChat::getInstace().show();
	}
	// 给最新的一个发送过来的好友发送消息
	char loginName[32] = { '\0' };
	memcpy(loginName, pdu->caData + 32, 32);
	PrivateChat::getInstace().setChatName(loginName);
	PrivateChat::getInstace().updateMsg(pdu);
}

void TcpClient::createDir(PDU* pdu)
{
	QMessageBox::information(this, "创建文件夹", pdu->caData);
}

void TcpClient::flushDir(PDU* pdu)
{
	OpeWidget::getInstance().getBook()->updateDirList(pdu);
}

void TcpClient::delDir(PDU* pdu)
{
	QMessageBox::information(this, "删除文件夹", pdu->caData);
}

void TcpClient::renameDir(PDU* pdu)
{
	QMessageBox::information(this, "重命名文件", pdu->caData);
}

void TcpClient::enterDir(PDU* pdu)
{
	// 如果请求成功了
	if (0 == strcmp(ENTER_DIR_OK, pdu->caData))
	{
		setCurPath(OpeWidget::getInstance().getBook()->getEnterPath());
		OpeWidget::getInstance().getBook()->updateDirList(pdu);
	}
	else
	{
		QMessageBox::information(this, "进入文件夹", pdu->caData);
	}
}

void TcpClient::downloadFilePre(PDU* pdu)
{
	char caFileName[32] = { '\0' };
	sscanf(pdu->caData, "%s %lld", caFileName, &(OpeWidget::getInstance().getBook()->m_iTotal));
	// 如果是有效数据
	if (strlen(caFileName) > 0 && OpeWidget::getInstance().getBook()->m_iTotal > 0)
	{
		OpeWidget::getInstance().getBook()->setDownloadStatus(true);//标记开始下载文件
		OpeWidget::getInstance().getBook()->m_iRevice = 0;
		OpeWidget::getInstance().getBook()->updateLocalDownloadFileName();
		// 只写模式打开文件，文件如果不存在则会被创建
		if (!OpeWidget::getInstance().getBook()->m_pFile.open(QIODevice::WriteOnly))
		{
			QMessageBox::warning(this, "下载文件", "下载文件失败：本地文件无法操作");
			// 由于服务器已经开始发送下载文件的数据了，这个时候应该怎么办呢？【】
			return;
		}
	}
}

void TcpClient::shareFileNote(PDU* pdu)
{
	qDebug() << "开始准备接受文件";
	char* pPath = new char[pdu->uiMsgLen];
	memcpy(pPath, (char*)pdu->caMsg, pdu->uiMsgLen);
	// aa/bb/cc/a.txt
	qDebug() << pPath;
	char* pos = strrchr(pPath, '/'); //找到最后一个 / 出现的位置
	qDebug() << pos;
	if (NULL != pos)
	{
		pos++; // 向右移动一位，因为 / 这个字符我们不需要，只需要文件名称，即a.txt
		QString strNote = QString("%1 share file -> %2\n do you accecpt").arg(pdu->caData).arg(pos);
		int ret = QMessageBox::question(this, "共享文件", strNote);
		if (QMessageBox::Yes == ret)
		{
			PDU* retPdu = mkPDU(pdu->uiMsgLen);
			retPdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_REQUEST;
			memcpy(retPdu->caMsg, pdu->caMsg, pdu->uiMsgLen);
			QString strName = TcpClient::getInstance().loginName();
			strcpy(retPdu->caData, strName.toStdString().c_str());
			m_tcpSocket.write((char*)retPdu, retPdu->uiPDULen);
		}
	}
}

void TcpClient::recvMsg()
{

	// 如果是文件下载状态中
	if (OpeWidget::getInstance().getBook()->getDownloadStatus())
	{
		QByteArray buffer = m_tcpSocket.readAll();
		// 简化命名使用
		Book* pBook = OpeWidget::getInstance().getBook();
		pBook->m_pFile.write(buffer);
		pBook->m_iRevice += buffer.size();
		if (pBook->m_iTotal == pBook->m_iRevice)
		{
			pBook->m_pFile.close();
			pBook->m_iTotal = 0;
			pBook->m_iRevice = 0;
			pBook->setDownloadStatus(false);
			QMessageBox::critical(this, "下载文件", "下载文件成功");
		}
		else if (pBook->m_iTotal < pBook->m_iRevice)
		{
			pBook->m_pFile.close();
			pBook->m_iTotal = 0;
			pBook->m_iRevice = 0;
			pBook->setDownloadStatus(false);
			QMessageBox::critical(this, "下载文件", "下载文件失败：传输的文件错误");
		}
		//其他情况代表数据还未下载完成
		return;
	}
	//通信对象的总大小
	uint uiPDULen = 0;
	// 先获取通信对象的总大小
	m_tcpSocket.read((char*)&uiPDULen, sizeof(int));
	// 消息数据大小
	uint uiMsgLen = uiPDULen - sizeof(PDU);
	// 创建通信对象空间
	PDU* pdu = mkPDU(uiMsgLen);
	// 更新通信对象总大小
	pdu->uiPDULen = uiPDULen;
	// 读取剩余数据内容到通信对象中
	m_tcpSocket.read((char*)pdu + sizeof(int), uiPDULen - sizeof(int));
	switch (pdu->uiMsgType)
	{
	case ENUM_MSG_TYPE_REGIST_RESPOND:
	{
		regist(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_REGIST_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_LOGIN_RESPOND:
	{
		login(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_LOGIN_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
	{
		OpeWidget::getInstance().getFriend()->showAllOnlineUser(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_ALL_ONLINE_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_SEARCH_USER_RESPOND:
	{
		searchUser(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_SEARCH_USER_RESPOND";
		break;
	}
	// TODO-----服务器转发请求  添加好友相关
	case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
		addFriendRequest(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_ADD_FRIEND_REQUEST";
		break;
	}
	case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND: {
		QMessageBox::information(this, "添加好友", pdu->caData);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_ADD_FRIEND_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_ADD_FRIEND_AGREE: {
		addFriendAgree(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_ADD_FRIEND_AGREE";
		break;
	}
	case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE: {
		addFriendFefuse(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_ADD_FRIEND_REFUSE";
		break;
	}
	case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND: {
		OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND";
		break;
	}
										   // 接受转发回来的消息
	case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: {
		deleteFriend(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST";
		break;
	}
	case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND: {
		QMessageBox::information(this, "删除好友", pdu->caData);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND";
		break;
	}
											// 转发消息
	case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST: {
		privateChat(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST";
		break;
	}
	case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: {
		OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_GROUP_CHAT_REQUEST";
		break;
	}
	case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
	{
		createDir(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_CREATE_DIR_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_FLUSH_DIR_RESPOND:
	{
		flushDir(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_FLUSH_DIR_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_DEL_DIR_RESPOND:
	{
		delDir(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_DEL_DIR_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_RENAME_DIR_RESPOND:
	{
		renameDir(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_RENAME_DIR_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
	{
		enterDir(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_ENTER_DIR_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
	{
		QMessageBox::information(this, "上传文件", pdu->caData);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_DEL_FILE_RESPOND:
	{
		QMessageBox::information(this, "删除文件", pdu->caData);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_DEL_FILE_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
	{
		qDebug() << pdu->caData;
		downloadFilePre(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:
	{
		QMessageBox::information(this, "分享文件", pdu->caData);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_SHARE_FILE_RESPOND";
		break;
	}
	case ENUM_MSG_TYPE_SHARE_FILE_NOTE:
	{
		shareFileNote(pdu);
		qDebug() << "receive msg type: ENUM_MSG_TYPE_SHARE_FILE_NOTE";
		break;
	}
	default:
	{
		qDebug() << "receive msg type: UNKNOWN";
		break;
	}
	}

	// 释放内存
	free(pdu);
	pdu = NULL;
}



void TcpClient::on_login_pb_clicked()
{
	QString strName = ui->name_le->text();
	QString strPwd = ui->pwd_le->text();
	// 账号密码不为空时才能登录
	if (!strName.isEmpty() && !strPwd.isEmpty())
	{
		// 保存登录用户的名称
		m_strLoginName = strName;
		userName = strName;
		PDU* pdu = mkPDU(0);
		pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
		// 前32个字符复制账号名

		//TODO-----这里可以限制一下字段的长度  不超过32
		strncpy(pdu->caData, strName.toStdString().c_str(), 32);
		// 后32个字符复制密码

		strncpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
		// 发送通信对象给服务器
		m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
		//m_tcpSocket.flush();
		// 释放内存
		free(pdu);
		pdu = NULL;
	}
	else
	{
		QMessageBox::critical(this, "登录", "登录失败：用户名或密码不能为空");
	}
}

void TcpClient::on_regist_pb_clicked()
{
	QString strName = ui->name_le->text();
	QString strPwd = ui->pwd_le->text();
	// 账号密码不为空时才能注册
	if (!strName.isEmpty() && !strPwd.isEmpty())
	{
		PDU* pdu = mkPDU(0);
		pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
		// 前32个字符复制账号名
		strncpy(pdu->caData, strName.toStdString().c_str(), 32);
		// 后32个字符复制密码
		strncpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
		// 发送通信对象给服务器
		m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
		// 释放内存
		free(pdu);
		pdu = NULL;
	}
	else
	{
		QMessageBox::critical(this, "注册", "注册失败：用户名或密码不能为空");
	}
}

void TcpClient::on_cancel_pb_clicked()
{
	// 链接服务器
	m_tcpSocket.connectToHost(QHostAddress(m_strIp), m_usPort);
}

void TcpClient::on_pwd_le_returnPressed()
{
	on_login_pb_clicked();
}


