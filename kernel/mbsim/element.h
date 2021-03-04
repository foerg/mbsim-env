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

#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#include "fmatvec/fmatvec.h"
#include "fmatvec/atom.h"
#include "mbsim/objectfactory.h"
#include "mbsim/utils/plotfeatureenum.h"
#include "mbsim/utils/initconfigenum.h"
#include "mbsim/namespace.h"
#include "mbsim/mbsim_event.h"
#include <hdf5serie/vectorserie.h>

namespace OpenMBV {
  class Group;
}

namespace H5 {
  class Group;
}

/**
 * \brief namespace MBSim
 */
namespace MBSim {

  extern const PlotFeatureEnum plotRecursive, openMBV, debug;

  class DynamicSystemSolver;
  class Frame;

  /**
   * \brief basic class of MBSim mainly for plotting
   * \author Martin Foerg
   * \date 2009-03-24 plot feature energy (Thorsten Schindler)
   * \date 2009-07-28 splitted interfaces (Thorsten Schindler)
   */
  class Element : virtual public fmatvec::Atom {
    public:
      /** \brief The stages of the initialization
       *
       * see also DynamicSystemSolver::init()
       */
      enum InitStage {
        resolveStringRef=0, /*!< resolve references given by a string path to the corrosponding pointer */
        preInit, /*!< Make some early initialization. TODO This should be split into detailed stages. */
        plotting, /*!< Build/initialize the plot structure */
        unknownStage, /*!< Init all the rest. TODO This should be split into detailed stages. */
        LASTINITSTAGE
      };

      /**
       * \brief constructor
       */
      Element(const std::string &name);

      /**
       * \brief destructor
       */
      ~Element() override = default;

#ifndef SWIG
      [[noreturn]]
#endif
      void throwError(const std::string &msg) const {
        throw MBSimError(this, msg);
      }

      /**
       * \brief sets the used dynamics system solver to the element
       * \param pointer to the dynamic system solver of which the element is part of
       */
      virtual void setDynamicSystemSolver(DynamicSystemSolver *sys) { ds = sys; }

      /**
       * \brief plots time dependent data
       * \param simulation time
       * \param simulation time step size for derivative calculation
       */
      virtual void plot();

      /**
       * \brief plots time dependent data at special events
       * \param simulation time
       * \param simulation time step size for derivative calculation
       */
      virtual void plotAtSpecialEvent() { }

      /**
       * \return element name
       */
      const std::string& getName() const { return name; }

      /**
       * \param element name
       */
      void setName(const std::string &str) { name = str; }

      // internal function do not use
      void setPath(const std::string &str) { path=str; }

      /**
       * \return dynamic system solver
       */
      DynamicSystemSolver* getDynamicSystemSolver() { return ds; }

      /**
       * \brief plots time series header
       * \param invocing parent class
       */
      virtual void init(InitStage stage, const InitConfigSet &config);

      /**
       * \brief creates the plotGroup for H5-output
       */
      virtual void createPlotGroup();

      /**
       * \return associated plot group
       */
      H5::GroupBase *getPlotGroup() { return plotGroup; }
      virtual H5::GroupBase *getFramesPlotGroup() { return nullptr; }
      virtual H5::GroupBase *getContoursPlotGroup() { return nullptr; }
      virtual H5::GroupBase *getGroupsPlotGroup() { return nullptr; }
      virtual H5::GroupBase *getObjectsPlotGroup() { return nullptr; }
      virtual H5::GroupBase *getLinksPlotGroup() { return nullptr; }
      virtual H5::GroupBase *getConstraintsPlotGroup() { return nullptr; }
      virtual H5::GroupBase *getObserversPlotGroup() { return nullptr; }

      /** Get the state of the plot feature pf.
       * Returns false if the plot feature pf is not set till now. */
      bool getPlotFeature(const PlotFeatureEnum &pf) {
        auto it=plotFeature.find(std::ref(pf));
        if(it==plotFeature.end())
          return false;
        return it->second;
      }

      /**
       * \brief Set a plot feature
       *
       * Set the plot feature pf of this object.
       */
      virtual void setPlotFeature(const PlotFeatureEnum &pf, bool value);

      /**
       * \brief Set a plot feature for the children of this object
       *
       * Set the plot feature pf of all children for set plot features.
       */
      void setPlotFeatureForChildren(const PlotFeatureEnum &pf, bool value);

      /**
       * \brief Set a plot feature for this object and the children of this object.
       *
       * This is a convenience function. It simply calls setPlotFeature and setPlotFeatureForChildren.
       */
      void setPlotFeatureRecursive(const PlotFeatureEnum &pf, bool value) { setPlotFeature(pf,value); setPlotFeatureForChildren(pf,value); }

      virtual void initializeUsingXML(xercesc::DOMElement *element);

      /**
       * \brief Get the object of type T represented by the path path.
       * Do not set any argurment other than path!
       */
      template<class T> T* getByPath(const std::string &path, bool initialCaller=true) const;

      /**
       * \brief Return the path of this object.
       * If relativeTo is not NULL return a relative path to relativeTo.
       * Do not set any argurment other than relTo and sep!
       */
      std::string getPath(const Element *relTo=nullptr, std::string sep="/") const;

      /**
       * \brief Get the Element named name in the container named container.
       */
      virtual Element* getChildByContainerAndName(const std::string &container, const std::string &name) const {
        throwError("This element has no containers with childs.");
      }

      virtual std::shared_ptr<OpenMBV::Group> getOpenMBVGrp() { return std::shared_ptr<OpenMBV::Group>(); }
      virtual std::shared_ptr<OpenMBV::Group> getFramesOpenMBVGrp() { return std::shared_ptr<OpenMBV::Group>(); }
      virtual std::shared_ptr<OpenMBV::Group> getContoursOpenMBVGrp() { return std::shared_ptr<OpenMBV::Group>(); }
      virtual std::shared_ptr<OpenMBV::Group> getGroupsOpenMBVGrp() { return std::shared_ptr<OpenMBV::Group>(); }
      virtual std::shared_ptr<OpenMBV::Group> getObjectsOpenMBVGrp() { return std::shared_ptr<OpenMBV::Group>(); }
      virtual std::shared_ptr<OpenMBV::Group> getLinksOpenMBVGrp() { return std::shared_ptr<OpenMBV::Group>(); }
      virtual std::shared_ptr<OpenMBV::Group> getConstraintsOpenMBVGrp() { return std::shared_ptr<OpenMBV::Group>(); }
      virtual std::shared_ptr<OpenMBV::Group> getObserversOpenMBVGrp() { return std::shared_ptr<OpenMBV::Group>(); }

      virtual Element* getParent() {return parent;}
      virtual const Element* getParent() const {return parent;}
      virtual void setParent(Element* parent_) {parent = parent_;}

      /**
       * \brief checks dependency on other elements.
       * \return a vector of elements the calling element depends on.
       */
      std::vector<Element*> getDependencies() const { return dependency; }

      void addDependency(Element* ele) { if(ele) dependency.push_back(ele); }

      /**
       * \brief computes the length of the pathes in the graph that represents
       * the dependencies between all elements.
       * \return the length of the longest path in the graph.
       */
      int computeLevel();

      virtual void updatePositions(Frame *frame) { }
      virtual void updateVelocities(Frame *frame) { }
      virtual void updateAccelerations(Frame *frame) { }
      virtual void updateJacobians(Frame *frame, int j=0) { }
      virtual void updateGyroscopicAccelerations(Frame *frame) { }

      virtual void resetUpToDate() { }

      const double& getTime() const;
      double getStepSize() const;

#ifndef SWIG
      const MBXMLUtils::DOMEvalException& getDOMEvalError() const { return domEvalError; };
#endif

    protected:
      Element *parent;

      /**
       * \brief name of element
       */
      std::string name;

      /**
       * \brief The path of this object.
       *  Is set during the init stage reorganizeHierarchy. Before this the path is calculated
       *  dynamically using getPath() after this stage getPath just returns this value.
       */
      std::string path;

#ifndef SWIG
      //! Special XML helper variable.
      MBXMLUtils::DOMEvalException domEvalError;
#endif

      /**
       * \brief dynamic system
       */
      DynamicSystemSolver *ds;

      /**
       * \brief time series
       */
      H5::VectorSerie<double> *plotVectorSerie;

      /**
       * \brief one entry of time series
       */
      std::vector<double> plotVector;

      /**
       * \brief columns of time series
       */
      std::vector<std::string> plotColumns;

      /**
       * \brief associated plot group
       */
      H5::GroupBase *plotGroup;

      void updatePlotFeatures();

      /**
       * \brief vector containing all dependencies.
       */
      std::vector<Element*> dependency;

      /**
       * \brief plot feature
       */
      PlotFeatureMap plotFeature, plotFeatureForChildren;
  };

  template<class T>
  T* Element::getByPath(const std::string &path, bool initialCaller) const {
    try {
      if(path.substr(0, 1) == "/") { // if absolute path ...
        if(parent) // .. and a parent exists ...
          return parent->getByPath<T>(path, false); // ... than call getByPath of the parent (walk to the top)
        else // .. and no parent exits ...
          return getByPath<T>(path.substr(1), false); // ... we are at top and call getByPath again with the leading "/" removed (call relative to top)
      }
      else if (path.substr(0, 3) == "../") { // if relative path to parent ...
        if(!parent)
          throwError("Reference to parent '"+path+"' requested but already at the root node.");
        return parent->getByPath<T>(path.substr(3), false); // ... call getByPath of the parent with the leading "../" removed
      }
      else { // if relative path to a child ...
        // extract the first path and all other paths (rest)
        size_t idx=path.find('/');
        std::string first=path.substr(0, idx);
        std::string rest;
        if(idx!=std::string::npos)
          rest=path.substr(idx+1);
        // get the object of the first child path by calling the virtual function getChildByContainerAndName
        size_t pos0=first.find('[');
        if(pos0==std::string::npos)
          throwError("Syntax error in subreference '"+first+"': no [ found.");
        std::string container=first.substr(0, pos0);
        if(first[first.size()-1]!=']')
          throwError("Syntax error in subreference '"+first+"': not ending with ].");
        std::string name=first.substr(pos0+1, first.size()-pos0-2);
        Element *e=getChildByContainerAndName(container, name);
        // if their are other child paths call getByPath of e for this
        if(!rest.empty())
          return e->getByPath<T>(rest, false);
        // this is the last relative path -> check type and return
        auto *t=dynamic_cast<T*>(e);
        if(t)
          return t;
        else
          throwError("Cannot cast this element to type "+container+".");
      }
    }
    catch(MBSimError &ex) {
      if(initialCaller)
        throwError("Evaluation of refernece '"+path+"' failed: Message from "+
          ex.getPath()+": "+ex.getErrorMessage());
      else
        throw ex;
    }
  }

}

#endif
