/* Copyright (C) 2004-2006  Martin Förg

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

 *
 * Contact:
 *   mfoerg@users.berlios.de
 *
 */

#ifndef _JOINT_H_
#define _JOINT_H_

#include <mbsim/link.h>
#include <mbsim/frame.h>

#ifdef HAVE_AMVIS
namespace AMVis {class CoilSpring;}
#endif

namespace MBSim {

  class DataInterfaceBase;
  class GeneralizedForceLaw;
  class GeneralizedImpactLaw;
  class FrictionForceLaw;
  class FrictionImpactLaw;

  /*! \brief Class for connections: Constraints on Frames
   *
   * */
  class Joint: public Link {

    protected:

      fmatvec::Index IT, IR;
      fmatvec::Mat forceDir, momentDir;
      fmatvec::Mat Wf, Wm;
      fmatvec::Mat JT;
      fmatvec::Vec WrP0P1, WvP0P1, WomP0P1;

      GeneralizedForceLaw *ffl, *fml;
      GeneralizedImpactLaw *fifl, *fiml;

      fmatvec::Vec gdn, gdd;
#ifdef HAVE_AMVIS
      AMVis::CoilSpring *coilspringAMVis;
      DataInterfaceBase *coilspringAMVisUserFunctionColor;
#endif

      Frame C;

    public: 
      Joint(const std::string &name);
      virtual ~Joint();
      virtual void connect(Frame *port1, Frame* port2);

      void calcxSize();
      void calcgSize();
      void calcgSizeActive();
      void calcgdSize();
      void calcgdSizeActive();
      void calclaSize();
      void calclaSizeForActiveg();
      void calcrFactorSize();

      void init();

      void setForceDirection(const fmatvec::Mat& fd);
      void setMomentDirection(const fmatvec::Mat& md);
      //void initPlotFiles(); 
      //void plot(double t, double dt=1);

      bool isActive() const {return true;}
      void checkActiveg() {}
      void checkActivegd() {}

      bool activeConstraintsChanged() {return false;}
      bool gActiveChanged() {return false;}

      void updateg(double t);
      void updategd(double t);
      void updatexd(double t);
      void updatedx(double t, double dt);
      void updateW(double t);
      void updatewb(double t);
      void updateh(double t);
      void updateJacobians(double t);

      void resizeJacobians(int j); 

      void updaterFactors();

      void solveConstraintsFixpointSingle();
      void solveImpactsFixpointSingle();
      void solveConstraintsGaussSeidel();
      void solveImpactsGaussSeidel();
      void solveImpactsRootFinding();
      void solveConstraintsRootFinding();
      void jacobianConstraints();
      void jacobianImpacts();
      bool isSetValued() const;

      void checkConstraintsForTermination();
      void checkImpactsForTermination();

      void setForceLaw(GeneralizedForceLaw * rc) {ffl = rc;}
      void setMomentLaw(GeneralizedForceLaw * rc) {fml = rc;}
      void setImpactForceLaw(GeneralizedImpactLaw * rc) {fifl = rc;}
      void setImpactMomentLaw(GeneralizedImpactLaw * rc) {fiml = rc;}

#ifdef HAVE_AMVIS
      void setAMVisSpring(AMVis::CoilSpring *spring_, DataInterfaceBase* funcColor=0) {coilspringAMVis= spring_; coilspringAMVisUserFunctionColor= funcColor;}
#endif
  };

}

#endif
