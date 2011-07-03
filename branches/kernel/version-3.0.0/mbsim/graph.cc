/* Copyright (C) 2004-2009 MBSim Development Team
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
 * Contact: mfoerg@users.berlios.de
 */

#include <config.h>
#include "mbsim/graph.h"
#include "mbsim/object.h"
#include "mbsim/frame.h"
#include "mbsim/extra_dynamic.h"

using namespace fmatvec; 
using namespace std;

namespace MBSim {

  Graph::Graph(const string &projectName) : DynamicSystem(projectName) {}

  Graph::~Graph() {}

  void Graph::updateStateDependentVariables(double t) {
    for(unsigned int i=0; i<obj.size(); i++) 
      for(unsigned int j=0; j<obj[i].size(); j++) 
	obj[i][j]->updateStateDependentVariables(t);
  }

  void Graph::updateJacobians(double t, int k) {
    for(unsigned int i=0; i<obj.size(); i++) 
      for(unsigned int j=0; j<obj[i].size(); j++) 
	obj[i][j]->updateJacobians(t,k);

  }

  void Graph::updatedu(double t, double dt) {
    ud = slvLLFac(LLM[0], h[0]*dt+r[0]);
  }

  void Graph::updatezd(double t) {
    qd = T*u;
    ud =  slvLLFac(LLM[0], h[0]+r[0]);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updatexd(t);

    for(vector<ExtraDynamic*>::iterator i = extraDynamic.begin(); i!= extraDynamic.end(); ++i) 
      (**i).updatexd(t);
  }

  void Graph::sethSize(int hSize_, int k) {
    if(k==1) // TODO schöner impl.
      DynamicSystem::sethSize(hSize_,k);
    else {
    hSize[k] = hSize_;
    //root->sethSize(hSize_,k);
    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) {
      (*i)->sethSize((*i)->getuSize(k)+(*i)->getuInd(k),k);
      (*i)->sethInd(0,k);
    }
    }
  } 

  void Graph::calcqSize() {
    qSize = 0;
    for(unsigned int i=0; i<obj.size(); i++) 
      for(unsigned int j=0; j<obj[i].size(); j++) {
	obj[i][j]->calcqSize();
	obj[i][j]->setqInd(qSize);
	qSize += obj[i][j]->getqSize();
      }
  }

  void Graph::calcuSize(int k) {
    if(k==1) // TODO schöner impl.
      DynamicSystem::calcuSize(k);
    else {
    uSize[k] = 0;
    for(unsigned int i=0; i<obj.size(); i++) 
      for(unsigned int j=0; j<obj[i].size(); j++) {
	obj[i][j]->calcuSize(k);
	obj[i][j]->setuInd(uSize[k],k);
	uSize[k] += obj[i][j]->getuSize(k);
      }
    }
  }

  //void Graph::updateInverseKineticsJacobians(double t) {
  //  for(unsigned int i=0; i<obj.size(); i++) 
  //    for(unsigned int j=0; j<obj[i].size(); j++) 
  //      obj[i][j]->updateInverseKineticsJacobians(t);

  //}

  void Graph::facLLM() {
    LLM[0] = facLL(M[0]); 
  }

  void Graph::addObject(int level, Object* object) {
    DynamicSystem::addObject(object);

    for(int i=obj.size(); i<=level; i++) {
      vector<Object*> vec;
      obj.push_back(vec);
    }

    obj[level].push_back(object);
  }

}

