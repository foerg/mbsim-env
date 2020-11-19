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

#include <config.h> 
#include "contact.h"
#include <mbsim/contours/contour.h>
#include <mbsim/constitutive_laws/generalized_force_law.h>
#include <mbsim/constitutive_laws/friction_force_law.h>
#include <mbsim/constitutive_laws/generalized_impact_law.h>
#include <mbsim/constitutive_laws/friction_impact_law.h>
#include <mbsim/contact_kinematics/spatialcontour_spatialcontour.h>
#include <mbsim/objectfactory.h>

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, Contact)

  Contact::~Contact() {
    delete contactKinematics;
    delete fcl;
    delete fdf;
    delete fnil;
    delete ftil;
  }

  void Contact::updatewb() {
    for (size_t i=0; i<contacts.size(); i++) {
      if(contacts[i].gdActive[SingleContact::normal]) {
        contacts[i].updatewb();
        contactKinematics->updatewb(contacts[i],i);
      }
    }
  }

  void Contact::updateW(int j) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateW(j);
  }

  void Contact::updateV(int j) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateV(j);
  }

  void Contact::updateh(int j) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateh(j);
  }

  void Contact::updateg() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateg();
  }

  void Contact::updategd() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updategd();
  }

  void Contact::updateStopVector() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateStopVector();
  }

  void Contact::updateStopVectorParameters() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateStopVectorParameters();
  }

  void Contact::updateJacobians(int j) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateJacobians(j);
  }

  void Contact::updateWRef(Mat& WParent, int j) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateWRef(WParent, j);
  }

  void Contact::updateVRef(Mat& VParent, int j) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateVRef(VParent, j);
  }

  void Contact::updatehRef(Vec& hParent, int j) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updatehRef(hParent, j);
  }

  void Contact::updaterRef(Vec& rParent, int j) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updaterRef(rParent, j);
  }

  void Contact::updatewbRef(Vec& wbParent) {
    Link::updatewbRef(wbParent);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updatewbRef(wbParent);
  }

  void Contact::updatelaRef(Vec& laParent) {
    Link::updatelaRef(laParent);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updatelaRef(laParent);
  }

  void Contact::updateLaRef(Vec& LaParent) {
    Link::updateLaRef(LaParent);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateLaRef(LaParent);
  }

  void Contact::updateInternalStateRef(Vec& curisParent, Vec& nextisParent) {
    Link::updateInternalStateRef(curisParent, nextisParent);
    if(contactKinematics)
      contactKinematics->updateInternalStateRef(curis, nextis);
  }

  void Contact::updategRef(Vec& gParent) {
    Link::updategRef(gParent);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updategRef(gParent);
  }

  void Contact::updategdRef(Vec& gdParent) {
    Link::updategdRef(gdParent);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updategdRef(gdParent);
  }

  void Contact::updateresRef(Vec& resParent) {
    Link::updateresRef(resParent);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateresRef(resParent);
  }

  void Contact::updaterFactorRef(Vec& rFactorParent) {
    Link::updaterFactorRef(rFactorParent);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updaterFactorRef(rFactorParent);
  }

  void Contact::updatesvRef(Vec &svParent) {
    Link::updatesvRef(svParent);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updatesvRef(svParent);
  }

  void Contact::updatejsvRef(VecInt &jsvParent) {
    Link::updatejsvRef(jsvParent);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updatejsvRef(jsvParent);
  }

  void Contact::updateLinkStatusRef(VecInt &LinkStatusParent) {
    Link::updateLinkStatusRef(LinkStatusParent);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateLinkStatusRef(LinkStatusParent);
  }

  void Contact::updateLinkStatusRegRef(VecInt &LinkStatusRegParent) {
    Link::updateLinkStatusRegRef(LinkStatusRegParent);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateLinkStatusRegRef(LinkStatusRegParent);
  }

  void Contact::calclaSize(int j) {
    Link::calclaSize(j);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->calclaSize(j);
      laSize += iter->getlaSize();
    }
  }

  void Contact::calcisSize() {
    Link::calcisSize();
    if(contactKinematics)
    {
      contactKinematics->calcisSize();
      contactKinematics->setisInd(isSize);
      isSize += contactKinematics->getisSize();
    }
  }

  void Contact::calcgSize(int j) {
    Link::calcgSize(j);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->calcgSize(j);
      gSize += iter->getgSize();
    }
  }

  void Contact::calcgdSize(int j) {
    Link::calcgdSize(j);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->calcgdSize(j);
      gdSize += iter->getgdSize();
    }
  }

  void Contact::calcrFactorSize(int j) {
    Link::calcrFactorSize(j);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->calcrFactorSize(j);
      rFactorSize += iter->getrFactorSize();
    }
  }

  void Contact::calcsvSize() {
    Link::calcsvSize();
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->calcsvSize();
      svSize += iter->getsvSize();
    }
  }

  void Contact::calcLinkStatusSize() {
    Link::calcLinkStatusSize();
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->calcLinkStatusSize();
      LinkStatusSize += iter->getLinkStatusSize();
    }
    LinkStatus.resize(LinkStatusSize);
  }

  void Contact::calcLinkStatusRegSize() {
    Link::calcLinkStatusRegSize();
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->calcLinkStatusRegSize();
      LinkStatusRegSize += iter->getLinkStatusRegSize();
    }
    LinkStatusReg.resize(LinkStatusRegSize);
  }

  void Contact::init(InitStage stage, const InitConfigSet &config) {
    if (stage ==resolveStringRef) {
      if(not saved_ref1.empty() and not saved_ref2.empty())
        connect(getByPath<Contour>(saved_ref1), getByPath<Contour>(saved_ref2));

      if(not contour[0] or not contour[1])
        throwError("Not all connections are given!");

      Link::init(stage, config);
    }
    else if (stage == preInit) {
      Link::init(stage, config);
      if(fcl->isSetValued() and not fnil)
        throwError("Normal impact law must be defined!");
      if(fdf and fdf->isSetValued() and not ftil)
        throwError("Tangential impact law must be defined!");
      if (contactKinematics == 0) {
        contactKinematics = contour[0]->findContactPairingWith(typeid(*contour[0]), typeid(*contour[1]));
        if (contactKinematics == 0) {
          contactKinematics = contour[1]->findContactPairingWith(typeid(*contour[1]), typeid(*contour[0]));
          if (contactKinematics == 0) {
            contactKinematics = contour[0]->findContactPairingWith(typeid(*contour[1]), typeid(*contour[0]));
            if (contactKinematics == 0) {
              contactKinematics = contour[1]->findContactPairingWith(typeid(*contour[0]), typeid(*contour[1]));
              if (contactKinematics == 0) {
		contactKinematics = new ContactKinematicsSpatialContourSpatialContour;
              }
            }
          }
        }
      }
      contactKinematics->setSearchAllContactPoints(searchAllCP);
      if(maxNumContacts>-1) contactKinematics->setMaximumNumberOfContacts(maxNumContacts);
      contactKinematics->assignContours(contour[0], contour[1]);
      contactKinematics->setTolerance(tol);
      for (int k = 0; k < contactKinematics->getMaximumNumberOfContacts(); ++k) {
        stringstream contactName;
        contactName << name << "_" <<0;
        if (contactKinematics->getMaximumNumberOfContacts() > 1) contactName << "_" << k;
        contacts.push_back(SingleContact(contactName.str()));
        contacts[k].connect(contour[0], contour[1]);
        contacts[k].plotFeature = plotFeature;
        contacts[k].plotFeatureForChildren = plotFeatureForChildren;

        //set the tolerances for the single contacts
        contacts[k].setGeneralizedRelativePositionTolerance(gTol);
        contacts[k].setGeneralizedRelativeVelocityTolerance(gdTol);
        contacts[k].setGeneralizedRelativeAccelerationTolerance(gddTol);
        contacts[k].setGeneralizedForceTolerance(laTol);
        contacts[k].setGeneralizedImpulseTolerance(LaTol);
        contacts[k].setGeneralizedRelativePositionCorrectionValue(gCorr);
        contacts[k].setGeneralizedRelativeVelocityCorrectionValue(gdCorr);
        contacts[k].setrMax(rMax);
      }

      for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
        //set parent
        iter->setParent(this);

        //set contact laws for children
        iter->setNormalForceLaw(fcl);
        iter->setNormalImpactLaw(fnil);
        iter->setTangentialForceLaw(fdf);
        iter->setTangentialImpactLaw(ftil);

        iter->init(stage, config);
      }
    }
    else if (stage == plotting) {
      Element::init(stage, config);
      for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
        iter->init(stage, config);
    }
    else {
      Link::init(stage, config);
      for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
        iter->init(stage, config);
      contactKinematics->setInitialGuess(zeta0);
    }
    //Don't call init()-routines for "sub"-contacts with stage "LASTINITSTAGE" as here is checked if contactKinematics has more than one possible contact point, which is only possible in multi-contact
    fcl->init(stage, config);
    if(fdf) fdf->init(stage, config);
    if(fnil) fnil->init(stage, config);
    if(ftil) ftil->init(stage, config);
  }

  bool Contact::isSetValued() const {
    return ((fcl and fcl->isSetValued()) or (fdf and fdf->isSetValued()));
  }

  bool Contact::isSingleValued() const {
    return not(fcl and fcl->isSetValued());
  }

  void Contact::updateLinkStatus() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateLinkStatus();
  }

  void Contact::updateLinkStatusReg() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updateLinkStatusReg();
  }

  bool Contact::isActive() const {
    for (vector<SingleContact>::const_iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      if (iter->isActive())
        return true;
    }
    return false;
  }

  bool Contact::gActiveChanged() {
    bool changed = false;
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      if (iter->gActiveChanged())
        changed = true;
    }
    return changed;
  }

  bool Contact::detectImpact() {
    bool impact = false;
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      if (iter->detectImpact())
        impact = true;
    }
    return impact;
  }

  void Contact::plot() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->plot();
  }

  void Contact::setDynamicSystemSolver(DynamicSystemSolver* sys) {
    Link::setDynamicSystemSolver(sys);

    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->setDynamicSystemSolver(ds);
  }

  void Contact::setNormalForceLaw(GeneralizedForceLaw *fcl_) {
    fcl = fcl_;
    fcl->setParent(this);
  }

  void Contact::setNormalImpactLaw(GeneralizedImpactLaw *fnil_) {
    fnil = fnil_;
    fnil->setParent(this);
  }

  void Contact::setTangentialForceLaw(FrictionForceLaw *fdf_) { 
    fdf = fdf_; 
    fdf->setParent(this);
  }

  void Contact::setTangentialImpactLaw(FrictionImpactLaw *ftil_) {
    ftil = ftil_;
    ftil->setParent(this);
  }

  void Contact::setgInd(int gInd_) {
    Link::setgInd(gInd_);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->setgInd(gInd_);
      gInd_ += iter->getgSize();
    }
  }

  void Contact::setgdInd(int gdInd_) {
    Link::setgdInd(gdInd_);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->setgdInd(gdInd_);
      gdInd_ += iter->getgdSize();
    }
  }

  void Contact::setsvInd(int svInd_) {
    Link::setsvInd(svInd_);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->setsvInd(svInd_);
      svInd_ += iter->getsvSize();
    }
  }

  void Contact::setlaInd(int laInd_) {
    Link::setlaInd(laInd_);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->setlaInd(laInd_);
      laInd_ += iter->getlaSize();
    }
  }

  void Contact::setrFactorInd(int rFactorInd_) {
    Link::setrFactorInd(rFactorInd_);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->setrFactorInd(rFactorInd_);
      rFactorInd_ += iter->getrFactorSize();
    }
  }

  void Contact::setcorrInd(int corrInd_) {
    Link::setcorrInd(corrInd_);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->setcorrInd(corrInd_);
      corrInd_ += iter->getcorrSize();
    }
  }

  void Contact::setLinkStatusInd(int LinkStatusInd_) {
    Link::setLinkStatusInd(LinkStatusInd_);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->setLinkStatusInd(LinkStatusInd_);
      LinkStatusInd_ += iter->getLinkStatusSize();
    }
  }

  void Contact::setLinkStatusRegInd(int LinkStatusRegInd_) {
    Link::setLinkStatusRegInd(LinkStatusRegInd_);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->setLinkStatusRegInd(LinkStatusRegInd_);
      LinkStatusRegInd_ += iter->getLinkStatusRegSize();
    }
  }

  void Contact::solveImpactsFixpointSingle() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->solveImpactsFixpointSingle();
  }

  void Contact::solveConstraintsFixpointSingle() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->solveConstraintsFixpointSingle();
  }

  void Contact::solveImpactsGaussSeidel() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->solveImpactsGaussSeidel();
  }

  void Contact::solveConstraintsGaussSeidel() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->solveConstraintsGaussSeidel();
  }

  void Contact::solveImpactsRootFinding() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->solveImpactsRootFinding();
  }

  void Contact::solveConstraintsRootFinding() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->solveConstraintsRootFinding();
  }

  void Contact::jacobianConstraints() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->jacobianConstraints();
  }

  void Contact::jacobianImpacts() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->jacobianImpacts();
  }

  void Contact::updaterFactors() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updaterFactors();
  }

  void Contact::checkConstraintsForTermination() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->checkConstraintsForTermination();
  }

  void Contact::checkImpactsForTermination() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->checkImpactsForTermination();
  }

  void Contact::checkActive(int j) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->checkActive(j);
  }

  int Contact::getFrictionDirections() {
    if (fdf)
      return fdf->getFrictionDirections();
    else
      return 0;
  }

  void Contact::connect(Contour *contour0, Contour* contour1) {
    contour[0] = contour0;
    contour[1] = contour1;
  }

  void Contact::LinearImpactEstimation(double t, Vec &gInActive_, Vec &gdInActive_, int *IndInActive_, Vec &gAct_, int *IndActive_) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->LinearImpactEstimation(t, gInActive_, gdInActive_, IndInActive_, gAct_, IndActive_);
  }

  void Contact::SizeLinearImpactEstimation(int *sizeInActive_, int *sizeActive_) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->SizeLinearImpactEstimation(sizeInActive_, sizeActive_);
  }

  void Contact::setGeneralizedForceTolerance(double tol) {
    Link::setGeneralizedForceTolerance(tol);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->setGeneralizedForceTolerance(tol);
  }

  void Contact::setGeneralizedImpulseTolerance(double tol) {
    Link::setGeneralizedImpulseTolerance(tol);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->setGeneralizedImpulseTolerance(tol);
  }

  void Contact::setGeneralizedRelativePositionTolerance(double tol) {
    Link::setGeneralizedRelativePositionTolerance(tol);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->setGeneralizedRelativePositionTolerance(tol);
  }

  void Contact::setGeneralizedRelativeVelocityTolerance(double tol) {
    Link::setGeneralizedRelativeVelocityTolerance(tol);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->setGeneralizedRelativeVelocityTolerance(tol);
  }

  void Contact::setGeneralizedRelativeAccelerationTolerance(double tol) {
    Link::setGeneralizedRelativeAccelerationTolerance(tol);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->setGeneralizedRelativeAccelerationTolerance(tol);
  }

  void Contact::setGeneralizedRelativePositionCorrectionValue(double tol) {
    Link::setGeneralizedRelativePositionCorrectionValue(tol);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->setGeneralizedRelativePositionCorrectionValue(tol);
  }

  void Contact::setGeneralizedRelativeVelocityCorrectionValue(double tol) {
    Link::setGeneralizedRelativeVelocityCorrectionValue(tol);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->setGeneralizedRelativeVelocityCorrectionValue(tol);
  }

  void Contact::setrMax(double rMax_) {
    Link::setrMax(rMax_);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->setrMax(rMax_);
  }

  void Contact::initializeUsingXML(DOMElement *element) {
    Link::initializeUsingXML(element);
    DOMElement *e;

    //Save contour names for initialization
    e = E(element)->getFirstElementChildNamed(MBSIM%"connect");
    saved_ref1 = E(e)->getAttribute("ref1");
    saved_ref2 = E(e)->getAttribute("ref2");

    //Set contact law
    e = E(element)->getFirstElementChildNamed(MBSIM%"normalForceLaw");
    GeneralizedForceLaw *gfl = ObjectFactory::createAndInit<GeneralizedForceLaw>(e->getFirstElementChild());
    setNormalForceLaw(gfl);

    //Get Impact law
    e = E(element)->getFirstElementChildNamed(MBSIM%"normalImpactLaw");
    if (e) {
      GeneralizedImpactLaw *gifl = ObjectFactory::createAndInit<GeneralizedImpactLaw>(e->getFirstElementChild());
      setNormalImpactLaw(gifl);
    }

    //Get Friction Force Law
    e = E(element)->getFirstElementChildNamed(MBSIM%"tangentialForceLaw");
    if (e) {
      FrictionForceLaw *ffl = ObjectFactory::createAndInit<FrictionForceLaw>(e->getFirstElementChild());
      setTangentialForceLaw(ffl);
    }

    //Get Friction Impact Law
    e = E(element)->getFirstElementChildNamed(MBSIM%"tangentialImpactLaw");
    if (e) {
      FrictionImpactLaw *fil = ObjectFactory::createAndInit<FrictionImpactLaw>(e->getFirstElementChild());
      setTangentialImpactLaw(fil);
    }

    e = E(element)->getFirstElementChildNamed(MBSIM%"searchAllContactPoints");
    if (e) setSearchAllContactPoints(E(e)->getText<bool>());

    e = E(element)->getFirstElementChildNamed(MBSIM%"initialGuess");
    if (e) setInitialGuess(E(e)->getText<Vec>());

    e = E(element)->getFirstElementChildNamed(MBSIM%"tolerance");
    if (e) setTolerance(E(e)->getText<double>());

    e = E(element)->getFirstElementChildNamed(MBSIM%"maximumNumberOfContacts");
    if (e) setMaximumNumberOfContacts(E(e)->getText<int>());
  }

  void Contact::updatecorrRef(Vec& corrParent) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updatecorrRef(corrParent);
  }

  void Contact::updatecorr(int j) {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->updatecorr(j);
  }

  void Contact::calccorrSize(int j) {
    Link::calccorrSize(j);
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter) {
      iter->calccorrSize(j);
      corrSize += iter->getcorrSize();
    }
  }

  void Contact::checkRoot() {
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->checkRoot();
  }

  void Contact::resetUpToDate() {
    updrrel = true;
    for (vector<SingleContact>::iterator iter = contacts.begin(); iter != contacts.end(); ++iter)
      iter->resetUpToDate();
  }

  void Contact::updateGeneralizedPositions() {
    contactKinematics->updateg(contacts);
    updrrel = false;
  }

}
