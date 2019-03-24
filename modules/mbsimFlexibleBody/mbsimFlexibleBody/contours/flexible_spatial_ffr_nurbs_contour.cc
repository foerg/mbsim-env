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
#include "mbsimFlexibleBody/contours/flexible_spatial_ffr_nurbs_contour.h"
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

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIMFLEX, FlexibleSpatialFfrNurbsContour)

  Vec2 FlexibleSpatialFfrNurbsContour::continueZeta(const Vec2 &zeta_) {
    Vec2 zeta(NONINIT);
    if(openEta)
      zeta = zeta_;
    else if(openXi) {
      zeta(0) = mod(zeta_(0)-etaNodes[0],etaNodes[1]-etaNodes[0])+etaNodes[0];
      zeta(1) = zeta_(1);
    }
    else {
      if(mod(zeta_(1)-xiNodes[0],2.*(xiNodes[1]-xiNodes[0]))+xiNodes[0]>xiNodes[1]) {
        zeta(0) = mod(zeta_(0)+0.5*(etaNodes[1]-etaNodes[0])-etaNodes[0],etaNodes[1]-etaNodes[0])+etaNodes[0];
        zeta(1) = xiNodes[1]-mod(zeta_(1)-xiNodes[0],xiNodes[1]-xiNodes[0]);
      }
      else {
        zeta(0) = mod(zeta_(0)-etaNodes[0],etaNodes[1]-etaNodes[0])+etaNodes[0];
        zeta(1) = mod(zeta_(1)-xiNodes[0],xiNodes[1]-xiNodes[0])+xiNodes[0];
      }
    }
    return zeta;
  }

  void FlexibleSpatialFfrNurbsContour::updateHessianMatrix(const Vec2 &zeta_) {
    Vec2 zeta = continueZeta(zeta_);
    srfPos.deriveAtH(zeta(0),zeta(1),2,hessPos);
    for(size_t i=0; i<srfPhi.size(); i++)
      srfPhi[i].deriveAtH(zeta(0),zeta(1),2,hessPhi[i]);
    zetaOld = zeta_;
  }

  void FlexibleSpatialFfrNurbsContour::updateGlobalRelativePosition(const Vec2 &zeta) {
    Vec3 KrPS = evalHessianMatrixPos(zeta)(0,0)(Range<Fixed<0>,Fixed<2> >());
    for(size_t i=0; i<srfPhi.size(); i++)
      KrPS += hessPhi[i](0,0)(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(i);
    WrPS = R->evalOrientation()*KrPS;
    updPos = false;
  }

  void FlexibleSpatialFfrNurbsContour::updateGlobalRelativeVelocity(const Vec2 &zeta) {
    if(fabs(zeta(0)-zetaOld(0))>1e-13 or fabs(zeta(1)-zetaOld(1))>1e-13) updateHessianMatrix(zeta);
    Vec3 Kvrel;
    for(size_t i=0; i<srfPhi.size(); i++)
      Kvrel += hessPhi[i](0,0)(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqdERel()(i);
    Wvrel = R->evalOrientation()*Kvrel;
    updVel = false;
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalKn_t(const Vec2 &zeta) {
    Vec3 Ksxt = crossProduct(evalKs(zeta),evalKt(zeta));
    Vec3 Ksxt_t = crossProduct(evalKs_t(zeta),evalKt(zeta)) + crossProduct(evalKs(zeta),evalKt_t(zeta));
    return Ksxt_t/nrm2(Ksxt) - Ksxt*((Ksxt.T()*Ksxt_t)/pow(nrm2(Ksxt),3));
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalKs_t(const Vec2 &zeta) {
    if(fabs(zeta(0)-zetaOld(0))>1e-13 or fabs(zeta(1)-zetaOld(1))>1e-13) updateHessianMatrix(zeta);
    Vec3 s_t;
    for(size_t i=0; i<srfPhi.size(); i++)
      s_t += hessPhi[i](1,0)(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqdERel()(i);
    return s_t;
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalKt_t(const Vec2 &zeta) {
    if(fabs(zeta(0)-zetaOld(0))>1e-13 or fabs(zeta(1)-zetaOld(1))>1e-13) updateHessianMatrix(zeta);
    Vec3 t_t;
    for(size_t i=0; i<srfPhi.size(); i++)
      t_t += hessPhi[i](0,1)(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqdERel()(i);
    return t_t;
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalKu_t(const Vec2 &zeta) {
    Vec3 Ks = evalKs(zeta);
    Vec3 Ks_t = evalKs_t(zeta);
    return Ks_t/nrm2(Ks) - Ks*((Ks.T()*Ks_t)/pow(nrm2(Ks),3));
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalKv_t(const Vec2 &zeta) {
    return crossProduct(evalKn_t(zeta),evalKu(zeta)) + crossProduct(evalKn(zeta),evalKu_t(zeta));
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalKrPS(const Vec2 &zeta) {
    Vec3 KrPS = evalHessianMatrixPos(zeta)(0,0)(Range<Fixed<0>,Fixed<2> >());
    for(size_t i=0; i<srfPhi.size(); i++)
      KrPS += hessPhi[i](0,0)(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(i);
    return KrPS;
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalKs(const Vec2 &zeta) {
    Vec3 s = evalHessianMatrixPos(zeta)(1,0)(Range<Fixed<0>,Fixed<2> >());
    for(size_t i=0; i<srfPhi.size(); i++)
      s += hessPhi[i](1,0)(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(i);
    return s;
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalKt(const Vec2 &zeta) {
    Vec3 t = evalHessianMatrixPos(zeta)(0,1)(Range<Fixed<0>,Fixed<2> >());
    for(size_t i=0; i<srfPhi.size(); i++)
      t += hessPhi[i](0,1)(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(i);
    return t;
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalKu(const Vec2 &zeta) {
    Vec3 Ks=evalKs(zeta);
    return Ks/nrm2(Ks);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalKv(const Vec2 &zeta) {
    return crossProduct(evalKn(zeta),evalKu(zeta));;
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalKn(const Vec2 &zeta) {
    Vec3 Kn = crossProduct(evalKs(zeta),evalKt(zeta));
    return Kn/nrm2(Kn);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer1Ks(const Vec2 &zeta) {
    Vec3 ds = evalHessianMatrixPos(zeta)(2,0)(Range<Fixed<0>,Fixed<2> >());
    for(size_t i=0; i<srfPhi.size(); i++)
      ds += hessPhi[i](2,0)(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(i);
    return ds;
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer2Ks(const Vec2 &zeta) {
    Vec3 ds = evalHessianMatrixPos(zeta)(1,1)(Range<Fixed<0>,Fixed<2> >());
    for(size_t i=0; i<srfPhi.size(); i++)
      ds += hessPhi[i](1,1)(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(i);
    return ds;
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer1Kt(const Vec2 &zeta) {
    Vec3 dt = evalHessianMatrixPos(zeta)(1,1)(Range<Fixed<0>,Fixed<2> >());
    for(size_t i=0; i<srfPhi.size(); i++)
      dt += hessPhi[i](1,1)(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(i);
    return dt;
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer2Kt(const Vec2 &zeta) {
    Vec3 dt = evalHessianMatrixPos(zeta)(0,2)(Range<Fixed<0>,Fixed<2> >());
    for(size_t i=0; i<srfPhi.size(); i++)
      dt += hessPhi[i](0,2)(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(i);
    return dt;
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer1Ku(const Vec2 &zeta) {
    Vec3 Ks = evalKs(zeta);
    Vec3 parDer1Ks = evalParDer1Ks(zeta);
    return parDer1Ks/nrm2(Ks) - Ks*((Ks.T()*parDer1Ks)/pow(nrm2(Ks),3));
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer2Ku(const Vec2 &zeta) {
    Vec3 Ks = evalKs(zeta);
    Vec3 parDer2Ks = evalParDer2Ks(zeta);
    return parDer2Ks/nrm2(Ks) - Ks*((Ks.T()*parDer2Ks)/pow(nrm2(Ks),3));
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer1Kv(const Vec2 &zeta) {
    return crossProduct(evalParDer1Kn(zeta),evalKu(zeta)) + crossProduct(evalKn(zeta),evalParDer1Ku(zeta));
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer2Kv(const Vec2 &zeta) {
    return crossProduct(evalParDer2Kn(zeta),evalKu(zeta)) + crossProduct(evalKn(zeta),evalParDer2Ku(zeta));
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer1Kn(const Vec2 &zeta) {
    Vec3 Ksxt = crossProduct(evalKs(zeta),evalKt(zeta));
    Vec3 Ksxtd = crossProduct(evalParDer1Ks(zeta),evalKt(zeta)) + crossProduct(evalKs(zeta),evalParDer1Kt(zeta));
    return Ksxtd/nrm2(Ksxt) - Ksxt*((Ksxt.T()*Ksxtd)/pow(nrm2(Ksxt),3));
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer2Kn(const Vec2 &zeta) {
    Vec3 Ksxt = crossProduct(evalKs(zeta),evalKt(zeta));
    Vec3 Ksxtd = crossProduct(evalParDer2Ks(zeta),evalKt(zeta)) + crossProduct(evalKs(zeta),evalParDer2Kt(zeta));
    return Ksxtd/nrm2(Ksxt) - Ksxt*((Ksxt.T()*Ksxtd)/pow(nrm2(Ksxt),3));
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalWn_t(const Vec2 &zeta) {
    return R->getOrientation()*evalKn_t(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalWs_t(const Vec2 &zeta) {
    return R->getOrientation()*evalKs_t(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalWt_t(const Vec2 &zeta) {
    return R->getOrientation()*evalKt_t(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalWu_t(const Vec2 &zeta) {
    return R->getOrientation()*evalKu_t(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalWv_t(const Vec2 &zeta) {
    return R->getOrientation()*evalKv_t(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalPosition(const Vec2 &zeta) {
    return R->evalPosition() + R->evalOrientation()*evalKrPS(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalWs(const Vec2 &zeta) {
    return R->getOrientation()*evalKs(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalWt(const Vec2 &zeta) {
    return R->getOrientation()*evalKt(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer1Ws(const Vec2 &zeta) {
    return R->getOrientation()*evalParDer1Ks(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer2Ws(const Vec2 &zeta) {
    return R->getOrientation()*evalParDer2Ks(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer1Wt(const Vec2 &zeta) {
    return R->getOrientation()*evalParDer1Kt(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer2Wt(const Vec2 &zeta) {
    return R->getOrientation()*evalParDer2Kt(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer1Wu(const Vec2 &zeta) {
    return R->getOrientation()*evalParDer1Ku(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer2Wu(const Vec2 &zeta) {
    return R->getOrientation()*evalParDer2Ku(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer1Wv(const Vec2 &zeta) {
    return R->getOrientation()*evalParDer1Kv(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer2Wv(const Vec2 &zeta) {
    return R->getOrientation()*evalParDer2Kv(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer1Wn(const Vec2 &zeta) {
    return R->evalOrientation()*evalParDer1Kn(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParDer2Wn(const Vec2 &zeta) {
    return R->evalOrientation()*evalParDer2Kn(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParWvCParEta(const Vec2 &zeta) {
    return crossProduct(R->evalAngularVelocity(),evalWs(zeta)) + evalWs_t(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParWvCParXi(const Vec2 &zeta) {
    return crossProduct(R->evalAngularVelocity(),evalWt(zeta)) + evalWt_t(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParWuPart(const Vec2 &zeta) {
    return crossProduct(R->evalAngularVelocity(),evalWu(zeta)) + evalWu_t(zeta);
  }

  Vec3 FlexibleSpatialFfrNurbsContour::evalParWvPart(const Vec2 &zeta) {
    return crossProduct(R->evalAngularVelocity(),evalWv(zeta)) + evalWv_t(zeta);
  }

  void FlexibleSpatialFfrNurbsContour::updatePositions(ContourFrame *frame) {
    throwError("(FlexibleSpatialFfrNurbsContour::updatePositions): not implemented");
  }

  void FlexibleSpatialFfrNurbsContour::updateVelocities(ContourFrame *frame) {
    frame->setVelocity(R->evalVelocity() + crossProduct(R->evalAngularVelocity(), evalGlobalRelativePosition(frame->evalZeta())) + evalGlobalRelativeVelocity(frame->evalZeta()));
  }

  void FlexibleSpatialFfrNurbsContour::updateAccelerations(ContourFrame *frame) {
    throwError("(FlexibleSpatialFfrNurbsContour::updateAccelerations): not implemented");
  }

  void FlexibleSpatialFfrNurbsContour::updateJacobians(ContourFrame *frame, int j) {
    if(fabs(frame->evalZeta()(0)-zetaOld(0))>1e-13 or fabs(frame->evalZeta()(1)-zetaOld(1))>1e-13) updateHessianMatrix(frame->evalZeta());
    Mat3xV Phi(srfPhi.size(),NONINIT);
    for(size_t i=0; i<srfPhi.size(); i++)
      Phi.set(i,hessPhi[i](0,0)(Range<Fixed<0>,Fixed<2> >()));
    Mat3xV J = R->evalJacobianOfTranslation(j) - tilde(evalGlobalRelativePosition(frame->evalZeta()))*R->evalJacobianOfRotation(j);
    J.add(RangeV(0,2),RangeV(frame->gethSize(j)-srfPhi.size(),frame->gethSize(j)-1),R->getOrientation()*Phi);
    frame->setJacobianOfTranslation(J,j);
  }

  void FlexibleSpatialFfrNurbsContour::updateGyroscopicAccelerations(ContourFrame *frame) {
    frame->setGyroscopicAccelerationOfTranslation(R->evalGyroscopicAccelerationOfTranslation() + crossProduct(R->evalGyroscopicAccelerationOfRotation(),evalGlobalRelativePosition(frame->evalZeta())) + crossProduct(R->evalAngularVelocity(),crossProduct(R->evalAngularVelocity(),evalGlobalRelativePosition(frame->evalZeta()))) + 2.*crossProduct(R->evalAngularVelocity(),evalGlobalRelativeVelocity(frame->evalZeta())));
  }

  void FlexibleSpatialFfrNurbsContour::init(InitStage stage, const InitConfigSet &config) {
    if (stage == preInit) {
      for(int i=0; i<index.rows(); i++)
        for(int j=0; j<index.cols(); j++)
          index(i,j) = static_cast<NodeBasedBody*>(parent)->getNodeIndex(index(i,j));
      R = static_cast<GenericFlexibleFfrBody*>(parent)->getFrameK();
      srfPos.resize(index.rows(),index.cols(),uKnot.size()-index.rows()-1,vKnot.size()-index.cols()-1);
      srfPos.setDegreeU(uKnot.size()-index.rows()-1);
      srfPos.setDegreeV(vKnot.size()-index.cols()-1);
      srfPos.setKnotU(uKnot);
      srfPos.setKnotV(vKnot);
      GeneralMatrix<Vec4> cp(index.rows(),index.cols());
      for(int i=0; i<index.rows(); i++) {
        for(int j=0; j<index.cols(); j++) {
          cp(i,j).set(RangeV(0,2),static_cast<GenericFlexibleFfrBody*>(parent)->getNodalRelativePosition(index(i,j)));
          cp(i,j)(3) = 1;
        }
      }
      if(not interpolation)
        srfPos.setCtrlPnts(cp);
      else {
        if(openEta)
          srfPos.globalInterpH(cp,etaDegree,xiDegree,NurbsSurface::Method(NurbsSurface::equallySpaced));
        else
          srfPos.globalInterpClosedUH(cp,etaDegree,xiDegree,NurbsSurface::Method(NurbsSurface::equallySpaced));
      }
      srfPhi.resize(static_cast<GenericFlexibleFfrBody*>(parent)->getNumberOfModeShapes());
      hessPhi.resize(srfPhi.size());
      for(size_t k=0; k<srfPhi.size(); k++) {
        srfPhi[k].resize(index.rows(),index.cols(),uKnot.size()-index.rows()-1,vKnot.size()-index.cols()-1);
        srfPhi[k].setDegreeU(uKnot.size()-index.rows()-1);
        srfPhi[k].setDegreeV(vKnot.size()-index.cols()-1);
        srfPhi[k].setKnotU(uKnot);
        srfPhi[k].setKnotV(vKnot);
        for(int i=0; i<index.rows(); i++) {
          for(int j=0; j<index.cols(); j++) {
            cp(i,j).set(RangeV(0,2),static_cast<GenericFlexibleFfrBody*>(parent)->getNodalShapeMatrixOfTranslation(index(i,j)).col(k));
            cp(i,j)(3) = 1;
          }
        }
        if(not interpolation)
          srfPhi[k].setCtrlPnts(cp);
        else {
          if(openEta)
            srfPhi[k].globalInterpH(cp,etaDegree,xiDegree,NurbsSurface::Method(NurbsSurface::equallySpaced));
          else
            srfPhi[k].globalInterpClosedUH(cp,etaDegree,xiDegree,NurbsSurface::Method(NurbsSurface::equallySpaced));
        }
      }
      if(not interpolation) {
        srfPos.resize(index.rows(),index.cols(),uKnot.size()-index.rows()-1,vKnot.size()-index.cols()-1);
        srfPos.setDegreeU(uKnot.size()-index.rows()-1);
        srfPos.setDegreeV(vKnot.size()-index.cols()-1);
        srfPos.setKnotU(uKnot);
        srfPos.setKnotV(vKnot);
      }
      else {
        VecV uk(index.rows(),NONINIT), vk(index.cols(),NONINIT), U, V;
        if(openEta) {
          srfPos.resize(index.rows(),index.cols(),etaDegree,xiDegree);
          U.resize(srfPos.knotU().size(),NONINIT);
          V.resize(srfPos.knotV().size(),NONINIT);
          updateUVecs(0, 1, uk, etaDegree, U);
          updateUVecs(0, 1, vk, xiDegree, V);
          if(not openXi)
            throwError("(FlexibleSpatialFfrNurbsContour::init): contour with open eta and closed xi not allowed");
        }
        else {
          srfPos.resize(index.rows()+etaDegree,index.cols(),etaDegree,xiDegree);
          U.resize(srfPos.knotU().size(),NONINIT);
          V.resize(srfPos.knotV().size(),NONINIT);
          updateUVecsClosed(0, 1, uk, etaDegree, U);
          updateUVecs(0, 1, vk, xiDegree, V);
        }
        srfPos.setKnotU(U);
        srfPos.setKnotV(V);
      }

      zetaOld.init(-1e10);
      etaNodes.resize(2);
      etaNodes[0] = srfPos.knotU()(srfPos.degreeU());
      etaNodes[1] = srfPos.knotU()(srfPos.knotU().size()-srfPos.degreeU()-1);
      xiNodes.resize(2);
      xiNodes[0] = srfPos.knotV()(srfPos.degreeV());
      xiNodes[1] = srfPos.knotV(srfPos.knotV().size()-srfPos.degreeV()-1);
    }
    else if(stage==plotting) {
      if(plotFeature[openMBV] and openMBVNurbsSurface) {
        openMBVNurbsSurface->setName(name);

        openMBVNurbsSurface->setUKnotVector(srfPos.knotU());
        openMBVNurbsSurface->setVKnotVector(srfPos.knotV());
        openMBVNurbsSurface->setNumberOfUControlPoints(srfPos.ctrlPnts().rows());
        openMBVNurbsSurface->setNumberOfVControlPoints(srfPos.ctrlPnts().cols());

        parent->getOpenMBVGrp()->addObject(openMBVNurbsSurface);
      }
    }
    FlexibleContour::init(stage, config);
  }

  ContourFrame* FlexibleSpatialFfrNurbsContour::createContourFrame(const string &name) {
    FloatingContourFrame *frame = new FloatingContourFrame(name);
    frame->setContourOfReference(this);
    return frame;
  }

  double FlexibleSpatialFfrNurbsContour::getCurvature(const Vec2 &zeta) {
    throwError("(FlexibleSpatialFfrNurbsContour::getCurvature): not implemented");
  }

  void FlexibleSpatialFfrNurbsContour::plot() {
    if(plotFeature[openMBV] and openMBVNurbsSurface) {
      vector<double> data;
      data.push_back(getTime()); //time
      //Control-Point coordinates
      for(int j=0; j<srfPos.ctrlPnts().cols(); j++) {
        for(int i=0; i<srfPos.ctrlPnts().rows(); i++) {
          Vec3 KrKP = srfPos.ctrlPnts()(i,j)(Range<Fixed<0>,Fixed<2> >());
          for(size_t k=0; k<srfPhi.size(); k++)
            KrKP += srfPhi[k].ctrlPnts()(i,j)(Range<Fixed<0>,Fixed<2> >())*static_cast<GenericFlexibleFfrBody*>(parent)->evalqERel()(k);
          Vec3 r = R->evalPosition() + R->evalOrientation()*KrKP;
          for(int k=0; k<3; k++)
            data.push_back(r(k));
          data.push_back(1);
          data.push_back(0);
        }
      }
      openMBVNurbsSurface->append(data);
    }
    FlexibleContour::plot();
  }

  void FlexibleSpatialFfrNurbsContour::initializeUsingXML(DOMElement * element) {
    FlexibleContour::initializeUsingXML(element);
    DOMElement * e;
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"interpolation");
    if(e) setInterpolation(E(e)->getText<bool>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"nodeNumbers");
    setNodeNumbers(E(e)->getText<MatVI>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"etaKnotVector");
    if(e) setEtaKnotVector(E(e)->getText<VecV>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"xiKnotVector");
    if(e) setXiKnotVector(E(e)->getText<VecV>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"etaDegree");
    if(e) setEtaDegree(E(e)->getText<int>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"xiDegree");
    if(e) setXiDegree(E(e)->getText<int>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"openEta");
    if(e) setOpenEta(E(e)->getText<bool>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"openXi");
    if(e) setOpenXi(E(e)->getText<bool>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"enableOpenMBV");
    if(e) {
      OpenMBVDynamicNurbsSurface ombv;
      ombv.initializeUsingXML(e);
      openMBVNurbsSurface=ombv.createOpenMBV();
    }
  }

  bool FlexibleSpatialFfrNurbsContour::isZetaOutside(const fmatvec::Vec2 &zeta) {
    if(openEta and (zeta(0) < etaNodes[0] or zeta(0) > etaNodes[etaNodes.size()-1]))
      return true;
    if(openXi and (zeta(1) < xiNodes[0] or zeta(1) > xiNodes[xiNodes.size()-1]))
      return true;
    return false;
  }

}
