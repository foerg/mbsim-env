#include "line.h"
#include <mbsim/dynamic_system.h>

using namespace MBSim;
using namespace std;
using namespace fmatvec;

Line::Line(string name) : Object(name), updJ(true) {
}

void Line::updateStateDependentVariables(double) {
  if(dependency.size()==0)
    flowrate=u(0);
  else {
    flowrate=0;
    for(size_t i=0; i<dependency.size(); i++)
      flowrate+=((Line*)dependency[i])->getFlowrate();
  }
}

void Line::calcuSize(int j) {
  if(dependency.size()==0)
    uSize[j]=1;
  else
    uSize[j]=0;
}

void Line::updateM() {
  M+=1.*JTJ(evalJ());
}

void Line::updateJacobians(int k) {
  // How to calcualte the reactive forces for relative hydraulic lines?
  // Don't know => do nothing => this leads to wrong reactive forces in the whole model
  // but this does not influence the dynamics.
  if(k==0) {
    if(dependency.size()==0) {
      if(M.size()==1)
        J.resize(1,1,INIT,1);
      else {
        J.resize(1,M.size());
        J(0,uInd[0])=1;
      }
    }
    else {
      J.resize(1,M.size());
      for(size_t i=0; i<dependency.size(); i++) {
        Mat Jdep=((Line*)dependency[i])->evalJ(k);
        J.add(0,RangeV(0,Jdep.cols()-1),Jdep);
      }
    }
  }
  updJ = false;
}

void Line::init(InitStage stage, const InitConfigSet &config) {
  if(stage==plotting) {
    plotColumns.push_back("flowrate");
  }
  Object::init(stage, config);
}

void Line::plot() {
  plotVector.push_back(flowrate);
  Object::plot();
}
