#ifndef DOWNLOADPROGRESSDIALOG_H
#define DOWNLOADPROGRESSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

class DownloadProgressDialog : public QDialog {
    Q_OBJECT
public:
    explicit DownloadProgressDialog(QWidget *parent = nullptr);
    void updateProgress(qint64 received, qint64 total);

private:
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
};

#endif  // DOWNLOADPROGRESSDIALOG_H