/* Copyright (C) 2004-2014 MBSim Development Team
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
 *
 * Contact: martin.o.foerg@googlemail.com
 */

#ifndef _CONTACT_H_
#define _CONTACT_H_

#include <mbsim/links/single_contact.h>
#include <map>

namespace MBSim {

  class ContactKinematics;
  class GeneralizedForceLaw;
  class GeneralizedImpactLaw;
  class FrictionForceLaw;
  class FrictionImpactLaw;

  /*! \brief class for contacts
   * \author Martin Foerg
   * \date 2009-04-02 some comments (Thorsten Schindler)
   * \date 2009-07-16 splitted link / object right hand side (Thorsten Schindler)
   * \date 2009-08-03 contacts can now visualize their ContactPointFrames (Markus Schneider)
   * \date 2010-07-06 added LinkStatus and LinearImpactEstimation for timestepper ssc (Robert Huber)
   * \date 2012-05-08 added LinkStatusReg for AutoTimeSteppingSSCIntegrator (Jan Clauberg)
   * \date 2014-09-16 contact forces are calculated on acceleration level (Thorsten Schindler)
   *
   * basic class for contacts between contours, mainly implementing geometrical informations of contact-pairings
   * 
   * Remarks:
   * - constitutive laws on acceleration and velocity level have to be set pairwise
   */
  class Contact : public Link {
    public:
      /*!
       * \brief constructor
       * \param name of contact
       */
      Contact(const std::string &name = "") : Link(name) { }

      /**
       * \brief destructor
       */
      ~Contact() override;

      /* INHERITED INTERFACE OF LINKINTERFACE */
      void updatewb() override;
      void updateW(int i = 0) override;
      void updateV(int i = 0) override;
      void updateh(int i = 0) override;
      void updateg() override;
      void updategd() override;
      void updateStopVector() override;
      void updateStopVectorParameters() override;
      void updateJacobians(int j = 0) override;
      /***************************************************/

      /* INHERITED INTERFACE OF LINK */
      void updateWRef(fmatvec::Mat &ref, int j = 0) override;
      void updateVRef(fmatvec::Mat &ref, int j = 0) override;
      void updatehRef(fmatvec::Vec &hRef, int j = 0) override;
      void updaterRef(fmatvec::Vec &hRef, int j = 0) override;
      void updatewbRef(fmatvec::Vec &ref) override;
      void updatelaRef(fmatvec::Vec& ref) override;
      void updateLaRef(fmatvec::Vec& ref) override;
      void updategRef(fmatvec::Vec& ref) override;
      void updategdRef(fmatvec::Vec& ref) override;
      void updateresRef(fmatvec::Vec& ref) override;
      void updaterFactorRef(fmatvec::Vec &ref) override;
      void updatesvRef(fmatvec::Vec &ref) override;
      void updatejsvRef(fmatvec::VecInt &ref) override;
      void updateLinkStatusRef(fmatvec::VecInt &LinkStatusParent) override;
      void updateLinkStatusRegRef(fmatvec::VecInt &LinkStatusRegParent) override;
      void calclaSize(int j) override;
      void calcgSize(int j) override;
      void calcgdSize(int j) override;
      void calcrFactorSize(int j) override;
      void calcsvSize() override;
      void calcLinkStatusSize() override;
      void calcLinkStatusRegSize() override;
      void init(InitStage stage, const InitConfigSet &config) override;
      bool isSetValued() const override;
      bool isSingleValued() const override;
      void updateLinkStatus() override;
      void updateLinkStatusReg() override;
      bool isActive() const override;
      bool gActiveChanged() override;
      bool detectImpact() override;
      void solveImpactsFixpointSingle() override;
      void solveConstraintsFixpointSingle() override;
      void solveImpactsGaussSeidel() override;
      void solveConstraintsGaussSeidel() override;
      void solveImpactsRootFinding() override;
      void solveConstraintsRootFinding() override;
      void jacobianConstraints() override;
      void jacobianImpacts() override;
      void updaterFactors() override;
      void checkConstraintsForTermination() override;
      void checkImpactsForTermination() override;
      void checkActive(int j) override;
      void setGeneralizedForceTolerance(double tol) override;
      void setGeneralizedImpulseTolerance(double tol) override;
      void setGeneralizedRelativePositionTolerance(double tol) override;
      void setGeneralizedRelativeVelocityTolerance(double tol) override;
      void setGeneralizedRelativeAccelerationTolerance(double tol) override;
      void setGeneralizedRelativePositionCorrectionValue(double corr) override;
      void setGeneralizedRelativeVelocityCorrectionValue(double corr) override;
      void setrMax(double rMax_) override;
      void setLinkStatusInd(int LinkStatusInd_) override;
      void setLinkStatusRegInd(int LinkStatusRegInd_) override;
      void setsvInd(int svInd_) override;
      void setlaInd(int laInd_) override;
      void setgInd(int gInd_) override;
      void setgdInd(int gdInd_) override;
      void setrFactorInd(int rFactorInd_) override;
      void LinearImpactEstimation(double t, fmatvec::Vec &gInActive_, fmatvec::Vec &gdInActive_, int *IndInActive_, fmatvec::Vec &gAct_, int *IndActive_) override;
      void SizeLinearImpactEstimation(int *sizeInActive_, int *sizeActive_) override;
      void updatecorrRef(fmatvec::Vec& ref) override;
      void updatecorr(int j) override;
      void calccorrSize(int j) override;
      void setcorrInd(int corrInd_) override;
      void checkRoot() override;
      /***************************************************/

      /* INHERITED INTERFACE OF ELEMENT */
      void plot() override;
      void setDynamicSystemSolver(DynamicSystemSolver *sys) override;
      /***************************************************/

      void resetUpToDate() override;

      /* GETTER / SETTER */

      void setNormalForceLaw(GeneralizedForceLaw *fcl_);
      GeneralizedForceLaw * getNormalForceLaw() const { return fcl; }
      void setNormalImpactLaw(GeneralizedImpactLaw *fnil_);
      void setTangentialForceLaw(FrictionForceLaw *fdf_); 
      void setTangentialImpactLaw(FrictionImpactLaw *ftil_);
      void setContactKinematics(ContactKinematics* ck) { contactKinematics = ck; }
      ContactKinematics* getContactKinematics() const { return contactKinematics; }

      const std::vector<SingleContact>& getSubcontacts() const { return contacts; }
      /***************************************************/

      /**
       * \return number of considered friction directions
       */
      virtual int getFrictionDirections();

      /*! connect two contours
       * \param contour0          first contour
       * \param contour1          second contour
       * \param contactKinematics The contact kinematics that should be used to compute the contact point.
       * \param name              Name of the contact in the output
       *
       */
      void connect(Contour *contour1, Contour* contour2);

      Contour* getContour(int i) { return contour[i]; }

      SingleContact& getSingleContact(int i) { return contacts[i]; }

      void initializeUsingXML(xercesc::DOMElement *element) override;

      void setSearchAllContactPoints(bool searchAllCP_) { searchAllCP = searchAllCP_; }
      void setInitialGuess(const fmatvec::MatV &zeta0_) { zeta0 <<= zeta0_; }

      /**
       * \brief set tolerance for root-finding
       */
      void setTolerance(double tol_) { tol = tol_; }

      /**
       * \brief set maximum number of contacts
       */
      void setMaximumNumberOfContacts(int maxNumContacts_) { maxNumContacts = maxNumContacts_; }

      void updateGeneralizedPositions() override;

    protected:
      /**
       * \brief list of the single sub-contact(-points)
       */
      std::vector<SingleContact> contacts;

      /**
       * \brief contact kinematics
       */
      ContactKinematics* contactKinematics{nullptr};

      std::vector<Contour*> contour{2};

      /*!
       * \brief plotFeatures of sub-contacts
       *
       * \todo: see remark of ckNames
       */
      std::map<std::pair<std::string, std::size_t>, bool> plotFeatureMap;

      /**
       * \brief force laws in normal and tangential direction on acceleration and velocity level
       */
      GeneralizedForceLaw *fcl{nullptr};

      /**
       * \brief force law defining relation between tangential velocities and tangential forces
       */
      FrictionForceLaw *fdf{nullptr};

      /**
       * \brief force law defining relation between penetration velocity and resulting normal impulses
       */
      GeneralizedImpactLaw *fnil{nullptr};

      /** 
       * \brief force law defining relation between tangential velocities and forces impulses
       */
      FrictionImpactLaw *ftil{nullptr};

      bool searchAllCP{false};

      fmatvec::MatV zeta0;

      /**
       * \brief tolerance for root-finding
       */
      double tol{1e-10};

      /**
       * \brief maximum number of contacts
       */
      int maxNumContacts{-1};

    private:
      std::string saved_ref1, saved_ref2;
  };

}

#endif /* _CONTACT_H_ */
