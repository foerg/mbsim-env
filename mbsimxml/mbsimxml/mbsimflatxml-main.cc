#include "config.h"
#include <cstring>
#include "mbsimflatxml.h"

using namespace std;
using namespace MBSim;

int main(int argc, char *argv[]) {
  Integrator *integrator;
  DynamicSystemSolver *dss;

  bool doNotIntegrate=false;
  if(argc>=2 && strcmp(argv[1],"--donotintegrate")==0)
    doNotIntegrate=true;

  int ret;
  ret=MBSimXML::preInitDynamicSystemSolver(argc, argv, dss);
  if(ret==-1) return 0; // help message was printed
  if(ret!=0) return ret;
  ret=MBSimXML::initDynamicSystemSolver(argc, argv, dss);
  if(ret!=0) return ret;
  ret=MBSimXML::initIntegrator(argc, argv, integrator);
  if(ret!=0) return ret;
  if(doNotIntegrate==false) {
    ret=MBSimXML::main(integrator, dss);
    if(ret!=0) return ret;
  }
  ret=MBSimXML::postMain(integrator, dss);
  if(ret!=0) return ret;

  return 0;
}
