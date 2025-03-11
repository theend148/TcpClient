#ifndef UPLOADPROGRESSDIALOG_H
#define UPLOADPROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>

class UploadProgressDialog : public QDialog {
    Q_OBJECT
public:
    explicit UploadProgressDialog(const QString& fileName, QWidget* parent = nullptr);
    void updateProgress(int percent, const QString& speed, const QString& remainingTime);
    void setCompleted();

private:
    QLabel* m_pFileNameLabel;
    QLabel* m_pProgressLabel;
    QProgressBar* m_pProgressBar;
};

#endif // UPLOADPROGRESSDIALOG_H 