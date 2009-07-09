#include "system.h"
#include <mbsim/integrators/integrators.h>

using namespace MBSim;
using namespace std;

int main (int argc, char* argv[]) {

  DynamicSystemSolver *sys = new System("MBS");

  sys->setStopIfNoConvergence(true,true);
  sys->init();

  TimeSteppingIntegrator integrator;

  integrator.setEndTime(0.5);
  integrator.setStepSize(5e-5);
  integrator.setPlotStepSize(5e-3);

  integrator.integrate(*sys);

  sys->closePlot();

  cout << "finished"<<endl;

  delete sys;

  return 0;

}

