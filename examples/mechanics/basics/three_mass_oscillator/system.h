#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "mbsim/group.h"

class System : public MBSim::Group {
  public:
    System(unsigned int type, double dDisk);
};

#endif /* _SYSTEM_H_ */

