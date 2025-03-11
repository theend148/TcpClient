#ifndef UPLOADPROGRESSDIALOG_H
#define UPLOADPROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QMap>

struct FileProgressInfo {
	QString fileName;
	qint64 fileSize;
	int progress;
	QString speed;
	QString remainingTime;
	bool completed;
};

class UploadProgressDialog : public QDialog {
	Q_OBJECT
public:
	explicit UploadProgressDialog(QWidget* parent = nullptr);

	void addFile(const QString& fileName, qint64 fileSize);
	void updateFileProgress(const QString& fileName, int percent, const QString& speed, const QString& remainingTime);
	void setFileCompleted(const QString& fileName);
	bool hasActiveUploads() const;

private:
	QTableWidget* m_pProgressTable;
	QMap<QString, int> m_fileRows;  // 文件名到行号的映射
	void setupUI();
	void updateTotalProgress();
};

#endif // UPLOADPROGRESSDIALOG_H 