#include "uploadprogressdialog.h"
#include <QHeaderView>

UploadProgressDialog::UploadProgressDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("文件上传进度");
    setMinimumSize(600, 400);
    
    // 初始化表格
    m_pProgressTable = new QTableWidget(this);
    
    setupUI();
}

void UploadProgressDialog::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // 配置表格
    m_pProgressTable->setColumnCount(5);
    m_pProgressTable->setHorizontalHeaderLabels(
        QStringList() << "文件名" << "大小" << "进度" << "速度" << "剩余时间");
    
    // 设置表格属性
    m_pProgressTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_pProgressTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_pProgressTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_pProgressTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_pProgressTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    
    m_pProgressTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pProgressTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    layout->addWidget(m_pProgressTable);
    setLayout(layout);
}

void UploadProgressDialog::addFile(const QString& fileName, qint64 fileSize)
{
    qDebug() << m_pProgressTable;
    if (m_pProgressTable == nullptr) {
        qDebug() << "m_pProgressTable is nullptr";
    }
    int row = m_pProgressTable->rowCount();
    m_pProgressTable->insertRow(row);
    m_fileRows[fileName] = row;
    
    // 文件名
    m_pProgressTable->setItem(row, 0, new QTableWidgetItem(fileName));
    
    // 文件大小
    QString sizeStr;
    if (fileSize < 1024) {
        sizeStr = QString("%1 B").arg(fileSize);
    } else if (fileSize < 1024 * 1024) {
        sizeStr = QString("%1 KB").arg(fileSize / 1024.0, 0, 'f', 2);
    } else if (fileSize < 1024 * 1024 * 1024) {
        sizeStr = QString("%1 MB").arg(fileSize / 1024.0 / 1024.0, 0, 'f', 2);
    } else {
        sizeStr = QString("%1 GB").arg(fileSize / 1024.0 / 1024.0 / 1024.0, 0, 'f', 2);
    }
    m_pProgressTable->setItem(row, 1, new QTableWidgetItem(sizeStr));
    
    // 初始进度
    m_pProgressTable->setItem(row, 2, new QTableWidgetItem("0%"));
    m_pProgressTable->setItem(row, 3, new QTableWidgetItem("等待中"));
    m_pProgressTable->setItem(row, 4, new QTableWidgetItem("-"));
}

void UploadProgressDialog::updateFileProgress(const QString& fileName, int percent, 
                                           const QString& speed, const QString& remainingTime)
{
    if (!m_fileRows.contains(fileName)) return;
    
    int row = m_fileRows[fileName];
    m_pProgressTable->setItem(row, 2, new QTableWidgetItem(QString("%1%").arg(percent)));
    m_pProgressTable->setItem(row, 3, new QTableWidgetItem(speed));
    m_pProgressTable->setItem(row, 4, new QTableWidgetItem(remainingTime));
    
    // 设置颜色
    for (int col = 0; col < m_pProgressTable->columnCount(); ++col) {
        QTableWidgetItem* item = m_pProgressTable->item(row, col);
        if (item) {
            item->setBackground(QColor(255, 255, 255));
        }
    }
}

void UploadProgressDialog::setFileCompleted(const QString& fileName)
{
    if (!m_fileRows.contains(fileName)) return;
    
    int row = m_fileRows[fileName];
    m_pProgressTable->setItem(row, 2, new QTableWidgetItem("100%"));
    m_pProgressTable->setItem(row, 3, new QTableWidgetItem("完成"));
    m_pProgressTable->setItem(row, 4, new QTableWidgetItem("-"));
    
    // 设置完成状态的颜色
    for (int col = 0; col < m_pProgressTable->columnCount(); ++col) {
        QTableWidgetItem* item = m_pProgressTable->item(row, col);
        if (item) {
            item->setBackground(QColor(200, 255, 200));  // 浅绿色表示完成
        }
    }
}

bool UploadProgressDialog::hasActiveUploads() const
{
    return m_pProgressTable->rowCount() > 0;
} 