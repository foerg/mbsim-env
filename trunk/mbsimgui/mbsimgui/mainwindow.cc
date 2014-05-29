/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2012 Martin Förg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <config.h>
#include "mainwindow.h"
#include "solver.h"
#include "frame.h"
#include "contour.h"
#include "object.h"
#include "link.h"
#include "observer.h"
#include "integrator.h"
#include "objectfactory.h"
#include "parameter.h"
#include "widget.h"
#include "treemodel.h"
#include "treeitem.h"
#include "element_view.h"
#include "embedding_view.h"
#include "integrator_view.h"
#include "embed.h"
#include "process.h"
#include <openmbv/mainwindow.h>
#include <utime.h>
#include <QtGui>
#include <mbxmlutils/octeval.h>
#include <mbxmlutils/preprocess.h>
#include <mbxmlutilshelper/getinstallpath.h>
#include <mbxmlutilshelper/dom.h>
#include <xercesc/dom/DOMProcessingInstruction.hpp>

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;
using namespace boost;
namespace bfs=boost::filesystem;

namespace MBSimGUI {

  bool currentTask;
  bool absolutePath = false;
  QDir mbsDir;

  MainWindow *mw;

  shared_ptr<DOMParser> MainWindow::parser=DOMParser::create(true);
  OctEval *MainWindow::octEval=NULL;
  vector<boost::filesystem::path> dependencies;
  NewParamLevel *MainWindow::octEvalParamLevel=NULL;

  MainWindow::MainWindow(QStringList &arg) : inlineOpenMBVMW(0) {

    mw = this;

    if(bfs::is_directory("/dev/shm"))
      uniqueTempDir=bfs::unique_path("/dev/shm/mbsimgui_%%%%-%%%%-%%%%-%%%%");
    else
      uniqueTempDir=bfs::unique_path(bfs::temp_directory_path()/"mbsimgui_%%%%-%%%%-%%%%-%%%%");
    bfs::create_directories(uniqueTempDir);

    mbsim = new Process(this);

    mbsimThread = new MBSimThread(this);
    mbsimThread->setParser(parser);
    connect(mbsimThread, SIGNAL(resultReady(int)), this, SLOT(preprocessFinished(int)));

    QString program = (MBXMLUtils::getInstallPath()/"bin"/"mbsimxml").string().c_str();
    QStringList arguments;
    arguments << "--onlyGenerateSchema" << (uniqueTempDir/"mbsimxml.xsd").string().c_str();
    int code=mbsim->getProcess()->execute(program,arguments);
    parser->loadGrammar(arguments[1].toStdString());

    elementList = new ElementView;

    initInlineOpenMBV();

    MBSimObjectFactory::initialize();
    octEval=new MBXMLUtils::OctEval(&dependencies);
    octEvalParamLevel=new NewParamLevel(*octEval);

    mbsimThread->setOctEval(octEval);
    mbsimThread->setDependencies(dependencies);

    QMenu *GUIMenu=new QMenu("GUI", menuBar());
    menuBar()->addMenu(GUIMenu);

    QAction *action = GUIMenu->addAction(style()->standardIcon(QStyle::StandardPixmap(QStyle::SP_DirHomeIcon)),"Workdir", this, SLOT(changeWorkingDir()));
    action->setStatusTip(tr("Change working directory"));

    GUIMenu->addSeparator();

    action = GUIMenu->addAction(Utils::QIconCached(QString::fromStdString((MBXMLUtils::getInstallPath()/"share"/"mbsimgui"/"icons"/"exit.svg").string())), "E&xit", this, SLOT(close()));
    action->setShortcuts(QKeySequence::Quit);
    action->setStatusTip(tr("Exit the application"));

    for (int i=0; i<maxRecentFiles; i++) {
      recentProjectFileActs[i] = new QAction(this);
      recentProjectFileActs[i]->setVisible(false);
      connect(recentProjectFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentProjectFile()));
    }
    QMenu *menu=new QMenu("Project", menuBar());
    action = menu->addAction("New", this, SLOT(newProject()));
    action->setShortcuts(QKeySequence::New);
    action = menu->addAction("Load", this, SLOT(loadProject()));
    action->setShortcuts(QKeySequence::Open);
    action = menu->addAction("Save as", this, SLOT(saveProjectAs()));
    action->setShortcuts(QKeySequence::SaveAs);
    actionSaveProject = menu->addAction("Save", this, SLOT(saveProject()));
    actionSaveProject->setShortcuts(QKeySequence::Save);
   //ProjMenu->addAction(style()->standardIcon(QStyle::StandardPixmap(QStyle::SP_DirOpenIcon)),"Load", this, SLOT(loadProj()));
    actionSaveProject->setDisabled(true);
    menu->addSeparator();
    //separatorAct = menu->addSeparator();
    for (int i = 0; i < maxRecentFiles; ++i)
      menu->addAction(recentProjectFileActs[i]);
    updateRecentProjectFileActions();
    menu->addSeparator();
    menuBar()->addMenu(menu);

    menu=new QMenu("Export", menuBar());
    actionSaveDataAs = menu->addAction("Export all data", this, SLOT(saveDataAs()));
    actionSaveMBSimH5DataAs = menu->addAction("Export MBSim data file", this, SLOT(saveMBSimH5DataAs()));
    actionSaveOpenMBVDataAs = menu->addAction("Export OpenMBV data", this, SLOT(saveOpenMBVDataAs()));
    actionSaveDataAs->setDisabled(true);
    actionSaveMBSimH5DataAs->setDisabled(true);
    actionSaveOpenMBVDataAs->setDisabled(true);
    menuBar()->addMenu(menu);

    menuBar()->addSeparator();
    QMenu *helpMenu=new QMenu("Help", menuBar());
    helpMenu->addAction("GUI Help...", this, SLOT(help()));
    helpMenu->addAction("About MBSim GUI", this, SLOT(about()));
    menuBar()->addMenu(helpMenu);

    QToolBar *toolBar = addToolBar("Tasks");
    actionSimulate = toolBar->addAction(style()->standardIcon(QStyle::StandardPixmap(QStyle::SP_MediaPlay)),"Start simulation");
    //actionSimulate->setStatusTip(tr("Simulate the multibody system"));
    actionSimulate->setStatusTip(tr("Simulate the multibody system"));
    connect(actionSimulate,SIGNAL(triggered()),this,SLOT(simulate()));
    toolBar->addAction(actionSimulate);
    QAction *actionInterrupt = toolBar->addAction(style()->standardIcon(QStyle::StandardPixmap(QStyle::SP_MediaStop)),"Interrupt simulation");
//    actionInterrupt->setDisabled(true);
    connect(actionInterrupt,SIGNAL(triggered()),mbsim,SLOT(interrupt()));
    toolBar->addAction(actionInterrupt);
    actionRefresh = toolBar->addAction(style()->standardIcon(QStyle::StandardPixmap(QStyle::SP_BrowserReload)),"Refresh 3D view");
//    actionRefresh->setDisabled(true);
    connect(actionRefresh,SIGNAL(triggered()),this,SLOT(refresh()));
    toolBar->addAction(actionRefresh);
    actionOpenMBV = toolBar->addAction(Utils::QIconCached(QString::fromStdString((MBXMLUtils::getInstallPath()/"share"/"mbsimgui"/"icons"/"openmbv.svg").string())),"OpenMBV");
    actionOpenMBV->setDisabled(true);
    connect(actionOpenMBV,SIGNAL(triggered()),this,SLOT(openmbv()));
    toolBar->addAction(actionOpenMBV);
    actionH5plotserie = toolBar->addAction(Utils::QIconCached(QString::fromStdString((MBXMLUtils::getInstallPath()/"share"/"mbsimgui"/"icons"/"h5plotserie.svg").string())),"H5plotserie");
    actionH5plotserie->setDisabled(true);
    connect(actionH5plotserie,SIGNAL(triggered()),this,SLOT(h5plotserie()));
    toolBar->addAction(actionH5plotserie);

    elementList->setModel(new ElementTreeModel);
    elementList->setColumnWidth(0,250);
    elementList->setColumnWidth(1,200);
    elementList->hideColumn(1);

    embeddingList = new EmbeddingView;
    embeddingList->setModel(new EmbeddingTreeModel);
    embeddingList->setColumnWidth(0,150);
    embeddingList->setColumnWidth(1,200);

    integratorView = new IntegratorView;

    connect(elementList,SIGNAL(pressed(QModelIndex)), this, SLOT(elementListClicked()));
    connect(embeddingList,SIGNAL(pressed(QModelIndex)), this, SLOT(parameterListClicked()));
    connect(elementList->selectionModel(),SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)), this, SLOT(selectionChanged(const QModelIndex&,const QModelIndex&)));

    QDockWidget *dockWidget1 = new QDockWidget("Multibody system");
    addDockWidget(Qt::LeftDockWidgetArea,dockWidget1);
    QWidget *widget1=new QWidget(dockWidget1);
    dockWidget1->setWidget(widget1);
    QGridLayout *widgetLayout1=new QGridLayout(widget1);
    widgetLayout1->setContentsMargins(0,0,0,0);
    widget1->setLayout(widgetLayout1);
    OpenMBVGUI::AbstractViewFilter *elementListFilter=new OpenMBVGUI::AbstractViewFilter(elementList, 0, 1);
    widgetLayout1->addWidget(elementListFilter, 0, 0);
    widgetLayout1->addWidget(elementList, 1, 0);

    QDockWidget *dockWidget3 = new QDockWidget("Embeddings");
    addDockWidget(Qt::LeftDockWidgetArea,dockWidget3);
    QWidget *widget3=new QWidget(dockWidget3);
    dockWidget3->setWidget(widget3);
    QGridLayout *widgetLayout3=new QGridLayout(widget3);
    widgetLayout3->setContentsMargins(0,0,0,0);
    widget3->setLayout(widgetLayout3);
    OpenMBVGUI::AbstractViewFilter *parameterListFilter=new OpenMBVGUI::AbstractViewFilter(embeddingList, 0, -2);
    widgetLayout3->addWidget(parameterListFilter, 0, 0);
    widgetLayout3->addWidget(embeddingList, 1, 0);

    QDockWidget *dockWidget2 = new QDockWidget("Integrator");
    addDockWidget(Qt::LeftDockWidgetArea,dockWidget2);
    dockWidget2->setWidget(integratorView);

    //tabifyDockWidget(dockWidget3,dockWidget4);
    //tabifyDockWidget(dockWidget2,dockWidget3);
    //QList<QTabBar *> tabList = findChildren<QTabBar *>();
    //tabBar = tabList.at(0);

    QWidget *centralWidget = new QWidget;  
    setCentralWidget(centralWidget);
    QHBoxLayout *mainlayout = new QHBoxLayout;
    centralWidget->setLayout(mainlayout);
    mainlayout->addWidget(inlineOpenMBVMW);

    QDockWidget *mbsimDW = new QDockWidget("MBSim Echo Area", this);
    addDockWidget(Qt::BottomDockWidgetArea, mbsimDW);
    QProcessEnvironment env=QProcessEnvironment::systemEnvironment();
    env.insert("MBXMLUTILS_XMLOUTPUT", "1");
    mbsim->getProcess()->setProcessEnvironment(env);
    mbsimDW->setWidget(mbsim); 
    connect(mbsim->getProcess(),SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(simulationFinished(int,QProcess::ExitStatus)));

    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    QStringList::iterator i, i2;

    if((i=std::find(arg.begin(), arg.end(), "--maximized"))!=arg.end()) {
      showMaximized();
      arg.erase(i);
    }

    QString fileProject;
    QRegExp filterProject(".+\\.mbsimprj\\.xml");
    QDir dir;
    dir.setFilter(QDir::Files);
    i=arg.begin();
    while(i!=arg.end() and (*i)[0]!='-') {
      dir.setPath(*i);
      if(dir.exists()) {
        QStringList file=dir.entryList();
        for(int j=0; j<file.size(); j++) {
          if(fileProject.isEmpty() and filterProject.exactMatch(file[j]))
            fileProject = dir.path()+"/"+file[j];
        }
        i2=i; i++; arg.erase(i2);
        continue;
      }
      if(QFile::exists(*i)) {
        if(fileProject.isEmpty() and filterProject.exactMatch(*i))
          fileProject = *i;
        i2=i; i++; arg.erase(i2);
        continue;
      }
      i++;
    }
    if(fileProject.size())
      loadProject(QDir::current().absoluteFilePath(fileProject));
    else
      newProject(false);

    setAcceptDrops(true);

  }

  void MainWindow::simulationFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if(currentTask==1) {
      inlineOpenMBVMW->openFile(uniqueTempDir.generic_string()+"/out1.ombv.xml");
      QModelIndex index = elementList->selectionModel()->currentIndex();
      ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
      Element *element=dynamic_cast<Element*>(model->getItem(index)->getItemData());
      if(element)
        highlightObject(element->getID());
    }
    actionSimulate->setDisabled(false);
    actionOpenMBV->setDisabled(false);
    actionH5plotserie->setDisabled(false);
    actionRefresh->setDisabled(false);
    statusBar()->showMessage(tr("Ready"));
  }

  void MainWindow::initInlineOpenMBV() {
    bfs::copy_file(MBXMLUtils::getInstallPath()/"share"/"mbsimgui"/"empty.ombv.xml",uniqueTempDir.generic_string()+"/out1.ombv.xml");
    bfs::copy_file(MBXMLUtils::getInstallPath()/"share"/"mbsimgui"/"empty.ombv.h5",uniqueTempDir.generic_string()+"/out1.ombv.h5");
    std::list<string> arg;
    arg.push_back("--wst");
    arg.push_back((MBXMLUtils::getInstallPath()/"share"/"mbsimgui"/"inlineopenmbv.ombv.wst").string());
    arg.push_back("/home/foerg/tmp/openmbv");
    inlineOpenMBVMW=new OpenMBVGUI::MainWindow(arg);
    inlineOpenMBVMW->openFile(uniqueTempDir.generic_string()+"/out1.ombv.xml");

    connect(inlineOpenMBVMW, SIGNAL(objectSelected(std::string, Object*)), this, SLOT(selectElement(std::string)));
    connect(inlineOpenMBVMW, SIGNAL(objectDoubleClicked(std::string, Object*)), elementList, SLOT(openEditor()));
  }

  MainWindow::~MainWindow() {
    delete mbsim;
    delete mbsimThread;
    bfs::remove_all(uniqueTempDir);
  }

  void MainWindow::changeWorkingDir() {
    QString dir = QFileDialog::getExistingDirectory (0, "Working directory", ".");
    if(not(dir.isEmpty())) {
      QString absoluteMBSFilePath = QDir::current().absoluteFilePath(fileProject);
      QDir::setCurrent(dir);
      mbsDir = QFileInfo(absoluteMBSFilePath).absolutePath();
      fileProject = QDir::current().relativeFilePath(absoluteMBSFilePath);
      updateRecentProjectFileActions();
    }
  }

  void MainWindow::closeEvent(QCloseEvent *event) {
    QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Application"),
        tr("MBS, parameter list or integrator may have been modified.\n"
          "Do you want to save your changes?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if(ret == QMessageBox::Save) {
      if(actionSaveProject->isEnabled())
        saveProject();
      else
        saveProjectAs();
      event->accept();
    } 
    else if(ret == QMessageBox::Discard) 
      event->accept();
    else if(ret == QMessageBox::Cancel) 
      event->ignore();
  }

  void MainWindow::highlightObject(const string &ID) {
    currentID = ID;
    inlineOpenMBVMW->highlightObject(ID);
  }

  void MainWindow::selectionChanged(const QModelIndex &current, const QModelIndex &previous) {
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    Element *element=dynamic_cast<Element*>(model->getItem(current)->getItemData());

    if(element) {

      EmbeddingTreeModel *emodel = static_cast<EmbeddingTreeModel*>(embeddingList->model());
      vector<Element*> parents = element->getParents();
      QModelIndex index = emodel->index(0,0);
      emodel->removeRow(index.row(), index.parent());
      if(parents.size()) {
        index = emodel->createEmbeddingItem(parents[0]);
        for(int i=0; i<parents.size()-1; i++)
          index = emodel->createEmbeddingItem(parents[i+1],index);
        emodel->createEmbeddingItem(element,index);
      }
      else
        index = emodel->createEmbeddingItem(element);
      embeddingList->expandAll();
      embeddingList->scrollTo(index.child(emodel->rowCount(index)-1,0),QAbstractItemView::PositionAtTop);

      highlightObject(element->getID());
    }
    else
      highlightObject("");
  }

  void MainWindow::elementListClicked() {
    if(QApplication::mouseButtons()==Qt::RightButton) {
      QModelIndex index = elementList->selectionModel()->currentIndex();
      TreeItemData *itemData = static_cast<ElementTreeModel*>(elementList->model())->getItem(index)->getItemData();
      if(itemData && index.column()==0) {
        QMenu *menu = itemData->createContextMenu();
        menu->exec(QCursor::pos());
        delete menu;
      } 
    }
  }

  void MainWindow::parameterListClicked() {
    if(QApplication::mouseButtons()==Qt::RightButton) {
      QModelIndex index = embeddingList->selectionModel()->currentIndex();
      if(index.column()==0) {
        Parameter *parameter = dynamic_cast<Parameter*>(static_cast<EmbeddingTreeModel*>(embeddingList->model())->getItem(index)->getItemData());
        if(parameter) {
          QMenu *menu = parameter->createContextMenu();
          menu->exec(QCursor::pos());
          delete menu;
          return;
        }
        Element *element = dynamic_cast<Element*>(static_cast<EmbeddingTreeModel*>(embeddingList->model())->getItem(index)->getItemData());
        if(element) {
          QMenu *menu = element->createEmbeddingContextMenu();
          menu->exec(QCursor::pos());
          delete menu;
        }
      } 
    }
  }

  void MainWindow::newProject(bool ask) {
    QMessageBox::StandardButton ret = QMessageBox::Ok;
    if(ask) 
      ret = QMessageBox::warning(this, "New Project", "Current project will be deleted", QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Ok) {
      newMBS();
      selectDOPRI5Integrator();
      actionSaveProject->setDisabled(true);
      fileProject="";
    }
    setWindowTitle("MBS.mbsimprj.xml");
  }

  void MainWindow::loadProject(const QString &file) {
    if(not(file.isEmpty())) {
      mbsDir = QFileInfo(file).absolutePath();
      QDir::setCurrent(QFileInfo(file).absolutePath());
      fileProject=QDir::current().relativeFilePath(file);
      setCurrentProjectFile(file);
      MBSimObjectFactory::initialize();
      shared_ptr<xercesc::DOMDocument> doc=MainWindow::parser->parse(file.toStdString());
      DOMElement *e=doc->getDocumentElement();
      DOMElement *ele0=doc->getDocumentElement();
      //setWindowTitle(QString::fromStdString(E(ele0)->getAttribute("name")));
      setWindowTitle(fileProject);

      DOMElement *ele1 = ele0->getFirstElementChild();
      Solver *solver=Embed<Solver>::createAndInit(ele1,0);
      solver->initialize();

      EmbeddingTreeModel *pmodel = static_cast<EmbeddingTreeModel*>(embeddingList->model());
      QModelIndex index = pmodel->index(0,0);
      pmodel->removeRows(index.row(), pmodel->rowCount(QModelIndex()), index.parent());
        
      ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
      index = model->index(0,0);
      if(model->rowCount(index))
        delete model->getItem(index)->getItemData();
      model->removeRow(index.row(), index.parent());
      model->createGroupItem(solver);

      elementList->selectionModel()->setCurrentIndex(model->index(0,0), QItemSelectionModel::ClearAndSelect);

      ele1 = ele1->getNextElementSibling();

      Integrator *integrator;
      if(E(ele1)->getTagName()==PV%"Embed") {
        DOMElement *ele2 = 0;
        if(E(ele1)->hasAttribute("href"))
          integrator=Integrator::readXMLFile(E(ele1)->getAttribute("href"));
        else {
          ele2 = ele1->getFirstElementChild();
          integrator=ObjectFactory::getInstance()->createIntegrator(ele2);
        }
        integrator->initializeUsingXMLEmbed(ele1);
        if(ele2)
          integrator->initializeUsingXML(ele2);
      } else {
        integrator=ObjectFactory::getInstance()->createIntegrator(ele1);
        integrator->initializeUsingXML(ele1);
      }
      integratorView->setIntegrator(integrator);

      actionSaveProject->setDisabled(false);

      mbsimxml(1);
    }
  }

  void MainWindow::loadProject() {
    QString file=QFileDialog::getOpenFileName(0, "XML project files", ".", "XML files (*.mbsimprj.xml)");
    if(file!="")
      loadProject(file);
  }

  void MainWindow::saveProjectAs() {
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    QModelIndex index = model->index(0,0);
    QString file=QFileDialog::getSaveFileName(0, "XML project files", QString("./")+QString::fromStdString(model->getItem(index)->getItemData()->getName())+".mbsimprj.xml", "XML files (*.mbsimprj.xml)");
    if(not(file.isEmpty())) {
      file = (file.length()>13 and file.right(13)==".mbsimprj.xml")?file:file+".mbsimprj.xml";
      mbsDir = QFileInfo(file).absolutePath();
      QDir::setCurrent(QFileInfo(file).absolutePath());
      fileProject=QDir::current().relativeFilePath(file);
      setCurrentProjectFile(file);
      setWindowTitle(fileProject);
      actionSaveProject->setDisabled(false);
      saveProject();
    }
  }

  DOMElement* MainWindow::writeProject(shared_ptr<xercesc::DOMDocument> &doc) {
    DOMElement *ele0=D(doc)->createElement(MBSIMXML%"MBSimProject");
    doc->insertBefore(ele0, NULL);
    E(ele0)->setAttribute("name", "Project");

    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    QModelIndex index = model->index(0,0);
    Solver *solver = static_cast<Solver*>(model->getItem(index)->getItemData());

    Embed<Solver>::writeXML(solver,ele0);
    Integrator *integrator = integratorView->getIntegrator();
    if(not(absolutePath) and integrator->isEmbedded())
      integrator->writeXMLFileEmbed(ele0);
    else
      integrator->writeXMLFile(ele0);

    return ele0;
  }

 void MainWindow::saveProject(const QString &fileName) {
    shared_ptr<xercesc::DOMDocument> doc=MainWindow::parser->createDocument();
    DOMElement *ele0=writeProject(doc);
    DOMParser::serialize(doc.get(), fileName.isEmpty()?fileProject.toStdString():fileName.toStdString());
  }

 void MainWindow::newMBS() {
   mbsDir = QDir::current();
   actionOpenMBV->setDisabled(true);
   actionH5plotserie->setDisabled(true);
   actionSaveDataAs->setDisabled(true);
   actionSaveMBSimH5DataAs->setDisabled(true);
   actionSaveOpenMBVDataAs->setDisabled(true);

   EmbeddingTreeModel *pmodel = static_cast<EmbeddingTreeModel*>(embeddingList->model());
   QModelIndex index = pmodel->index(0,0);
   pmodel->removeRows(index.row(), pmodel->rowCount(QModelIndex()), index.parent());

   ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
   index = model->index(0,0);
   if(model->rowCount(index))
     delete model->getItem(index)->getItemData();
   model->removeRow(index.row(), index.parent());
   Solver *solver = new Solver("MBS",0);
   model->createGroupItem(solver,QModelIndex());

   elementList->selectionModel()->setCurrentIndex(model->index(0,0), QItemSelectionModel::ClearAndSelect);

   mbsimxml(1);
 }

  void MainWindow::selectIntegrator() {
    QMenu *menu = integratorView->createContextMenu();
    menu->exec(QCursor::pos());
    delete menu;
  }

  void MainWindow::selectDOPRI5Integrator() {
    integratorView->setIntegrator(0);
  }

  void MainWindow::selectRADAU5Integrator() {
    integratorView->setIntegrator(1);
  }

  void MainWindow::selectLSODEIntegrator() {
    integratorView->setIntegrator(2);
  }

  void MainWindow::selectLSODARIntegrator() {
    integratorView->setIntegrator(3);
  }

  void MainWindow::selectTimeSteppingIntegrator() {
    integratorView->setIntegrator(4);
  }

  void MainWindow::selectEulerExplicitIntegrator() {
    integratorView->setIntegrator(5);
  }

  void MainWindow::selectRKSuiteIntegrator() {
    integratorView->setIntegrator(6);
  }

  // update model parameters including additional paramters from paramList
  void MainWindow::updateOctaveParameters(Element *element) {
    shared_ptr<xercesc::DOMDocument> doc=MainWindow::parser->createDocument();
    vector<Element*> parents = element->getParents();
    Parameters param;
    for(int i=0; i<parents.size(); i++)
      param.addParameters(parents[i]->getParameters());
    param.addParameters(element->getParameters());
    string counterName = element->getCounterName();
    ScalarParameter *counter = NULL;
    if(not(counterName.empty())) {
      counter = new ScalarParameter(counterName,"1");
      param.addParameter(counter);
    }
    param.writeXMLFile(doc.get());
    if(counter) delete counter;

    DOMElement *root = doc->getDocumentElement();
    if(!E(root)->getFirstProcessingInstructionChildNamed("OriginalFilename")) {
      DOMProcessingInstruction *filenamePI=doc->createProcessingInstruction(X()%"OriginalFilename",
          X()%"MBS.mbsimparam.xml");
      root->insertBefore(filenamePI, root->getFirstChild());
    }

    try {
      D(doc)->validate();
    }
    catch(...) {
      cout << "Error in validation" << endl;
      return;
    }

    try {
      // remove all parameters from octave using delete and new NewParamLevel
      // (this will not work for nested parameters in embed!???)
      delete octEvalParamLevel;
      octEvalParamLevel=new NewParamLevel(*octEval);
      // add parameter
      octEval->addParamSet(doc->getDocumentElement());
    }
    catch(DOMEvalExceptionList error) {
      cout << "An exception occurred in updateOctaveParameters: " << error.what() << endl;
    }
    catch(runtime_error error) {
      cout << "An unknown exception occurred in updateOctaveParameters: " << error.what() << endl;
    }
    catch(...) {
      cout << "An unknown exception occurred in updateOctaveParameters." << endl;
    }
  }

  void MainWindow::saveDataAs() {
    QString dir = QFileDialog::getExistingDirectory (0, "Export simulation data", ".");
    if(dir != "") {
      QDir directory(dir);
      QMessageBox::StandardButton ret = QMessageBox::Ok;
      if(directory.count()>2)
        ret = QMessageBox::warning(this, tr("Application"), tr("Directory not empty. Overwrite existing files?"), QMessageBox::Ok | QMessageBox::Cancel);
      if(ret == QMessageBox::Ok) {
        saveMBSimH5Data(dir+"/MBS.mbsim.h5");
        saveOpenMBVXMLData(dir+"/MBS.ombv.xml");
        saveOpenMBVH5Data(dir+"/MBS.ombv.h5");
      }
    }
  }

  void MainWindow::saveMBSimH5DataAs() {
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    QModelIndex index = model->index(0,0);
    QString file=QFileDialog::getSaveFileName(0, "Export MBSim H5 file", QString("./")+QString::fromStdString(model->getItem(index)->getItemData()->getName())+".mbsim.h5", "H5 files (*.mbsim.h5)");
    if(file!="") {
      saveMBSimH5Data(file);
    }
  }

  void MainWindow::saveMBSimH5Data(const QString &file) {
    if(QFile::exists(file))
      QFile::remove(file);
    QFile::copy(QString::fromStdString(uniqueTempDir.generic_string())+"/out0.mbsim.h5",file);
  }

  void MainWindow::saveOpenMBVDataAs() {
    QString dir = QFileDialog::getExistingDirectory (0, "Export OpenMBV data", ".");
    if(dir != "") {
      QDir directory(dir);
      QMessageBox::StandardButton ret = QMessageBox::Ok;
      if(directory.count()>2)
        ret = QMessageBox::warning(this, tr("Application"), tr("Directory not empty. Overwrite existing files?"), QMessageBox::Ok | QMessageBox::Cancel);
      if(ret == QMessageBox::Ok) {
        saveOpenMBVXMLData(dir+"/MBS.ombv.xml");
        saveOpenMBVH5Data(dir+"/MBS.ombv.h5");
      }
    }
  }

  void MainWindow::saveOpenMBVXMLData(const QString &file) {
    if(QFile::exists(file))
      QFile::remove(file);
    QFile::copy(QString::fromStdString(uniqueTempDir.generic_string())+"/out0.ombv.xml",file);
  }

  void MainWindow::saveOpenMBVH5Data(const QString &file) {
    if(QFile::exists(file))
      QFile::remove(file);
    QFile::copy(QString::fromStdString(uniqueTempDir.generic_string())+"/out0.ombv.h5",file);
  }

  void MainWindow::mbsimxml(int task) {
    absolutePath = true;
    QModelIndex index = elementList->model()->index(0,0);
    Solver *slv=dynamic_cast<Solver*>(static_cast<ElementTreeModel*>(elementList->model())->getItem(index)->getItemData());
    Integrator *integ=integratorView->getIntegrator();
    if(!slv || !integ)
      return;

    QString sTask = QString::number(task); 
    string saveName=slv->getName();
    slv->setName("out"+sTask.toStdString());
    QString projectFile=QString::fromStdString(uniqueTempDir.generic_string())+"/in"+sTask+".mbsimprj.xml";

    currentTask = task;

    if(task==1 and OpenMBVGUI::MainWindow::getInstance()->getObjectList()->invisibleRootItem()->childCount())
      static_cast<OpenMBVGUI::Group*>(OpenMBVGUI::MainWindow::getInstance()->getObjectList()->invisibleRootItem()->child(0))->unloadFileSlot();

    shared_ptr<xercesc::DOMDocument> doc=MainWindow::parser->createDocument();
    writeProject(doc);
    slv->setName(saveName);
    absolutePath = false;

    mbsimThread->setDocument(doc);
    mbsimThread->setProjectFile(projectFile);
    mbsimThread->start();
  }

  void MainWindow::preprocessFinished(int result) {
    if(result==0) {
      QString uniqueTempDir_ = QString::fromStdString(uniqueTempDir.generic_string());
      QString projectFile=uniqueTempDir_+"/in"+QString::number(currentTask)+".mbsimprj.xml";
      QStringList arg;
      if(currentTask==1)
        arg.append("--stopafterfirststep");
      arg.append(projectFile);
      mbsim->getProcess()->setWorkingDirectory(uniqueTempDir_);
      mbsim->clearOutputAndStart((MBXMLUtils::getInstallPath()/"bin"/"mbsimflatxml").string().c_str(), arg);
    }
    else
      mbsim->preprocessFailed(mbsimThread->getErrorText());
  }

  void MainWindow::refresh() {
    mbsimxml(1);
  }

  void MainWindow::simulate() {
    mbsimxml(0);
    actionSaveDataAs->setDisabled(false);
    actionSaveMBSimH5DataAs->setDisabled(false);
    actionSaveOpenMBVDataAs->setDisabled(false);
    actionSimulate->setDisabled(true);
    actionOpenMBV->setDisabled(true);
    actionH5plotserie->setDisabled(true);
    actionRefresh->setDisabled(true);
    statusBar()->showMessage(tr("Simulating"));
  }

  void MainWindow::openmbv() {
    QString name = QString::fromStdString(uniqueTempDir.generic_string()+"/out0.ombv.xml");
    if(QFile::exists(name)) {
      QStringList arg;
      arg.append("--autoreload");
      arg.append(name);
      QProcess::startDetached((MBXMLUtils::getInstallPath()/"bin"/"openmbv").string().c_str(), arg);
    }
  }

  void MainWindow::h5plotserie() {
    QString name = QString::fromStdString(uniqueTempDir.generic_string()+"/out0.mbsim.h5");
    if(QFile::exists(name)) {
      QStringList arg;
      arg.append(name);
      QProcess::startDetached((MBXMLUtils::getInstallPath()/"bin"/"h5plotserie").string().c_str(), arg);
    }
  }

  void MainWindow::selectElement(string ID) {
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    map<string, QModelIndex>::iterator it=model->idEleMap.find(ID);
    if(it!=model->idEleMap.end())
      elementList->selectionModel()->setCurrentIndex(it->second,QItemSelectionModel::ClearAndSelect);
  }

  void MainWindow::help() {
    QMessageBox::information(this, "MBSim GUI - GUI Help", 
        "<h1>GUI Help</h1>"
        "tbd"
        );
  }

  void MainWindow::about() {
    QMessageBox::about(this, "About MBSim GUI",
        "<h1>MBSim GUI - A frontend for MBSim</h1>"
        "<p>Copyright &copy; Martin Foerg <tt>&lt;martin.o.foerg@googlemail.com&gt;</tt><p/>"
        "<p>Licensed under the General Public License (see file COPYING).</p>"
        "<p>This is free software; see the source for copying conditions.  There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.</p>"
        "<h2>Authors:</h2>"
        "<ul>"
        "  <li>Martin Foerg <tt>&lt;martin.o.foerg@googlemail.com&gt;</tt> (Maintainer) </li>"
        "</ul>"
        "<h2>This program uses:</h2>"
        "<ul>"
        "tbd"
        //"  <li>'Qt - A cross-platform application and UI framework' by Nokia from <tt>http://www.qtsoftware.com</tt> (License: GPL/LGPL)</li>"
        //"  <li>'mbsimflatxml - tbd <tt>http://code.google.com/p/mbsim-env</tt> (Licence: Qwt/LGPL)</li>"
        //"  <li>'OpenMBV - tbd <tt>http://code.google.com/p/openmbv</tt> (License: LGPL)</li>"
        //"  <li>'h5plotserie - tbd <tt>http://code.google.com/p/hdf5serie</tt> (License: NCSA-HDF)</li>"
        //"  <li>...</li>"
        "</ul>"
        );
  }

  void MainWindow::removeElement() {
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    QModelIndex index = elementList->selectionModel()->currentIndex();
    Element *element = static_cast<Element*>(model->getItem(index)->getItemData());
    element->getParent()->removeElement(element);
    model->removeRow(index.row(), index.parent());
    mbsimxml(1);
  }

  void MainWindow::saveElementAs() {
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    QModelIndex index = elementList->selectionModel()->currentIndex();
    QString file=QFileDialog::getSaveFileName(0, "XML model files", QString("./")+QString::fromStdString(model->getItem(index)->getItemData()->getName())+".xml", "XML files (*.xml)");
    if(file!="")
      static_cast<Element*>(model->getItem(index)->getItemData())->writeXMLFileEmbed(file.toStdString());
  }

  void MainWindow::addFrame(Frame *frame) {
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    QModelIndex index = elementList->selectionModel()->currentIndex();
    QModelIndex containerIndex = index.data().toString()=="frames"?index:index.child(0,0); 
    frame->setName(frame->getName()+toStr(model->getItem(containerIndex)->getID()));
    frame->getParent()->addFrame(frame);
    model->createFrameItem(frame,containerIndex);
    QModelIndex currentIndex = containerIndex.child(model->rowCount(containerIndex)-1,0);
    elementList->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::ClearAndSelect);
    elementList->openEditor();
  }

  void MainWindow::addContour(Contour *contour) {
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    QModelIndex index = elementList->selectionModel()->currentIndex();
    QModelIndex containerIndex = (index.row()==0)?index.child(1,0):index;
    contour->setName(contour->getName()+toStr(model->getItem(containerIndex)->getID()));
    contour->getParent()->addContour(contour);
    model->createContourItem(contour,containerIndex);
    QModelIndex currentIndex = containerIndex.child(model->rowCount(containerIndex)-1,0);
    elementList->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::ClearAndSelect);
    elementList->openEditor();
  }

  void MainWindow::addGroup(Group *group) {
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    QModelIndex index = elementList->selectionModel()->currentIndex();
    QModelIndex containerIndex = (index.row()==0)?index.child(2,0):index;
    group->setName(group->getName()+toStr(model->getItem(containerIndex)->getID()));
    group->getParent()->addGroup(group);
    model->createGroupItem(group,containerIndex);
    QModelIndex currentIndex = containerIndex.child(model->rowCount(containerIndex)-1,0);
    elementList->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::ClearAndSelect);
    elementList->openEditor();
  }

  void MainWindow::addObject(Object *object) {
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    QModelIndex index = elementList->selectionModel()->currentIndex();
    QModelIndex containerIndex = (index.row()==0)?index.child(3,0):index;
    object->setName(object->getName()+toStr(model->getItem(containerIndex)->getID()));
    object->getParent()->addObject(object);
    model->createObjectItem(object,containerIndex);
    QModelIndex currentIndex = containerIndex.child(model->rowCount(containerIndex)-1,0);
    elementList->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::ClearAndSelect);
    elementList->openEditor();
  }

  void MainWindow::addLink(Link *link) {
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    QModelIndex index = elementList->selectionModel()->currentIndex();
    QModelIndex containerIndex = (index.row()==0)?index.child(4,0):index;
    link->setName(link->getName()+toStr(model->getItem(containerIndex)->getID()));
    link->getParent()->addLink(link);
    model->createLinkItem(link,containerIndex);
    QModelIndex currentIndex = containerIndex.child(model->rowCount(containerIndex)-1,0);
    elementList->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::ClearAndSelect);
    elementList->openEditor();
  }

  void MainWindow::addObserver(Observer *observer) {
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    QModelIndex index = elementList->selectionModel()->currentIndex();
    QModelIndex containerIndex = (index.row()==0)?index.child(5,0):index;
    observer->setName(observer->getName()+toStr(model->getItem(containerIndex)->getID()));
    observer->getParent()->addObserver(observer);
    model->createObserverItem(observer,containerIndex);
    QModelIndex currentIndex = containerIndex.child(model->rowCount(containerIndex)-1,0);
    elementList->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::ClearAndSelect);
    elementList->openEditor();
  }

  void MainWindow::addParameter(Parameter *parameter) {
    QModelIndex index = embeddingList->selectionModel()->currentIndex();
    EmbeddingTreeModel *model = static_cast<EmbeddingTreeModel*>(embeddingList->model());
    Element *element = static_cast<Element*>(model->getItem(index)->getItemData());
    parameter->setName(parameter->getName()+toStr(model->getItem(index)->getID()));
    QModelIndex newIndex = model->createParameterItem(parameter,index);
    element->addParameter(parameter);
    embeddingList->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
    embeddingList->openEditor();
  }

  void MainWindow::removeParameter() {
    QModelIndex index = elementList->selectionModel()->currentIndex();
    ElementTreeModel *model = static_cast<ElementTreeModel*>(elementList->model());
    Element *element=static_cast<Element*>(model->getItem(index)->getItemData());
    EmbeddingTreeModel *pmodel = static_cast<EmbeddingTreeModel*>(embeddingList->model());
    QModelIndex pindex = embeddingList->selectionModel()->currentIndex();
    Parameter *parameter=static_cast<Parameter*>(pmodel->getItem(pindex)->getItemData());
    element->removeParameter(parameter);
    pmodel->removeRow(pindex.row(), pindex.parent());
  }

  void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
      event->acceptProposedAction();
    }
  }

  void MainWindow::dropEvent(QDropEvent *event) {
    for (int i = 0; i < event->mimeData()->urls().size(); i++) {
      QString path = event->mimeData()->urls()[i].toLocalFile().toLocal8Bit().data();
      if(path.endsWith("mbsimprj.xml")) {
        QFile Fout(path);
        if (Fout.exists())
          loadProject(Fout.fileName());
      }
    }
  }

  void MainWindow::openRecentProjectFile() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
      loadProject(action->data().toString());
  }

  void MainWindow::setCurrentProjectFile(const QString &fileName) {

    QSettings settings;
    QStringList files = settings.value("recentProjectFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > maxRecentFiles)
      files.removeLast();

    settings.setValue("recentProjectFileList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
      MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
      if (mainWin)
        mainWin->updateRecentProjectFileActions();
    }
  }

  void MainWindow::updateRecentProjectFileActions() {
    QSettings settings;
    QStringList files = settings.value("recentProjectFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)maxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
      QString text = QDir::current().relativeFilePath(files[i]);
      recentProjectFileActs[i]->setText(text);
      recentProjectFileActs[i]->setData(files[i]);
      recentProjectFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < maxRecentFiles; ++j)
      recentProjectFileActs[j]->setVisible(false);

    //separatorAct->setVisible(numRecentFiles > 0);
  }

}
