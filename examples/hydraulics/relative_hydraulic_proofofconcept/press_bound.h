#ifndef PRESS_BOUND_H
#define PRESS_BOUND_H

#include <mbsim/links/link.h>
#include <vector>
#include "line.h"

namespace MBSim {

class PressBound : public Link {
  protected:
    double p;
    std::vector<Line*> conIn, conOut;
  public:
    PressBound(std::string name);
    void updateg() {}
    void updategd() {}
    void updateWRef(fmatvec::Mat&, int) {}
    void updateVRef(fmatvec::Mat&, int) {}
    void updatehRef(fmatvec::Vec&, int) {}
    void updatedhdqRef(fmatvec::Mat&, int) {}
    void updatedhduRef(fmatvec::SqrMat&, int) {}
    void updatedhdtRef(fmatvec::Vec&, int) {}
    void updaterRef(fmatvec::Vec&, int) {}
    void updaterRef(fmatvec::Vec&) {}
    bool isActive() const { return false; }
    virtual bool isSingleValued() const { return true; }
    bool gActiveChanged() { return false; }
    void setPressure(double p_) { p=p_; }
    void addInConnection(Line *l) { conIn.push_back(l); }
    void addOutConnection(Line *l) { conOut.push_back(l); }
    void updateh(int k=0);
};

}

#endif
