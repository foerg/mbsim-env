#ifndef _STRIBECK_FRICTION_H
#define _STRIBECK_FRICTION_H

#include "function.h"

class Friction : public fmatvec::Function<double(double)> {
  public:
    /*! 
     * \brief constructor
     * \param friction coefficients
     */
    Friction(double mu0_, double mu1_, double mu2_, double k_) : mu0(mu0_), mu1(mu1_), mu2(mu2_), k(k_) {}

    /*! 
     * \brief destructor
     */
    virtual ~Friction() {}

    /* INHERITED INTERFACE OF FUNCTION1 */
    virtual double operator()(const double& gT);
    /***************************************************/

  protected:
    /** 
     * \brief friction coefficients
     */
    double mu0, mu1, mu2, k;
};

inline double Friction::operator()(const double& gT) { return mu0+mu1/(1.+mu2*pow(gT,k)); }

#endif /* _STRIBECK_FRICTION_H */

