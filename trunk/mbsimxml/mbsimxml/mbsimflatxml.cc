#include "config.h"
#include <stdlib.h>
#include <iostream>
#include "mbxmlutilstinyxml/tinyxml.h"
#include "mbxmlutilstinyxml/tinynamespace.h"

#include "mbsim/dynamic_system_solver.h"
#include "mbsim/objectfactory.h"
#include "mbsim/xmlnamespacemapping.h"
#include "mbsim/integrators/integrator.h"
#include "mbsimxml/mbsimflatxml.h"

using namespace std;
using namespace MBXMLUtils;

namespace MBSim {

int MBSimXML::preInitDynamicSystemSolver(int argc, char *argv[], DynamicSystemSolver*& dss) {

  // print namespace-prefix mapping
  if(argc==2 && strcmp(argv[1], "--printNamespacePrefixMapping")==0) {
    map<string, string> nsprefix=XMLNamespaceMapping::getNamespacePrefixMapping();
    for(map<string, string>::iterator it=nsprefix.begin(); it!=nsprefix.end(); it++)
      cout<<it->first<<" "<<it->second<<endl;
    return 1;
  }


  // help
  if(argc<3 || argc>4) {
    cout<<"Usage: mbsimflatxml [--donotintegrate|--savestatevector|--stopafterfirststep]"<<endl;
    cout<<"                    <mbsimfile> <mbsimintegratorfile>"<<endl;
    cout<<"   or: mbsimflatxml --printNamespacePrefixMapping"<<endl;
    cout<<endl;
    cout<<"Copyright (C) 2004-2009 MBSim Development Team"<<endl;
    cout<<"This is free software; see the source for copying conditions. There is NO"<<endl;
    cout<<"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."<<endl;
    cout<<endl;
    cout<<"Licensed under the GNU Lesser General Public License (LGPL)"<<endl;
    cout<<endl;
    cout<<"--donotintegrate               Stop after the initialization stage, do not integrate"<<endl;
    cout<<"--stopafterfirststep           Stop after outputting the first step (usually at t=0)"<<endl;
    cout<<"                               This generates a HDF5 output file with only one time serie"<<endl;
    cout<<"--savefinalstatevector         Save the state vector to the file \"statevector.asc\" after integration"<<endl;
    cout<<"--printNamespacePrefixMapping  Print the recommended mapping of XML namespaces to XML prefix"<<endl;
    cout<<"<mbsimfile>                    The preprocessed mbsim xml file"<<endl;
    cout<<"<mbsimintegratorfile>          The preprocessed mbsim integrator xml file"<<endl;
    return 1;
  }


  int startArg=1;
  if(strcmp(argv[1],"--donotintegrate")==0 || strcmp(argv[1],"--savefinalstatevector")==0 || strcmp(argv[1],"--stopafterfirststep")==0)
    startArg=2;


  // load MBSim XML document
  TiXmlDocument *doc=new TiXmlDocument;
  if(doc->LoadFile(argv[startArg])==false)
    throw MBSimError(string("ERROR! Unable to load file: ")+argv[startArg]);
  TiXml_PostLoadFile(doc);
  TiXmlElement *e=doc->FirstChildElement();
  TiXml_setLineNrFromProcessingInstruction(e);
  map<string,string> dummy;
  incorporateNamespace(e, dummy);

  // create object for root element and check correct type
  dss=ObjectFactory<Element>::create<DynamicSystemSolver>(e);
  if(dss==0)
    throw MBSimError("ERROR! The root element of the MBSim model file must be of type '{"MBSIMNS"}DynamicSystemSolver'");

  // If enviornment variable MBSIMREORGANIZEHIERARCHY=false then do NOT reorganize.
  // In this case it is not possible to simulate a relativ kinematics (tree structures).
  char *reorg=getenv("MBSIMREORGANIZEHIERARCHY");
  if(reorg && strcmp(reorg, "false")==0)
    dss->setReorganizeHierarchy(false);
  else
    dss->setReorganizeHierarchy(true);

  dss->initializeUsingXML(e);
  delete doc;

  return 0;
}

void MBSimXML::initDynamicSystemSolver(int argc, char *argv[], DynamicSystemSolver*& dss) {
  if(strcmp(argv[1],"--donotintegrate")==0)
    dss->setTruncateSimulationFiles(false);

  dss->initialize();
}

void MBSimXML::plotInitialState(Integrator*& integrator, DynamicSystemSolver*& dss) {
  int zSize=dss->getzSize();
  fmatvec::Vec z(zSize);
  if(integrator->getInitialState().size())
    z = integrator->getInitialState();
  else
    dss->initz(z);          
  dss->computeInitialCondition();
  dss->plot(z, 0);
}

void MBSimXML::initIntegrator(int argc, char *argv[], Integrator *&integrator) {
  int startArg=1;
  if(strcmp(argv[1],"--donotintegrate")==0 || strcmp(argv[1],"--savefinalstatevector")==0 || strcmp(argv[1],"--stopafterfirststep")==0)
    startArg=2;

  TiXmlElement *e;

  // load MBSimIntegrator XML document
  TiXmlDocument *doc=new TiXmlDocument;
  if(doc->LoadFile(argv[startArg+1])==false)
    throw MBSimError(string("ERROR! Unable to load file: ")+argv[startArg+1]);
  TiXml_PostLoadFile(doc);
  e=doc->FirstChildElement();
  TiXml_setLineNrFromProcessingInstruction(e);
  map<string,string> dummy;
  incorporateNamespace(e, dummy);

  // create integrator
  integrator=ObjectFactory<Integrator>::create<Integrator>(e);
  if(integrator==0)
    throw MBSimError("ERROR! The root element of the MBSim integrator file must be of type '{"MBSIMINTNS"}Integrator'");
  integrator->initializeUsingXML(e);
  delete doc;
}

void MBSimXML::main(Integrator *&integrator, DynamicSystemSolver *&dss) {
  integrator->integrate(*dss);
}

void MBSimXML::postMain(int argc, char *argv[], Integrator *&integrator, DynamicSystemSolver*& dss) {

  if(strcmp(argv[1],"--savefinalstatevector")==0)
    dss->writez("statevector.asc", false);
  delete dss;
  delete integrator;
}

}
