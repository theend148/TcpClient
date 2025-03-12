#include "downloadprogressdialog.h"

DownloadProgressDialog::DownloadProgressDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("文件下载进度");
    setFixedSize(300, 100);

    QVBoxLayout *layout = new QVBoxLayout(this);
    
    m_statusLabel = new QLabel(this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_progressBar);
}

void DownloadProgressDialog::updateProgress(qint64 received, qint64 total) {
    int percentage = (received * 100) / total;
    m_progressBar->setValue(percentage);
    m_statusLabel->setText(QString("已下载: %1 / %2 字节").arg(received).arg(total));
} 