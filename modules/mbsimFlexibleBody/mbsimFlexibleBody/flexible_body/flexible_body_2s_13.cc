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

#include<config.h>
#include "mbsimFlexibleBody/flexible_body/flexible_body_2s_13.h"
//#include "mbsimFlexibleBody/contours/nurbs_disk_2s.h"
#include "mbsim/dynamic_system.h"
#include "mbsim/utils/eps.h"
#include <mbsim/utils/rotarymatrices.h>

#include <iostream>

using namespace std;
using namespace fmatvec;
using namespace MBSim;

namespace MBSimFlexibleBody {

  Mat condenseMatrixRows(Mat C, RangeV I) {
    Mat B(C.rows() - (I.end() - I.start() + 1), C.cols());
    RangeV upperPart(0, I.start() - 1);
    RangeV lowerPartC(I.end() + 1, C.rows() - 1);
    RangeV lowerPartB(I.start(), B.rows() - 1);
    RangeV AllCols(0, C.cols() - 1);

    B.set(upperPart, AllCols, C(upperPart, AllCols)); // upper
    B.set(lowerPartB, AllCols, C(lowerPartC, AllCols)); // lower
    return B;
  }

  Mat condenseMatrixCols(Mat C, RangeV I) {
    Mat B(C.rows(), C.cols() - (I.end() - I.start() + 1));
    RangeV leftPart(0, I.start() - 1);
    RangeV rightPartC(I.end() + 1, C.cols() - 1);
    RangeV rightPartB(I.start(), B.cols() - 1);
    RangeV AllRows(0, C.rows() - 1);

    B.set(AllRows, leftPart, C(AllRows, leftPart)); // left
    B.set(AllRows, rightPartB, C(AllRows, rightPartC)); // right
    return B;
  }

  SymMat condenseMatrix(SymMat C, RangeV I) {
    // build size of result matrix
    SymMat B(C.size() - (I.end() - I.start() + 1));
    RangeV upperPart(0, I.start() - 1);
    RangeV lowerPartC(I.end() + 1, C.size() - 1);
    RangeV lowerPartB(I.start(), B.size() - 1);

    // assemble result matrix
    B.set(upperPart, C(upperPart)); // upper left
    B.set(lowerPartB, upperPart, C(lowerPartC, upperPart)); // upper right
    B.set(lowerPartB, C(lowerPartC)); // lower right
    return B;
  }

  void MapleOutput(Mat C, std::string MatName, std::string file) {
    ofstream dat(file, ios::app);
    dat << MatName;
    dat << " := Matrix([";
    for (int i = 0; i < C.rows(); i++) {
      dat << "[";
      for (int j = 0; j < C.cols(); j++) {
        dat << C(i, j);
        if (j < C.cols() - 1)
          dat << ", ";
      }
      dat << "]";
      if (i != C.rows() - 1)
        dat << ",";
    }
    dat << "]):";
    dat << '\n';
    dat.close();
  }


  void MapleOutput(SymMat C, std::string MatName, std::string file) {
    ofstream dat(file, ios::app);
    dat << MatName;
    dat << " := Matrix([";
    for (int i = 0; i < C.rows(); i++) {
      dat << "[";
      for (int j = 0; j < C.cols(); j++) {
        dat << C(i, j);
        if (j < C.cols() - 1)
          dat << ", ";
      }
      dat << "]";
      if (i != C.rows() - 1)
        dat << ",";
    }
    dat << "]):";
    dat << '\n';
    dat.close();
  }

  FlexibleBody2s13::FlexibleBody2s13(const string &name) : FlexibleBody2s(name), Elements(0), NodeDofs(3), RefDofs(0), E(0.), nu(0.), rho(0.), d(3, INIT, 0.), Ri(0), Ra(0), dr(0), dj(0), m0(0), J0(INIT, 0.), degV(3), degU(3), drawDegree(0), currentElement(0), nr(0), nj(0), Nodes(0), Dofs(0), LType(innerring), A(3, EYE), G(3, EYE), updExt(true), updAG(true) {
//    contour = new NurbsDisk2s("SurfaceContour");
//    addContour(contour);

    // frame in axis
  //  Vec s(2, fmatvec::INIT, 0.);
  //  addFrame("COG", s);
  }

  void FlexibleBody2s13::updateM() {
    M = MConst;
  }

  void FlexibleBody2s13::updateh(int j) {
    h[j] = -K * q;
  }

  void FlexibleBody2s13::updatedhdz() {
    updateh();
    for(int i = 0; i < dhdq.cols(); i++)
      for(int j = 0; j < dhdq.rows(); j++)
        dhdq(i, j) = -K(i, j);
  }

  void FlexibleBody2s13::plot() {
//      if(getPlotFeature(openMBV) && openMBVBody) {
//        vector<double> data;
//        data.push_back(getTime()); //time
//
//        ContourPointData cp;
//
//        //center of gravity
//        cp.getLagrangeParameterPosition()(0) = 0.;
//        cp.getLagrangeParameterPosition()(1) = 0.;
//        contour->updateKinematicsForFrame(cp, Frame::position_cosy); // kinematics of the center of gravity of the disk (TODO frame feature)
//
//        //Translation of COG
//        data.push_back(cp.getFrameOfReference().getPosition()(0)); //global x-coordinate
//        data.push_back(cp.getFrameOfReference().getPosition()(1)); //global y-coordinate
//        data.push_back(cp.getFrameOfReference().getPosition()(2)); //global z-coordinate
//
//        //Rotation of COG
//        Vec AlphaBetaGamma = AIK2Cardan(cp.getFrameOfReference().getOrientation());
//        data.push_back(AlphaBetaGamma(0));
//        data.push_back(AlphaBetaGamma(1));
//        data.push_back(AlphaBetaGamma(2));
//
//        //Control-Point coordinates
//        for(int i = 0; i < nr + 1; i++) {
//          for(int j = 0; j < nj + degU; j++) {
//            data.push_back(contour->getControlPoints(j, i)(0)); //global x-coordinate
//            data.push_back(contour->getControlPoints(j, i)(1)); //global y-coordinate
//            data.push_back(contour->getControlPoints(j, i)(2)); //global z-coordinate
//          }
//        }
//
//        //inner ring
//        for(int i = 0; i < nj; i++) {
//          for(int j = 0; j < drawDegree; j++) {
//            cp.getLagrangeParameterPosition()(0) = Ri;
//            cp.getLagrangeParameterPosition()(1) = 2 * M_PI * (i * drawDegree + j) / (nj * drawDegree);
//            contour->updateKinematicsForFrame(cp, Frame::position);
//            Vec pos = cp.getFrameOfReference().getPosition();
//
//            data.push_back(pos(0)); //global x-coordinate
//            data.push_back(pos(1)); //global y-coordinate
//            data.push_back(pos(2)); //global z-coordinate
//
//          }
//        }
//
//        //outer Ring
//        for(int i = 0; i < nj; i++) {
//          for(int j = 0; j < drawDegree; j++) {
//            cp.getLagrangeParameterPosition()(0) = Ra;
//            cp.getLagrangeParameterPosition()(1) = 2 * M_PI * (i * drawDegree + j) / (nj * drawDegree);
//            contour->updateKinematicsForFrame(cp, Frame::position);
//            Vec pos = cp.getFrameOfReference().getPosition();
//
//            data.push_back(pos(0)); //global x-coordinate
//            data.push_back(pos(1)); //global y-coordinate
//            data.push_back(pos(2)); //global z-coordinate
//          }
//        }
//
//        ((OpenMBV::NurbsDisk*) openMBVBody.get())->append(data);
//      }
    FlexibleBody2s::plot();
  }

  void FlexibleBody2s13::setNumberElements(int nr_, int nj_) {
    nr = nr_;
    nj = nj_;
    degV = min(degV, nr); // radial adaptation of spline degree to have correct knot vector
    degU = min(degU, nj); // azimuthal adaptation of spline degree to have correct knot vector
    Elements = nr * nj;
    Nodes = (nr + 1) * nj;

    Dofs = RefDofs + Nodes * NodeDofs;

    qSize = Dofs - NodeDofs * nj; // missing one node row because of bearing
    uSize[0] = qSize;
    uSize[1] = qSize; // TODO

    qext.resize(Dofs);
    uext.resize(Dofs);

    q0.resize(qSize);
    u0.resize(uSize[0]);
  }

  void FlexibleBody2s13::BuildElement(const Vec &s) {
    assert(Ri <= s(0)); // is the input on the disk?
    assert(Ra >= s(0));

    currentElement = int((s(0) - Ri) / dr) * nj + int(s(1) / dj); // which element is involved?
  }

  double FlexibleBody2s13::computeThickness(const double &r_) {
    return d(0) + d(1) * r_ + d(2) * r_ * r_; // quadratic parameterization
  }

  void FlexibleBody2s13::resetUpToDate() {
    FlexibleBody2s::resetUpToDate();
    updExt = true;
    updAG = true;
  }

  void FlexibleBody2s13::updateExt() {
    qext = Jext * q;
    uext = Jext * u;
    updExt = false;
  }

}
