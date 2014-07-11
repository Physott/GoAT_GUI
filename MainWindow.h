#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QFile>
#include "ConfigurationDialog.h"
#include "TabRunByRun.h"
#include "TabComplete.h"
#include "TabLog.h"

#include "ConfigFile.h"
#include "ConfigGui.h"
//#include "Functions.cpp"


#include "TGraph.h"
#include "TQtWidget.h"
#include "TCanvas.h"
#include "TLine.h"

#include "Functions.cpp"
#include <QFileSystemWatcher>
#include <QProcess>
#include <TROOT.h>
#include <QFuture>

#include <QWaitCondition>
#include <QMutex>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setAbsolutePath(std::string);


private:
    bool continueScanning;
    std::vector<std::string> ACQUFilesQueue;
    std::vector<std::string> FinishedACQUFiles;

    Ui::MainWindow *ui;

    QFuture<void> future;
    QThread *thread;

    void FileChecker();
    void TakeANap(int ms);

    ConfigurationDialog *configurationDialog;

    TabRunByRun* tabRunByRun;
    TabComplete* tabComplete;
    TabLog* tabLog;

    std::string absolutePath;
    ConfigGUI configGUI;
    QFileSystemWatcher * watcher;

    std::string curFile;

    QProcess* GoATProcess;
    QStringList* GoATarguments;

    QProcess* PhysicsProcess;
    QStringList* PhysicsArguments;

    int OpeningAtempt;

    /* Test */
    QWaitCondition waitCondition;
    QMutex mutex;


private slots:

  void ACQUdirChanged(QString path);
  void newGoatFile();
  void newPhysicsFile();
  void ForceRunGoAT();
  void ForceRunPhysics();
  void killGoatProcess();
  void killPhysicsProcess();
  void on_actionEdig_config_file_triggered();
  void RunGoat();
};

#endif // MAINWINDOW_H
