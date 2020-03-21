/* Copyright (C) 2017 Markus Friedrich
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 *  
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *  
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "boost_odeint_integrator.h"

// runge_kutta_dopri5<Vec>
#include <boost/numeric/odeint/stepper/runge_kutta_dopri5.hpp>
#include <boost/numeric/odeint/stepper/dense_output_runge_kutta.hpp>

// bulirsch_stoer<Vec>
#include <boost/numeric/odeint/stepper/bulirsch_stoer_dense_out.hpp>

// euler<Vec>
#include <boost/numeric/odeint/stepper/euler.hpp>

// rosenbrock4<double>
#include <boost/numeric/odeint/stepper/rosenbrock4_dense_output.hpp>

namespace MBSim {

  namespace BoostOdeintHelper {

    // Definition of concepts of DOS. (see header file BoostOdeintDOS)



    // runge_kutta_dopri5<Vec>
  
    // type definitions
    typedef boost::numeric::odeint::runge_kutta_dopri5<fmatvec::Vec> RKDOPRI5Stepper;
    typedef boost::numeric::odeint::controlled_runge_kutta<RKDOPRI5Stepper> ControlledRK;
    typedef boost::numeric::odeint::dense_output_runge_kutta<ControlledRK> DOSRK;

    // DOS concept for the boost odeint runge_kutta_dopri5<Vec> stepper
    class RKDOPRI5 : public DOSRK {
      public:
        typedef ExplicitSystemTag SystemCategory;
        typedef typename ControlledRK::stepper_category UnderlayingStepperCategory;
        RKDOPRI5(double aTol, double rTol, double dtMax) :
          DOSRK(ControlledRK(ControlledRK::error_checker_type(aTol, rTol), ControlledRK::step_adjuster_type(dtMax), RKDOPRI5Stepper())) {}
    };



    // bulirsch_stoer<Vec>

    // type definitions
    typedef boost::numeric::odeint::bulirsch_stoer_dense_out<fmatvec::Vec> DOSBS;

    // DOS concept for the boost odeint bulirsch_stoer_dense_out<Vec> stepper.
    class BulirschStoer : public DOSBS {
      public:
        typedef ExplicitSystemTag SystemCategory;
        typedef boost::numeric::odeint::controlled_stepper_tag UnderlayingStepperCategory;
        BulirschStoer(double aTol, double rTol, double dtMax) : DOSBS(aTol, rTol, 1.0, 1.0, dtMax) {}
    };



    // euler<Vec>

    // type definitions
    typedef boost::numeric::odeint::euler<fmatvec::Vec> EulerStepper;
    typedef boost::numeric::odeint::dense_output_runge_kutta<EulerStepper> DOSEuler;

    // DOS concept for the boost odeint euler<Vec> stepper
    class Euler : public DOSEuler {
      public:
        typedef ExplicitSystemTag SystemCategory;
        typedef typename EulerStepper::stepper_category UnderlayingStepperCategory;
        Euler(double aTol, double rTol, double dtMax) : DOSEuler() {}
    };



    // rosenbrock4<double>

    // type definitions
    typedef boost::numeric::odeint::rosenbrock4<double> RB4;
    typedef boost::numeric::odeint::rosenbrock4_controller<RB4> ControlledRB4;
    typedef boost::numeric::odeint::rosenbrock4_dense_output<ControlledRB4> DOSRB4;
  
    // DOS concept for the boost odeint rosenbrock4<double> stepper
    class Rosenbrock4 : public DOSRB4 {
      public:
        typedef ImplicitSystemTag SystemCategory;
        typedef typename ControlledRB4::stepper_category UnderlayingStepperCategory;
        Rosenbrock4(double aTol, double rTol, double dtMax) : DOSRB4(ControlledRB4(aTol, rTol, dtMax)) {}
    };

  }

  // explicit integrators
  typedef BoostOdeintDOS<BoostOdeintHelper::RKDOPRI5     > BoostOdeintDOS_RKDOPRI5;
  typedef BoostOdeintDOS<BoostOdeintHelper::BulirschStoer> BoostOdeintDOS_BulirschStoer;
  // not working due to but in boost odeint, see https://github.com/boostorg/odeint/pull/27
  // typedef BoostOdeintDOS<BoostOdeintHelper::Euler        > BoostOdeintDOS_Euler;
  // implicit integrators
  typedef BoostOdeintDOS<BoostOdeintHelper::Rosenbrock4  > BoostOdeintDOS_Rosenbrock4;

}
