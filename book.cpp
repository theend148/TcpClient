#include "book.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileDialog>
#include <algorithm>
#include <vector>

#include "QInputDialog"
#include "QMessageBox"
#include "my.h"
#include "sharefile.h"
#include "tcpclient.h"
#include "uploadprogressdialog.h"

qint64 Book::CHUNK_SIZE = 20 * 1024 * 1024;  // 2MB

// TODO-----这里的组件创建方式梳理一下
Book::Book(QWidget* parent) : QWidget(parent) {
    m_pDownload = false;
    m_pTimer = new QTimer;
    m_pBookListW = new QListWidget;
    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pRenameDirPB = new QPushButton("重命名文件夹");
    m_pFlushDirPB = new QPushButton("刷新文件夹");
    m_pUploadFilePB = new QPushButton("上传文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pDownloadFilePB = new QPushButton("下载文件");
    m_pShareFilePB = new QPushButton("分享文件");

    QVBoxLayout* dirLayout = new QVBoxLayout;
    dirLayout->addWidget(m_pReturnPB);
    dirLayout->addWidget(m_pCreateDirPB);
    dirLayout->addWidget(m_pDelDirPB);
    dirLayout->addWidget(m_pRenameDirPB);
    dirLayout->addWidget(m_pFlushDirPB);

    QVBoxLayout* fileLayout = new QVBoxLayout;
    fileLayout->addWidget(m_pUploadFilePB);
    fileLayout->addWidget(m_pDelFilePB);
    fileLayout->addWidget(m_pDownloadFilePB);
    fileLayout->addWidget(m_pShareFilePB);

    QHBoxLayout* hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(m_pBookListW);
    hBoxLayout->addLayout(dirLayout);
    hBoxLayout->addLayout(fileLayout);

    setLayout(hBoxLayout);
    // 槽与信号
    connect(m_pCreateDirPB, &QPushButton::clicked, this, &Book::createDir);
    connect(m_pFlushDirPB, &QPushButton::clicked, this, &Book::flushDir);
    connect(m_pDelDirPB, &QPushButton::clicked, this, &Book::delDir);
    connect(m_pRenameDirPB, &QPushButton::clicked, this, &Book::renameDir);
    connect(m_pBookListW, &QListWidget::doubleClicked, this, &Book::enterDir);
    connect(m_pReturnPB, &QPushButton::clicked, this, &Book::returnPre);
    connect(m_pUploadFilePB, &QPushButton::clicked, this, &Book::uploadPre);
    connect(m_pTimer, &QTimer::timeout, this, &Book::processUploadQueue);
    connect(m_pDelFilePB, &QPushButton::clicked, this, &Book::delFile);
    connect(m_pDownloadFilePB, &QPushButton::clicked, this,
            &Book::downloadFile);
    connect(m_pShareFilePB, &QPushButton::clicked, this, &Book::shareFile);

    // 添加上传进度更新的定时器
    m_pProgressTimer = new QTimer(this);
    connect(m_pProgressTimer, &QTimer::timeout, this,
            &Book::updateUploadProgress);

    m_isUploading = false;
}

void Book::updateDirList(const PDU* pdu) {
    if (NULL == pdu) {
        return;
    }
    // 清除之前的列表
    m_pBookListW->clear();

    // 创建两个向量分别存储文件夹和文件
    std::vector<QString> directories;
    std::vector<QString> files;

    // 解析所有文件信息
    FileInfo* pFileInfo = NULL;
    int iFileCount = pdu->uiMsgLen / sizeof(FileInfo);
    for (int i = 0; i < iFileCount; i++) {
        pFileInfo = (FileInfo*)(pdu->caMsg) + i;
        if (0 == pFileInfo->iFileType) {
            directories.push_back(QString(pFileInfo->caFileName));
        } else if (1 == pFileInfo->iFileType) {
            files.push_back(QString(pFileInfo->caFileName));
        }
    }

    // 对文件夹和文件名分别排序
    std::sort(directories.begin(), directories.end());
    std::sort(files.begin(), files.end());

    // 先添加所有文件夹
    for (int i = 2; i < directories.size(); i++) {
        QListWidgetItem* pItem = new QListWidgetItem;
        pItem->setIcon(QIcon(QPixmap("../../../map/dir.png")));
        pItem->setText(directories[i]);
        m_pBookListW->addItem(pItem);
    }

    // 再添加所有文件
    for (const QString& fileName : files) {
        QListWidgetItem* pItem = new QListWidgetItem;
        pItem->setIcon(QIcon(QPixmap("../../../map/reg.jpg")));
        pItem->setText(fileName);
        m_pBookListW->addItem(pItem);
    }
}

QString Book::getEnterPath() { return m_enterPath; }

void Book::setDownloadStatus(bool status) { m_pDownload = status; }

bool Book::getDownloadStatus() { return m_pDownload; }

QString Book::getSaveFilePath() { return m_strSaveFilePath; }

void Book::updateLocalDownloadFileName() {
    m_pFile.setFileName(m_strSaveFilePath);
}

QString Book::getShareFileName() { return m_shareFileName; }

void Book::createDir()  // 创建文件夹
{
    QString strDirName =
        QInputDialog::getText(this, "新建文件夹", "新文件夹名称");
    if (strDirName.isEmpty()) {
        QMessageBox::warning(this, "新建文件夹", "新文件夹名称不能为空");
        return;
    }
    if (strDirName.size() > 32) {
        QMessageBox::warning(this, "新建文件夹",
                             "新文件夹名称不能超过32个字符");
        return;
    }
    QString strLoginName = TcpClient::getInstance().loginName();
    QString strCurPath = TcpClient::getInstance().curPath();
    PDU* pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
    // 用户名、新文件夹名称放在caData， 当前路径放在caMsg
    strncpy(pdu->caData, strLoginName.toStdString().c_str(),
            strLoginName.size());
    strncpy(pdu->caData + 32, strDirName.toStdString().c_str(),
            strDirName.size());
    memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(),
           strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::flushDir() {
    QString strCurPath = TcpClient::getInstance().curPath();
    PDU* pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_DIR_REQUEST;
    strncpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(),
            strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::delDir() {
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem* pItem = m_pBookListW->currentItem();
    if (NULL == pItem) {
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
        return;
    }
    QString strDelName = pItem->text();
    PDU* pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
    strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.size());
    memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(),
           strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::renameDir() {
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem* pItem = m_pBookListW->currentItem();
    if (NULL == pItem) {
        QMessageBox::warning(this, "重命名文件", "请选择要重命名的文件");
        return;
    }
    QString strOldName = pItem->text();
    QString strNewName =
        QInputDialog::getText(this, "重命名文件", "请输入新的文件名");
    if (strNewName.isEmpty()) {
        QMessageBox::warning(this, "重命名文件", "新文件名不能为空");
        return;
    }
    PDU* pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_DIR_REQUEST;
    strncpy(pdu->caData, strOldName.toStdString().c_str(), strOldName.size());
    strncpy(pdu->caData + 32, strNewName.toStdString().c_str(),
            strNewName.size());
    memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(),
           strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::enterDir(const QModelIndex& index) {
    QString strDirName = index.data().toString();

    QString strCurPath = TcpClient::getInstance().curPath();

    // 缓存数据
    m_enterPath = strCurPath + QString("/%1").arg(strDirName);

    QFileInfo fileInfo(m_enterPath);
    // 处理双击文件下载
    if (fileInfo.isFile()) {
        downloadFile();
    } else {
        PDU* pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
        strncpy(pdu->caData, strDirName.toStdString().c_str(),
                strDirName.size());
        memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(),
               strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,
                                                      pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::returnPre() {
    // 当前目录
    QString strCurPath = TcpClient::getInstance().curPath();
    // 用户根目录
    QString strRootPath = "../users/" + TcpClient::getInstance().loginName();
    // 如果当前目录就是用户根目录，代表没有上级目录了
    if (strCurPath == strRootPath) {
        QMessageBox::warning(this, "返回",
                             "返回上一级失败：当前已经在用户根目录了");
        return;
    }
    // ./alice/aa/bb -> ./alice/aa
    int idx = strCurPath.lastIndexOf('/');
    strCurPath.remove(idx, strCurPath.size() - idx);
    // 更新当前所在目录位置
    TcpClient::getInstance().setCurPath(strCurPath);
    // 刷新目录文件为当前目录位置的文件
    flushDir();
}

QString Book::calculateFileMD5(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Md5);
    if (hash.addData(&file)) {
        file.close();
        return hash.result().toHex();
    }
    file.close();
    return QString();
}

void Book::createUploadQueue() {
    m_uploadQueue.clear();
    qint64 offset = 0;
    qint64 chunkIndex = 0;

    while (offset < m_totalFileSize) {
        FileChunk chunk;
        chunk.index = chunkIndex++;
        chunk.offset = offset;
        chunk.size = qMin(CHUNK_SIZE, m_totalFileSize - offset);
        chunk.uploaded = m_uploadedChunks.contains(chunk.index);

        if (!chunk.uploaded) {
            std::cout << "chunk" << chunk.index << "not complete\n";
            m_uploadQueue.enqueue(chunk);
        }

        offset += CHUNK_SIZE;
    }
}

void Book::initializeUpload() {
    QString strCurPath = TcpClient::getInstance().curPath();
    int idx = m_strUploadFilePath.lastIndexOf('/');
    QString fileName =
        m_strUploadFilePath.right(m_strUploadFilePath.size() - idx - 1);

    // 构造要发送的信息字符串，以确保分配足够的空间
    QString msgStr = QString("%1 %2 %3 %4")
                         .arg(fileName)
                         .arg(m_totalFileSize)
                         .arg(m_strFileMD5)
                         .arg(CHUNK_SIZE);

    // 使用实际的字节大小
    int msgLen = msgStr.toUtf8().size() + 1;  // +1 for null terminator

    // 发送初始化请求
    PDU* pdu = mkPDU(msgLen);
    pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_INIT_REQUEST;

    // 将路径存储在caData中
    strncpy(pdu->caData, strCurPath.toStdString().c_str(), 64);

    // 在caMsg中存储：文件名 文件大小 MD5 分片大小
    QByteArray msgData = msgStr.toUtf8();
    memcpy(pdu->caMsg, msgData.data(),
           msgData.size() + 1);  // +1 to include null terminator

    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::uploadPre() {
    QString strCurPath = TcpClient::getInstance().curPath();
    QStringList filePaths =
        QFileDialog::getOpenFileNames(this, "选择要上传的文件");

    if (filePaths.isEmpty()) {
        return;
    }

    // 如果进度对话框不存在，创建它
    if (!m_pUploadDialog) {
        m_pUploadDialog = new UploadProgressDialog(this);
    }

    // 处理每个选中的文件
    for (const QString& filePath : filePaths) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "上传文件",
                                 QString("无法打开文件: %1").arg(filePath));
            continue;
        }

        UploadFileInfo fileInfo;
        fileInfo.filePath = filePath;
        fileInfo.fileSize = file.size();
        file.close();

        fileInfo.fileMD5 = calculateFileMD5(filePath);
        if (fileInfo.fileMD5.isEmpty()) {
            QMessageBox::warning(this, "上传文件",
                                 QString("计算文件MD5失败: %1").arg(filePath));
            continue;
        }

        int idx = filePath.lastIndexOf('/');
        fileInfo.fileName = filePath.right(filePath.size() - idx - 1);

        // 将文件添加到进度对话框
        m_pUploadDialog->addFile(fileInfo.fileName, fileInfo.fileSize);

        // 将文件信息添加到上传队列
        m_uploadFileQueue.enqueue(fileInfo);
    }

    // 显示进度对话框
    m_pUploadDialog->show();

    // 如果当前没有在上传，开始上传队列中的第一个文件
    if (!m_isUploading && !m_uploadFileQueue.isEmpty()) {
        startNextFileUpload();
    }
}

void Book::startNextFileUpload() {
    if (m_uploadFileQueue.isEmpty()) {
        m_isUploading = false;
        return;
    }

    m_isUploading = true;
    UploadFileInfo& fileInfo = m_uploadFileQueue.head();

    // 设置当前文件信息
    m_strUploadFilePath = fileInfo.filePath;
    m_totalFileSize = fileInfo.fileSize;
    m_strFileMD5 = fileInfo.fileMD5;

    // 清空已上传分片列表
    m_uploadedChunks.clear();

    // 发送初始化请求
    initializeUpload();

    m_uploadStartTime = QDateTime::currentMSecsSinceEpoch();
    m_lastUploadedBytes = 0;

    // 启动进度更新定时器
    m_pProgressTimer->start(1000);
}

void Book::processUploadQueue() {
    m_pTimer->stop();

    if (m_uploadQueue.isEmpty()) {
        // 当前文件上传完成
        if (!m_uploadFileQueue.isEmpty()) {
            QString completedFileName = m_uploadFileQueue.head().fileName;
            if (m_pUploadDialog) {
                m_pUploadDialog->setFileCompleted(completedFileName);
            }
            m_uploadFileQueue.dequeue();

            // 停止进度更新定时器
            m_pProgressTimer->stop();
        }

        // 检查是否还有文件需要上传
        if (!m_uploadFileQueue.isEmpty()) {
            // 开始上传下一个文件
            QTimer::singleShot(1000, this, &Book::startNextFileUpload);
        } else {
            // 所有文件上传完成
            m_isUploading = false;

            // 如果进度对话框存在且没有活动的上传
            if (m_pUploadDialog && !m_pUploadDialog->hasActiveUploads()) {
                QTimer::singleShot(2000, [this]() {
                    if (m_pUploadDialog) {
                        m_pUploadDialog->close();
                        m_pUploadDialog->hide();
                        m_pUploadDialog->deleteLater();
                        m_pUploadDialog = nullptr;
                    }
                });
            }
        }

        flushDir();
        return;
    }

    FileChunk chunk = m_uploadQueue.dequeue();
    QFile file(m_strUploadFilePath);

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }

    // 定位到分片位置
    if (!file.seek(chunk.offset)) {
        file.close();
        QMessageBox::warning(this, "上传文件", "文件定位失败");
        return;
    }
    char* pBuffer = new char[chunk.size];

    // 读取分片数据
    int chunkData = file.read(pBuffer, chunk.size);
    file.close();

    if (chunkData != chunk.size) {
        QMessageBox::warning(this, "上传文件", "读取分片数据失败");
        return;
    }

    // 发送分片数据
    PDU* pdu = mkPDU(chunkData);
    pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_CHUNK_REQUEST;

    // 在caData中存储分片信息：文件MD5 分片索引 分片大小
    sprintf(pdu->caData, "%s %lld %lld", m_strFileMD5.toStdString().c_str(),
            chunk.index, chunk.size);

    // 将分片数据复制到消息体
    memcpy(pdu->caMsg, pBuffer, chunkData);

    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    delete[] pBuffer;
    pBuffer = nullptr;
    pdu = NULL;

    // 设置定时器等待下一个分片上传
}

// TODO-----这里没有设置消息类型   服务器那边因为实现在收到上传文件的时候
// 已经将正在上传文件的状态设置成了true  所以直接被条件捕获
void Book::uploadFileData() {
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    // 如果打开文件失败
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }
    // 有人做过实验，4096传输数据时效率最高
    char* pBuffer = new char[4096];
    qint64 ret = 0;
    // tanchuang
    while (true) {
        // 读取文件中的数据
        ret = file.read(pBuffer, 4096);
        // 如果读取到了文件中的数据，则发送给服务器
        if (ret > 0 && ret <= 4096) {
            TcpClient::getInstance().getTcpSocket().write(pBuffer, ret);

            if (TcpClient::getInstance().getTcpSocket().bytesToWrite() >
                50 * 1024 * 1024) {
                TcpClient::getInstance().getTcpSocket().flush();
            }
            /*cout << TcpClient::getInstance().getTcpSocket().bytesToWrite();
tanchuang;*/
            // TODO-----这里使用flush  强制强制将缓冲区中的数据写入到输出流中，
            // 确保数据被及时发送或存储，而不是等待缓冲区满或流被关闭
            //  可以设置一个阈值  使用 bytesToWrite()获取缓冲区中待发送的字节数
            //  超过阈值 使用flush函数
            // TcpClient::getInstance().getTcpSocket().flush();
        }
        // 如果文件读取结束了，就结束循环
        else if (ret == 0) {
            break;
        }
        // 如果ret < 0 或者 ret > 4096,则代表读取错误了
        else {
            QMessageBox::warning(this, "上传文件",
                                 "上传文件失败：读取文件内容失败");
            break;
        }
    }
    delete[] pBuffer;
    pBuffer = NULL;
}

void Book::delFile() {
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem* pItem = m_pBookListW->currentItem();
    if (NULL == pItem) {
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
        return;
    }
    QString strDelName = pItem->text();
    PDU* pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
    strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.size());
    memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(),
           strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::downloadFile() {
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem* pItem = m_pBookListW->currentItem();
    if (NULL == pItem) {
        QMessageBox::warning(this, "下载文件", "请选择要下载的文件");
        return;
    }
    QString strDownloadName = pItem->text();
    QString strSaveFilePath = QFileDialog::getSaveFileName();
    if (strSaveFilePath.isEmpty()) {
        QMessageBox::warning(this, "下载文件", "请指定要保存的位置");
        m_strSaveFilePath.clear();
        return;
    }
    m_strSaveFilePath = strSaveFilePath;
    PDU* pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
    strcpy(pdu->caData, strDownloadName.toStdString().c_str());
    memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(),
           strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::shareFile() {
    QListWidgetItem* pItem = m_pBookListW->currentItem();
    if (NULL == pItem) {
        QMessageBox::warning(this, "分享文件", "请选择要分享的文件");
        return;
    }
    m_shareFileName = pItem->text();
    Friend* pFriend = OpeWidget::getInstance().getFriend();
    QListWidget* pFriendList = pFriend->getFriendList();
    ShareFile::getInstance().updateFriend(pFriendList);
    if (ShareFile::getInstance().isHidden()) {
        ShareFile::getInstance().show();
    }
}

// 修改updateUploadProgress函数
void Book::updateUploadProgress() {
    if (m_totalFileSize <= 0) return;

    UploadFileInfo& currentFile = m_uploadFileQueue.head();

    // 计算已上传的字节数
    qint64 uploadedBytes = 0;
    for (qint64 chunkIndex : m_uploadedChunks) {
        qint64 chunkSize =
            qMin(CHUNK_SIZE, m_totalFileSize - chunkIndex * CHUNK_SIZE);
        uploadedBytes += chunkSize;
    }

    // 计算当前进度百分比
    int progressPercent =
        static_cast<int>((uploadedBytes * 100) / m_totalFileSize);

    // 计算上传速度
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsedTime = currentTime - m_uploadStartTime;

    if (elapsedTime > 0) {
        double uploadSpeed = (uploadedBytes - m_lastUploadedBytes) /
                             (elapsedTime / 1000.0) / 1024.0;
        m_lastUploadedBytes = uploadedBytes;
        m_uploadStartTime = currentTime;

        int remainingSeconds = 0;
        if (uploadSpeed > 0) {
            remainingSeconds = static_cast<int>(
                (m_totalFileSize - uploadedBytes) / 1024.0 / uploadSpeed);
        }

        QString speedText =
            QString("%1 KB/s").arg(QString::number(uploadSpeed, 'f', 2));
        QString remainingText;

        if (remainingSeconds > 60) {
            int minutes = remainingSeconds / 60;
            int seconds = remainingSeconds % 60;
            remainingText = QString("%1分%2秒").arg(minutes).arg(seconds);
        } else {
            remainingText = QString("%1秒").arg(remainingSeconds);
        }

        // 更新进度对话框中当前文件的进度
        if (m_pUploadDialog) {
            m_pUploadDialog->updateFileProgress(currentFile.fileName,
                                                progressPercent, speedText,
                                                remainingText);
        }
    }
}

// 在收到服务器确认分片上传成功的响应后调用此函数
void Book::onChunkUploadSuccess(qint64 chunkIndex) {
    m_uploadedChunks.append(chunkIndex);

    // 继续处理队列中的下一个分片
    m_pTimer->start(100);  // 短暂延迟后继续上传
}

// 在Book的析构函数中添加清理代码
Book::~Book() {
    if (m_pUploadDialog) {
        delete m_pUploadDialog;
        m_pUploadDialog = nullptr;
    }
}
