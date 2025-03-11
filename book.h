#ifndef BOOK_H
#define BOOK_H

#include <QFile>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QPushButton>
#include <QQueue>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QtCore/QCryptographicHash>
#include <QProgressBar>
#include <QLabel>

#include "protocol.h"
#include "uploadprogressdialog.h"

struct FileChunk {
    qint64 index;
    qint64 offset;
    qint64 size;
    bool uploaded;
};

class Book : public QWidget {
    Q_OBJECT
public:
    explicit Book(QWidget* parent = 0);
    void updateDirList(const PDU* pdu);
    QString getEnterPath();
    void setDownloadStatus(bool status);  // 设置下载状态
    bool getDownloadStatus();
    QString getSaveFilePath();
    void updateLocalDownloadFileName();  // 更新本地下载文件的文件名称
    QString getShareFileName();
    qint64 m_iTotal;   // 下载文件的总字节大小
    qint64 m_iRevice;  // 下载文件目前接受到文件的大小
    QFile m_pFile;     // 用于本地下载文件使用
    ~Book();
signals:

public slots:
    void createDir();
    void flushDir();
    void delDir();
    void renameDir();
    void enterDir(const QModelIndex& index);
    // 返回上一级
    void returnPre();
    // 上传文件-预先发送消息
    void uploadPre();
    // 真实发送文件
    void uploadFileData();
    void delFile();
    void downloadFile();
    void shareFile();
    void initializeUpload();
    void processUploadQueue();
    QString calculateFileMD5(const QString& filePath);
    void createUploadQueue();
    void updateUploadProgress();
    void onChunkUploadSuccess(qint64 chunkIndex);

private:
    QListWidget* m_pBookListW;
    QPushButton* m_pReturnPB;
    QPushButton* m_pCreateDirPB;
    QPushButton* m_pDelDirPB;
    QPushButton* m_pRenameDirPB;
    QPushButton* m_pFlushDirPB;
    QPushButton* m_pUploadFilePB;
    QPushButton* m_pDelFilePB;
    QPushButton* m_pDownloadFilePB;
    QPushButton* m_pShareFilePB;
    QString m_enterPath;          // 进入文件夹的路径
    QString m_strUploadFilePath;  // 上传文件的路径
    QString m_strSaveFilePath;    // 保存文件的路径
    bool m_pDownload;             // 是否处于文件下载状态
    QString m_shareFileName;
    QString m_strFileMD5;

    QTimer* m_pProgressTimer;
    qint64 m_uploadStartTime;
    qint64 m_lastUploadedBytes;
    UploadProgressDialog* m_pUploadDialog;

public:
    QQueue<FileChunk> m_uploadQueue;
    QVector<qint64> m_uploadedChunks;
    static qint64 CHUNK_SIZE;  // 2MB
    qint64 m_totalFileSize;
    QTimer* m_pTimer;  // 定时器执行上传文件，【防止发送文件数据过快导致粘包】
};


#endif  // BOOK_H
