#include "uploadprogressdialog.h"
#include <QPushButton>

UploadProgressDialog::UploadProgressDialog(const QString& fileName, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("文件上传进度");
    setFixedSize(400, 150);

    m_pFileNameLabel = new QLabel(QString("正在上传: %1").arg(fileName));
    m_pProgressLabel = new QLabel("准备上传...");
    m_pProgressBar = new QProgressBar;
    m_pProgressBar->setRange(0, 100);
    m_pProgressBar->setValue(0);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(m_pFileNameLabel);
    layout->addWidget(m_pProgressBar);
    layout->addWidget(m_pProgressLabel);

    setLayout(layout);
}

void UploadProgressDialog::updateProgress(int percent, const QString& speed, const QString& remainingTime)
{
    m_pProgressBar->setValue(percent);
    m_pProgressLabel->setText(QString("已上传: %1% - 速度: %2 - 剩余时间: %3")
                            .arg(percent)
                            .arg(speed)
                            .arg(remainingTime));
}

void UploadProgressDialog::setCompleted()
{
    m_pProgressLabel->setText("上传完成");
    m_pProgressBar->setValue(100);
} 