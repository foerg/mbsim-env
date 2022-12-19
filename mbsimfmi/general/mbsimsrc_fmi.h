#ifndef _MBSIMFMI_MBSIMSRC_FMI_H_
#define _MBSIMFMI_MBSIMSRC_FMI_H_

namespace MBSim {
  class DynamicSystemSolver;
  class Integrator;
}

/*! Include this header file for source models which should be exported as FMI
 * and implement this function.
 * \param dss must return the pointer to the DynamicSystemSolver of your system.
 * This DynamicSystemSolver must not be initialized but the model must be fully
 * build up!
 * (The DynamicSystemSolver::initialize() function is the first function which
 * is called by FMI during execution of the model.)
 */
extern "C" void mbsimSrcFMI(MBSim::DynamicSystemSolver *&dss, MBSim::Integrator *&integrator);

namespace MBSimFMI {
  // just a function pointer typedef for the above function
  using mbsimSrcFMIPtr = void (*)(MBSim::DynamicSystemSolver *&, MBSim::Integrator *&);
}

#endif
