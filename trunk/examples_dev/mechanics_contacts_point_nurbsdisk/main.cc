#include "system.h"
#include <mbsim/integrators/integrators.h>

using namespace MBSim;
using namespace std;

int main (int argc, char* argv[]) {

  DynamicSystemSolver *sys = new System("MBS");

  sys->setStopIfNoConvergence(true,true);
  sys->init();

  ThetaTimeSteppingIntegrator integrator;

  integrator.setEndTime(0.5);
  integrator.setStepSize(5e-4);
  integrator.setPlotStepSize(5e-4);

  integrator.integrate(*sys);

  sys->closePlot();

  cout << "finished"<<endl;

  delete sys;

  return 0;

}

