/* Copyright (C) 2004-2014 MBSim Development Team
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 * 
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: martin.o.foerg@googlemail.com
 */

#ifndef _LINK_H_
#define _LINK_H_

#include "mbsim/element.h"
#include "mbsim/mbsim_event.h"

namespace H5 {
  class Group;
}

namespace MBSim {

  extern const PlotFeatureEnum generalizedRelativePosition, generalizedRelativeVelocity, generalizedForce;

  /** 
   * \brief general link to one or more objects
   * \author Martin Foerg
   * \date 2009-03-26 some comments (Thorsten Schindler)
   * \date 2009-04-06 ExtraDynamicInterface included / MechanicalLink added (Thorsten Schindler)
   * \date 2009-07-16 splitted link / object right hand side (Thorsten Schindler)
   * \date 2009-07-27 enhanced structure for implicit integration (Thorsten Schindler)
   * \date 2009-07-28 splitted interfaces (Thorsten Schindler)
   * \date 2009-12-14 revised inverse kinetics (Martin Foerg)
   * \date 2010-07-06 added LinkStatus and LinearImpactEstimation for timestepper ssc (Robert Huber)
   * \date 2012-05-08 added LinkStatusReg for AutoTimeSteppingSSCIntegrator (Jan Clauberg)
   * \date 2014-09-16 contact forces are calculated on acceleration level (Thorsten Schindler) 
   */
  //class Link : public Element, public LinkInterface, public ExtraDynamicInterface {
  class Link : public Element {
    public:
      /**
       * \brief constructor
       * \param name of link
       */
      Link(const std::string &name);

      virtual void calcSize() { }
      int getGeneralizedRelativePositionSize() { if(updSize) calcSize(); return ng; }
      int getGeneralizedRelativeVelocitySize() { if(updSize) calcSize(); return ngd; }
      int getGeneralizedForceSize() { if(updSize) calcSize(); return nla; }

      virtual void updateg() { }
      virtual void updategd() { }
      virtual void updatewb() { }
      virtual void updateW(int i=0) { }
      virtual void updateV(int i=0) { }
      virtual void updateh(int i=0) { }
      virtual void updateStopVector() { }
      virtual void updateStopVectorParameters() { }
      virtual void updateLinkStatus() { }
      virtual void updateLinkStatusReg() { }
      virtual void updateJacobians(int j=0) { }
      virtual void updateb() { }

      virtual void updatedx();
      virtual void updatexd() { }
      virtual void calcxSize() { xSize = 0; }
      virtual const fmatvec::Vec& getx() const { return x; }
      virtual fmatvec::Vec& getx() { return x; }
      virtual void setxInd(int xInd_) { xInd = xInd_; };
      virtual int getxSize() const { return xSize; }
      virtual void updatexRef(fmatvec::Vec& ref);
      virtual void updatexdRef(fmatvec::Vec& ref);
      virtual void updatedxRef(fmatvec::Vec& ref);
      virtual void updatebRef(fmatvec::Mat &hRef);
      void init(InitStage stage, const InitConfigSet &config) override;
      virtual void initz();
      virtual void writez(H5::GroupBase *group);
      virtual void readz0(H5::GroupBase *group);

      virtual void setbInd(int bInd_) { bInd = bInd_; };

      /* INHERITED INTERFACE OF ELEMENT */
      void plot() override;
      /***************************************************/

      /* INTERFACE TO BE DEFINED IN DERIVED CLASS */
      /**
       * \brief references to contact force direction matrix of dynamic system parent
       */
      virtual void updateWRef(fmatvec::Mat& ref, int i=0) = 0;

      /**
       * \brief references to condensed contact force direction matrix of dynamic system parent
       */
      virtual void updateVRef(fmatvec::Mat& ref, int i=0) = 0;

      /**
       * \brief references to complete and link smooth force vector of dynamic system parent
       */
      virtual void updatehRef(fmatvec::Vec &hRef, int i=0) = 0;

      /**
       * \brief references to nonsmooth force vector of dynamic system parent
       */
      virtual void updaterRef(fmatvec::Vec &ref, int i=0) = 0;

      /**
       * \brief references to TODO of dynamic system parent
       */
      virtual void updatewbRef(fmatvec::Vec &ref);

      /**
       * \brief references to contact force parameter of dynamic system parent
       */
      virtual void updatelaRef(fmatvec::Vec& ref);

      /**
       * \brief references to contact force parameter of dynamic system parent
       */
      virtual void updateLaRef(fmatvec::Vec& ref);

      /**
       * \brief delete reference to contact force parameter of dynamic system parent
       */
      virtual void deletelaRef();

      /**
       * \brief references to contact relative distances of dynamic system parent
       */
      virtual void updategRef(fmatvec::Vec& ref);

      /**
       * \brief references to contact relative velocities of dynamic system parent
       */
      virtual void updategdRef(fmatvec::Vec& ref);

      /**
       * \brief references to residuum of nonlinear contact equations of dynamic system parent
       */
      virtual void updateresRef(fmatvec::Vec& ref);

      /**
       * \brief references to rfactors of dynamic system parent
       */
      virtual void updaterFactorRef(fmatvec::Vec& ref);

      /**
       * \brief references to stopvector of dynamic system parent (root function for event driven integration)
       */
      virtual void updatesvRef(fmatvec::Vec &sv);

      /**
       * \brief references to stopvector evaluation of dynamic system parent (root detection with corresponding bool array by event driven integrator)
       */
      virtual void updatejsvRef(fmatvec::VecInt &jsvParent);

      /**
       * \brief reference to vector of link status (for set valued links with piecewise link equations)
       */
       virtual void updateLinkStatusRef(fmatvec::VecInt &LinkStatusParent);

      /**
       * \brief reference to vector of link status (for single-valued links)
       */
       virtual void updateLinkStatusRegRef(fmatvec::VecInt &LinkStatusRegParent);

      /**
       * \brief calculates size of contact force parameters
       */
      virtual void calclaSize(int j) { laSize = 0; }

      /**
       * \brief calculates size of relative distances
       */
      virtual void calcgSize(int j) { gSize = 0; }

      /**
       * \brief calculates size of gap velocities
       * \param flag to decide which contacts are included in the calculation
       *
       * see SingleContact for the implementation and DynamicSystem for explanation
       */
      virtual void calcgdSize(int j) { gdSize = 0; }

      /**
       * \brief calculates size of rfactors
       */
      virtual void calcrFactorSize(int j) { rFactorSize = 0; }

      /**
       * \brief calculates size of rfactors
       */
      virtual void calcbSize() { bSize = 0; }

      /**
       * \brief calculates size of stopvector (root function for event driven integration)
       */
      virtual void calcsvSize() { svSize = 0; }
      
      /**
       * \brief calculates size of vector LinkStatus
       */
      virtual void calcLinkStatusSize() { LinkStatusSize =0;}

      /**
       * \brief calculates size of vector LinkStatusReg
       */
      virtual void calcLinkStatusRegSize() { LinkStatusRegSize =0;}

      /**
       * \brief asks the link if it contains force laws that contribute to the lagrange multiplier and is therefore set valued
       *
       * \return set valued force laws used within the link?
       */
      virtual bool isSetValued() const { return false; }

      /*!
       * \brief asks the link if it contains single valued force laws that contribute to the right-hand side vector h
       *
       * \return single valued force laws used within link?
       */
      virtual bool isSingleValued() const { return false; }

      /**
       * \return are link equations active?
       */
      virtual bool isActive() const = 0;

      /**
       * \return has the relative distance vector changed?
       */
      virtual bool gActiveChanged() = 0;
      
      /**
       * \return has an impact occured?
       */
      virtual bool detectImpact() { return false; }

      /**
       * solve impact equations of motion with single step fixed point scheme on velocity level
       * \param time step-size
       */
      virtual void solveImpactsFixpointSingle() { throwError("(Link::solveImpactsFixpointSingle): Not implemented."); }

      /**
       * solve contact equations of motion with single step fixed point scheme
       */
      virtual void solveConstraintsFixpointSingle() { throwError("(Link::solveConstraintsFixpointSingle): Not implemented."); }

      /**
       * solve impact equations of motion with Gauss-Seidel scheme on velocity level
       * \param time step-size
       */
      virtual void solveImpactsGaussSeidel() { throwError("(Link::solveImpactsGaussSeidel): Not implemented."); }

      /**
       * solve contact equations of motion with Gauss-Seidel scheme
       */
      virtual void solveConstraintsGaussSeidel() { throwError("(Link::solveConstraintsGaussSeidel): Not implemented."); }

      /**
       * solve impact equations of motion with Newton scheme on velocity level
       * \param time step-size
       */
      virtual void solveImpactsRootFinding() { throwError("(Link::solveImpactsRootFinding): Not implemented."); }

      /**
       * solve contact equations of motion with Newton scheme
       */
      virtual void solveConstraintsRootFinding() { throwError("(Link::solveConstraintsRootFinding): Not implemented."); }

      /**
       * \brief computes JACOBIAN and mass action matrix of nonlinear contact equations
       */
      virtual void jacobianConstraints() { throwError("(Link::jacobianConstraints): Not implemented."); }

      /**
       * \brief computes JACOBIAN and mass action matrix of nonlinear contact equations on velocity level
       */
      virtual void jacobianImpacts() { throwError("(Link::jacobianImpacts): Not implemented."); }

      /**
       * \brief update relaxation factors for contact equations
       */
      virtual void updaterFactors() { throwError("(Link::updaterFactors): Not implemented."); }

      /**
       * \brief verify underlying force laws on velocity level concerning given tolerances
       */
      virtual void checkImpactsForTermination() { throwError("(Link::checkImpactsForTermination): Not implemented."); }
      
      /**
       * \brief verify underlying force laws concerning given tolerances
       */
      virtual void checkConstraintsForTermination() { throwError("(Link::checkConstraintsForTermination): Not implemented."); }

      /**
       * \brief check if set-valued contacts are active and set corresponding attributes
       * \param flag to decide which criteria are used to define 'activity'
       *
       * see SingleContact for the implementation and DynamicSystem for explanation
       */
      virtual void checkActive(int j) { }

      /**
       * \brief compute potential energy
       */
      virtual double evalPotentialEnergy() { return 0; }

      virtual void setGeneralizedForceTolerance(double tol) { laTol = tol; }
      virtual void setGeneralizedImpulseTolerance(double tol) { LaTol = tol; }
      virtual void setGeneralizedRelativePositionTolerance(double tol) { gTol = tol; }
      virtual void setGeneralizedRelativeVelocityTolerance(double tol) { gdTol = tol; }
      virtual void setGeneralizedRelativeAccelerationTolerance(double tol) { gddTol = tol; }
      virtual void setGeneralizedRelativePositionCorrectionValue(double corr) { gCorr = corr; }
      virtual void setGeneralizedRelativeVelocityCorrectionValue(double corr) { gdCorr = corr; }
      virtual void setrMax(double rMax_) { rMax = rMax_; }
      virtual void setLinkStatusInd(int LinkStatusInd_) { LinkStatusInd = LinkStatusInd_; };
      virtual void setLinkStatusRegInd(int LinkStatusRegInd_) { LinkStatusRegInd = LinkStatusRegInd_; };
      virtual void setlaInd(int laInd_) { laInd = laInd_;Ila=fmatvec::RangeV(laInd,laInd+laSize-1); }
      virtual void setgInd(int gInd_) { gInd = gInd_; Ig=fmatvec::RangeV(gInd,gInd+gSize-1); }
      virtual void setgdInd(int gdInd_) { gdInd = gdInd_; }
      virtual void setrFactorInd(int rFactorInd_) { rFactorInd = rFactorInd_; }
      /**
       * \brief get gap distance and calculate gap velocity of unilateral links to estimate impacts within the next step
       * \param gInActive gap distance of inactive links (return)
       * \param gdInActive gap velocities of inactive links (return)
       * \param IndInActive index for gInActive/gdInActive; incremented with size after storage (return and input)
       * \param gAct gap distance of active links (return)
       * \param IndActive index for gActive; incremented with size after storage (return and input)
      */
      virtual void LinearImpactEstimation(double t, fmatvec::Vec &gInActive_,fmatvec::Vec &gdInActive_,int *IndInActive_,fmatvec::Vec &gAct_,int *IndActive_) { }
      
      /**
       * \brief calculates the number of active and inactive unilateral constraints and increments sizeActive/sizeInActive
       */
      virtual void SizeLinearImpactEstimation(int *sizeInActive_, int *sizeActive_) { }
      virtual void updatecorr(int j) { corr.init(0); }
      virtual void updatecorrRef(fmatvec::Vec &ref);
      virtual void calccorrSize(int j) { corrSize = 0; }
      virtual void setcorrInd(int corrInd_) { corrInd = corrInd_; }
      virtual void checkRoot() { }
      /***************************************************/

      void setx(const fmatvec::Vec &x_) { x = x_; }
      
      virtual void setsvInd(int svInd_) { svInd = svInd_; };
      int getsvSize() const { return svSize; }

      int getLinkStatusSize() const { return LinkStatusSize; }

      int getLinkStatusRegSize() const { return LinkStatusRegSize; }

      const fmatvec::Vec& getla(bool check=true) const;
      const fmatvec::Vec& getLa(bool check=true) const;
      fmatvec::Vec& getla(bool check=true);
      fmatvec::Vec& getLa(bool check=true);
      fmatvec::Vec& getwb(bool check=true);

      int getlaInd() const { return laInd; } 
      int getlaSize() const { return laSize; } 
      int getbSize() const { return bSize; }

      const fmatvec::Vec& evalg();
      const fmatvec::Vec& evalgd();
      const fmatvec::Vec& evalla();
      const fmatvec::Vec& evalLa();
      const fmatvec::Vec& evalwb();
      const fmatvec::Vec& evalxd();

      int getgdInd() const { return gdInd; } 
      int getgSize() const { return gSize; } 
      int getgdSize() const { return gdSize; } 

      int getrFactorSize() const { return rFactorSize; } 
      
      const fmatvec::VecInt& getrFactorUnsure() const { return rFactorUnsure; }

      void resetUpToDate() override { updrrel = true; updvrel = true; updla = true; }

      virtual void updateGeneralizedPositions() { updrrel = false; }
      virtual void updateGeneralizedVelocities() { updvrel = false; }
      virtual void updateGeneralizedForces() { updla = false; }

      const fmatvec::VecV& evalGeneralizedRelativePosition() { if(updrrel) updateGeneralizedPositions(); return rrel; }
      const fmatvec::VecV& evalGeneralizedRelativeVelocity() { if(updvrel) updateGeneralizedVelocities(); return vrel; }
      const fmatvec::VecV& evalGeneralizedForce() { if(updla) updateGeneralizedForces(); return lambda; }

      fmatvec::VecV& getGeneralizedRelativePosition(bool check=true) {  assert((not check) or (not updrrel)); return rrel; }
      fmatvec::VecV& getGeneralizedRelativeVelocity(bool check=true) {  assert((not check) or (not updvrel)); return vrel; }
      fmatvec::VecV& getGeneralizedForce(bool check=true) {  assert((not check) or (not updla)); return lambda; }

      /**
       * \brief saves contact forces for use as starting value in next time step
       */
      void savela();

      /**
       * \brief saves contact impulses for use as starting value in next time step
       */
      void saveLa();

      /**
       * \brief load contact forces for use as starting value
       */
      void initla();

      /**
       * \brief load contact impulses for use as starting value
       */
      void initLa();

      /**
       * \brief decrease rfactor if mass action matrix is not diagonal dominant (cf. Foerg: Dissertation, page 80 et seq.) 
       */
      void decreaserFactors();

      
      int getcorrSize() const { return corrSize; } 

      bool getUpdaterrel() const { return updrrel; }

    protected:
      int ng, ngd, nla;

      /** 
       * \brief order one parameters
       */
      fmatvec::Vec x;

      /** 
       * \brief differentiated order one parameters 
       */
      fmatvec::Vec xd;

      fmatvec::Vec dx;

      /**
       * \brief order one initial value
       */
      fmatvec::Vec x0;

      /**
       * \brief size  and local index of order one parameters
       */
      int xSize, xInd;

      /**
       * \brief stop vector for event driven integration (root function)
       */
      fmatvec::Vec sv;

      /**
       * \brief evaluation of roots of stop vector with a boolean vector
       */
      fmatvec::VecInt jsv;

      /**
       * \brief size and local index of stop vector
       */
      int svSize, svInd;
             
      /**
       * for set valued links with piecewise link equation (e.g. unilateral contacts or coulomb friction)
       * \brief status of link (default 0) describing which piece of the equation is valid (e.g. stick or slip)
       */
      fmatvec::VecInt LinkStatus;

      /**
       * \brief size and local index of link status vector (set-valued)
       */
       int LinkStatusSize, LinkStatusInd;
    
       /**
       * for single valued links
       * \brief status of link
       */
      fmatvec::VecInt LinkStatusReg;

      /**
       * \brief size and local index of single-valued link status vector
       */
       int LinkStatusRegSize, LinkStatusRegInd; 

      /**
       * \brief relative position, relative velocity, contact force and impact parameters
       */
      fmatvec::Vec g, gd, la, La;
      
      /*!
       * \brief contact forces of smooth contact laws
       */
      fmatvec::Vec laS;

      /**
       * \brief size and local index of relative distances
       */
      int gSize, gInd;

      /**
       * \brief size and local index of relative velocities
       */
      int gdSize, gdInd;

      /**
       * \brief size and local index of contact force parameters
       */
      int laSize, laInd;

      /**
       * \brief size and local index of contact force parameters
       */
      int bSize, bInd;

      /**
       * \brief local index of relative distances and contact force parameters
       */
      fmatvec::RangeV Ig, Ila;
      
      /**
       * \brief tolerance for relative position, relative velocity, relative acceleration, force and impact
       */
      double gTol, gdTol, gddTol, laTol, LaTol;

      /**
       * \brief correction factor for relative position and relative velocity
       */
      double gCorr, gdCorr;
      
      /**
       * \brief attribute to save contact force parameter of previous time step
       */
      fmatvec::Vec la0, La0;

      /**
       * \brief vector of rfactors for relaxation of contact equations
       */
      fmatvec::Vec rFactor;

      /**
       * \brief boolean vector defining if rfactor belongs to not diagonal dominant mass action matrix (cf. Foerg Dissertation, page 80 et seq.)
       */
      fmatvec::VecInt rFactorUnsure;

      /**
       * \brief size and local index of rfactors
       */
      int rFactorSize, rFactorInd;
      
      /**
       * \brief maximum r-factor
       */
      double rMax;

      /**
       * residuum of nonlinear contact equations
       */
      fmatvec::Vec res;

      /** 
       * \brief force direction matrix for nonsmooth right hand side
       */
      std::vector<fmatvec::Mat> W[2];

      /**
       * \brief reduced force direction matrix for nonsmooth right hand side
       */
      std::vector<fmatvec::Mat> V[2];
      
      /**
       * \brief smooth complete and link right hand side
       */
      std::vector<fmatvec::Vec> h[2];
      
      /**
       * \brief smooth Jacobians for implicit integration
       */
      std::vector<fmatvec::Mat> dhdq;
      std::vector<fmatvec::Mat> dhdu;
      std::vector<fmatvec::Vec> dhdt;
      
      /**
       * \brief nonsmooth right hand side
       */
      std::vector<fmatvec::Vec> r[2];
      
      /**
       * \brief TODO
       */
      fmatvec::Vec wb;

      /**
       * \brief TODO
       */
      fmatvec::Mat b;

      int corrSize, corrInd;
      fmatvec::Vec corr;

      fmatvec::VecV rrel, vrel, lambda;

      bool updSize, updrrel, updvrel, updla;
  };
}

#endif /* _LINK_H_ */
