/* Copyright (C) 2004-2009 MBSim Development Team
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

#ifndef _OPENMBV_UTILS_H_
#define _OPENMBV_UTILS_H_

#include <fmatvec/fmatvec.h>
#include <openmbvcppinterface/arrow.h>
#include <openmbvcppinterface/frame.h>
#include <openmbvcppinterface/sphere.h>
#include <openmbvcppinterface/cuboid.h>
#include <openmbvcppinterface/frustum.h>
#include <openmbvcppinterface/extrusion.h>
#include <openmbvcppinterface/cylindricalgear.h>
#include <openmbvcppinterface/rack.h>
#include <openmbvcppinterface/coilspring.h>
#include <openmbvcppinterface/pointset.h>
#include <openmbvcppinterface/indexedlineset.h>
#include <openmbvcppinterface/indexedfaceset.h>
#include <openmbvcppinterface/nurbscurve.h>
#include <openmbvcppinterface/nurbssurface.h>

#include <utility>

namespace MBSim {

  class OpenMBVBody {
    protected:
      std::string id;
    public:
      OpenMBVBody() = default;
      void initializeUsingXML(xercesc::DOMElement *element);
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::Body> &object);
  };

  class OpenMBVColoredBody : public OpenMBVBody {
    protected:
      fmatvec::Vec3 dc;
      double tp, ps, lw;
    public:
      OpenMBVColoredBody(const fmatvec::Vec3 &dc_="[-1;1;1]", double tp_=0, double ps_=0, double lw_=0) : dc(dc_), tp(tp_), ps(ps_), lw(lw_) { }
      void initializeUsingXML(xercesc::DOMElement *element);
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::DynamicColoredBody> &object);
  };

  class OpenMBVDynamicColoredBody : public OpenMBVColoredBody {
    protected:
      unsigned int cR;
      double minCol, maxCol;
      std::vector<std::string> cRL;
    public:
      OpenMBVDynamicColoredBody(unsigned cR_=0, double minCol_=0, double maxCol_=1, const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw), cR(cR_), minCol(minCol_), maxCol(maxCol_), cRL(1,"none") { }
      void initializeUsingXML(xercesc::DOMElement *element);
      unsigned int getColorRepresentation() const { return cR; }
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::DynamicColoredBody> &object);
  };

  class OpenMBVArrow : public OpenMBVDynamicColoredBody {
    public:
      enum Type {
        line,
        fromHead,
        toHead,
        bothHeads,
        fromDoubleHead,
        toDoubleHead,
        bothDoubleHeads
      };
      enum ReferencePoint {
        toPoint,
        fromPoint,
        midPoint
      };
      enum ColorRepresentation {
        none=0,
        absoluteValue
      };
    protected:
      double sL, sS;
      OpenMBVArrow::Type type;
      OpenMBVArrow::ReferencePoint refPoint;
    public:
      OpenMBVArrow(double sL_=1, double sS_=1, const OpenMBVArrow::Type &type_=OpenMBVArrow::toHead, const OpenMBVArrow::ReferencePoint &refPoint_=OpenMBVArrow::fromPoint, unsigned int cR=0, double minCol=0, double maxCol=1, const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0);
      void initializeUsingXML(xercesc::DOMElement *element);
      std::shared_ptr<OpenMBV::Arrow> createOpenMBV();
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::Arrow> &object);
  };

  class OpenMBVInteractionArrow : public OpenMBVArrow {
    public:
      enum SideOfInteraction {
        action=0,
        reaction,
        both
      };
    protected:
      unsigned int sI;
    public:
      OpenMBVInteractionArrow(unsigned int sI_=0, double sL=1, double sS=1, const OpenMBVArrow::Type &type=OpenMBVArrow::toHead, const OpenMBVArrow::ReferencePoint &refPoint=OpenMBVArrow::fromPoint, unsigned int cR=0, double minCol=0, double maxCol=1, const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0);
      void initializeUsingXML(xercesc::DOMElement *element);
      unsigned int getSideOfInteraction() const { return sI; }
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::Arrow> &object);
  };

  class OpenMBVFrictionArrow : public OpenMBVInteractionArrow {
    public:
      enum ColorRepresentation {
        none=0,
        absoluteValue,
        stickslip
      };
    public:
      OpenMBVFrictionArrow(unsigned int sI=0, double sL=1, double sS=1, const OpenMBVArrow::Type &type=OpenMBVArrow::toHead, const OpenMBVArrow::ReferencePoint &refPoint=OpenMBVArrow::fromPoint, unsigned int cR=0, double minCol=0, double maxCol=1, const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0);
  };

  class OpenMBVFrame : public OpenMBVColoredBody {
    protected:
      double size, offset;
    public:
      OpenMBVFrame(double size_=1, double offset_=1, const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw), size(size_), offset(offset_) { }
      void initializeUsingXML(xercesc::DOMElement *element);
      std::shared_ptr<OpenMBV::Frame> createOpenMBV();
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::Frame> &object);
  };

  class OpenMBVPoint : public OpenMBVColoredBody {
    public:
      OpenMBVPoint(const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw) { }
      std::shared_ptr<OpenMBV::PointSet> createOpenMBV();
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::PointSet> &object);
  };

  class OpenMBVSphere : public OpenMBVColoredBody {
    protected:
      double r;
    public:
      OpenMBVSphere(double r_=1, const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw), r(r_) { }
      void initializeUsingXML(xercesc::DOMElement *element);
      std::shared_ptr<OpenMBV::Sphere> createOpenMBV();
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::Sphere> &object);
  };

  class OpenMBVLine : public OpenMBVColoredBody {
    protected:
      double l;
    public:
      OpenMBVLine(double l_=1, const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw), l(l_) { }
      void initializeUsingXML(xercesc::DOMElement *element);
      std::shared_ptr<OpenMBV::IndexedLineSet> createOpenMBV();
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::IndexedLineSet> &object);
  };

  class OpenMBVPlane : public OpenMBVColoredBody {
    protected:
      fmatvec::Vec2 l;
    public:
      OpenMBVPlane(const fmatvec::Vec2 &l_=fmatvec::Vec2(fmatvec::INIT,1.), const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw), l(l_) { }
      void initializeUsingXML(xercesc::DOMElement *element);
      std::shared_ptr<OpenMBV::Cuboid> createOpenMBV();
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::Cuboid> &object);
  };

  class OpenMBVCuboid : public OpenMBVColoredBody {
    protected:
      fmatvec::Vec3 l;
    public:
      OpenMBVCuboid(const fmatvec::Vec3 &l_=fmatvec::Vec3(fmatvec::INIT,1.), const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw), l(l_) { }
      void initializeUsingXML(xercesc::DOMElement *element);
      std::shared_ptr<OpenMBV::Cuboid> createOpenMBV();
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::Cuboid> &object);
  };

  class OpenMBVCircle : public OpenMBVColoredBody {
    protected:
      double r;
    public:
      OpenMBVCircle(double r_=1, const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw), r(r_) { }
      void initializeUsingXML(xercesc::DOMElement *element);
      std::shared_ptr<OpenMBV::Frustum> createOpenMBV();
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::Frustum> &object);
  };

  class OpenMBVFrustum : public OpenMBVColoredBody {
    protected:
      double t, b, h;
    public:
      OpenMBVFrustum(double t_=1, double b_=1, double h_=1, const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw), t(t_), b(b_), h(h_) { }
      void initializeUsingXML(xercesc::DOMElement *element);
      std::shared_ptr<OpenMBV::Frustum> createOpenMBV();
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::Frustum> &object);
  };

  class OpenMBVExtrusion : public OpenMBVColoredBody {
    protected:
      double h;
    public:
      OpenMBVExtrusion(double h_=1, const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw), h(h_) { }
      void initializeUsingXML(xercesc::DOMElement *element);
      std::shared_ptr<OpenMBV::Extrusion> createOpenMBV();
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::Extrusion> &object);
  };

  class OpenMBVCylindricalGear : public OpenMBVColoredBody {
    public:
      OpenMBVCylindricalGear(const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw) { }
      std::shared_ptr<OpenMBV::CylindricalGear> createOpenMBV();
  };

  class OpenMBVRack : public OpenMBVColoredBody {
    public:
      OpenMBVRack(const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw) { }
      std::shared_ptr<OpenMBV::Rack> createOpenMBV();
  };

  class OpenMBVCoilSpring : public OpenMBVDynamicColoredBody {
    public:
      enum Type {
        tube,
        scaledTube,
        polyline
      };
      enum ColorRepresentation {
        none=0,
        deflection,
        tensileForce,
        compressiveForce,
        absoluteForce
      };
    protected:
      double r, cr, sf, n, l;
      OpenMBVCoilSpring::Type type;
    public:
      OpenMBVCoilSpring(double r_=1, double cr_=-1, double sf_=1, double n_=3, double l_=-1, OpenMBVCoilSpring::Type type_=OpenMBVCoilSpring::tube, unsigned int cR=0, double minCol=0, double maxCol=1, const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0);
      void initializeUsingXML(xercesc::DOMElement *element);
      std::shared_ptr<OpenMBV::CoilSpring> createOpenMBV();
    protected:
      void initializeObject(const std::shared_ptr<OpenMBV::CoilSpring> &object);
  };

  class OpenMBVIndexedLineSet : public OpenMBVColoredBody {
    public:
      OpenMBVIndexedLineSet(const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw) { }
      std::shared_ptr<OpenMBV::IndexedLineSet> createOpenMBV();
  };

  class OpenMBVIndexedFaceSet : public OpenMBVColoredBody {
    public:
      OpenMBVIndexedFaceSet(const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw) { }
      std::shared_ptr<OpenMBV::IndexedFaceSet> createOpenMBV();
  };

  class OpenMBVNurbsCurve : public OpenMBVColoredBody {
    public:
      OpenMBVNurbsCurve(const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw) { }
      std::shared_ptr<OpenMBV::NurbsCurve> createOpenMBV();
  };

  class OpenMBVNurbsSurface : public OpenMBVColoredBody {
    public:
      OpenMBVNurbsSurface(const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw) { }
      std::shared_ptr<OpenMBV::NurbsSurface> createOpenMBV();
  };

  class OpenMBVPlanarContour : public OpenMBVColoredBody {
    protected:
      std::vector<double> nodes;
      bool filled;
    public:
      OpenMBVPlanarContour(const std::vector<double> &nodes_=std::vector<double>(), bool filled_=false, const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw), nodes(nodes_), filled(filled_) { }
      std::shared_ptr<OpenMBV::RigidBody> createOpenMBV();
      void initializeUsingXML(xercesc::DOMElement *element);
      const std::vector<double>& getNodes() const { return nodes; }
      bool getFilled() const { return filled; }
  };

  class OpenMBVSpatialContour : public OpenMBVColoredBody {
    protected:
      std::vector<double> etaNodes, xiNodes;
    public:
      OpenMBVSpatialContour(const std::vector<double> &etaNodes_=std::vector<double>(), const std::vector<double> &xiNodes_=std::vector<double>(), const fmatvec::Vec3 &dc="[-1;1;1]", double tp=0, double ps=0, double lw=0) : OpenMBVColoredBody(dc,tp,ps,lw), etaNodes(etaNodes_), xiNodes(xiNodes_) { }
      std::shared_ptr<OpenMBV::RigidBody> createOpenMBV();
      void initializeUsingXML(xercesc::DOMElement *element);
      const std::vector<double>& getEtaNodes() const { return etaNodes; }
      const std::vector<double>& getXiNodes() const { return xiNodes; }
  };

}

#endif
