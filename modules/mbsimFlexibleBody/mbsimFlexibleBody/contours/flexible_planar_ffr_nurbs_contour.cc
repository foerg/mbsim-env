/* Copyright (C) 2004-2018 MBSim Development Team
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

#include <config.h>
#include "mbsimFlexibleBody/contours/flexible_planar_ffr_nurbs_contour.h"
#include "mbsimFlexibleBody/flexible_body/generic_flexible_ffr_body.h"
#include "mbsim/frames/floating_contour_frame.h"
#include "mbsim/utils/utils.h"
#include <openmbvcppinterface/group.h>

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimFlexibleBody {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIMFLEX, FlexiblePlanarFfrNurbsContour)

  double FlexiblePlanarFfrNurbsContour::continueEta(double eta_) {
    double eta;
    if(open)
      eta = eta_;
    else
      eta = mod(eta_-etaNodes[0],etaNodes[1]-etaNodes[0])+etaNodes[0];
    return eta;
  }

  void FlexiblePlanarFfrNurbsContour::updateHessianMatrix(double eta_) {
    double eta = continueEta(eta_);
    crvPos.deriveAtH(eta,2,hessPos);
    for(size_t i=0; i<crvPhi.size(); i++)
      crvPhi[i].deriveAtH(eta,2,hessPhi[i]);
    etaOld = eta_;
  }

  void FlexiblePlanarFfrNurbsContour::updateGlobalRelativePosition(double eta) {
    Vec3 KrPS = evalHessianMatrixPos(eta).row(0).T()(Range<Fixed<0>,Fixed<2> >());
    for(size_t i=0; i<crvPhi.size(); i++)
      KrPS += hessPhi[i].row(0).T()(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(i);
    WrPS = R->evalOrientation()*KrPS;
    updPos = false;
  }

  void FlexiblePlanarFfrNurbsContour::updateGlobalRelativeVelocity(double eta) {
    if(fabs(eta-etaOld)>1e-13) updateHessianMatrix(eta);
    Vec3 Kvrel;
    for(size_t i=0; i<crvPhi.size(); i++)
      Kvrel += hessPhi[i].row(0).T()(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqdERel()(i);
    Wvrel = R->evalOrientation()*Kvrel;
    updVel = false;
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalKs_t(const Vec2 &zeta) {
    if(fabs(zeta(0)-etaOld)>1e-13) updateHessianMatrix(zeta(0));
    Vec3 s_t;
    for(size_t i=0; i<crvPhi.size(); i++)
      s_t += hessPhi[i].row(1).T()(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqdERel()(i);
    return s_t;
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalKu_t(const Vec2 &zeta) {
    Vec3 Ks = evalKs(zeta);
    Vec3 Ks_t = evalKs_t(zeta);
    return Ks_t/nrm2(Ks) - Ks*((Ks.T()*Ks_t)/pow(nrm2(Ks),3));
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalKrPS(const Vec2 &zeta) {
    Vec3 KrPS = evalHessianMatrixPos(zeta(0)).row(0).T()(Range<Fixed<0>,Fixed<2> >());
    for(size_t i=0; i<crvPhi.size(); i++)
      KrPS += hessPhi[i].row(0).T()(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(i);
    return KrPS;
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalKs(const Vec2 &zeta) {
    Vec3 s = evalHessianMatrixPos(zeta(0)).row(1).T()(Range<Fixed<0>,Fixed<2> >());
    for(size_t i=0; i<crvPhi.size(); i++)
      s += hessPhi[i].row(1).T()(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(i);
    return s;
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalKt(const Vec2 &zeta) {
    static Vec3 Kt("[0;0;1]");
    return Kt;
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalParDer1Ks(const Vec2 &zeta) {
    Vec3 ds = evalHessianMatrixPos(zeta(0)).row(2).T()(Range<Fixed<0>,Fixed<2> >());
    for(size_t i=0; i<crvPhi.size(); i++)
      ds += hessPhi[i].row(2).T()(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(i);
    return ds;
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalParDer1Ku(const Vec2 &zeta) {
    Vec3 Ks = evalKs(zeta);
    Vec3 parDer1Ks = evalParDer1Ks(zeta);
    return parDer1Ks/nrm2(Ks) - Ks*((Ks.T()*parDer1Ks)/pow(nrm2(Ks),3));
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalWs_t(const Vec2 &zeta) {
    return R->getOrientation()*evalKs_t(zeta);
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalWu_t(const Vec2 &zeta) {
    return R->getOrientation()*evalKu_t(zeta);
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalPosition(const Vec2 &zeta) {
    return R->evalPosition() + R->evalOrientation()*evalKrPS(zeta);
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalWs(const Vec2 &zeta) {
    return R->getOrientation()*evalKs(zeta);
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalWt(const Vec2 &zeta) {
    return R->getOrientation()*evalKt(zeta);
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalParDer1Ws(const Vec2 &zeta) {
    return R->getOrientation()*evalParDer1Ks(zeta);
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalParDer1Wu(const Vec2 &zeta) {
    return R->getOrientation()*evalParDer1Ku(zeta);
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalParWvCParEta(const Vec2 &zeta) {
    return crossProduct(R->evalAngularVelocity(),evalWs(zeta)) + evalWs_t(zeta);
  }

  Vec3 FlexiblePlanarFfrNurbsContour::evalParWuPart(const Vec2 &zeta) {
    return crossProduct(R->evalAngularVelocity(),evalWu(zeta)) + evalWu_t(zeta);
  }

  void FlexiblePlanarFfrNurbsContour::updatePositions(ContourFrame *frame) {
    throwError("(FlexiblePlanarFfrNurbsContour::updatePositions): not implemented");
  }

  void FlexiblePlanarFfrNurbsContour::updateVelocities(ContourFrame *frame) {
    frame->setVelocity(R->evalVelocity() + crossProduct(R->evalAngularVelocity(), evalGlobalRelativePosition(frame->evalEta())) + evalGlobalRelativeVelocity(frame->evalEta()));
  }

  void FlexiblePlanarFfrNurbsContour::updateAccelerations(ContourFrame *frame) {
    throwError("(FlexiblePlanarFfrNurbsContour::updateAccelerations): not implemented");
  }

  void FlexiblePlanarFfrNurbsContour::updateJacobians(ContourFrame *frame, int j) {
    if(fabs(frame->evalEta()-etaOld)>1e-13) updateHessianMatrix(frame->evalEta());
    Mat3xV Phi(crvPhi.size(),NONINIT);
    for(size_t i=0; i<crvPhi.size(); i++)
      Phi.set(i,hessPhi[i].row(0).T()(Range<Fixed<0>,Fixed<2> >()));
    Mat3xV J = R->evalJacobianOfTranslation(j) - tilde(evalGlobalRelativePosition(frame->evalEta()))*R->evalJacobianOfRotation(j);
    J.add(RangeV(0,2),RangeV(frame->gethSize(j)-crvPhi.size(),frame->gethSize(j)-1),R->getOrientation()*Phi);
    frame->setJacobianOfTranslation(J,j);
  }

  void FlexiblePlanarFfrNurbsContour::updateGyroscopicAccelerations(ContourFrame *frame) {
    frame->setGyroscopicAccelerationOfTranslation(R->evalGyroscopicAccelerationOfTranslation() + crossProduct(R->evalGyroscopicAccelerationOfRotation(),evalGlobalRelativePosition(frame->evalEta())) + crossProduct(R->evalAngularVelocity(),crossProduct(R->evalAngularVelocity(),evalGlobalRelativePosition(frame->evalEta()))) + 2.*crossProduct(R->evalAngularVelocity(),evalGlobalRelativeVelocity(frame->evalEta())));
  }

  void FlexiblePlanarFfrNurbsContour::init(InitStage stage, const InitConfigSet &config) {
    if (stage == preInit) {
      for(int i=0; i<index.size(); i++)
        index(i) = static_cast<NodeBasedBody*>(parent)->getNodeIndex(index(i));
      R = static_cast<GenericFlexibleFfrBody*>(parent)->getFrameK();
      crvPos.resize(index.size(),knot.size()-index.size()-1);
      crvPos.setDegree(knot.size()-index.size()-1);
      crvPos.setKnot(knot);
      MatVx4 cp(index.size());
      for(int i=0; i<index.size(); i++) {
        const Vec3 &x = static_cast<GenericFlexibleFfrBody*>(parent)->getNodalRelativePosition(index(i));
        for(int j=0; j<3; j++)
          cp(i,j) = x(j);
        cp(i,3) = 1;
      }
      if(not interpolation)
        crvPos.setCtrlPnts(cp);
      else {
        if(open)
          crvPos.globalInterpH(cp,degree,NurbsCurve::Method(NurbsCurve::equallySpaced));
        else
          crvPos.globalInterpClosedH(cp,degree,NurbsCurve::Method(NurbsCurve::equallySpaced));
      }
      crvPhi.resize(static_cast<GenericFlexibleFfrBody*>(parent)->getNumberOfModeShapes());
      hessPhi.resize(crvPhi.size());
      for(size_t k=0; k<crvPhi.size(); k++) {
        crvPhi[k].resize(index.size(),knot.size()-index.size()-1);
        crvPhi[k].setDegree(knot.size()-index.size()-1);
        crvPhi[k].setKnot(knot);
        for(int i=0; i<index.size(); i++) {
          const Vec3 &x = static_cast<GenericFlexibleFfrBody*>(parent)->getNodalShapeMatrixOfTranslation(index(i)).col(k);
          for(int j=0; j<3; j++)
            cp(i,j) = x(j);
          cp(i,3) = 1;
        }
        if(not interpolation)
          crvPhi[k].setCtrlPnts(cp);
        else {
          if(open)
            crvPhi[k].globalInterpH(cp,degree,NurbsCurve::Method(NurbsCurve::equallySpaced));
          else
            crvPhi[k].globalInterpClosedH(cp,degree,NurbsCurve::Method(NurbsCurve::equallySpaced));
        }
      }
      etaNodes.resize(2);
      etaNodes[0] = crvPos.knot()(crvPos.degree());
      etaNodes[1] = crvPos.knot()(crvPos.knot().size()-crvPos.degree()-1);
    }
    else if(stage==plotting) {
      if(plotFeature[openMBV] and openMBVNurbsCurve) {
        openMBVNurbsCurve->setName(name);

        openMBVNurbsCurve->setKnotVector(crvPos.knot());
        openMBVNurbsCurve->setNumberOfControlPoints(crvPos.ctrlPnts().rows());

        parent->getOpenMBVGrp()->addObject(openMBVNurbsCurve);
      }
    }
    FlexibleContour::init(stage, config);
  }

  ContourFrame* FlexiblePlanarFfrNurbsContour::createContourFrame(const string &name) {
    FloatingContourFrame *frame = new FloatingContourFrame(name);
    frame->setContourOfReference(this);
    return frame;
  }

  double FlexiblePlanarFfrNurbsContour::getCurvature(const Vec2 &zeta) {
    throwError("(FlexiblePlanarFfrNurbsContour::getCurvature): not implemented");
  }

  void FlexiblePlanarFfrNurbsContour::plot() {
    if(plotFeature[openMBV] and openMBVNurbsCurve) {
      vector<double> data;
      data.push_back(getTime()); //time
      //Control-Point coordinates
      for(int i=0; i<crvPos.ctrlPnts().rows(); i++) {
        Vec3 KrPS = crvPos.ctrlPnts().row(i).T()(Range<Fixed<0>,Fixed<2> >());
        for(size_t k=0; k<crvPhi.size(); k++)
          KrPS += crvPhi[k].ctrlPnts().row(i).T()(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(k);
        Vec3 r = R->evalPosition() + R->evalOrientation()*KrPS;
        for(int j=0; j<3; j++)
          data.push_back(r(j));
        data.push_back(1);
        data.push_back(0);
      }
      openMBVNurbsCurve->append(data);
    }
    FlexibleContour::plot();
  }

  void FlexiblePlanarFfrNurbsContour::initializeUsingXML(DOMElement * element) {
    FlexibleContour::initializeUsingXML(element);
    DOMElement * e;
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"interpolation");
    if(e) setInterpolation(E(e)->getText<bool>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"nodeNumbers");
    setNodeNumbers(E(e)->getText<VecVI>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"knotVector");
    if(e) setKnotVector(E(e)->getText<VecV>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"degree");
    if(e) setDegree(E(e)->getText<int>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"open");
    if(e) setOpen(E(e)->getText<bool>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"enableOpenMBV");
    if(e) {
      OpenMBVDynamicNurbsCurve ombv;
      ombv.initializeUsingXML(e);
      openMBVNurbsCurve=ombv.createOpenMBV();
    }
  }

  bool FlexiblePlanarFfrNurbsContour::isZetaOutside(const fmatvec::Vec2 &zeta) {
    return open and (zeta(0) < etaNodes[0] or zeta(0) > etaNodes[etaNodes.size()-1]); 
  }

}
