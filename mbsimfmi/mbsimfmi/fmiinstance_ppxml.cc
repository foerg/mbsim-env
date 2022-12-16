#include "../config.h"
#include <fmiinstance.h>
#include <mbsim/dynamic_system_solver.h>
#include <mbsim/integrators/integrator.h>
#include <mbxmlutilshelper/thislinelocation.h>
#include <mbsimxml/mbsimflatxml.h>
#include <mbsimxml/mbsimxml.h>
#include <mbsim/objectfactory.h>
#include <mbxmlutils/eval.h>
#include <mbxmlutils/preprocess.h>
#include "../general/xmlpp_utils.h"
#include "boost/filesystem/fstream.hpp"

using namespace std;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace boost::filesystem;

namespace {
  void convertVariableToParamSet(size_t startIndex, const vector<std::shared_ptr<MBSimFMI::Variable> > &var,
                                 std::shared_ptr<Preprocess::ParamSet> &param, const shared_ptr<Eval> &eval);
}

namespace MBSimFMI {

  void FMIInstance::addModelParametersAndCreateSystem(vector<std::shared_ptr<Variable> > &varSim) {
    // get the model file
    path resourcesDir=boost::filesystem::path(fmuLoc()).parent_path().parent_path().parent_path();
    // path to XML project file relative to resources/model
    boost::filesystem::ifstream xmlProjectStream(resourcesDir/"model"/"XMLProjectFile.txt");
    string xmlProjectFile;
    getline(xmlProjectStream, xmlProjectFile);
    path mbsimxmlfile=resourcesDir/"model"/xmlProjectFile;

    // load all MBSim modules
    msg(Debug)<<"Load MBSim modules."<<endl;
    MBSimXML::loadModules();
    // check for errors during ObjectFactory
    string errorMsg3(ObjectFactory::getAndClearErrorMsg());
    if(!errorMsg3.empty())
      throw runtime_error("The following errors occured during the loading of MBSim modules object factory:\n"+errorMsg3);

    // init the validating parser with the mbsimxml schema file
    msg(Debug)<<"Create MBSim XML schema file including all modules."<<endl;
    auto xmlCatalog=getMBSimXMLCatalog({}, true);
    std::shared_ptr<MBXMLUtils::DOMParser> validatingParser=DOMParser::create(xmlCatalog->getDocumentElement());
  
    // load MBSim project XML document
    msg(Debug)<<"Read MBSim XML model file."<<endl;
    std::shared_ptr<xercesc::DOMDocument> doc=validatingParser->parse(mbsimxmlfile, nullptr, false);
    xercesc::DOMElement *ele=doc->getDocumentElement();

    // create a clean evaluator (get the evaluator name first form the dom)
    string evalName="octave"; // default evaluator
    xercesc::DOMElement *evaluator;
    if(E(ele)->getTagName()==PV%"Embed") {
      // if the root element IS A Embed than the <evaluator> element is the first child of the
      // first (none pv:Parameter) child of the root element
      auto r=ele->getFirstElementChild();
      if(E(r)->getTagName()==PV%"Parameter")
        r=r->getNextElementSibling();
      evaluator=E(r)->getFirstElementChildNamed(PV%"evaluator");
    }
    else
      // if the root element IS NOT A Embed than the <evaluator> element is the first child root element
      evaluator=E(ele)->getFirstElementChildNamed(PV%"evaluator");
    if(evaluator)
      evalName=X()%E(evaluator)->getFirstTextChild()->getData();
    shared_ptr<Eval> eval=Eval::createEvaluator(evalName);

    // set param according data in var
    std::shared_ptr<Preprocess::ParamSet> param=std::make_shared<Preprocess::ParamSet>();
    convertVariableToParamSet(varSim.size(), var, param, eval);

    // preprocess XML file
    vector<path> dependencies;
    msg(Debug)<<"Preprocess MBSim XML model."<<endl;
    Preprocess::preprocess(validatingParser, eval, dependencies, ele, param);

    // convert the parameter set from the mbxmlutils preprocessor to a "Variable" vector
    msg(Debug)<<"Convert XML parameters to FMI parameters."<<endl;
    vector<std::shared_ptr<Variable> > xmlParam;
    convertParamSetToVariable(param, xmlParam, eval);
    // build a set of all Parameter's in var
    set<string> useParam;
    for(auto & it : var)
      if(it->getType()==Parameter)
        useParam.insert(it->getName());
    // remove all variables in xmlParam which are not in var (these were not added as a parameter by the --param
    // option during creating of the FMU
    vector<std::shared_ptr<Variable> > xmlParam2;
    for(auto & it : xmlParam)
      if(useParam.find(it->getName())!=useParam.end())
        xmlParam2.push_back(it);
    xmlParam=xmlParam2;
    // add model parameters to varSim
    msg(Debug)<<"Create model parameter variables."<<endl;
    varSim.insert(varSim.end(), xmlParam.begin(), xmlParam.end());
  
    // create object for DynamicSystemSolver
    msg(Debug)<<"Create DynamicSystemSolver."<<endl;
    doc->normalizeDocument();
    dss.reset(ObjectFactory::createAndInit<DynamicSystemSolver>(E(ele)->getFirstElementChildNamed(MBSIM%"DynamicSystemSolver")));

    if(cosim) {
      msg(Debug)<<"Create Integrator."<<endl;
      auto eleDSS=E(ele)->getFirstElementChildNamed(MBSIM%"DynamicSystemSolver");
      integrator.reset(ObjectFactory::createAndInit<Integrator>(eleDSS->getNextElementSibling()));
    }
  }

}

namespace {

  double getValueAsDouble(MBSimFMI::Variable &v) {
    switch(v.getDatatypeChar()) {
      case 'r':
        return v.getValue(double());
      case 'i':
        return v.getValue(int());
      case 'b':
        return v.getValue(bool());
      default:
        throw runtime_error("Internal error: Unknwon type.");
    }
  }

  string getName(MBSimFMI::Variable &v) {
    string name=v.getName();
    return name.substr(0, name.find('['));
  }

  size_t getCol(MBSimFMI::Variable &v) {
    string name=v.getName();
    name=name.substr(name.find('[')+1);
    name=name.substr(name.find(',')+1);
    return boost::lexical_cast<size_t>(name.substr(0, name.size()-1));
  }

  void convertVariableToParamSet(size_t startIndex, const vector<std::shared_ptr<MBSimFMI::Variable> > &var,
                                 std::shared_ptr<Preprocess::ParamSet> &param, const shared_ptr<Eval> &eval) {
    for(auto it=var.begin()+startIndex; it!=var.end(); ++it) {
      // skip all but parameters
      if((*it)->getType()!=MBSimFMI::Parameter)
        continue;
      // handle string parameters (can only be scalars)
      if((*it)->getDatatypeChar()=='s') {
        if(!param->emplace((*it)->getName(), eval->create((*it)->getValue(string()))).second)
          throw runtime_error("Cannot add parameter. A parameter with the same name already exists.");
      }
      // handle none string parameters (can be scalar, vector or matrix)
      else {
        // get name and type (scalar, vector or matrix)
        string name=(*it)->getName();
        size_t poss=name.find('[');
        Eval::ValueType type=Eval::ScalarType;
        if(poss!=string::npos) {
          type=Eval::VectorType;
          if(name.substr(poss+1).find(',')!=string::npos)
            type=Eval::MatrixType;
        }
        name=name.substr(0, poss);
        Eval::Value value;
        switch(type) {
          case Eval::ScalarType:
            value=eval->create(getValueAsDouble(**it));
            break;
          case Eval::VectorType: {
            vector<double> vec;
            for(; it!=var.end() && name==getName(**it); ++it)
              vec.push_back(getValueAsDouble(**it));
            --it;
            value=eval->create(vec);
            break;
          }
          case Eval::MatrixType: {
            vector<vector<double> > mat;
            for(; it!=var.end() && name==getName(**it);) {
              vector<double> row;
              for(; it!=var.end() && name==getName(**it) && !(getCol(**it)==1 && row.size()>0); ++it)
                row.push_back(getValueAsDouble(**it));
              mat.push_back(row);
            }
            --it;
            value=eval->create(mat);
            break;
          }
          default:
            throw runtime_error("Internal error: Unknwon type.");
        }
        if(!param->emplace(name, value).second)
          throw runtime_error("Cannot add parameter. A parameter with the same name already exists.");
      }
    }
  }

}
