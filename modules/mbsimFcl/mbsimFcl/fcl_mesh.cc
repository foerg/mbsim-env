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
#include "fcl_mesh.h"
#include "mbsimFcl/namespace.h"
#include "fcl/geometry/bvh/BVH_model.h"
#include "fcl/math/bv/utility.h"
#include <openmbvcppinterface/indexedfaceset.h>

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;
using namespace fcl;

namespace MBSimFcl {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIMFCL, FclMesh)

  void FclMesh::init(InitStage stage, const InitConfigSet &config) {
    if(stage==preInit) {
      vector<Vector3d> vertices(vertex.rows());
      vector<Triangle> triangles(triangle.rows());
      for(int i=0; i<vertex.rows(); i++) {
        for(int j=0; j<vertex.cols(); j++)
          vertices[i](j) = vertex(i,j);
      }
      for(int i=0; i<triangle.rows(); i++)
          triangles[i].set(triangle(i,0),triangle(i,1),triangle(i,2));
      if(collisionStructure==AABB) {
        cg = shared_ptr<CollisionGeometry<double> >(new BVHModel<fcl::AABB<double> >);
        static_cast<BVHModel<fcl::AABB<double> >*>(cg.get())->beginModel();
        static_cast<BVHModel<fcl::AABB<double> >*>(cg.get())->addSubModel(vertices, triangles);
        static_cast<BVHModel<fcl::AABB<double> >*>(cg.get())->endModel();
      }
      else if(collisionStructure==KDOP16) {
        cg = shared_ptr<CollisionGeometry<double> >(new BVHModel<fcl::KDOP<double,16> >);
        static_cast<BVHModel<fcl::KDOP<double,16> >*>(cg.get())->beginModel();
        static_cast<BVHModel<fcl::KDOP<double,16> >*>(cg.get())->addSubModel(vertices, triangles);
        static_cast<BVHModel<fcl::KDOP<double,16> >*>(cg.get())->endModel();
      }
      else if(collisionStructure==KDOP18) {
        cg = shared_ptr<CollisionGeometry<double> >(new BVHModel<fcl::KDOP<double,18> >);
        static_cast<BVHModel<fcl::KDOP<double,18> >*>(cg.get())->beginModel();
        static_cast<BVHModel<fcl::KDOP<double,18> >*>(cg.get())->addSubModel(vertices, triangles);
        static_cast<BVHModel<fcl::KDOP<double,18> >*>(cg.get())->endModel();
      }
      else if(collisionStructure==KDOP24) {
        cg = shared_ptr<CollisionGeometry<double> >(new BVHModel<fcl::KDOP<double,24> >);
        static_cast<BVHModel<fcl::KDOP<double,24> >*>(cg.get())->beginModel();
        static_cast<BVHModel<fcl::KDOP<double,24> >*>(cg.get())->addSubModel(vertices, triangles);
        static_cast<BVHModel<fcl::KDOP<double,24> >*>(cg.get())->endModel();
      }
      else if(collisionStructure==kIOS) {
        cg = shared_ptr<CollisionGeometry<double> >(new BVHModel<fcl::kIOS<double> >);
        static_cast<BVHModel<fcl::kIOS<double> >*>(cg.get())->beginModel();
        static_cast<BVHModel<fcl::kIOS<double> >*>(cg.get())->addSubModel(vertices, triangles);
        static_cast<BVHModel<fcl::kIOS<double> >*>(cg.get())->endModel();
      }
      else if(collisionStructure==OBB) {
        cg = shared_ptr<CollisionGeometry<double> >(new BVHModel<fcl::OBB<double> >);
        static_cast<BVHModel<fcl::OBB<double> >*>(cg.get())->beginModel();
        static_cast<BVHModel<fcl::OBB<double> >*>(cg.get())->addSubModel(vertices, triangles);
        static_cast<BVHModel<fcl::OBB<double> >*>(cg.get())->endModel();
      }
      else if(collisionStructure==OBBRSS) {
        cg = shared_ptr<CollisionGeometry<double> >(new BVHModel<fcl::OBBRSS<double> >);
        static_cast<BVHModel<fcl::OBBRSS<double> >*>(cg.get())->beginModel();
        static_cast<BVHModel<fcl::OBBRSS<double> >*>(cg.get())->addSubModel(vertices, triangles);
        static_cast<BVHModel<fcl::OBBRSS<double> >*>(cg.get())->endModel();
      }
      else if(collisionStructure==RSS) {
        cg = shared_ptr<CollisionGeometry<double> >(new BVHModel<fcl::RSS<double> >);
        static_cast<BVHModel<fcl::RSS<double> >*>(cg.get())->beginModel();
        static_cast<BVHModel<fcl::RSS<double> >*>(cg.get())->addSubModel(vertices, triangles);
        static_cast<BVHModel<fcl::RSS<double> >*>(cg.get())->endModel();
      }
      else if(collisionStructure==unknown)
        throwError("(FclMesh::init): unknown collision structure");
    }
    else if (stage == plotting) {
      if(plotFeature[openMBV] && openMBVRigidBody) {
        vector<Index> indices(triangle.rows()*4);
        for(int i=0; i<triangle.rows(); i++) {
          indices[4*i] = triangle(i,0);
          indices[4*i+1] = triangle(i,1);
          indices[4*i+2] = triangle(i,2);
          indices[4*i+3] = -1;
        }
        static_pointer_cast<OpenMBV::IndexedFaceSet>(openMBVRigidBody)->setVertexPositions(vertex);
        static_pointer_cast<OpenMBV::IndexedFaceSet>(openMBVRigidBody)->setIndices(indices);
      }
    }
    FclContour::init(stage, config);
  }

  void FclMesh::initializeUsingXML(DOMElement *element) {
    FclContour::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIMFCL%"vertices");
    vertex = E(e)->getText<MatVx3>();
    e=E(element)->getFirstElementChildNamed(MBSIMFCL%"triangles");
    Matrix<General,Var,Fixed<3>,Index> triangle1based=E(e)->getText<Matrix<General,Var,Fixed<3>,Index>>();
    triangle.resize(triangle1based.rows(),NONINIT);
    for(int i=0; i<triangle.rows(); i++)
      for(int j=0; j<triangle.cols(); j++)
        triangle(i,j) = triangle1based(i,j) - 1;
    e=E(element)->getFirstElementChildNamed(MBSIMFCL%"collisionStructure");
    if (e) {
      std::string str=X()%E(e)->getFirstTextChild()->getData();
      str=str.substr(1,str.length()-2);
      if(str=="AABB") collisionStructure=AABB;
      else if(str=="KDOP16") collisionStructure=KDOP16;
      else if(str=="KDOP18") collisionStructure=KDOP18;
      else if(str=="KDOP24") collisionStructure=KDOP24;
      else if(str=="kIOS") collisionStructure=kIOS;
      else if(str=="OBB") collisionStructure=OBB;
      else if(str=="OBBRSS") collisionStructure=OBBRSS;
      else if(str=="RSS") collisionStructure=RSS;
      else collisionStructure=unknown;
    }
    e=E(element)->getFirstElementChildNamed(MBSIMFCL%"enableOpenMBV");
    if(e) {
      OpenMBVColoredBody ombv;
      ombv.initializeUsingXML(e);
      openMBVRigidBody=ombv.createOpenMBV<OpenMBV::IndexedFaceSet>();
    }
  }

}
