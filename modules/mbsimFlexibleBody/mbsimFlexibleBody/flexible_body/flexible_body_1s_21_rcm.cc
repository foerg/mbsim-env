/* Copyright (C) 2004-2015 MBSim Development Team
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
 * Contact: thorsten.schindler@mytum.de
 */

#include <config.h>
#include "mbsimFlexibleBody/flexible_body/flexible_body_1s_21_rcm.h"
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_21_rcm.h"
#include "mbsim/frames/contour_frame.h"
#include "mbsimFlexibleBody/frames/frame_1s.h"
#include "mbsimFlexibleBody/frames/node_frame.h"
#include "mbsim/mbsim_event.h"
#include "mbsim/utils/utils.h"
#include "mbsim/utils/eps.h"
#include "mbsim/environment.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/dynamic_system_solver.h"

#include "nurbs++/nurbs.h"
#include "nurbs++/vector.h"

using namespace PLib;

using namespace fmatvec;
using namespace std;
using namespace MBSim;

namespace MBSimFlexibleBody {

  FlexibleBody1s21RCM::FlexibleBody1s21RCM(const string &name, bool openStructure) : FlexibleBody1s(name,openStructure), Elements(0), l0(0), E(0), A(0), I(0), rho(0), rc(0), dm(0), dl(0), initialized(false) {
  }

  void FlexibleBody1s21RCM::BuildElements() {
    for (int i = 0; i < Elements; i++) {
      int n = 5 * i;

      if (i < Elements - 1 || openStructure == true) {
        qElement[i] = q(RangeV(n, n + 7));
        uElement[i] = u(RangeV(n, n + 7));
      }
      else { // last finite element and ring closure
        qElement[i].set(RangeV(0, 4), q(RangeV(n, n + 4)));
        uElement[i].set(RangeV(0, 4), u(RangeV(n, n + 4)));
        qElement[i].set(RangeV(5, 7), q(RangeV(0, 2)));
        if (qElement[i](2) - q(2) > 0.0)
          qElement[i](7) += 2 * M_PI;
        else
          qElement[i](7) -= 2 * M_PI;
        uElement[i].set(RangeV(5, 7), u(RangeV(0, 2)));
      }
    }
    updEle = false;
  }

  void FlexibleBody1s21RCM::GlobalVectorContribution(int n, const fmatvec::Vec& locVec, fmatvec::Vec& gloVec) {
    int j = 5 * n;

    if (n < Elements - 1 || openStructure == true) {
      gloVec.add(RangeV(j, j + 7), locVec);
    }
    else { // ring closure at finite element (end,1) with angle difference 2*M_PI
      gloVec.add(RangeV(j, j + 4), locVec(RangeV(0, 4)));
      gloVec.add(RangeV(0, 2), locVec(RangeV(5, 7)));
    }
  }

  void FlexibleBody1s21RCM::GlobalMatrixContribution(int n, const fmatvec::Mat& locMat, fmatvec::Mat& gloMat) {
    int j = 5 * n;

    if (n < Elements - 1 || openStructure == true) {
      gloMat.add(RangeV(j, j + 7), RangeV(j, j + 7), locMat);
    }
    else { // ring closure at finite element (end,1) with angle difference 2*M_PI
      gloMat.add(RangeV(j, j + 4), RangeV(j, j + 4), locMat(RangeV(0, 4), RangeV(0, 4)));
      gloMat.add(RangeV(j, j + 4), RangeV(0, 2), locMat(RangeV(0, 4), RangeV(5, 7)));
      gloMat.add(RangeV(0, 2), RangeV(j, j + 4), locMat(RangeV(5, 7), RangeV(0, 4)));
      gloMat.add(RangeV(0, 2), RangeV(0, 2), locMat(RangeV(5, 7), RangeV(5, 7)));
    }
  }

  void FlexibleBody1s21RCM::GlobalMatrixContribution(int n, const fmatvec::SymMat& locMat, fmatvec::SymMat& gloMat) {
    int j = 5 * n;

    if (n < Elements - 1 || openStructure == true) {
      gloMat.add(RangeV(j, j + 7), locMat);
    }
    else { // ring closure at finite element (end,1) with angle difference 2*M_PI
      gloMat.add(RangeV(j, j + 4), locMat(RangeV(0, 4)));
      gloMat.add(RangeV(j, j + 4), RangeV(0, 2), locMat(RangeV(0, 4), RangeV(5, 7)));
      gloMat.add(RangeV(0, 2), locMat(RangeV(5, 7)));
    }
  }

  void FlexibleBody1s21RCM::updatePositions(Frame1s *frame) {
    Vec3 tmp(NONINIT);
    Vec3 X = getPositions(frame->getParameter());
    tmp(0) = X(0);
    tmp(1) = X(1);
    tmp(2) = 0.; // temporary vector used for compensating planar description
    frame->setPosition(R->evalPosition() + R->evalOrientation() * tmp);
    tmp(0) = cos(X(2));
    tmp(1) = sin(X(2));
    frame->getOrientation(false).set(0, R->getOrientation() * tmp);
    tmp(0) = -sin(X(2));
    tmp(1) = cos(X(2));
    frame->getOrientation(false).set(1, R->getOrientation() * tmp);
    frame->getOrientation(false).set(2, R->getOrientation().col(2));
  }

  void FlexibleBody1s21RCM::updateVelocities(Frame1s *frame) {
    Vec3 tmp(NONINIT);
    Vec3 X = getVelocities(frame->getParameter());
    tmp(0) = X(0);
    tmp(1) = X(1);
    tmp(2) = 0.;
    frame->setVelocity(R->evalOrientation() * tmp);
    tmp(0) = 0.;
    tmp(1) = 0.;
    tmp(2) = X(2);
    frame->setAngularVelocity(R->evalOrientation() * tmp);
  }

  void FlexibleBody1s21RCM::updateAccelerations(Frame1s *frame) {
    throwError("(FlexibleBody1s21RCM::updateAccelerations): Not implemented.");
  }

  void FlexibleBody1s21RCM::updateJacobians(Frame1s *frame, int j) {
    RangeV All(0, 3 - 1);
    Mat Jacobian(qSize, 3, INIT, 0.);

    double sLocal;
    int currentElement;
    BuildElement(frame->getParameter(), sLocal, currentElement);
    Mat Jtmp = static_cast<FiniteElement1s21RCM*>(discretization[currentElement])->JGeneralized(getqElement(currentElement), sLocal);
    if (currentElement < Elements - 1 || openStructure) {
      Jacobian.set(RangeV(5 * currentElement, 5 * currentElement + 7), All, Jtmp);
    }
    else { // ringstructure
      Jacobian.set(RangeV(5 * currentElement, 5 * currentElement + 4), All, Jtmp(RangeV(0, 4), All));
      Jacobian.set(RangeV(0, 2), All, Jtmp(RangeV(5, 7), All));
    }

    frame->setJacobianOfTranslation(R->evalOrientation()(RangeV(0, 2), RangeV(0, 1)) * Jacobian(RangeV(0, qSize - 1), RangeV(0, 1)).T(),j);
    frame->setJacobianOfRotation(R->evalOrientation()(RangeV(0, 2), RangeV(2, 2)) * Jacobian(RangeV(0, qSize - 1), RangeV(2, 2)).T(),j);
  }

  void FlexibleBody1s21RCM::updateGyroscopicAccelerations(Frame1s *frame) {
    throwError("(FlexibleBody1s21RCM::updateGyroscopicAccelerations): Not implemented.");
  }

  void FlexibleBody1s21RCM::updatePositions(NodeFrame *frame) {
    Vec3 tmp(NONINIT);
    int node = frame->getNodeNumber();
    tmp(0) = q(5 * node + 0);
    tmp(1) = q(5 * node + 1);
    tmp(2) = 0.; // temporary vector used for compensating planar description
    frame->setPosition(R->evalPosition() + R->evalOrientation() * tmp);
    tmp(0) = cos(q(5 * node + 2));
    tmp(1) = sin(q(5 * node + 2));
    frame->getOrientation(false).set(0, R->getOrientation() * tmp);
    tmp(0) = -sin(q(5 * node + 2));
    tmp(1) = cos(q(5 * node + 2));
    frame->getOrientation(false).set(1, R->getOrientation() * tmp);
    frame->getOrientation(false).set(2, R->getOrientation().col(2));
  }

  void FlexibleBody1s21RCM::updateVelocities(NodeFrame *frame) {
    Vec3 tmp(NONINIT);
    int node = frame->getNodeNumber();
    tmp(0) = u(5 * node + 0);
    tmp(1) = u(5 * node + 1);
    tmp(2) = 0.;
    frame->setVelocity(R->evalOrientation() * tmp);
    tmp(0) = 0.;
    tmp(1) = 0.;
    tmp(2) = u(5 * node + 2);
    frame->setAngularVelocity(R->evalOrientation() * tmp);
  }

  void FlexibleBody1s21RCM::updateAccelerations(NodeFrame *frame) {
    throwError("(FlexibleBody1s21RCM::updateAccelerations): Not implemented.");
  }

  void FlexibleBody1s21RCM::updateJacobians(NodeFrame *frame, int j) {
    RangeV All(0, 3 - 1);
    Mat Jacobian(qSize, 3, INIT, 0.);
    int node = frame->getNodeNumber();

    Jacobian.set(RangeV(5 * node, 5 * node + 2), All, DiagMat(3, INIT, 1.0));
    Jacobian.set(RangeV(5 * node, 5 * node + 2), All, DiagMat(3, INIT, 1.0));

    frame->setJacobianOfTranslation(R->evalOrientation()(RangeV(0, 2), RangeV(0, 1)) * Jacobian(RangeV(0, qSize - 1), RangeV(0, 1)).T(),j);
    frame->setJacobianOfRotation(R->evalOrientation()(RangeV(0, 2), RangeV(2, 2)) * Jacobian(RangeV(0, qSize - 1), RangeV(2, 2)).T(),j);
  }

  void FlexibleBody1s21RCM::updateGyroscopicAccelerations(NodeFrame *frame) {
    throwError("(FlexibleBody1s21RCM::updateGyroscopicAccelerations): Not implemented.");
  }

  void FlexibleBody1s21RCM::init(InitStage stage, const InitConfigSet &config) {
    if (stage == unknownStage) {
      FlexibleBody1s::init(stage, config);

      initialized = true;

      l0 = L / Elements;
      Vec g = R->getOrientation()(RangeV(0, 2), RangeV(0, 1)).T() * ds->getMBSimEnvironment()->getAccelerationOfGravity();
      for (int i = 0; i < Elements; i++) {
        qElement.emplace_back(8, INIT, 0.);
        uElement.emplace_back(8, INIT, 0.);
        discretization.push_back(new FiniteElement1s21RCM(l0, A * rho, E * A, E * I, g));
        if (fabs(rc) > epsroot)
          static_cast<FiniteElement1s21RCM*>(discretization[i])->setCurlRadius(rc);
        static_cast<FiniteElement1s21RCM*>(discretization[i])->setMaterialDamping(dm);
        static_cast<FiniteElement1s21RCM*>(discretization[i])->setLehrDamping(dl);
      }
    }
    else if (stage == plotting) {
      for (int i = 0; i < plotElements.size(); i++) {
        addToPlot("eps (" + toString(plotElements(i)) + ")"); // 0
        addToPlot("epsp(" + toString(plotElements(i)) + ")"); // 1
        addToPlot("xS  (" + toString(plotElements(i)) + ")"); // 2
        addToPlot("yS  (" + toString(plotElements(i)) + ")"); // 3
        addToPlot("xSp (" + toString(plotElements(i)) + ")"); // 4
        addToPlot("ySp (" + toString(plotElements(i)) + ")"); // 5
        addToPlot("Dal (" + toString(plotElements(i)) + ")"); // 6
        addToPlot("Dalp(" + toString(plotElements(i)) + ")"); // 7
      }
      FlexibleBody1s::init(stage, config);
    }
    else
      FlexibleBody1s::init(stage, config);
  }

  void FlexibleBody1s21RCM::plot() {
    for (int i = 0; i < plotElements.size(); i++) {
      Vec elementData = static_cast<FiniteElement1s21RCM*>(discretization[i])->computeAdditionalElementData(getqElement(plotElements(i)), getuElement(plotElements(i)));
      for (int j = 0; j < elementData.size(); j++)
        Element::plot(elementData(j));
    }
    FlexibleBody1s::plot();
  }

  void FlexibleBody1s21RCM::setNumberElements(int n) {
    Elements = n;
    if (openStructure)
      qSize = 5 * n + 3;
    else
      qSize = 5 * n;
    uSize[0] = qSize;
    uSize[1] = qSize; // TODO
    q0.resize(qSize);
    u0.resize(uSize[0]);
  }

  void FlexibleBody1s21RCM::setCurlRadius(double r) {
    rc = r;
    if (initialized)
      for (int i = 0; i < Elements; i++)
        static_cast<FiniteElement1s21RCM*>(discretization[i])->setCurlRadius(rc);
  }

  void FlexibleBody1s21RCM::setMaterialDamping(double d) {
    dm = d;
    if (initialized)
      for (int i = 0; i < Elements; i++)
        static_cast<FiniteElement1s21RCM*>(discretization[i])->setMaterialDamping(dm);
  }

  void FlexibleBody1s21RCM::setLehrDamping(double d) {
    dl = d;
    if (initialized)
      for (int i = 0; i < Elements; i++)
        static_cast<FiniteElement1s21RCM*>(discretization[i])->setLehrDamping(dl);
  }

  Vec3 FlexibleBody1s21RCM::getPositions(double sGlobal) {
    double sLocal;
    int currentElement;
    BuildElement(sGlobal, sLocal, currentElement); // Lagrange parameter of affected FE
    return static_cast<FiniteElement1s21RCM*>(discretization[currentElement])->getPositions(getqElement(currentElement), sLocal);
  }

  Vec3 FlexibleBody1s21RCM::getVelocities(double sGlobal) {
    double sLocal;
    int currentElement;
    BuildElement(sGlobal, sLocal, currentElement); // Lagrange parameter of affected FE
    return static_cast<FiniteElement1s21RCM*>(discretization[currentElement])->getVelocities(getqElement(currentElement), getuElement(currentElement), sLocal);
  }

  double FlexibleBody1s21RCM::computePhysicalStrain(double sGlobal) {
    double sLocal;
    int currentElement;
    BuildElement(sGlobal, sLocal, currentElement); // Lagrange parameter of affected FE
    return static_cast<FiniteElement1s21RCM*>(discretization[currentElement])->computePhysicalStrain(getqElement(currentElement));
  }

  void FlexibleBody1s21RCM::initRelaxed(double alpha) {
    if (!initialized) {
      if (Elements == 0)
        throwError("(FlexibleBody1s21RCM::initRelaxed): Set number of finite elements!");
      Vec q0Dummy(q0.size(), INIT, 0.);
      if (openStructure) {
        Vec direction(2);
        direction(0) = cos(alpha);
        direction(1) = sin(alpha);

        for (int i = 0; i <= Elements; i++) {
          q0Dummy.set(RangeV(5 * i + 0, 5 * i + 1), direction * double(L / Elements * i));
          q0Dummy(5 * i + 2) = alpha;
        }
      }
      else {
        double R = L / (2 * M_PI);
        double a_ = sqrt(R * R + (L / Elements * L / Elements) / 16.) - R;

        for (int i = 0; i < Elements; i++) {
          double alpha_ = i * (2 * M_PI) / Elements;
          q0Dummy(5 * i + 0) = R * cos(alpha_);
          q0Dummy(5 * i + 1) = R * sin(alpha_);
          q0Dummy(5 * i + 2) = alpha_ + M_PI / 2.;
          q0Dummy(5 * i + 3) = a_;
          q0Dummy(5 * i + 4) = a_;
        }
      }
      setq0(q0Dummy);
      setu0(Vec(q0Dummy.size(), INIT, 0.));
    }
  }

  void FlexibleBody1s21RCM::initInfo() {
    FlexibleBody1s::init(unknownStage, InitConfigSet());
    l0 = L / Elements;
    Vec g = Vec("[0.;0.;0.]");
    for (int i = 0; i < Elements; i++) {
      discretization.push_back(new FiniteElement1s21RCM(l0, A * rho, E * A, E * I, g));
      qElement.emplace_back(discretization[0]->getqSize(), INIT, 0.);
      uElement.emplace_back(discretization[0]->getuSize(), INIT, 0.);
    }
  }

  void FlexibleBody1s21RCM::exportPositionVelocity(const string& filenamePos, const string& filenameVel /*= string( )*/, const int & deg /* = 3*/, const bool &writePsFile /*= false*/) {

    //    PlNurbsCurved curvePos;
    //    PlNurbsCurved curveVel;
    //
    //    if (!openStructure) {
    //      PLib::Vector<PLib::HPoint3Dd> NodelistPos(Elements + deg);
    //      PLib::Vector<PLib::HPoint3Dd> NodelistVel(Elements + deg);
    //
    //      for (int i = 0; i < Elements + deg; i++) {  // +deg-Elements are needed, as the curve is closed
    //        ContourPointData cp(i);
    //        if (i >= Elements)
    //        cp.getNodeNumber() = i - Elements;
    //
    ////        updateKinematicsForFrame(cp, Frame::position);
    //        NodelistPos[i] = HPoint3Dd(cp.getFrameOfReference().getPosition()(0), cp.getFrameOfReference().getPosition()(1), cp.getFrameOfReference().getPosition()(2), 1);// Third component is zero as Nurbs library supports only 3D interpolation
    //
    //        if (not filenameVel.empty()) {
    ////          updateKinematicsForFrame(cp, Frame::velocity_cosy);
    //
    //          SqrMat3 TMPMat = cp.getFrameOfReference().getOrientation();
    //          SqrMat3 AKI(INIT, 0.);
    //          AKI.set(0, trans(TMPMat.col(1)));
    //          AKI.set(1, trans(TMPMat.col(0)));
    //          AKI.set(2, trans(TMPMat.col(2)));
    //          Vec3 Vel(INIT, 0.);
    //          Vel = AKI * cp.getFrameOfReference().getVelocity();
    //
    //          NodelistVel[i] = HPoint3Dd(Vel(0), Vel(1), Vel(2), 1);
    //        }
    //      }
    //
    //      /*create own uVec and uvec like in nurbsdisk_2s*/
    //      PLib::Vector<double> uvec = PLib::Vector<double>(Elements + deg);
    //      PLib::Vector<double> uVec = PLib::Vector<double>(Elements + deg + deg + 1);
    //
    //      const double stepU = L / Elements;
    //
    //      uvec[0] = 0;
    //      for (int i = 1; i < uvec.size(); i++) {
    //        uvec[i] = uvec[i - 1] + stepU;
    //      }
    //
    //      uVec[0] = (-deg) * stepU;
    //      for (int i = 1; i < uVec.size(); i++) {
    //        uVec[i] = uVec[i - 1] + stepU;
    //      }
    //
    //      curvePos.globalInterpClosedH(NodelistPos, uvec, uVec, deg);
    //      curvePos.write(filenamePos.c_str());
    //
    //      if (writePsFile) {
    //        string psfile = filenamePos + ".ps";
    //
    //        msg(Debug) << curvePos.writePS(psfile.c_str(), 0, 2.0, 5, false) << endl;
    //      }
    //
    //      if (not filenameVel.empty()) {
    //        curveVel.globalInterpClosedH(NodelistVel, uvec, uVec, deg);
    //        curveVel.write(filenameVel.c_str());
    //      }
    //    }
  }

  void FlexibleBody1s21RCM::importPositionVelocity(const string & filenamePos, const string & filenameVel /* = string( )*/) {

    PlNurbsCurved curvePos;
    PlNurbsCurved curveVel;
    curvePos.read(filenamePos.c_str());
    if (not filenameVel.empty())
      curveVel.read(filenameVel.c_str());

    double l0 = L / Elements;
    Vec q0Dummy(q0.size(), INIT, 0.);
    Vec u0Dummy(u0.size(), INIT, 0.);
    Point3Dd prevBinStart;

    for (int i = 0; i < Elements; i++) {
      Point3Dd posStart = curvePos.pointAt(i * l0);
      Point3Dd pos1Quart = curvePos.pointAt(i * l0 + l0 / 4.);
      Point3Dd posHalf = curvePos.pointAt(i * l0 + l0 / 2.);
      Point3Dd pos3Quart = curvePos.pointAt(i * l0 + l0 * 3. / 4.);
      Point3Dd tangStart = curvePos.derive3D(i * l0, 1);
      tangStart /= norm(tangStart);
      Point3Dd velHalf = curvePos.derive3D(i * l0 + l0 / 2., 1);

      q0Dummy(i * 5) = posStart.x(); // x
      q0Dummy(i * 5 + 1) = posStart.y();// y
      q0Dummy(i * 5 + 2) = ArcTan(tangStart.x(), tangStart.y());// phi

      q0Dummy(i * 5 + 3) = -absolute((pos1Quart.x() - posHalf.x()) * (-velHalf.y()) - (pos1Quart.y() - posHalf.y()) * (-velHalf.x())) / sqrt(velHalf.x() * velHalf.x() + velHalf.y() * velHalf.y());// cL
      q0Dummy(i * 5 + 4) = -absolute((pos3Quart.x() - posHalf.x()) * velHalf.y() - (pos3Quart.y() - posHalf.y()) * velHalf.x()) / sqrt(velHalf.x() * velHalf.x() + velHalf.y() * velHalf.y());// cR

      if (not filenameVel.empty()) {
        Point3Dd binStart = curvePos.derive3D(i * l0, 2);
        binStart = crossProduct(binStart, tangStart);
        binStart /= norm(binStart);
        if (i > 0) {
          if (dot(prevBinStart, binStart) < 0)
            binStart = -1. * binStart;
        }
        prevBinStart = binStart;
        Point3Dd norStart = crossProduct(binStart, tangStart);

        SqrMat3 AIK(INIT, 0.);
        AIK(0, 0) = tangStart.x();
        AIK(1, 0) = tangStart.y();
        AIK(2, 0) = tangStart.z();
        AIK(0, 1) = norStart.x();
        AIK(1, 1) = norStart.y();
        AIK(2, 1) = norStart.z();
        AIK(0, 2) = binStart.x();
        AIK(1, 2) = binStart.y();
        AIK(2, 2) = binStart.z();

        Point3Dd velStart = curveVel.pointAt(i * l0);

        Vec velK(3, INIT, 0.);
        velK(0) = velStart.x();
        velK(1) = velStart.y();
        velK(2) = velStart.z();
        Vec velI = trans(R->getOrientation()) * AIK * velK;

        u0Dummy(i * 5) = velI(0);
        u0Dummy(i * 5 + 1) = velI(1);
      }
    }
    setq0(q0Dummy);
    if (not filenameVel.empty())
      setu0(u0Dummy);

    if (msgAct(Debug)) {
      for (double i = 0; i < Elements; i++) {
        msg(Debug) << "i=" << i << endl << curvePos.pointAt(i) << endl;
      }
      msg(Debug) << "Test of Nurbs-Curve" << endl;
      string psfile = "test.ps";
      msg(Debug) << curvePos.writePS(psfile.c_str(), 0, 2.0, 5, false) << endl;
    }
  }

  void FlexibleBody1s21RCM::BuildElement(const double& sGlobal, double& sLocal, int& currentElement) {
    double remainder = fmod(sGlobal, L);
    if (openStructure && sGlobal >= L)
      remainder += L; // remainder \in (-eps,L+eps)
    if (!openStructure && sGlobal < 0.)
      remainder += L; // remainder \in [0,L)

    currentElement = int(remainder / l0);
    sLocal = remainder - (0.5 + currentElement) * l0; // Lagrange-Parameter of the affected FE with sLocal==0 in the middle of the FE and sGlobal==0 at the beginning of the beam

    // contact solver computes too large sGlobal at the end of the entire beam is not considered only for open structure
    // for closed structure even sGlobal < L (but sGlobal ~ L) values could lead - due to numerical problems - to a wrong currentElement computation
    if (currentElement >= Elements) {
      currentElement = Elements - 1;
      sLocal += l0;
    }
  }
}
