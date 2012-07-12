/* Copyright (C) 2004-2009 MBSim Development Team
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
#include <mbsim/contact.h>
#include <mbsim/contour.h>
#include <mbsim/contour_pdata.h>
#include <mbsim/dynamic_system_solver.h>
#include <mbsim/constitutive_laws.h>
#include <mbsim/contact_kinematics/contact_kinematics.h>
#include <mbsim/utils/contact_utils.h>
#include <mbsim/utils/function.h>
#include <mbsim/utils/utils.h>
#include <mbsim/objectfactory.h>
#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/group.h>
#include <openmbvcppinterface/frame.h>
#include <openmbvcppinterface/arrow.h>
#include <openmbvcppinterface/objectfactory.h>
#include <mbsim/utils/eps.h>
#include <mbsim/utils/rotarymatrices.h>
#endif

using namespace std;
using namespace fmatvec;

namespace MBSim {
  extern double tP;
  extern bool gflag;

  Contact::Contact(const string &name) : LinkMechanics(name), contactKinematics(0), fcl(0), fdf(0), fnil(0), ftil(0), cpData(0), gActive(0), gActive0(0), gdActive(0), gddActive(0), gk(0), gdk(0), gdnk(0), gddk(0), lak(0), svk(0), rFactork(0), jsvk(0), fF(0), WF(0), laSizek(0), laIndk(0), gSizek(0), gIndk(0), gdSizek(0), gdIndk(0), svSizek(0), svIndk(0), rFactorSizek(0), rFactorIndk(0)
#ifdef HAVE_OPENMBVCPPINTERFACE
                                         , openMBVContactGrp(0), openMBVContactFrame(0), openMBVNormalForceArrow(0), openMBVFrictionArrow(0), openMBVContactFrameSize(0), openMBVContactFrameEnabled(true), contactArrow(NULL), frictionArrow(NULL)
#endif
                        , corrSizek(0), corrIndk(0)                   , saved_ref1(""), saved_ref2("")
					   {
					   }

  Contact::~Contact() {
    if(contactKinematics) { delete contactKinematics; contactKinematics=0; }
    /* Delete will fail if the same object is used for more than one Contact.
     * TODO: A delete concept (who deletes what) is still missing in MBSim.
    if(fcl) { delete fcl; fcl=0; }
    if(fdf) { delete fdf; fdf=0; }
    if(fnil) { delete fnil; fnil=0; }
    if(ftil) { delete ftil; ftil=0; }*/

    for(vector<ContourPointData*>::iterator i = cpData.begin(); i != cpData.end(); ++i)
      delete[] *i;
    for(vector<Mat*>::iterator i = Wk[0].begin(); i != Wk[0].end(); ++i)
      delete[] *i;
    for(vector<Mat*>::iterator i = Vk[0].begin(); i != Vk[0].end(); ++i)
      delete[] *i;
    for(vector<Mat3V*>::iterator i = fF.begin(); i != fF.end(); ++i)
      delete[] *i;
    for(vector<Vec3*>::iterator i = WF.begin(); i != WF.end(); ++i)
      delete[] *i;
    for(vector<unsigned int*>::iterator i = gdActive.begin(); i != gdActive.end(); ++i)
      delete[] *i;
    for(vector<unsigned int*>::iterator i = gddActive.begin(); i != gddActive.end(); ++i)
      delete[] *i;
  }

  void Contact::updatewb(double t, int j) {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gdActive[k]) {
        for(unsigned i=0; i<contour.size(); i++) 
          wbk[k] += fF[k][i](Range<Fixed<0>,Fixed<2> >(),Range<Var,Var>(0,laSizek[k]-1)).T()*cpData[k][i].getFrameOfReference().getGyroscopicAccelerationOfTranslation(j);
      }
    }
    contactKinematics->updatewb(wbk.begin(),gk.begin(),cpData.begin());
  }

  void Contact::updateW(double t, int j) {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gActive[k]) {
        fF[k][1].set(0, cpData[k][0].getFrameOfReference().getOrientation().col(0));
        if(getFrictionDirections()) {
          fF[k][1].set(1, cpData[k][0].getFrameOfReference().getOrientation().col(1));
          if(getFrictionDirections() > 1)
            fF[k][1].set(2, cpData[k][0].getFrameOfReference().getOrientation().col(2));
        }

        fF[k][0] = -fF[k][1];

        for(unsigned int i=0; i<contour.size(); i++) 
          Wk[j][k][i] += cpData[k][i].getFrameOfReference().getJacobianOfTranslation(j).T()*fF[k][i](Range<Fixed<0>,Fixed<2> >(),Range<Var,Var>(0,laSizek[k]-1));
      }
    }
  }

  void Contact::updateV(double t, int j) {
    if(getFrictionDirections()) {
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        if(gdActive[k][0] && !gdActive[k][1]) {
          for(unsigned int i=0; i<contour.size(); i++) 
            Vk[j][k][i] += cpData[k][i].getFrameOfReference().getJacobianOfTranslation(j).T()*fF[k][i](Range<Fixed<0>,Fixed<2> >(),iT)*fdf->dlaTdlaN(gdk[k](1,getFrictionDirections()), lak[k](0));
        }
      }
    }
  }

  void Contact::updateh(double t, int j) {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) { // gActive should not be checked, e.g. because of possible predamping in constitutive laws
      lak[k](0) = (*fcl)(gk[k](0),gdk[k](0), this);
      if(fdf) lak[k](1,getFrictionDirections()) = (*fdf)(gdk[k](1,getFrictionDirections()),fabs(lak[k](0)));

      WF[k][1] =  cpData[k][0].getFrameOfReference().getOrientation().col(0)*lak[k](0);
      if(getFrictionDirections()) {
        WF[k][1] += cpData[k][0].getFrameOfReference().getOrientation().col(1)*lak[k](1);
        if(getFrictionDirections() > 1)
          WF[k][1] += cpData[k][0].getFrameOfReference().getOrientation().col(2)*lak[k](2);
      }
      WF[k][0] = -WF[k][1];
      for(unsigned int i=0; i<contour.size(); i++) {
        h[j][i] += cpData[k][i].getFrameOfReference().getJacobianOfTranslation(j).T()*WF[k][i];
      }
    }
  }

  void Contact::updateg(double t) {
    contactKinematics->updateg(gk.begin(),cpData.begin());
  }

  void Contact::updategd(double t) {
    const bool flag = fcl->isSetValued();
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if((flag && gdActive[k][0]) || (!flag && fcl->isActive(gk[k](0),0))) { // TODO: nicer implementation
        for(unsigned int i=0; i<2; i++) contour[i]->updateKinematicsForFrame(cpData[k][i],velocities); // angular velocity necessary e.g. see ContactKinematicsSpherePlane::updatewb

        Vec3 Wn = cpData[k][0].getFrameOfReference().getOrientation().col(0);

        Vec3 WvD = cpData[k][1].getFrameOfReference().getVelocity() - cpData[k][0].getFrameOfReference().getVelocity();

        gdk[k](0) = Wn.T()*WvD;

        if(gdk[k].size()>1) {
          Mat3V Wt(gdk[k].size()-1);
          Wt.set(0, cpData[k][0].getFrameOfReference().getOrientation().col(1));
          if(gdk[k].size() > 2)
            Wt.set(1, cpData[k][0].getFrameOfReference().getOrientation().col(2));

          gdk[k](1,gdk[k].size()-1) = Wt.T()*WvD;
        }
      }
    }
  }

  void Contact::updateStopVector(double t) {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gActive[k]!=gdActive[k][0])
	throw;
      if(gActive[k]) { 
        svk[k](0) = gddk[k](0)>gddTol ? -1 : 1;
        if(gdActive[k][1]) {
          if(getFrictionDirections()) {
            svk[k](1) = nrm2(gddk[k](1,getFrictionDirections()))>gddTol ? -1 : 1;
            if((int)svk[k](1) == -1)
              gddkBuf[k] = gddk[k];
          }
        } 
        else {
          if(getFrictionDirections() == 1)
            svk[k](1) = gdk[k](1) > 0 ? 1 : -1;
          else if(getFrictionDirections() == 2) {
	    svk[k](1) = gdk[k](1)+gdk[k](2); // TODO: is there a better concept?
	  }
        }
      }
      else {
        svk[k](0) = gk[k](0) > 0 ? 1 : -1;
        if(getFrictionDirections())
          svk[k](1) = 1;
      }
    }
  }

  void Contact::updateJacobians(double t, int j) {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) 
      if(gActive[k])
        for(unsigned int i=0; i<2; i++) contour[i]->updateJacobiansForFrame(cpData[k][i],j);
  }

  void Contact::updateWRef(const Mat& WParent, int j) {
    for(unsigned i=0; i<contour.size(); i++) {
      int hInd =  contour[i]->gethInd(j);
      Index I = Index(hInd,hInd+contour[i]->gethSize(j)-1);
      Index J = Index(laInd,laInd+laSize-1);
      W[j][i]>>WParent(I,J);
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        Index Ik = Index(0,W[j][i].rows()-1);
        Index Jk = Index(laIndk[k],laIndk[k]+laSizek[k]-1);
        Wk[j][k][i]>>W[j][i](Ik,Jk);
      }
    }
  } 

  void Contact::updateVRef(const Mat& VParent, int j) {
    for(unsigned i=0; i<contour.size(); i++) {
      int hInd =  contour[i]->gethInd(j);
      Index J = Index(laInd,laInd+laSize-1);
      Index I = Index(hInd,hInd+contour[i]->gethSize(j)-1);
      V[j][i]>>VParent(I,J);
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        Index Ik = Index(0,V[j][i].rows()-1);
        Index Jk = Index(laIndk[k],laIndk[k]+laSizek[k]-1);
        Vk[j][k][i]>>V[j][i](Ik,Jk);
      }
    } 
  }

  void Contact::updatehRef(const Vec& hParent, int j) {
    for(unsigned i=0; i<contour.size(); i++) {
      int hInd =  contour[i]->gethInd(j);
      Index I = Index(hInd,hInd+contour[i]->gethSize(j)-1);
      h[j][i]>>hParent(I);
    }
  } 

  void Contact::updatewbRef(const Vec& wbParent) {
    LinkMechanics::updatewbRef(wbParent);
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) 
      wbk[k] >> wb(laIndk[k],laIndk[k]+laSizek[k]-1);
  }

  void Contact::updatelaRef(const Vec& laParent) {
    LinkMechanics::updatelaRef(laParent);
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) 
      lak[k] >> la(laIndk[k],laIndk[k]+laSizek[k]-1);
  }

  void Contact::updategRef(const Vec& gParent) {
    LinkMechanics::updategRef(gParent);
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) 
      gk[k] >> g(gIndk[k],gIndk[k]+gSizek[k]-1);
  }

  void Contact::updategdRef(const Vec& gdParent) {
    LinkMechanics::updategdRef(gdParent);
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) 
      gdk[k] >> gd(gdIndk[k],gdIndk[k]+gdSizek[k]-1);
  }

  void Contact::updaterFactorRef(const Vec& rFactorParent) {
    LinkMechanics::updaterFactorRef(rFactorParent);
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) 
      rFactork[k] >> rFactor(rFactorIndk[k],rFactorIndk[k]+rFactorSizek[k]-1);
  }

  void Contact::updatesvRef(const Vec &svParent) {
    LinkMechanics::updatesvRef(svParent);
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) 
      svk[k] >> sv(svIndk[k],svIndk[k]+svSizek[k]-1);
  }

  void Contact::updatejsvRef(const VecInt &jsvParent) {
    LinkMechanics::updatejsvRef(jsvParent);
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) 
      jsvk[k] >> jsv(svIndk[k],svIndk[k]+svSizek[k]-1);
  }

  void Contact::calcxSize() {
    LinkMechanics::calcxSize();
    xSize = 0;
  }

  void Contact::calclaSize(int j) {
    LinkMechanics::calclaSize(j);
    if(j==0) { // IA
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        laIndk[i] = laSize;
        laSizek[i] = 1+getFrictionDirections();
        laSize += laSizek[i];
      }
    } 
    else if(j==1) { // IG
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        laIndk[i] = laSize;
        laSizek[i] = gActive[i]*(1+getFrictionDirections());
        laSize += laSizek[i];
      }
    } 
    else if(j==2) { // IB
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        laIndk[i] = laSize;
        laSizek[i] = gActive[i]*gdActive[i][0]*(1+getFrictionDirections());
        laSize += laSizek[i];
      }
    } 
    else if(j==3) { // IH
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        laIndk[i] = laSize;
        laSizek[i] = gActive[i]*gdActive[i][0]*(1+gdActive[i][1]*getFrictionDirections());
        laSize += laSizek[i];
      }
    }
    else if(j==4) { // IG
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        laIndk[i] = laSize;
        laSizek[i] = gActive[i];
        laSize += laSizek[i];
      }
    } 
    else if(j==5) { // IB
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        laIndk[i] = laSize;
        laSizek[i] = gActive[i]*gdActive[i][0];
        laSize += laSizek[i];
      }
    } 
    else
      throw;
  }

  void Contact::calcgSize(int j) {
    LinkMechanics::calcgSize(j);
    if(j==0) { // IA
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        gIndk[i] = gSize;
        gSizek[i] = 1;
        gSize += gSizek[i];
      }
    }
    else if(j==1) { // IG
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        gIndk[i] = gSize;
        gSizek[i] = gActive[i];
        gSize += gSizek[i];
      }
    }
    else if(j==2) { // IB
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        gIndk[i] = gSize;
        gSizek[i] = gActive[i]*gdActive[i][0];
        gSize += gSizek[i];
      }
    }
    else
      throw;
  }

  void Contact::calcgdSize(int j) {
    LinkMechanics::calcgdSize(j);
    if(j==0) { // IA
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        gdIndk[i] = gdSize;
        gdSizek[i] = 1+getFrictionDirections();
        gdSize += gdSizek[i];
      }
    }
    else if(j==1) { // IG
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        gdIndk[i] = gdSize;
        gdSizek[i] = gActive[i]*(1+getFrictionDirections());
        gdSize += gdSizek[i];
      }
    }
    else if(j==2) { // IB
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        gdIndk[i] = gdSize;
        gdSizek[i] = gActive[i]*gdActive[i][0]*(1+getFrictionDirections());
        gdSize += gdSizek[i];
      }
    }
    else if(j==3) { // IH
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        gdIndk[i] = gdSize;
        gdSizek[i] = gActive[i]*gdActive[i][0]*(1+gdActive[i][1]*getFrictionDirections());
        gdSize += gdSizek[i];
      }
    }
    else
      throw;
  }

  void Contact::calcrFactorSize(int j) {
    LinkMechanics::calcrFactorSize(j);
    if(j==0) { // IA
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        rFactorIndk[i] = rFactorSize;
        rFactorSizek[i] = 1+min(getFrictionDirections(),1);
        rFactorSize += rFactorSizek[i];
      }
    } 
    else if(j==1) { // IG
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        rFactorIndk[i] = rFactorSize;
        rFactorSizek[i] = gActive[i]*(1+min(getFrictionDirections(),1));
        rFactorSize += rFactorSizek[i];
      }
    }
    else if(j==2) { // IB
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        rFactorIndk[i] = rFactorSize;
        rFactorSizek[i] = gActive[i]*gdActive[i][0]*(1+min(getFrictionDirections(),1));
        rFactorSize += rFactorSizek[i];
      }
    }
    else if(j==3) { // IB
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        rFactorIndk[i] = rFactorSize;
        rFactorSizek[i] = gActive[i]*gdActive[i][0]*(1+gdActive[i][1]*min(getFrictionDirections(),1));
        rFactorSize += rFactorSizek[i];
      }
    }
  }

  void Contact::calcsvSize() {
    LinkMechanics::calcsvSize();
    svSize = 0;
    for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
      svIndk[i] = svSize;
      svSizek[i] = isSetValued() ? 1+min(getFrictionDirections(),1) : 0;
      //svSizek[i] = isSetValued() ? 1 : 0;
      svSize += svSizek[i];
    }
  }

  void Contact::calcLinkStatusSize() {
    LinkMechanics::calcLinkStatusSize();
    int n = contactKinematics->getNumberOfPotentialContactPoints();
    LinkStatusSize= n;
    LinkStatus.resize(LinkStatusSize);
  }

  void Contact::calcLinkStatusRegSize() {
    LinkMechanics::calcLinkStatusRegSize();
    int n = contactKinematics->getNumberOfPotentialContactPoints();
    LinkStatusRegSize= n;
    LinkStatusReg.resize(LinkStatusRegSize);
  }

  void Contact::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if(saved_ref1!="" && saved_ref2!="")
        connect(getByPath<Contour>(saved_ref1), getByPath<Contour>(saved_ref2));
      LinkMechanics::init(stage);
    }
    else if(stage==resize) {
      LinkMechanics::init(stage);
      int n = contactKinematics->getNumberOfPotentialContactPoints();

      la.resize(n*(1+getFrictionDirections()));
      g.resize(n);
      gd.resize(n*(1+getFrictionDirections()));
      gdd.resize(gd.size());
      gdn.resize(gd.size());
      //LinkStatusSize= n;
      //LinkStatus.resize(LinkStatusSize);

      for(vector<ContourPointData*>::iterator i = cpData.begin(); i != cpData.end(); ++i) delete[] *i;
      cpData.clear(); // clear container first, because InitStage resize is called twice (before and after the reorganization)
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        if(getFrictionDirections() == 0)
          gdActive[i][1] = false;

        cpData.push_back(new ContourPointData[2]);

        cpData[i][0].getFrameOfReference().setName("0");
        cpData[i][1].getFrameOfReference().setName("1");

	cpData[i][0].getFrameOfReference().sethSize(contour[0]->gethSize(0),0);
        cpData[i][0].getFrameOfReference().sethSize(contour[0]->gethSize(1),1);
        cpData[i][1].getFrameOfReference().sethSize(contour[1]->gethSize(0),0);
        cpData[i][1].getFrameOfReference().sethSize(contour[1]->gethSize(1),1);

        int laSizek = gdActive[i][0]+gdActive[i][1]*getFrictionDirections();

        Wk[0].push_back(new Mat[2]);
        Wk[0][i][0].resize(contour[0]->gethSize(),laSizek);
        Wk[0][i][1].resize(contour[1]->gethSize(),laSizek);

        Vk[0].push_back(new Mat[2]);
        Vk[0][i][0].resize(contour[0]->gethSize(),laSizek);
        Vk[0][i][1].resize(contour[1]->gethSize(),laSizek);

        Wk[1].push_back(new Mat[2]);
        Wk[1][i][0].resize(contour[0]->gethSize(1),laSizek);
        Wk[1][i][1].resize(contour[1]->gethSize(1),laSizek);

        Vk[1].push_back(new Mat[2]);
        Vk[1][i][0].resize(contour[0]->gethSize(1),laSizek);
        Vk[1][i][1].resize(contour[1]->gethSize(1),laSizek);

        fF.push_back(new Mat3V[2]);
        fF[i][0].resize(laSizek);
        fF[i][1].resize(laSizek);

        WF.push_back(new Vec3[2]);
      }
    }
    else if(stage==unknownStage) {
      LinkMechanics::init(stage);

      iT = Range<Var,Var>(1,getFrictionDirections());

      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        cpData[k][0].getFrameOfReference().getJacobianOfTranslation(0).resize(contour[0]->getReferenceJacobianOfTranslation(0).cols());
        cpData[k][0].getFrameOfReference().getJacobianOfTranslation(1).resize(contour[0]->getReferenceJacobianOfTranslation(1).cols());
        cpData[k][0].getFrameOfReference().getJacobianOfRotation(0).resize(contour[0]->getReferenceJacobianOfRotation(0).cols());
        cpData[k][0].getFrameOfReference().getJacobianOfRotation(1).resize(contour[0]->getReferenceJacobianOfRotation(1).cols());
        cpData[k][1].getFrameOfReference().getJacobianOfTranslation(0).resize(contour[1]->getReferenceJacobianOfTranslation(0).cols());
        cpData[k][1].getFrameOfReference().getJacobianOfTranslation(1).resize(contour[1]->getReferenceJacobianOfTranslation(1).cols());
        cpData[k][1].getFrameOfReference().getJacobianOfRotation(0).resize(contour[1]->getReferenceJacobianOfRotation(0).cols());
        cpData[k][1].getFrameOfReference().getJacobianOfRotation(1).resize(contour[1]->getReferenceJacobianOfRotation(1).cols());
	lak[k] >> la(k*(1+getFrictionDirections()),(k+1)*(1+getFrictionDirections())-1);
	gdk[k] >> gd(k*(1+getFrictionDirections()),(k+1)*(1+getFrictionDirections())-1);
        gdnk[k] >> gdn(k*(1+getFrictionDirections()),(k+1)*(1+getFrictionDirections())-1);
        gddk[k] >> gdd(k*(1+getFrictionDirections()),(k+1)*(1+getFrictionDirections())-1);
        gk[k] >> g(k,k);
        gddkBuf[k].resize(1+getFrictionDirections());
      }
    }
    else if(stage==preInit) {
      LinkMechanics::init(stage);

      if(contactKinematics==0)
        contactKinematics = contour[0]->findContactPairingWith(contour[0]->getType(), contour[1]->getType());
      if(contactKinematics==0)
        contactKinematics = contour[1]->findContactPairingWith(contour[1]->getType(), contour[0]->getType());
      if(contactKinematics==0)
        contactKinematics = contour[0]->findContactPairingWith(contour[1]->getType(), contour[0]->getType());
      if(contactKinematics==0)
        contactKinematics = contour[1]->findContactPairingWith(contour[0]->getType(), contour[1]->getType());
      if(contactKinematics==0)
        throw MBSimError("ERROR in "+getName()+" (Contact::init): Unknown contact pairing between Contour \""+contour[0]->getType()+"\" and Contour\""+contour[1]->getType()+"\"!");

      contactKinematics->assignContours(contour[0],contour[1]);

      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        gActive.push_back(int(1));
        gActive0.push_back(int(1));
        gdActive.push_back(new unsigned int[2]);
        gddActive.push_back(new unsigned int[2]);
        for(int j=0; j<1+min(1,getFrictionDirections()); j++) gdActive[i][j] = 1;
        for(int j=1+min(1,getFrictionDirections()); j<2; j++) gdActive[i][j] = 0;
        for(int j=0; j<1+min(1,getFrictionDirections()); j++) gddActive[i][j] = 1;
        for(int j=1+min(1,getFrictionDirections()); j<2; j++) gddActive[i][j] = 0;

        gk.push_back(Vec(1));
        gdk.push_back(Vec(1));
        gdnk.push_back(Vec(1));
        gddk.push_back(Vec(1));
        gddkBuf.push_back(Vec(1));
        lak.push_back(Vec());
        wbk.push_back(Vec());
        svk.push_back(Vec());
        jsvk.push_back(VecInt());
        rFactork.push_back(Vec());
        laSizek.push_back(int(0));
        laIndk.push_back(int(0));
        gSizek.push_back(int(0));
        gIndk.push_back(int(0));
        gdSizek.push_back(int(0));
        gdIndk.push_back(int(0));
        svSizek.push_back(int(0));
        svIndk.push_back(int(0));
        rFactorSizek.push_back(int(0));
        rFactorIndk.push_back(int(0));

        corrk.push_back(Vec(1));
        corrSizek.push_back(int(0));
        corrIndk.push_back(int(0));
        rootID.push_back(int(0));
      }
    }
    else if(stage==MBSim::plot) {
      updatePlotFeatures();
      if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
        if(getPlotFeature(openMBV)==enabled && (openMBVContactFrameSize>epsroot() || contactArrow || frictionArrow)) {
          openMBVContactGrp = new OpenMBV::Group();
          openMBVContactGrp->setName(name+"_ContactGroup");
          openMBVContactGrp->setExpand(false);
          parent->getOpenMBVGrp()->addObject(openMBVContactGrp);
          for(unsigned int i=0; i<cpData.size(); i++) {
            if(openMBVContactFrameSize>epsroot()) {
              vector<OpenMBV::Frame*> temp;
              temp.push_back(new OpenMBV::Frame);
              temp.push_back(new OpenMBV::Frame);
              openMBVContactFrame.push_back(temp);
              for(unsigned int k=0; k<2; k++) { // frames
                openMBVContactFrame[i][k]->setOffset(1.);
                openMBVContactFrame[i][k]->setSize(openMBVContactFrameSize);
                openMBVContactFrame[i][k]->setName("ContactPoint_"+numtostr((int)i)+(k==0?"A":"B"));
                openMBVContactFrame[i][k]->setEnable(openMBVContactFrameEnabled);
                openMBVContactGrp->addObject(openMBVContactFrame[i][k]);
              }
            }
            // arrows
            OpenMBV::Arrow *arrow;
            if(contactArrow) {
              arrow=new OpenMBV::Arrow(*contactArrow);
              arrow->setName("NormalForce_"+numtostr((int)i)+"B");
              openMBVNormalForceArrow.push_back(arrow); // normal force
              openMBVContactGrp->addObject(arrow);
            }
            if(frictionArrow && getFrictionDirections()>0) { // friction force
              arrow=new OpenMBV::Arrow(*frictionArrow);
              arrow->setName("FrictionForce_"+numtostr((int)i)+"B");
              openMBVFrictionArrow.push_back(arrow);
              openMBVContactGrp->addObject(arrow);
            }
          }
        }
#endif
        if(getPlotFeature(linkKinematics)==enabled) {
          for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
            plotColumns.push_back("g["+numtostr(i)+"]("+numtostr(0)+")");
            for(int j=0; j<1+getFrictionDirections(); ++j) 
              plotColumns.push_back("gd["+numtostr(i)+"]("+numtostr(j)+")");
          }
        }
        if(getPlotFeature(generalizedLinkForce)==enabled) {
          for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
            for(int j=0; j<1+getFrictionDirections(); ++j)
              plotColumns.push_back("la["+numtostr(i)+"]("+numtostr(j)+")");
          }
        }
	PlotFeatureStatus pfKinematics = getPlotFeature(linkKinematics);
	PlotFeatureStatus pfKinetics = getPlotFeature(generalizedLinkForce);
	setPlotFeature(linkKinematics,disabled);
	setPlotFeature(generalizedLinkForce,disabled);
	LinkMechanics::init(stage);
	setPlotFeature(linkKinematics,pfKinematics);
	setPlotFeature(generalizedLinkForce,pfKinetics);
      }
    }
    else
      LinkMechanics::init(stage);
  }

  bool Contact::isSetValued() const {
    bool flag = fcl->isSetValued();
    if(fdf) 
      flag |= fdf->isSetValued();
    return flag;
  }

 
  void Contact::updateLinkStatus(double t) {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if (gActive[k])  {
        LinkStatus(k) = 2;
        if (ftil) {
          if (ftil->isSticking(lak[k](1,getFrictionDirections()),gdnk[k](1,getFrictionDirections()),gdk[k](1,getFrictionDirections()),lak[k](0),LaTol,gdTol)) LinkStatus(k) = 3;
          else LinkStatus(k) = 4;
        }
      }
      else LinkStatus(k) = 1;
    }
  }

  void Contact::updateLinkStatusReg(double t) {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if (gActive[k])  {
        LinkStatusReg(k) = 2;
      }
      else {
        LinkStatusReg(k) = 1;
      }
    }
  }

  bool Contact::isActive() const {
    for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
      if(gActive[i])
        return true;
    }
    return false;
  }

  bool Contact::gActiveChanged() {
    bool changed = false;
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gActive0[k] != gActive[k])
        changed = true;
      gActive0[k] = gActive[k];
    }
    return changed;
  }

  void Contact::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
      if(getPlotFeature(openMBV)==enabled && (openMBVContactFrameSize>epsroot() || contactArrow || frictionArrow)) {
        for(unsigned int i=0; i<cpData.size(); i++) {
          // frames
          if(openMBVContactFrameSize>epsroot()) {
            for (unsigned int k=0; k<2; k++) {
              vector<double> data;
              data.push_back(t);
              data.push_back(cpData[i][k].getFrameOfReference().getPosition()(0));
              data.push_back(cpData[i][k].getFrameOfReference().getPosition()(1));
              data.push_back(cpData[i][k].getFrameOfReference().getPosition()(2));
              Vec3 cardan=AIK2Cardan(cpData[i][k].getFrameOfReference().getOrientation());
              data.push_back(cardan(0));
              data.push_back(cardan(1));
              data.push_back(cardan(2));
              data.push_back(0);
              openMBVContactFrame[i][k]->append(data);
            }
          }
          // arrows
          // normal force
          vector<double> data;
          if(contactArrow) {
            data.push_back(t);
            data.push_back(cpData[i][1].getFrameOfReference().getPosition()(0));
            data.push_back(cpData[i][1].getFrameOfReference().getPosition()(1));
            data.push_back(cpData[i][1].getFrameOfReference().getPosition()(2));
            Vec3 F(INIT,0);
            if(isSetValued()) {
              if(gActive[i]) F=fF[i][1].col(0)*lak[i](0)/dt;
            }
            else
              F=cpData[i][0].getFrameOfReference().getOrientation().col(0)*lak[i](0);
            data.push_back(F(0));
            data.push_back(F(1));
            data.push_back(F(2));
            data.push_back(nrm2(F));
            openMBVNormalForceArrow[i]->append(data);
          }
          if(frictionArrow && getFrictionDirections()>0) { // friction force
            data.clear();
            data.push_back(t);
            data.push_back(cpData[i][1].getFrameOfReference().getPosition()(0));
            data.push_back(cpData[i][1].getFrameOfReference().getPosition()(1));
            data.push_back(cpData[i][1].getFrameOfReference().getPosition()(2));
            Vec3 F(INIT,0);
            if(isSetValued()) {                    // TODO switch between stick and slip not possible with TimeStepper
              if(gActive[i] && lak[i].size()>1) { // stick friction
                F=fF[i][1].col(1)*lak[i](1)/dt;
                if(getFrictionDirections()>1)
                  F+=fF[i][1].col(2)*lak[i](2)/dt;
              }
              if(gActive[i] && lak[i].size()==1) // slip friction
                F=fF[i][1](Range<Fixed<0>,Fixed<2> >(),iT)*fdf->dlaTdlaN(gdk[i](1,getFrictionDirections()), lak[i](0))*lak[i](0)/dt;
            }
            else {
              F=cpData[i][0].getFrameOfReference().getOrientation().col(1)*lak[i](1);
              if(getFrictionDirections()>1)
                F+=cpData[i][0].getFrameOfReference().getOrientation().col(2)*lak[i](2);
            }
            data.push_back(F(0));
            data.push_back(F(1));
            data.push_back(F(2));
            data.push_back((isSetValued() && lak[i].size()>1)?1:0.5); // draw in green if slipping and draw in red if sticking
            openMBVFrictionArrow[i]->append(data);
          }
        }
      }
#endif
      if(getPlotFeature(linkKinematics)==enabled) {
        bool flag = fcl->isSetValued();
        for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
          plotVector.push_back(gk[i](0)); //gN
          if((flag && gActive[i]) || (!flag && fcl->isActive(gk[i](0),0))) {
            for(int j=0; j<1+getFrictionDirections(); j++)
              plotVector.push_back(gdk[i](j)); //gd
          } 
          else {
            for(int j=0; j<1+getFrictionDirections(); j++)
              plotVector.push_back(NAN); //gd
          }
        }
      }
      if(getPlotFeature(generalizedLinkForce)==enabled) {
        for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
          if(gActive[i] && gdActive[i][0]) {
            plotVector.push_back(lak[i](0)/(isSetValued()?dt:1.));
            if(gdActive[i][1]) {
              for(int j=0; j<getFrictionDirections(); j++)
                plotVector.push_back(lak[i](1+j)/(isSetValued()?dt:1.));
            }
            else {
              if(fdf) {
                Vec buf = fdf->dlaTdlaN(gdk[i](1,getFrictionDirections()), lak[i](0))*lak[i](0);
                for(int j=0; j<getFrictionDirections(); j++)
                  plotVector.push_back(buf(j)/(isSetValued()?dt:1.));
              }
            }
          } 
          else {
            for(int j=0; j<1+getFrictionDirections() ; j++)
              plotVector.push_back(0);
          }
        }
      }
      PlotFeatureStatus pfKinematics = getPlotFeature(linkKinematics);
      PlotFeatureStatus pfKinetics = getPlotFeature(generalizedLinkForce);
      setPlotFeature(linkKinematics,disabled);
      setPlotFeature(generalizedLinkForce,disabled);
      LinkMechanics::plot(t, dt);
      setPlotFeature(linkKinematics,pfKinematics);
      setPlotFeature(generalizedLinkForce,pfKinetics);
    }
  }

  void Contact::closePlot() {
    if(getPlotFeature(plotRecursive)==enabled) {
      LinkMechanics::closePlot();
    }
  }

  void Contact::solveImpactsFixpointSingle(double dt) {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gActive[k]) {

        const double *a = ds->getGs()();
        const int *ia = ds->getGs().Ip();
        const int *ja = ds->getGs().Jp();
        const Vec &laMBS = ds->getla();
        const Vec &b = ds->getb();

        gdnk[k](0) = b(laInd+laIndk[k]);
        for(int j=ia[laInd+laIndk[k]]; j<ia[laInd+laIndk[k]+1]; j++)
          gdnk[k](0) += a[j]*laMBS(ja[j]);

        lak[k](0) = fnil->project(lak[k](0), gdnk[k](0), gdk[k](0), rFactork[k](0));

        for(int i=1; i<=getFrictionDirections(); i++) {
          gdnk[k](i) = b(laInd+laIndk[k]+i);
          for(int j=ia[laInd+laIndk[k]+i]; j<ia[laInd+laIndk[k]+1+i]; j++)
            gdnk[k](i) += a[j]*laMBS(ja[j]);
        }

        if(ftil)
          lak[k](1,getFrictionDirections()) = ftil->project(lak[k](1,getFrictionDirections()), gdnk[k](1,getFrictionDirections()), gdk[k](1,getFrictionDirections()), lak[k](0), rFactork[k](1));
      }
    }
  }

  void Contact::solveConstraintsFixpointSingle() {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gdActive[k][0]) {

        const double *a = ds->getGs()();
        const int *ia = ds->getGs().Ip();
        const int *ja = ds->getGs().Jp();
        const Vec &laMBS = ds->getla();
        const Vec &b = ds->getb();

        gddk[k](0) = b(laInd+laIndk[k]);
        for(int j=ia[laInd+laIndk[k]]; j<ia[laInd+laIndk[k]+1]; j++)
          gddk[k](0) += a[j]*laMBS(ja[j]);

        lak[k](0) = fcl->project(lak[k](0), gddk[k](0), rFactork[k](0));

        if(gdActive[k][1]) {
          for(int i=1; i<=getFrictionDirections(); i++) {
            gddk[k](i) = b(laInd+laIndk[k]+i);
            for(int j=ia[laInd+laIndk[k]+i]; j<ia[laInd+laIndk[k]+1+i]; j++)
              gddk[k](i) += a[j]*laMBS(ja[j]);
          }

          if(fdf)
            lak[k](1,getFrictionDirections()) = fdf->project(lak[k](1,getFrictionDirections()), gddk[k](1,getFrictionDirections()), lak[k](0), rFactork[k](1));
        }
      }
    }
  } 

  void Contact::solveImpactsGaussSeidel(double dt) {
    assert(getFrictionDirections() <= 1);

    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gActive[k]) {

        const double *a = ds->getGs()();
        const int *ia = ds->getGs().Ip();
        const int *ja = ds->getGs().Jp();
        const Vec &laMBS = ds->getla();
        const Vec &b = ds->getb();

        gdnk[k](0) = b(laInd+laIndk[k]);
        for(int j=ia[laInd+laIndk[k]]+1; j<ia[laInd+laIndk[k]+1]; j++)
          gdnk[k](0) += a[j]*laMBS(ja[j]);

        const double om = 1.0;
        const double buf = fnil->solve(a[ia[laInd+laIndk[k]]], gdnk[k](0), gdk[k](0));
        lak[k](0) += om*(buf - lak[k](0));

        if(getFrictionDirections()) {
          gdnk[k](1) = b(laInd+laIndk[k]+1);
          for(int j=ia[laInd+laIndk[k]+1]+1; j<ia[laInd+laIndk[k]+2]; j++)
            gdnk[k](1) += a[j]*laMBS(ja[j]);

          if(ftil) {
            Vec buf = ftil->solve(ds->getG()(Index(laInd+laIndk[k]+1,laInd+laIndk[k]+getFrictionDirections())), gdnk[k](1,getFrictionDirections()), gdk[k](1,getFrictionDirections()), lak[k](0));
            lak[k](1,getFrictionDirections()) += om*(buf - lak[k](1,getFrictionDirections()));
          }
        }
      }
    }
  }

  void Contact::solveConstraintsGaussSeidel() {
    assert(getFrictionDirections() <= 1);

    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gdActive[k][0]) {

        const double *a = ds->getGs()();
        const int *ia = ds->getGs().Ip();
        const int *ja = ds->getGs().Jp();
        const Vec &laMBS = ds->getla();
        const Vec &b = ds->getb();

        gddk[k](0) = b(laInd+laIndk[k]);
        for(int j=ia[laInd+laIndk[k]]+1; j<ia[laInd+laIndk[k]+1]; j++)
          gddk[k](0) += a[j]*laMBS(ja[j]);

        const double om = 1.0; // relaxation parameter omega (cf. Foerg, dissertation, p. 102)
        const double buf = fcl->solve(a[ia[laInd+laIndk[k]]], gddk[k](0));
        lak[k](0) += om*(buf - lak[k](0));

        if(getFrictionDirections() && gdActive[k][1]) {
          gddk[k](1) = b(laInd+laIndk[k]+1);
          for(int j=ia[laInd+laIndk[k]+1]+1; j<ia[laInd+laIndk[k]+2]; j++)
            gddk[k](1) += a[j]*laMBS(ja[j]);

          if(fdf) {
            Vec buf = fdf->solve(ds->getG()(Index(laInd+laIndk[k]+1,laInd+laIndk[k]+getFrictionDirections())), gddk[k](1,getFrictionDirections()), lak[k](0));
            lak[k](1,getFrictionDirections()) += om*(buf - lak[k](1,getFrictionDirections()));
          }
        }
      }
    }
  }

  void Contact::solveImpactsRootFinding(double dt) {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gActive[k]) {

        const double *a = ds->getGs()();
        const int *ia = ds->getGs().Ip();
        const int *ja = ds->getGs().Jp();
        const Vec &laMBS = ds->getla();
        const Vec &b = ds->getb();

        for(int i=0; i < 1+getFrictionDirections(); i++) {
          gdnk[k](i) = b(laInd+laIndk[k]+i);
          for(int j=ia[laInd+laIndk[k]+i]; j<ia[laInd+laIndk[k]+1+i]; j++)
            gdnk[k](i) += a[j]*laMBS(ja[j]);
        }

        res(0) = lak[k](0) - fnil->project(lak[k](0), gdnk[k](0), gdk[k](0), rFactork[k](0));
        if(ftil) 
          res(1,getFrictionDirections()) = lak[k](1,getFrictionDirections()) - ftil->project(lak[k](1,getFrictionDirections()), gdnk[k](1,getFrictionDirections()), gdk[k](1,getFrictionDirections()), lak[k](0), rFactork[k](1));
      }
    }
  }

  void Contact::solveConstraintsRootFinding() {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gdActive[k][0]) {

        const double *a = ds->getGs()();
        const int *ia = ds->getGs().Ip();
        const int *ja = ds->getGs().Jp();
        const Vec &laMBS = ds->getla();
        const Vec &b = ds->getb();

        for(int i=0; i < 1+getFrictionDirections(); i++) {
          gddk[k](i) = b(laInd+laIndk[k]+i);
          for(int j=ia[laInd+laIndk[k]+i]; j<ia[laInd+laIndk[k]+1+i]; j++)
            gddk[k](i) += a[j]*laMBS(ja[j]);
        }

        res(0) = lak[k](0) - fcl->project(lak[k](0), gddk[k](0), rFactork[k](0));
        if(fdf) 
          res(1,getFrictionDirections()) = lak[k](1,getFrictionDirections()) - fdf->project(lak[k](1,getFrictionDirections()), gddk[k](1,getFrictionDirections()), lak[k](0), rFactork[k](1));
      }
    }
  }

  void Contact::jacobianConstraints() {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gdActive[k][0]) {

        const SqrMat Jprox = ds->getJprox();
        const SqrMat G = ds->getG();

        RowVec jp1=Jprox.row(laInd+laIndk[k]);
        RowVec e1(jp1.size());
        e1(laInd+laIndk[k]) = 1;
        Vec diff = fcl->diff(lak[k](0), gddk[k](0), rFactork[k](0));

        jp1 = e1-diff(0)*e1; // -diff(1)*G.row(laInd+laIndk[k])
        for(int i=0; i<G.size(); i++) 
          jp1(i) -= diff(1)*G(laInd+laIndk[k],i);

        if(getFrictionDirections() == 1) {
          Mat diff = fdf->diff(lak[k](1,1), gddk[k](1,1), lak[k](0), rFactork[k](1));
          RowVec jp2=Jprox.row(laInd+laIndk[k]+1);
          RowVec e2(jp2.size());
          e2(laInd+laIndk[k]+1) = 1;
          Mat e(2,jp2.size());
          e(0,laInd+laIndk[k]) = 1;
          e(1,laInd+laIndk[k]+1) = 1;
          jp2 = e2-diff(0,2)*e1-diff(0,0)*e2; // -diff(1)*G.row(laInd+laIndk[k])
          //jp2 = e2-diff.row(0)(0,1)*e; // -diff(1)*G.row(laInd+laIndk[k])
          for(int i=0; i<G.size(); i++) 
            jp2(i) -= diff(0,1)*G(laInd+laIndk[k]+1,i);

        }
        else if(getFrictionDirections() == 2) {
          Mat diff = ftil->diff(lak[k](1,2), gddk[k](1,2), gdk[k](1,2), lak[k](0), rFactork[k](1));
          Mat jp2=Jprox(Index(laInd+laIndk[k]+1,laInd+laIndk[k]+2),Index(0,Jprox.cols()-1));
          Mat e2(2,jp2.cols());
          e2(0,laInd+laIndk[k]+1) = 1;
          e2(1,laInd+laIndk[k]+2) = 1;
          jp2 = e2-diff(Index(0,1),Index(4,4))*e1-diff(Index(0,1),Index(0,1))*e2; // -diff(Index(0,1),Index(4,5))*G(Index(laInd+laIndk[k]+1,laInd+laIndk[k]+2),Index(0,G.size()-1))
          for(int i=0; i<G.size(); i++) {
            jp2(0,i) = diff(0,2)*G(laInd+laIndk[k]+1,i)+diff(0,3)*G(laInd+laIndk[k]+2,i);
            jp2(1,i) = diff(1,2)*G(laInd+laIndk[k]+1,i)+diff(1,3)*G(laInd+laIndk[k]+2,i);
          }
        }
      }
    }
  }

  void Contact::jacobianImpacts() {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gActive[k]) {

        const SqrMat Jprox = ds->getJprox();
        const SqrMat G = ds->getG();

        RowVec jp1=Jprox.row(laInd+laIndk[k]);
        RowVec e1(jp1.size());
        e1(laInd+laIndk[k]) = 1;
        Vec diff = fnil->diff(lak[k](0), gdnk[k](0), gdk[k](0), rFactork[k](0));

        jp1 = e1-diff(0)*e1; // -diff(1)*G.row(laInd+laIndk[k])
        for(int i=0; i<G.size(); i++) 
          jp1(i) -= diff(1)*G(laInd+laIndk[k],i);

        if(getFrictionDirections() == 1) {
          Mat diff = ftil->diff(lak[k](1,1), gdnk[k](1,1), gdk[k](1,1), lak[k](0), rFactork[k](1));
          RowVec jp2=Jprox.row(laInd+laIndk[k]+1);
          RowVec e2(jp2.size());
          e2(laInd+laIndk[k]+1) = 1;
          Mat e(2,jp2.size());
          e(0,laInd+laIndk[k]) = 1;
          e(1,laInd+laIndk[k]+1) = 1;
          jp2 = e2-diff(0,2)*e1-diff(0,0)*e2; // -diff(1)*G.row(laInd+laIndk[k])
          //jp2 = e2-diff.row(0)(0,1)*e; // -diff(1)*G.row(laInd+laIndk[k])
          for(int i=0; i<G.size(); i++) 
            jp2(i) -= diff(0,1)*G(laInd+laIndk[k]+1,i);

        } 
        else if(getFrictionDirections() == 2) {
          Mat diff = ftil->diff(lak[k](1,2), gdnk[k](1,2), gdk[k](1,2), lak[k](0), rFactork[k](1));
          Mat jp2=Jprox(Index(laInd+laIndk[k]+1,laInd+laIndk[k]+2),Index(0,Jprox.cols()-1));
          Mat e2(2,jp2.cols());
          e2(0,laInd+laIndk[k]+1) = 1;
          e2(1,laInd+laIndk[k]+2) = 1;
          jp2 = e2-diff(Index(0,1),Index(4,4))*e1-diff(Index(0,1),Index(0,1))*e2; // -diff(Index(0,1),Index(4,5))*G(Index(laInd+laIndk[k]+1,laInd+laIndk[k]+2),Index(0,G.size()-1))
          for(int i=0; i<G.size(); i++) {
            jp2(0,i) = diff(0,2)*G(laInd+laIndk[k]+1,i)+diff(0,3)*G(laInd+laIndk[k]+2,i);
            jp2(1,i) = diff(1,2)*G(laInd+laIndk[k]+1,i)+diff(1,3)*G(laInd+laIndk[k]+2,i);
          }
        }
      }
    }
  }

  void Contact::updaterFactors() {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gdActive[k][0]) {

        const double *a = ds->getGs()();
        const int *ia = ds->getGs().Ip();

        double sumN = 0;
        for(int j=ia[laInd+laIndk[k]]+1; j<ia[laInd+laIndk[k]+1]; j++)
          sumN += fabs(a[j]);
        const double aN = a[ia[laInd+laIndk[k]]];
        if(aN > sumN) {
          rFactorUnsure(0) = 0;
          rFactork[k](0) = 1.0/aN;
        } 
        else {
          rFactorUnsure(0) = 1;
          rFactork[k](0) = rMax/aN;
        }
        double sumT1 = 0;
        double sumT2 = 0;
        double aT1, aT2;
        if(fdf && gdActive[k][1]) {
          if(getFrictionDirections() == 1) {
            for(int j=ia[laInd+laIndk[k]+1]+1; j<ia[laInd+laIndk[k]+2]; j++)
              sumT1 += fabs(a[j]);
            aT1 = a[ia[laInd+laIndk[k]+1]];
            if(aT1 > sumT1) {
              rFactorUnsure(1)=0;
              rFactork[k](1) = 1.0/aT1;
            }
            else {
              rFactorUnsure(1)=1;
              rFactork[k](1) = rMax/aT1;
            }
          } 
          else if(getFrictionDirections() == 2) {
            for(int j=ia[laInd+laIndk[k]+1]+1; j<ia[laInd+laIndk[k]+2]; j++)
              sumT1 += fabs(a[j]);
            for(int j=ia[laInd+laIndk[k]+2]+1; j<ia[laInd+laIndk[k]+3]; j++)
              sumT2 += fabs(a[j]);
            aT1 = a[ia[laInd+laIndk[k]+1]];
            aT2 = a[ia[laInd+laIndk[k]+2]];

            // TODO rFactorUnsure
            if(aT1 - sumT1 >= aT2 - sumT2) 
              if(aT1 + sumT1 >= aT2 + sumT2) 
                rFactork[k](1) = 2.0/(aT1+aT2+sumT1-sumT2);
              else 
                rFactork[k](1) = 1.0/aT2;
            else 
              if(aT1 + sumT1 < aT2 + sumT2) 
                rFactork[k](1) = 2.0/(aT1+aT2-sumT1+sumT2);
              else 
                rFactork[k](1) = 1.0/aT1;
          }
        }
      }
    }
  }

  void Contact::checkConstraintsForTermination() {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gdActive[k][0]) {

        const double *a = ds->getGs()();
        const int *ia = ds->getGs().Ip();
        const int *ja = ds->getGs().Jp();
        const Vec &laMBS = ds->getla();
        const Vec &b = ds->getb();

        for(unsigned int i=0; i < 1+ gdActive[k][1]*getFrictionDirections(); i++) {
          gddk[k](i) = b(laInd+laIndk[k]+i);
          for(int j=ia[laInd+laIndk[k]+i]; j<ia[laInd+laIndk[k]+1+i]; j++)
            gddk[k](i) += a[j]*laMBS(ja[j]);
        }

        if(!fcl->isFulfilled(lak[k](0),gddk[k](0),laTol,gddTol)) {
          ds->setTermination(false);
          return;
        }
        if(fdf && gdActive[k][1]) { 
          if(!fdf->isFulfilled(lak[k](1,getFrictionDirections()),gddk[k](1,getFrictionDirections()),lak[k](0),laTol,gddTol)) {
            ds->setTermination(false);
            return;
          }
        }
      }
    }
  }

  void Contact::checkImpactsForTermination(double dt) {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gActive[k]) {

        const double *a = ds->getGs()();
        const int *ia = ds->getGs().Ip();
        const int *ja = ds->getGs().Jp();
        const Vec &laMBS = ds->getla();
        const Vec &b = ds->getb();

        for(int i=0; i < 1+getFrictionDirections(); i++) {
          gdnk[k](i) = b(laInd+laIndk[k]+i);
          for(int j=ia[laInd+laIndk[k]+i]; j<ia[laInd+laIndk[k]+1+i]; j++)
            gdnk[k](i) += a[j]*laMBS(ja[j]);
        }

        if(!fnil->isFulfilled(lak[k](0),gdnk[k](0),gdk[k](0),LaTol,gdTol)) {
          ds->setTermination(false);
          return;
        }
        if(ftil) { 
          if(!ftil->isFulfilled(lak[k](1,getFrictionDirections()),gdnk[k](1,getFrictionDirections()),gdk[k](1,getFrictionDirections()),lak[k](0),LaTol,gdTol)) {
            ds->setTermination(false);
            return;
          }
        }
      }
    }
  }

  void Contact::checkActive(int j) { 
    if(j==1) { // formerly checkActiveg()
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        gActive[k] = fcl->isActive(gk[k](0),gTol) ? 1 : 0; 
        gdActive[k][0] = gActive[k];
        gdActive[k][1] = gdActive[k][0];
      }
    }
    else if(j==2) { // formerly checkActivegd()
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        gdActive[k][0] = gActive[k] ? (fcl->remainsActive(gdk[k](0),gdTol) ? 1 : 0) : 0; 
        gdActive[k][1] = getFrictionDirections() && gdActive[k][0] ? (fdf->isSticking(gdk[k](1,getFrictionDirections()),gdTol) ? 1 : 0) : 0; 
        gddActive[k][0] = gdActive[k][0];
        gddActive[k][1] = gdActive[k][1];
      }
    }
    else if(j==3) { // formerly checkActivegdn()
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        if(gActive[k]) { // Contact was closed
          if(gdnk[k](0) <= gdTol) { // Contact stays closed // TODO bilateral contact
            gdActive[k][0] = true;
            gddActive[k][0] = true;
            if(getFrictionDirections()) {
              if(nrm2(gdnk[k](1,getFrictionDirections())) <= gdTol) {
                gdActive[k][1] = true;
                gddActive[k][1] = true;
              }
              else {
                gdActive[k][1] = false;
                gddActive[k][1] = false;
              }
            }
          } 
          else { // Contact will open
            gdActive[k][0] = false;
            gdActive[k][1] = false;
            gddActive[k][0] = false;
            gddActive[k][1] = false;
          }
        }
      }
    }
    else if(j==4) { // formerly checkActivegdd()
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        if(gActive[k]) {
          if(gdActive[k][0]) {
            if(gddk[k](0) <= gddTol) { // Contact stays closed
              gddActive[k][0] = true;
              if(getFrictionDirections()) {
                if(gdActive[k][1]) {
                  if(nrm2(gddk[k](1,getFrictionDirections())) <= gddTol)
                    gddActive[k][1] = true;
                  else {
                    gddActive[k][1] = false;
                  }
                }
              }
            }
            else { // Contact will open
              gddActive[k][0] = false;
              gddActive[k][1] = false;
            }
          }
        }
      }
    }
    else if(j==5) {
      for(int i=0; i<contactKinematics->getNumberOfPotentialContactPoints(); i++) {
        if(gActive[i]) {
          if(gdActive[i][0]) {
            if(gdActive[i][1]) {
              if(!gddActive[i][1]) {
                gdActive[i][1] = false;
              }
            }
            if(!gddActive[i][0]) {
              gActive[i] = false;
              gdActive[i][0] = false;
              gdActive[i][1] = false;
            }
          }
          else
            gActive[i] = false;
        }
      }
    }
    else if(j==6) { // nur nach schließenden Kontakten schauen
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        if(rootID[k]==3) {
          gActive[k] = true;
          gdActive[k][0] = true;
          gdActive[k][1] = true;
          gddActive[k][0] = true;
          gddActive[k][1] = true;
        }
      }
    } else if(j==7) { // nur nach Gleit-Haft-Übergängen schauen
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        if(getFrictionDirections()) {
          if(rootID[k]==2) {
            gdActive[k][1] = true;
            gddActive[k][1] = true;
          }
        }
      }
    } else if(j==8) { // nur nach öffnenden Kontakten und Haft-Gleit-Übergängen schauen
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        if(jsvk[k](0) && rootID[k]==1) { // Kontakt öffnet
          gddActive[k][0] = false;
          gddActive[k][1] = false;
        }
        if(getFrictionDirections()) {
          if(jsvk[k](1) && rootID[k]==1) { // Haft-Gleitübergang
            gddActive[k][1] = false;
          }
        }
      }
    }
    else
      throw;
  }

  int Contact::getFrictionDirections() {
    if(fdf) 
      return fdf->getFrictionDirections();
    else
      return 0;
  }

  void Contact::connect(Contour *contour0, Contour* contour1) {
    LinkMechanics::connect(contour0);
    LinkMechanics::connect(contour1);
  }

  void Contact::computeCurvatures(Vec & r) const {
    contactKinematics->computeCurvatures(r, cpData[0]);
  }

  void Contact::LinearImpactEstimation(Vec &gInActive_,Vec &gdInActive_,int *IndInActive_,Vec &gAct_,int *IndActive_) {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gActive[k]) {
	gAct_(*IndActive_) = gk[k](0);
	(*IndActive_)++;
      }
      else {
	for(unsigned int i=0; i<2; i++) contour[i]->updateKinematicsForFrame(cpData[k][i],velocities); 
	Vec3 Wn = cpData[k][0].getFrameOfReference().getOrientation().col(0);
	Vec3 WvD = cpData[k][1].getFrameOfReference().getVelocity() - cpData[k][0].getFrameOfReference().getVelocity();
	gdInActive_(*IndInActive_) = Wn.T()*WvD;
	gInActive_(*IndInActive_) = gk[k](0);
	(*IndInActive_)++;
      }
    } 

  }

  void Contact::SizeLinearImpactEstimation(int *sizeInActive_, int *sizeActive_) { 
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      if(gActive[k]) (*sizeActive_)++;
      else (*sizeInActive_)++;
    } 
  }

  void Contact::initializeUsingXML(TiXmlElement *element) {
    LinkMechanics::initializeUsingXML(element);
    TiXmlElement *e;
    e=element->FirstChildElement(MBSIMNS"contactForceLaw");
    GeneralizedForceLaw *gfl=ObjectFactory::getInstance()->createGeneralizedForceLaw(e->FirstChildElement());
    setContactForceLaw(gfl);
    gfl->initializeUsingXML(e->FirstChildElement());
    e=e->NextSiblingElement();
    GeneralizedImpactLaw *gifl=ObjectFactory::getInstance()->createGeneralizedImpactLaw(e->FirstChildElement());
    if(gifl) {
      setContactImpactLaw(gifl);
      gifl->initializeUsingXML(e->FirstChildElement());
      e=e->NextSiblingElement();
    }
    FrictionForceLaw *ffl=ObjectFactory::getInstance()->createFrictionForceLaw(e->FirstChildElement());
    if(ffl) {
      setFrictionForceLaw(ffl);
      ffl->initializeUsingXML(e->FirstChildElement());
      e=e->NextSiblingElement();
    }
    FrictionImpactLaw *fil=ObjectFactory::getInstance()->createFrictionImpactLaw(e->FirstChildElement());
    if(fil) {
      setFrictionImpactLaw(fil);
      fil->initializeUsingXML(e->FirstChildElement());
    }
    e=element->FirstChildElement(MBSIMNS"connect");
    saved_ref1=e->Attribute("ref1");
    saved_ref2=e->Attribute("ref2");
#ifdef HAVE_OPENMBVCPPINTERFACE
    if(element->FirstChildElement(MBSIMNS"enableOpenMBVContactPoints"))
      enableOpenMBVContactPoints(getDouble(element->FirstChildElement(MBSIMNS"enableOpenMBVContactPoints")));
    e=element->FirstChildElement(MBSIMNS"openMBVNormalForceArrow");
    if(e) {
      OpenMBV::Arrow *arrow=dynamic_cast<OpenMBV::Arrow*>(OpenMBV::ObjectFactory::createObject(e->FirstChildElement()));
      arrow->initializeUsingXML(e->FirstChildElement()); // first initialize, because setOpenMBVForceArrow calls the copy constructor on arrow
      setOpenMBVNormalForceArrow(arrow);
      e=e->NextSiblingElement();
    }
    e=element->FirstChildElement(MBSIMNS"openMBVFrictionArrow");
    if(e) {
      OpenMBV::Arrow *arrow=dynamic_cast<OpenMBV::Arrow*>(OpenMBV::ObjectFactory::createObject(e->FirstChildElement()));
      arrow->initializeUsingXML(e->FirstChildElement()); // first initialize, because setOpenMBVForceArrow calls the copy constructor on arrow
      setOpenMBVFrictionArrow(arrow);
      e=e->NextSiblingElement();
    }
#endif
  }

  void Contact::updatecorrRef(const Vec& corrParent) {
    LinkMechanics::updatecorrRef(corrParent);
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) 
      corrk[k] >> corr(corrIndk[k],corrIndk[k]+corrSizek[k]-1);
  }

  void Contact::updatecorr(int j) {
    if(j==1) { // IG position
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        if(gActive[k]) { // Contact was closed
          if(gdActive[k][0]) 
            corrk[k](0) = 0; // Contact stays closed, regular projection
          else
            corrk[k](0) = 1e-14; // Contact opens, projection to positive normal distance
        }
      }
    }
    else if(j==2) { 
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        if(gActive[k] && gdActive[k][0]) { // Contact was closed
          if(gddActive[k][0])
            corrk[k](0) = 0; // Contact stays closed, regular projection
          else
            corrk[k](0) = 1e-14; // Contact opens, projection to positive normal distance
        }
      }
    }
    else if(j==4) { 
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        if(rootID[k]==1)
          gddk[k] = gddkBuf[k];
        if(gActive[k] && gdActive[k][0]) { // Contact was closed
          if(gddActive[k][0])
            corrk[k](0) = 0; // Contact stays closed, regular projection
          else
            corrk[k](0) = 1e-16; // Contact opens, projection to positive normal distance
          if(getFrictionDirections()) {
            if(gdActive[k][1]) { // Contact was sticking
              if(gddActive[k][1]) {
                corrk[k](1) = 0; // Contact stays sticking, regular projection
                if(getFrictionDirections()>1) 
                  corrk[k](2) = 0; // Contact stays sticking, regular projection
              }
              else {
                corrk[k](1) = gddk[k](1)>0?1e-16:-1e-16; // Contact slides, projection to positive normal velocity
                if(getFrictionDirections()>1) 
                  corrk[k](2) = gddk[k](2)>0?1e-16:-1e-16; // Contact slides, projection to positive normal velocity
              }
            }
          }
        }
      }
    }
    else
      throw;
  }

  void Contact::calccorrSize(int j) {
    LinkMechanics::calccorrSize(j);
    if(j==1) { // IG
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        corrIndk[k] = corrSize;
        corrSizek[k] = gActive[k];
        corrSize += corrSizek[k];
      }
    }
    else if(j==2) { // IB
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        corrIndk[k] = corrSize;
        corrSizek[k] = gActive[k]*gdActive[k][0];
        corrSize += corrSizek[k];
      }
    }
//    else if(j==3) { // IG
//      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
//        corrIndk[k] = corrSize;
//        corrSizek[k] = gActive[i]*(1+getFrictionDirections());
//        corrSize += corrSizek[k];
//      }
//    }
    else if(j==4) { // IH
      for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
        corrIndk[k] = corrSize;
        corrSizek[k] = gActive[k]*gdActive[k][0]*(1+gdActive[k][1]*getFrictionDirections());
        corrSize += corrSizek[k];
      }
    }
    else
      throw;
  }

  void Contact::checkRoot() {
    for(int k=0; k<contactKinematics->getNumberOfPotentialContactPoints(); k++) {
      rootID[k] = 0;
      if(jsvk[k](0)) {
        if(gActive[k])
          rootID[k] = 1; // Contact was closed -> opening
        else
          rootID[k] = 3; // Contact was open -> impact
      }
      if(getFrictionDirections()) {
        if(jsvk[k](1)) {
          if(gdActive[k][1])
            rootID[k] = 1; // Contact was sticking -> sliding
          else {
            if(getFrictionDirections() == 1)
              rootID[k] = 2; // Contact was sliding -> sticking
            else if(nrm2(gdk[k](1,getFrictionDirections()))<=gdTol)
              rootID[k] = 2; // Contact was sliding -> sticking
          }
        }
      }
      ds->setRootID(max(ds->getRootID(),rootID[k]));
    }
  }

}

