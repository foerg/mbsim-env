#include "system.h"
#include <mbsim/integrators/integrators.h>

using namespace MBSim;

int main (int argc, char* argv[]) {

  bool setValued=false;

  System *sys = new System("MBS", setValued);
  sys->setPlotFeature(Element::plotRecursive, Element::enabled);
  sys->setPlotFeature(Element::state, Element::enabled);
  sys->setPlotFeature(Element::globalPosition, Element::enabled);
  sys->setStopIfNoConvergence(true, true);
  sys->initialize();

  Integrator * integrator;
  if (setValued) {
    integrator = new TimeSteppingIntegrator();
    static_cast<TimeSteppingIntegrator*>(integrator)->setStepSize(1e-4);
  }
  else
    integrator = new LSODEIntegrator();
  integrator->setEndTime(4e-0);
  integrator->setPlotStepSize(1e-3);
  integrator->integrate(*sys);

  sys->closePlot();
  delete sys;
  delete integrator;

  return 0;
}
