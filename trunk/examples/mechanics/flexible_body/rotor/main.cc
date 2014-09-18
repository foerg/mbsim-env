#include "system.h"
#include <mbsim/integrators/integrators.h>

using namespace MBSim;
using namespace MBSimIntegrator;
using namespace std;

int main (int argc, char* argv[]) {

  System *sys = new System("MBS");
  sys->initialize();

  HETS2Integrator integrator;
  integrator.setEndTime(0.5);
  integrator.setStepSize(1e-5);
  integrator.setPlotStepSize(1e-3);

  integrator.integrate(*sys);

  cout << "finished"<<endl;

  delete sys;

  return 0;

}

