#ifndef _PERLCHAINCOSSERAT_H
#define _PERLCHAINCOSSERAT_H

#include "mbsim/dynamic_system_solver.h"
#include "mbsimFlexibleBody/flexible_body/1s_cosserat.h"
#include "mbsimFlexibleBody/flexible_body/1s_21_cosserat.h"
#include "mbsimFlexibleBody/flexible_body/1s_33_cosserat.h"
#include "mbsim/objects/rigid_body.h"
#include <string>

class System : public MBSim::DynamicSystemSolver {
  public:
    System(const std::string &projectName);

    void reduce(const std::string & h5file);

  protected:
    /** flexible ring */
    MBSimFlexibleBody::FlexibleBody1s21Cosserat *rod;

    /** vector of balls */
    std::vector<MBSim::RigidBody*> balls;
};

#endif

