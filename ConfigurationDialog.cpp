#include "ConfigurationDialog.h"
#include "ui_configurationdialog.h"

ConfigurationDialog::ConfigurationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigurationDialog),
    HorizontalPads(1),
    VerticalPads(2)
{
    ui->setupUi(this);

    DoneConstructing = false;
    if (!configGUI.loadGUIConfigFile(QCoreApplication::applicationDirPath().toStdString() + std::string("/config/settings.gui")))
    {
        std::cout << "GUI Config file not found at: " <<
                     QCoreApplication::applicationDirPath().toStdString() + std::string("/config/settings.gui") << std::endl;
        return;
    }

    /*
     *  Allowing label take multiple lines
     */
    ui->labelInputDirectory3->setWordWrap(true);
    ui->labelInputFile3->setWordWrap(true);
    ui->labelOutputFile3->setWordWrap(true);

    /*
     * Disabling toolBox items
     */
    ui->toolBox->setItemEnabled(0, true);
    ui->toolBox->setItemEnabled(1, false);
    ui->toolBox->setItemEnabled(2, false);
    ui->toolBox->setItemEnabled(3, false);

    /*
     * Disabling Cuts tab
     */
    ui->tabWidget->setTabEnabled(1, false);
    ui->pushButtonSelectCuts->setEnabled(false);

    /* Keep Save button Enabled at all times */
    //ui->pushButton->setEnabled(false);

    /*
     * Constaining to numeric fields
     */
    ui->lineEditConfigPreSortParticleTotal->setValidator( new QIntValidator(0, 100, this) );
    ui->lineEditConfigPreSortParticleCB->setValidator( new QIntValidator(0, 100, this) );
    ui->lineEditConfigPreSortParticleTaps->setValidator( new QIntValidator(0, 100, this) );
    ui->lineEditConfigPreSortEnergySum->setValidator( new QDoubleValidator(0, 100000, 3, this) );


    ui->TWidgetCut->GetCanvas()->SetFillStyle(4100);
    ui->TWidgetCut->GetCanvas()->SetFillColor(10);
    ui->TWidgetCut->GetCanvas()->Divide(HorizontalPads, VerticalPads);

    ui->TWidgetCut->EnableSignalEvents(kMousePressEvent);
    ui->TWidgetCut->EnableSignalEvents(kMouseReleaseEvent);
    //ui->TWidgetCut->EnableSignalEvents(kMouseMoveEvent);
    //ui->TWidgetCut->EnableSignalEvents(kMouseDoubleClickEvent);
    //ui->TWidgetCut->EnableSignalEvents(kKeyPressEvent);
    //ui->TWidgetCut->EnableSignalEvents(kEnterEvent);
    //ui->TWidgetCut->EnableSignalEvents(kLeaveEvent);

    connect(ui->TWidgetCut, SIGNAL(RootEventProcessed(TObject*,uint,TCanvas*)), this , SLOT(RootSpinBoxEvent(TObject*,uint,TCanvas*)));
    connect(ui->TWidgetCut, SIGNAL(RootEventProcessed(TObject*,uint,TCanvas*)), this , SLOT(RootCanvasEvent(TObject*,uint,TCanvas*)));


    /*
     * Setting default start path when browsing for file.
     */
    lastCutPath = "/home/";

    /* Temporary */
    ui->tabWidgetOutput->setEnabled(false);
    ui->tabWidgetInput->setTabEnabled(1, false);

    configFile.loadGoATConfigFile(QCoreApplication::applicationDirPath().toStdString() + std::string("/config/GoAT-config.dat"));
    ParticleNames = configFile.getVectorParticleNames();
    ParticleNumbers = configFile.getVectorPParticleNumbers();
    ParticleAMin = configFile.getVectorParticleAMin();
    ParticleAMax = configFile.getVectorParticleAMax();
    ParticleNameToIndex = configFile.getMapPS();


    /* In futute it is possible to remap names */
    for(int i = 0; i < (int)ParticleNames.size(); i++)
        ui->ParticleBox->addItem(ParticleNames[i].c_str());

    DoneConstructing = true;
}

ConfigurationDialog::~ConfigurationDialog()
{
    delete ui;
}


void ConfigurationDialog::on_checkBoxCPreSort_stateChanged(int arg1)
{
    if (arg1 == 2)
    {
        ui->toolBox->setItemEnabled(0, true);
    }  else {
        ui->toolBox->setItemEnabled(0, false);
    }
}

void ConfigurationDialog::on_checkBoxCPR_stateChanged(int arg1)
{
    if (arg1 == 2)
    {
        ui->toolBox->setItemEnabled(1, true);
        configFile.setParticleReconstruction(1);

    }  else {
        ui->toolBox->setItemEnabled(1, false);
        configFile.setParticleReconstruction(0);
    }
}

void ConfigurationDialog::on_checkBoxCMR_stateChanged(int arg1)
{
    if (arg1 == 2)
    {
        ui->toolBox->setItemEnabled(2, true);
        configFile.setMesonReconstruction(1);
    }  else {
        ui->toolBox->setItemEnabled(2, false);
        configFile.setMesonReconstruction(0);
    }
}

void ConfigurationDialog::on_checkBoxCPS_stateChanged(int arg1)
{
    if (arg1 == 2)
    {
        ui->toolBox->setItemEnabled(3, true);
    }  else {
        ui->toolBox->setItemEnabled(3, false);
    }
}

void ConfigurationDialog::on_buttonInputBrowse_clicked()
{
    QString textList("Files: ");

    QStringList InputFilesList = QFileDialog::getOpenFileNames(
                            this,
                            "Select one or more files to open",
                            "/home",
                            "Root files (*.root);;All Files (*)");

    QStringList::Iterator it = InputFilesList.begin();

    textList.append(QString::number((InputFilesList).count())).append("<br>");

    while(it != InputFilesList.end())
    {
        QStringList parts = (*it).split("/");
        textList.append(parts.at(parts.size()-1)).append(" ");
        ++it;
    }

    ui->labelInputFile3->setText(textList);

    if (!InputFilesList.empty())
    if (!InputFilesList.first().isNull())
    {

        if (this->importTreeFromFile("treeRawEvent", InputFilesList.first().toStdString()))
        {
            this->updateTQtWidget();
            ui->tabWidget->setTabEnabled(1, true);
            ui->pushButtonSelectCuts->setEnabled(true);
            ui->pushButton->setEnabled(true);
            configGUI.setLastACQUFile(InputFilesList.first().toStdString());
            return;
        }
    }

    ui->labelInputFile3->setText("Invalid file.");

}

void ConfigurationDialog::on_buttonInputBrowseDir_clicked()
{
    QString InputDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                "/home",
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);
    ui->labelInputDirectory3->setText(InputDir);
    //ui->tabWidgetConfiguration->show();
}

void ConfigurationDialog::on_buttonOutputBrowse_clicked()
{
    QString OutputDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                "/home",
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);
    ui->labelOutputFile3->setText(OutputDir);
}

void ConfigurationDialog::on_pushButton_clicked()
{
    configFile.setVectorParticleNumbers(ParticleNumbers);
    configFile.setVectorParticleAMin(ParticleAMin);
    configFile.setVectorParticleAMax(ParticleAMax);

    configGUI.writeGUIConfigFile(QCoreApplication::applicationDirPath().toStdString() + std::string("/config/settings.gui"));
    configFile.writeGoATConfigFile(QCoreApplication::applicationDirPath().toStdString() + std::string("/config/GoAT-config.dat"));
    this->close();
}

void ConfigurationDialog::on_pushButtonPRCBProtons_clicked()
{
    QString inputFile = QFileDialog::getOpenFileName(
                            this,
                            "Select file open",
                            lastCutPath.c_str(),
                            "Root files (*.root);;All Files (*)");

    if (inputFile.isEmpty() || inputFile.isNull())
        return;
    lastCutPath = QFileInfo(inputFile).absolutePath().toStdString();

    QStringList parts = inputFile.split("/");

    ui->labelPRProtons->setText(parts.at(parts.size()-1));
    configFile.setCut_dE_E_CB_Proton(inputFile.toStdString());
}

void ConfigurationDialog::on_pushButtonPRCBElectrons_clicked()
{
    QString inputFile = QFileDialog::getOpenFileName(
                            this,
                            "Select file open",
                            lastCutPath.c_str(),
                            "Root files (*.root);;All Files (*)");

    if (inputFile.isEmpty() || inputFile.isNull())
        return;
    lastCutPath = QFileInfo(inputFile).absolutePath().toStdString();

    QStringList parts = inputFile.split("/");

    ui->labelPRElectrons->setText(parts.at(parts.size()-1));
    configFile.setCut_dE_E_CB_Electron(inputFile.toStdString());
}

void ConfigurationDialog::on_pushButtonPRCBPions_clicked()
{
    QString inputFile = QFileDialog::getOpenFileName(
                            this,
                            "Select file open",
                            lastCutPath.c_str(),
                            "Root files (*.root);;All Files (*)");

    if (inputFile.isEmpty() || inputFile.isNull())
        return;
    lastCutPath = QFileInfo(inputFile).absolutePath().toStdString();

    QStringList parts = inputFile.split("/");

    ui->labelPRPions->setText(parts.at(parts.size()-1));
    configFile.setCut_dE_E_CB_Pion(inputFile.toStdString());
}

void ConfigurationDialog::on_pushButtonPRTapsProtons_clicked()
{
    QString inputFile = QFileDialog::getOpenFileName(
                            this,
                            "Select file open",
                            lastCutPath.c_str(),
                            "Root files (*.root);;All Files (*)");

    if (inputFile.isEmpty() || inputFile.isNull())
        return;
    lastCutPath = QFileInfo(inputFile).absolutePath().toStdString();

    QStringList parts = inputFile.split("/");

    ui->labelPRTapsProtons->setText(parts.at(parts.size()-1));
    configFile.setCut_dE_E_Taps_Proton(inputFile.toStdString());
}

void ConfigurationDialog::on_pushButtonPRTapsElectrons_clicked()
{
    QString inputFile = QFileDialog::getOpenFileName(
                            this,
                            "Select file open",
                            lastCutPath.c_str(),
                            "Root files (*.root);;All Files (*)");

    if (inputFile.isEmpty() || inputFile.isNull())
        return;
    lastCutPath = QFileInfo(inputFile).absolutePath().toStdString();

    QStringList parts = inputFile.split("/");

    ui->labelPRTapsElectrons->setText(parts.at(parts.size()-1));
    configFile.setCut_dE_E_Taps_Electron(inputFile.toStdString());
}

void ConfigurationDialog::on_pushButtonPRTapsPions_clicked()
{
    QString inputFile = QFileDialog::getOpenFileName(
                            this,
                            "Select file open",
                            lastCutPath.c_str(),
                            "Root files (*.root);;All Files (*)");

    if (inputFile.isEmpty() || inputFile.isNull())
        return;
    lastCutPath = QFileInfo(inputFile).absolutePath().toStdString();

    QStringList parts = inputFile.split("/");

    ui->labelPRTapsPions->setText(parts.at(parts.size()-1));
    configFile.setCut_dE_E_Taps_Pion(inputFile.toStdString());
}


void ConfigurationDialog::on_lineEditConfigPreSortParticleTotal_editingFinished()
{
    configFile.setSortRawNParticlesTotal(resolvingComboBoxIndex(ui->comboBox->currentIndex()),
                                         ui->lineEditConfigPreSortParticleTotal->text().toStdString());
}

void ConfigurationDialog::on_lineEditConfigPreSortParticleCB_editingFinished()
{
    configFile.setSortRawNParticlesCB(resolvingComboBoxIndex(ui->comboBox_3->currentIndex()),
                                       ui->lineEditConfigPreSortParticleCB->text().toStdString());
}

void ConfigurationDialog::on_lineEditConfigPreSortParticleTaps_editingFinished()
{
    configFile.setSortRawNParticlesTAPS(resolvingComboBoxIndex(ui->comboBox_4->currentIndex()),
                                       ui->lineEditConfigPreSortParticleTaps->text().toStdString());
}

void ConfigurationDialog::on_comboBox_currentIndexChanged(int index)
{
    configFile.setSortRawNParticlesTotal(resolvingComboBoxIndex(index),
                                         ui->lineEditConfigPreSortParticleTotal->text().toStdString());
}

void ConfigurationDialog::on_comboBox_3_currentIndexChanged(int index)
{
    configFile.setSortRawNParticlesCB(resolvingComboBoxIndex(index),
                                         ui->lineEditConfigPreSortParticleCB->text().toStdString());
}

void ConfigurationDialog::on_comboBox_4_currentIndexChanged(int index)
{
    configFile.setSortRawNParticlesTAPS(resolvingComboBoxIndex(index),
                                         ui->lineEditConfigPreSortParticleTaps->text().toStdString());
}

void ConfigurationDialog::on_lineEditConfigPreSortEnergySum_editingFinished()
{
    configFile.setSortRawEnergyCB(ui->lineEditConfigPreSortEnergySum->text().toStdString());
}

void ConfigurationDialog::RootCanvasEvent(TObject* object, uint event, TCanvas* canvas)
{

        for(unsigned int i = 1; i <= HorizontalPads*VerticalPads; i++)
        {
            ui->TWidgetCut->cd(i);
            if (gPad != nullptr)
            {
                gPad->SetFillColor(0);
                canvas->GetPad(i)->SetFrameLineColor(0);
            }
        }

        if (((TVirtualPad*)canvas->GetPadPointer()) != nullptr)
        {
            ((TVirtualPad*)canvas->GetPadPointer())->cd();
             gPad->SetFillColor(5);

            GlobalMax = MaxValMap[gPad->GetName()];
            GlobalMin = MinValMap[gPad->GetName()];
            this->currentTName = std::string(gPad->GetName());
        }
}

void ConfigurationDialog::RootSpinBoxEvent(TObject* object, uint event, TCanvas* canvas)
{
    if (GlobalMin && GlobalMax)
    {
       ui->spinBoxMin->setValue(GlobalMin->GetX1());
       ui->spinBoxMax->setValue(GlobalMax->GetX1());
    }
}

void ConfigurationDialog::on_spinBoxMin_valueChanged(double arg1)
{
    if (GlobalMin)
    {
        GlobalMin->SetX1(arg1);
        GlobalMin->SetX2(arg1);
        ui->TWidgetCut->Refresh();
    }
    if (this->currentTName == std::string("SelectCuts1"))
        configFile.setTimeCutMinApparatus1(std::to_string(arg1));
    if (this->currentTName == std::string("SelectCuts2"))
        configFile.setTimeCutMinApparatus2(std::to_string(arg1));
}

void ConfigurationDialog::on_spinBoxMax_valueChanged(double arg1)
{
    if (GlobalMax)
    {
        GlobalMax->SetX1(arg1);
        GlobalMax->SetX2(arg1);
        ui->TWidgetCut->Refresh();
    }
    if (this->currentTName == std::string("SelectCuts1"))
        configFile.setTimeCutMaxApparatus1(std::to_string(arg1));
    if (this->currentTName == std::string("SelectCuts2"))
        configFile.setTimeCutMaxApparatus2(std::to_string(arg1));
}

void ConfigurationDialog::updateTQtWidget()
{


    ui->TWidgetCut->cd(1);
    gPad->SetName("SelectCuts1");
    tree->Draw("time>>TimeCB(20000,-1000,1000)", "Apparatus==1");
    CutMin1   = new TLine(-20, -10e20, -20, 10e20);
    CutMin1->SetLineColor(2);
    CutMin1->Draw();

    CutMax1   = new TLine(20, -10e20, 20, 10e20);
    CutMax1->SetLineColor(6);
    CutMax1->Draw();

    ui->TWidgetCut->cd(2);
    gPad->SetName("SelectCuts2");
    tree->Draw("time>>TimeCBA(20000,-100,100)", "Apparatus==2");
    CutMin2   = new TLine(-20, -10e20, -20, 10e20);
    CutMin2->SetLineColor(2);
    CutMin2->Draw();

    CutMax2   = new TLine(20, -10e20, 20, 10e20);
    CutMax2->SetLineColor(6);
    CutMax2->Draw();


    /* Mapping pad names to CutLines */
    MinValMap["SelectCuts1"] = CutMin1;
    MinValMap["SelectCuts2"] = CutMin2;

    MaxValMap["SelectCuts1"] = CutMax1;
    MaxValMap["SelectCuts2"] = CutMax2;


    /* Setting default controller to first pad */
    GlobalMax = CutMax1;
    GlobalMin = CutMin1;

    ui->TWidgetCut->Refresh();
    this->currentTName = "SelectCuts1";
}

bool ConfigurationDialog::importTreeFromFile(char* treename, std::string filename)
{
    TFile*  file    = TFile::Open(filename.c_str());
    if(!file)
    {
        std::cout << "could not open file" << std::endl;
        return false;
    }
    file->GetObject(treename, tree);
    if(!tree) {
        std::cout << "Could not create TTree" << std::endl;
        return false;
    }

    ui->tabWidget->setTabEnabled(1, true);
    return true;
}

bool ConfigurationDialog::importTreeFromFile(std::string treename, std::string filename)
{
    return importTreeFromFile(treename.c_str(), filename);
}


void ConfigurationDialog::on_pushButtonSelectCuts_clicked()
{
    ui->tabWidget->setCurrentIndex(1);
}

void ConfigurationDialog::updateUIText()
{
    configFile.loadGoATConfigFile(QCoreApplication::applicationDirPath().toStdString() + std::string("/config/GoAT-config.dat"));

    // Input filename
    ui->labelInputFile3->setText(QString(configGUI.getLastFile().c_str()));

    // PreSort
    if (configFile.getSortRawNParticlesTotal() != configFile.nullValue &&
        configFile.getSortRawNParticlesCB() != configFile.nullValue &&
        configFile.getSortRawNParticlesTAPS() != configFile.nullValue )
    {
        ui->checkBoxCPreSort->setChecked(true);

        // PreSort particle numbers
        ui->lineEditConfigPreSortParticleTotal->setText(configFile.getSortRawNParticlesTotal().substr(0, configFile.getSortRawNParticlesTotal().length()-1).c_str());
        ui->lineEditConfigPreSortParticleCB->setText(configFile.getSortRawNParticlesCB().substr(0, configFile.getSortRawNParticlesCB().length()-1).c_str());
        ui->lineEditConfigPreSortParticleTaps->setText(configFile.getSortRawNParticlesTAPS().substr(0, configFile.getSortRawNParticlesTAPS().length()-1).c_str());

        // Corresponding combo boxes
        ui->comboBox->setCurrentIndex(InverseResolvingBoxIndex(configFile.getSortRawNParticlesTotal().substr(configFile.getSortRawNParticlesTotal().length()-1, 1)));
        ui->comboBox_3->setCurrentIndex(InverseResolvingBoxIndex(configFile.getSortRawNParticlesCB().substr(configFile.getSortRawNParticlesCB().length()-1, 1)));
        ui->comboBox_4->setCurrentIndex(InverseResolvingBoxIndex(configFile.getSortRawNParticlesTAPS().substr(configFile.getSortRawNParticlesTAPS().length()-1, 1)));
    }

    // Min Energy value
    ui->lineEditConfigPreSortEnergySum->setText(QString(configFile.getSortRawEnergyCB().substr(0, configFile.getSortRawEnergyCB().length() - 1).c_str()));

    // If cuts are specified
    if (configFile.getCut_dE_E_CB_Proton() != configFile.nullValue ||
        configFile.getCut_dE_E_CB_Pion() != configFile.nullValue ||
        configFile.getCut_dE_E_CB_Electron() != configFile.nullValue ||
        configFile.getCut_dE_E_Taps_Proton() != configFile.nullValue ||
        configFile.getCut_dE_E_Taps_Pion()    != configFile.nullValue ||
        configFile.getCut_dE_E_Taps_Electron() != configFile.nullValue)
    {
        ui->checkBoxCPR->setChecked(true);

        // Cut locations
        ui->labelPRProtons->setText(QString(configFile.getCut_dE_E_CB_Proton().c_str()));
        ui->labelPRPions->setText(QString(configFile.getCut_dE_E_CB_Pion().c_str()));
        ui->labelPRElectrons->setText(QString(configFile.getCut_dE_E_CB_Electron().c_str()));

        ui->labelPRTapsProtons->setText(QString(configFile.getCut_dE_E_Taps_Proton().c_str()));
        ui->labelPRTapsPions->setText(QString(configFile.getCut_dE_E_Taps_Pion().c_str()));
        ui->labelPRTapsElectrons->setText(QString(configFile.getCut_dE_E_Taps_Electron().c_str()));
    }

        ui->lineEditConfigMRWidthPi0->setText(QString(configFile.getCut_IM_Width_Pi0().c_str()));
        ui->lineEditConfigMRWidthEta->setText(QString(configFile.getCut_IM_Width_Eta().c_str()));
        ui->lineEditConfigMRWidthEtaprime->setText(QString(configFile.getCut_IM_Width_Eta_Prime().c_str()));

    if (configFile.getMesonReconstruction() == "1")
    {
        ui->checkBoxCMR->setChecked(true);
    }


    if (configFile.getSortNParticles() != ConfigFile::nullValue && configFile.getSortNParticles().length() > 0)
    {
        ui->lineEditPSSortN->setText(configFile.getSortNParticles().substr(0, configFile.getSortNParticles().length() - 1).c_str());
        ui->comboBoxPSSortN->setCurrentIndex(InverseResolvingBoxIndex(configFile.getSortNParticles().substr(configFile.getSortNParticles().length()-1, 1)));
    }

    if (configFile.getPSpi0Number()!= ConfigFile::nullValue && configFile.getPSpi0Number().length() > 0)
    {
        ui->lineEditPSNum1->setText(configFile.getPSpi0Number().substr(0, configFile.getPSpi0Number().length()-1).c_str());
        ui->lineEditPSMin1->setText(configFile.getPSpi0AMin().c_str());
        ui->lineEditPSMax1->setText(configFile.getPSpi0AMax().c_str());
        ui->comboBoxPS1->setCurrentIndex(InverseResolvingBoxIndex(configFile.getPSpi0Number().substr(configFile.getPSpi0Number().length()-1, 1)));
    }

    if (configFile.getSortNParticles() != ConfigFile::nullValue)
    {
        ui->checkBoxCPS->setChecked(true);
    }


//    ParticleNames = configFile.getVectorParticleNames();
//    ParticleNumbers = configFile.getVectorPParticleNumbers();
//    ParticleAMin = configFile.getVectorParticleAMin();
//    ParticleAMax = configFile.getVectorParticleAMax();
//    ParticleNameToIndex = configFile.getMapPS();



//    /* In futute it is possible to remap names */
//    for(int i = 0; i < (int)ParticleNames.size(); i++)
//        ui->ParticleBox->addItem(ParticleNames[i].c_str());



//    for(int i = 0; i <(int)ParticleNames.size(); i++)
//    {
//        if (!ParticleNumbers[i].empty())
//           std::cout << ParticleNames[i] << " " << ParticleNumbers[i]<< " " << ParticleAMin[i] << " " << ParticleAMax[i] << std::endl;
//     }

    /* Updates list of particles that are being used */
    std::string ActiveParticles("");
    for(int i = 0; i <(int)ParticleNames.size(); i++)
        if (!ParticleNumbers[i].empty() && !ParticleAMin[i].empty() && !ParticleAMax[i].empty())
            ActiveParticles.append(ParticleNames[i] + " ");
    ui->labelPSParticleList->setText(ActiveParticles.c_str());

    ui->pushButton->setEnabled(true);

    DoneConstructing = true;
}

void ConfigurationDialog::on_lineEditConfigMRWidthPi0_editingFinished()
{
    if (!ui->lineEditConfigMRWidthPi0->text().toStdString().empty())
    configFile.setCut_IM_Width_Pi0(ui->lineEditConfigMRWidthPi0->text().toStdString());
}

void ConfigurationDialog::on_lineEditConfigMRWidthEta_editingFinished()
{
    if (!ui->lineEditConfigMRWidthEta->text().toStdString().empty())
    configFile.setCut_IM_Width_Eta(ui->lineEditConfigMRWidthEta->text().toStdString());
}

void ConfigurationDialog::on_lineEditConfigMRWidthEtaprime_editingFinished()
{
    if (!ui->lineEditConfigMRWidthEtaprime->text().toStdString().empty())
    configFile.setCut_IM_Width_Eta_Prime(ui->lineEditConfigMRWidthEtaprime->text().toStdString());
}


void ConfigurationDialog::on_comboBoxPS1_currentIndexChanged(int index)
{
    // Bug: changing only sign does not store index.
    //std::cout << index << resolvingComboBoxIndex(index) << std::endl;

    //articleNumbers[index] = ui->lineEditPSNum1->text().toStdString() + resolvingComboBoxIndex(index);
    //configFile.setPSpi0Number(resolvingComboBoxIndex(index),
    //                          ui->lineEditPSNum1->text().toStdString());
}

void ConfigurationDialog::on_lineEditPSNum1_editingFinished()
{
//    ParticleNumbers[ui->ParticleBox->currentIndex()] = ui->lineEditPSNum1->text().toStdString() + resolvingComboBoxIndex(ui->comboBoxPS1->currentIndex());
//    //configFile.setPSpi0Number(resolvingComboBoxIndex(ui->comboBoxPS1->currentIndex()),
//    //                          ui->lineEditPSNum1->text().toStdString());
//    std::cout << "done: " << ParticleNumbers[ui->ParticleBox->currentIndex()] << std::endl;
//    std::cout << "index: " << ui->ParticleBox->currentIndex() << std::endl;
}

void ConfigurationDialog::on_lineEditPSMin1_editingFinished()
{
//    ParticleAMin[ui->ParticleBox->currentIndex()] = ui->lineEditPSMin1->text().toStdString();
//   // configFile.setPSpi0AMin(ui->lineEditPSMin1->text().toStdString());
//    std::cout << "done: " << ParticleAMin[ui->ParticleBox->currentIndex()] << std::endl;
//    std::cout << "index: " << ui->ParticleBox->currentIndex() << std::endl;
}

void ConfigurationDialog::on_lineEditPSMax1_editingFinished()
{
//    ParticleAMax[ui->ParticleBox->currentIndex()] = ui->lineEditPSMax1->text().toStdString();
//    //configFile.setPSpi0AMax(ui->lineEditPSMax1->text().toStdString());
//    std::cout << "done: " << ParticleAMax[ui->ParticleBox->currentIndex()] << std::endl;
//    std::cout << "index: " << ui->ParticleBox->currentIndex() << std::endl;
}

void ConfigurationDialog::on_comboBoxPSSortN_currentIndexChanged(int index)
{
    configFile.setSortNParticles(resolvingComboBoxIndex(index),
                                 ui->lineEditPSSortN->text().toStdString());
}

void ConfigurationDialog::on_lineEditPSSortN_editingFinished()
{
    configFile.setSortNParticles(resolvingComboBoxIndex(ui->comboBoxPSSortN->currentIndex()),
                                 ui->lineEditPSSortN->text().toStdString());
}


void ConfigurationDialog::setPostReconstructionFields(int DropIndex, std::string number, std::string AMin, std::string AMax)
{

}

void ConfigurationDialog::on_ParticleBox_currentIndexChanged(int index)
{    if (!DoneConstructing) return;
    if (!ParticleNumbers[index].empty() && !ParticleAMin[index].empty() && !ParticleAMax[index].empty())
    {
        ui->comboBoxPS1->setCurrentIndex(InverseResolvingBoxIndex(ParticleNumbers[index].substr(ParticleNumbers[index].length()-1, 1)));
        ui->lineEditPSNum1->setText(ParticleNumbers[index].substr(0,  ParticleNumbers[index].length() - 1).c_str());
        ui->lineEditPSMin1->setText(ParticleAMin[index].substr(0,4).c_str());
        ui->lineEditPSMax1->setText(ParticleAMax[index].substr(0,4).c_str());
    } else {
        ui->lineEditPSNum1->setText("");
        ui->lineEditPSMin1->setText("");
        ui->lineEditPSMax1->setText("");
    }
}

void ConfigurationDialog::on_lineEditPSNum1_textEdited(const QString &arg1)
{
    if (!DoneConstructing) return;

    ParticleNumbers[ui->ParticleBox->currentIndex()] = ui->lineEditPSNum1->text().toStdString() + resolvingComboBoxIndex(ui->comboBoxPS1->currentIndex());
    UpdateLabel();
}

void ConfigurationDialog::on_lineEditPSMin1_textEdited(const QString &arg1)
{
    if (!DoneConstructing) return;

    ParticleAMin[ui->ParticleBox->currentIndex()] = ui->lineEditPSMin1->text().toStdString().substr(0, 4);
    UpdateLabel();
}

void ConfigurationDialog::on_lineEditPSMax1_textEdited(const QString &arg1)
{
    if (!DoneConstructing) return;

    ParticleAMax[ui->ParticleBox->currentIndex()] = ui->lineEditPSMax1->text().toStdString().substr(0, 4);
    UpdateLabel();
}

void ConfigurationDialog::UpdateLabel()
{
    /* Updates list of particles that are being used */
    std::string ActiveParticles("");
    for(int i = 0; i <(int)ParticleNames.size(); i++)
        if (!ParticleNumbers[i].empty() && !ParticleAMin[i].empty() && !ParticleAMax[i].empty())
            ActiveParticles.append(ParticleNames[i] + " ");
    ui->labelPSParticleList->setText(ActiveParticles.c_str());
}
