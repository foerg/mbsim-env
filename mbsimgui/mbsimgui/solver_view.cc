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
#include "integrator.h"
#include "analyzer.h"
#include "solver_view.h"
#include "solver_property_dialog.h"
#include "project.h"
#include "utils.h"
#include "mainwindow.h"
#include <QApplication>

namespace MBSimGUI {

  extern MainWindow *mw;

  SolverContextMenu::SolverContextMenu(const std::vector<QString> &type, QWidget *parent) : QMenu(parent) {
    auto *action=new QAction(QIcon::fromTheme("document-properties"), "Edit", this);
    connect(action,&QAction::triggered,this,[=](){ mw->openSolverEditor(); });
    addAction(action);
    action=new QAction(QIcon::fromTheme("document-properties"), "View XML", this);
    connect(action,&QAction::triggered,mw,&MainWindow::viewSolverSource);
    addAction(action);
    addSeparator();
    action = new QAction(QIcon::fromTheme("document-save-as"), "Export", this);
    connect(action,&QAction::triggered,mw,&MainWindow::exportSolver);
    addAction(action);
    addSeparator();
    action = new QAction(QIcon::fromTheme("document-open"), "Embed", this);
    connect(action,&QAction::triggered,this,[=](){ mw->loadSolver(true); });
    addAction(action);
    action = new QAction(QIcon::fromTheme("document-open"), "Import", this);
    connect(action,&QAction::triggered,this,[=](){ mw->loadSolver(); });
    addAction(action);
    addSeparator();
    QActionGroup *actionGroup = new QActionGroup(this);
    QMenu *analyzers = new QMenu("Analyzers", this);
    for(size_t i = 0; i<2; i++) {
      QAction *action = new QAction(type[i], analyzers);
      actionGroup->addAction(action);
      analyzers->addAction(action);
    }
    QMenu *integrators = new QMenu("Integrators", this);
    for(size_t i = 2; i<type.size(); i++) {
      QAction *action = new QAction(type[i], integrators);
      actionGroup->addAction(action);
      integrators->addAction(action);
    }
    addMenu(analyzers);
    addMenu(integrators);
    connect(actionGroup,QOverload<QAction*>::of(&QActionGroup::triggered),this,&SolverContextMenu::selectSolver);
  }

  void SolverContextMenu::selectSolver(QAction *action) {
    QActionGroup *actionGroup = action->actionGroup();
    QList<QAction*> list = actionGroup->actions();
    mw->selectSolver(list.indexOf(action));
  }

  SolverView::SolverView()  {
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    setLayout(layout);
    lineEdit = new QLineEdit;
    layout->addWidget(lineEdit);
    label = new QLabel;
    layout->addWidget(label);
    type.emplace_back("Eigenanalyzer");
    type.emplace_back("Harmonic response analyzer");
    type.emplace_back("Boost odeint DOS RKDOPRI5");
    type.emplace_back("Boost odeint DOS Burlisch Stoer");
    type.emplace_back("Boost odeint DOS Rosenbrock4");
    type.emplace_back("DASPK integrator");
    type.emplace_back("DOPRI5 integrator");
    type.emplace_back("DOP853 integrator");
    type.emplace_back("Explicit Euler integrator");
    type.emplace_back("HETS2 integrator");
    type.emplace_back("Implicit Euler integrator");
    type.emplace_back("LSODA integrator");
    type.emplace_back("LSODE integrator");
    type.emplace_back("LSODI integrator");
    type.emplace_back("ODEX integrator");
    type.emplace_back("PHEM56 integrator");
    type.emplace_back("RADAU integrator");
    type.emplace_back("RADAU5 integrator");
    type.emplace_back("RKSuite integrator");
    type.emplace_back("RODAS integrator");
    type.emplace_back("SEULEX integrator");
    type.emplace_back("Theta time stepping integrator");
    type.emplace_back("Time stepping integrator");
    type.emplace_back("Time stepping SSC integrator");
    lineEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(lineEdit,&QLineEdit::customContextMenuRequested,this,&SolverView::openContextMenu);

    lineEdit->installEventFilter(new SolverMouseEvent(lineEdit));
    lineEdit->setReadOnly(true);
  }

  void SolverView::openContextMenu() {
    QMenu *menu = createContextMenu();
    menu->exec(QCursor::pos());
    delete menu;
  }

  Solver* SolverView::createSolver(int i_) {
    i = i_;
    setText(type[i]);
    if(i==0)
      return new Eigenanalyzer;
    else if(i==1)
      return new HarmonicResponseAnalyzer;
    else if(i==2)
      return new BoostOdeintDOS_RKDOPRI5;
    else if(i==3)
      return new BoostOdeintDOS_BulirschStoer;
    else if(i==4)
      return new BoostOdeintDOS_Rosenbrock4;
    else if(i==5)
      return new DASPKIntegrator;
    else if(i==6)
      return new DOPRI5Integrator;
    else if(i==7)
      return new DOP853Integrator;
    else if(i==8)
      return new ExplicitEulerIntegrator;
    else if(i==9)
      return new HETS2Integrator;
    else if(i==10)
      return new ImplicitEulerIntegrator;
    else if(i==11)
      return new LSODAIntegrator;
    else if(i==12)
      return new LSODEIntegrator;
    else if(i==13)
      return new LSODIIntegrator;
    else if(i==14)
      return new ODEXIntegrator;
    else if(i==15)
      return new PHEM56Integrator;
    else if(i==16)
      return new RADAUIntegrator;
    else if(i==17)
      return new RADAU5Integrator;
    else if(i==18)
      return new RKSuiteIntegrator;
    else if(i==19)
      return new RODASIntegrator;
    else if(i==20)
      return new SEULEXIntegrator;
    else if(i==21)
      return new ThetaTimeSteppingIntegrator;
    else if(i==22)
      return new TimeSteppingIntegrator;
    else if(i==23)
      return new TimeSteppingSSCIntegrator;
    return nullptr;
  }

  void SolverView::setSolver(Solver *solver) {
    label->setText(QString::number(solver->getSelfEmbeded()));
    if(dynamic_cast<Eigenanalyzer*>(solver))
      i=0;
    else if(dynamic_cast<HarmonicResponseAnalyzer*>(solver))
      i=1;
    else if(dynamic_cast<BoostOdeintDOS_RKDOPRI5*>(solver))
      i=2;
    else if(dynamic_cast<BoostOdeintDOS_BulirschStoer*>(solver))
      i=3;
    else if(dynamic_cast<BoostOdeintDOS_Rosenbrock4*>(solver))
      i=4;
    else if(dynamic_cast<DASPKIntegrator*>(solver))
      i=5;
    else if(dynamic_cast<DOPRI5Integrator*>(solver))
      i=6;
    else if(dynamic_cast<DOP853Integrator*>(solver))
      i=7;
    else if(dynamic_cast<ExplicitEulerIntegrator*>(solver))
      i=8;
    else if(dynamic_cast<HETS2Integrator*>(solver))
      i=9;
    else if(dynamic_cast<ImplicitEulerIntegrator*>(solver))
      i=10;
    else if(dynamic_cast<LSODAIntegrator*>(solver))
      i=11;
    else if(dynamic_cast<LSODEIntegrator*>(solver))
      i=12;
    else if(dynamic_cast<LSODIIntegrator*>(solver))
      i=13;
    else if(dynamic_cast<ODEXIntegrator*>(solver))
      i=14;
    else if(dynamic_cast<PHEM56Integrator*>(solver))
      i=15;
    else if(dynamic_cast<RADAUIntegrator*>(solver))
      i=16;
    else if(dynamic_cast<RADAU5Integrator*>(solver))
      i=17;
    else if(dynamic_cast<RKSuiteIntegrator*>(solver))
      i=18;
    else if(dynamic_cast<RODASIntegrator*>(solver))
      i=19;
    else if(dynamic_cast<SEULEXIntegrator*>(solver))
      i=20;
    else if(dynamic_cast<ThetaTimeSteppingIntegrator*>(solver))
      i=21;
    else if(dynamic_cast<TimeSteppingIntegrator*>(solver))
      i=22;
    else if(dynamic_cast<TimeSteppingSSCIntegrator*>(solver))
      i=23;
    setText(type[i]);
  }

  bool SolverMouseEvent::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::MouseButtonDblClick) {
      mw->openSolverEditor();
      return true;
    }
    else if(event->type() == QEvent::MouseButtonPress) {
      mw->solverViewClicked();
      return true;
    }
    else
      return QObject::eventFilter(obj, event);
  }

}
