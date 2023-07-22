#include "config.h"
#include <mbxmlutils/eval.h>
#include <mbsimxml/mbsimflatxml.h>
#include <boost/dll.hpp>
#include <mbxmlutilshelper/shared_library.h>
#include <mbsim/objectfactory.h>
#include <mbsim/dynamic_system_solver.h>
#include <mbsim/integrators/integrator.h>
#include <mbsimxml/mbsimxml.h>
#include <openmbvcppinterface/objectfactory.h>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "zip.h"
#include <../general/fmi_variables.h>
#include <../general/xmlpp_utils.h>
#include <../general/mbsimsrc_fmi.h>

using namespace std;
using namespace boost::filesystem;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;
using namespace MBSimFMI;

namespace {
  // some platform dependent values
#ifdef _WIN32
  string USERENVVAR("USERNAME");
  path LIBDIR="bin";
#else
  string USERENVVAR("USER");
  path LIBDIR="lib";
#endif

  void copyShLibToFMU(const std::shared_ptr<DOMParser> &parser,
                      CreateZip &fmuFile, const path &dst, const path &depdstdir, const path &src);
}

int main(int argc, char *argv[]) {
  try {
    // check for errors during ObjectFactory
    string errorMsg(OpenMBV::ObjectFactory::getAndClearErrorMsg());
    if(!errorMsg.empty()) {
      cerr<<"The following errors occured during the pre-main code of the OpenMBVC++Interface object factory:"<<endl;
      cerr<<errorMsg;
      cerr<<"Exiting now."<<endl;
      return 1;
    }

    // check for errors during ObjectFactory
    string errorMsg2(ObjectFactory::getAndClearErrorMsg());
    if(!errorMsg2.empty()) {
      cerr<<"The following errors occured during the pre-main code of the MBSim object factory:"<<endl;
      cerr<<errorMsg2;
      cerr<<"Exiting now."<<endl;
      return 1;
    }

    boost::filesystem::path installPath(boost::dll::program_location().parent_path().parent_path());

    // help
    if(argc<2) {
      cout<<"Usage: "<<argv[0]<<" [--nocompress] [--cosim] [--param <name> [--param <name> ...]]"<<endl;
      cout<<"  <MBSim Project XML File>|<MBSim FMI shared library>"<<endl;
      cout<<endl;
      cout<<"Creates mbsim.fmu in the current directory."<<endl;
      cout<<"The MBSim model is taken from the provided XML project file or shared library file."<<endl;
      cout<<"For XML project model files --param may be specified 0 to n times. Where <name> defines"<<endl;
      cout<<"a parameter name of the DynamicSystemSolver Embed element which should be"<<endl;
      cout<<"provided as an FMU parameter. Note that only parameters which do not depend"<<endl;
      cout<<"on any other parameters are allowed. If no --param option is given all are exported."<<endl;
      cout<<"Use --noparam to ignore all parameters defined the DynamicSystemSolver Embed element."<<endl;
      cout<<"Use --cosim to generate a FMI for Cosimulation FMU instead of a"<<endl;
      cout<<"FMI for Model-Exchange FMU."<<endl;
      return 0;
    }

    // get parameters
    int optcount=0;
    set<string> useParam;
    bool compress=true;
    bool noParam=false;
    bool cosim=false;
    for(int i=1; i<argc-1; ++i) {
      if(string(argv[i])=="--param")
        useParam.insert(argv[i+1]);
      if(string(argv[i])=="--nocompress") {
        compress=false;
        optcount++;
      }
      if(string(argv[i])=="--noparam") {
        noParam=true;
        optcount++;
      }
      if(string(argv[i])=="--cosim") {
        cosim=true;
        optcount++;
      }
    }

    // get model file
    if(2*useParam.size()+1+optcount>=argc) {
      cout<<"Wrong command line."<<endl;
      return 1;
    }
    path inputFilename=argv[2*useParam.size()+1+optcount];

    // xml or shared library
    bool xmlFile=false;
    if(boost::iequals(extension(inputFilename), ".mbsx"))
      xmlFile=true;

    // create FMU zip file
    CreateZip fmuFile("mbsim.fmu", compress);

    // load all MBSim modules
    cout<<"Load MBSim modules."<<endl;
    set<path> moduleLibs=MBSimXML::loadModules();
    // check for errors during ObjectFactory
    string errorMsg3(ObjectFactory::getAndClearErrorMsg());
    if(!errorMsg3.empty()) {
      cerr<<"The following errors occured during the loading of MBSim modules object factory:"<<endl;
      cerr<<errorMsg3;
      cerr<<"Exiting now."<<endl;
      return 1;
    }

    // create parser (none validating parser for use in copyShLibToFMU)
    std::shared_ptr<DOMParser> parserNoneVali=DOMParser::create();

    // note: the order of these variable definitions is importend for proper deallocation
    std::shared_ptr<DynamicSystemSolver> dss;
    std::shared_ptr<Integrator> integrator;

    vector<path> dependencies;

    PredefinedParameterStruct predefinedParameterStruct;
    cout<<"Create predefined parameters."<<endl;
    vector<std::shared_ptr<Variable> > var;
    addPredefinedParameters(cosim, var, predefinedParameterStruct, true);

    vector<std::shared_ptr<Variable> > xmlParam;

    string evalName="octave"; // default evaluator
    std::shared_ptr<Eval> eval;

    // Create dss from XML file
    if(xmlFile) {
      // init the validating parser with the mbsimxml schema file
      cout<<"Create MBSimXML XML schema including all modules."<<endl;
      auto xmlCatalog=getMBSimXMLCatalog();

      // create parser (a validating parser for XML input and a none validating parser for shared library input)
      cout<<"Create validating XML parser."<<endl;
      std::shared_ptr<DOMParser> parser=DOMParser::create(xmlCatalog->getDocumentElement());

      // load MBSim project XML document
      cout<<"Load MBSim model from XML project file."<<endl;
      std::shared_ptr<DOMDocument> modelDoc=parser->parse(inputFilename, &dependencies, false);
      DOMElement *modelEle=modelDoc->getDocumentElement();

      // create a clean evaluator (get the evaluator name first form the dom)
      DOMElement *evaluator;
      if(E(modelEle)->getTagName()==PV%"Embed") {
        // if the root element IS A Embed than the <evaluator> element is the first child of the
        // first (none pv:Parameter) child of the root element
        auto r=modelEle->getFirstElementChild();
        if(E(r)->getTagName()==PV%"Parameter")
          r=r->getNextElementSibling();
        evaluator=E(r)->getFirstElementChildNamed(PV%"evaluator");
      }
      else
        // if the root element IS NOT A Embed than the <evaluator> element is the first child root element
        evaluator=E(modelEle)->getFirstElementChildNamed(PV%"evaluator");
      if(evaluator)
        evalName=X()%E(evaluator)->getFirstTextChild()->getData();
      eval=Eval::createEvaluator(evalName, &dependencies);

      // preprocess XML file
      cout<<"Preprocess XML project file."<<endl;
      std::shared_ptr<Preprocess::ParamSet> param=std::make_shared<Preprocess::ParamSet>();
      Preprocess::preprocess(parser, eval, dependencies, modelEle, param);

      // convert the parameter list from the mbxmlutils preprocessor to a Variable vector
      convertParamSetToVariable(param, xmlParam, eval);
      // remove all variables which are not in useParam
      vector<std::shared_ptr<Variable> > xmlParam2;
      for(auto & it : xmlParam) {
        string name=it->getName();
        size_t pos=name.find('[');
        if(pos!=string::npos)
          name=name.substr(0, pos);
        if(!noParam && (useParam.size()==0 || useParam.find(name)!=useParam.end())) {
          cout<<"Using MBSimProject parameter '"<<name<<"'."<<endl;
          xmlParam2.push_back(it);
        }
      }
      xmlParam=xmlParam2;
      cout<<"Create model parameters."<<endl;
      var.insert(var.end(), xmlParam.begin(), xmlParam.end());

      if(xmlParam.empty()) {
        // adapt the evaluator in the dom (reset evaluator because it may change if the root element is a Embed)
        evaluator=E(modelEle)->getFirstElementChildNamed(PV%"evaluator");
        if(evaluator)
          E(evaluator)->getFirstTextChild()->setData(X()%"xmlflat");
        else {
          evaluator=D(modelDoc)->createElement(PV%"evaluator");
          evaluator->appendChild(modelDoc->createTextNode(X()%"xmlflat"));
          modelEle->insertBefore(evaluator, modelEle->getFirstChild());
        }

        cout<<"Copy preprocessed XML model file to FMU."<<endl;
        // no parameters -> save preprocessed model to FMU
        string ppModelStr;
        // serialize to string: this automatically normalized modelEle
        DOMParser::serializeToString(modelEle, ppModelStr, false);
        fmuFile.add(path("resources")/"model"/"Model.flat.mbsx", ppModelStr);
      }
      else {
        cout<<"Copy original XML model file to FMU."<<endl;
        // normalize modelEle
        D(modelEle->getOwnerDocument())->normalizeDocument();
        // parameters existing -> save original XML file to FMU including all dependencies
        // Note: We copy the XML model file including all dependencies to resources/model. However the full absolute path
        // (excluding the leading '/' or 'c:\') of each file is added as subdir of resources/model. This will
        // enable XML models with file references to all parent directories. Hence only absolute file references
        // are not allowed!

        // path to XML project file relative to resources/model
        fmuFile.add(path("resources")/"model"/"XMLProjectFile.txt", absolute(inputFilename).relative_path().string());
        // copy XML project file
        fmuFile.add(path("resources")/"model"/absolute(inputFilename).relative_path(), inputFilename);
        // copy dependencies
        cout<<"Copy dependent files of the original XML model file to FMU."<<endl;
        for(auto & dependency : dependencies)
          if(!is_directory(dependency)) {
            if(dependency.is_absolute())
              throw runtime_error("A XML model file with parameters may only reference files by a relative path.\n"
                                  "However the model references the absolute file '"+dependency.string()+"'.\n"+
                                  "Use the --noparam options OR rework the model to not contain any absolute file path.");
            cout<<"."<<flush;
            fmuFile.add(path("resources")/"model"/current_path().relative_path()/dependency, dependency);
          }
        cout<<endl;
      }

      // create object for DynamicSystemSolver
      // Note: The document modelEle must be normalized (done above, see above)
      cout<<"Build up the model."<<endl;
      DOMElement *dssEle=E(modelEle)->getFirstElementChildNamed(MBSIM%"DynamicSystemSolver");
      dss.reset(ObjectFactory::createAndInit<DynamicSystemSolver>(dssEle));

      // create object for Integrator (just to get the start/end time for DefaultExperiment)
      integrator.reset(ObjectFactory::createAndInit<Integrator>(dssEle->getNextElementSibling()));
    }
    // Create dss from shared library
    else {
      // save binary model to FMU
      cout<<"Copy the shared library model file and dependencies to FMU."<<endl;
      copyShLibToFMU(parserNoneVali, fmuFile, path("resources")/"model"/("libmbsimfmi_model"+SHEXT), path("resources")/"model",
                     inputFilename);
      cout<<endl;

      // load the shared library and call mbsimSrcFMI function to get the dss
      cout<<"Build up the model (by just getting it from the shared library)."<<endl;
      DynamicSystemSolver *dssPtr;
      Integrator *integratorPtr;
      SharedLibrary::getSymbol<mbsimSrcFMIPtr>(canonical(inputFilename).string(), "mbsimSrcFMI")(dssPtr, integratorPtr);
      dss.reset(dssPtr);

      if(!cosim)
        // we do not have a integrator for src models (but this is only used for convinence settings)
        integrator.reset();
      else {
        if(!integratorPtr)
          throw runtime_error("A cosim FMU is generated but not Integrator is provided.");
        integrator.reset(integratorPtr);
      }
    }

    // disable all plotting (we wont any output here)
    dss->setPlotFeatureRecursive(plotRecursive, false);
    dss->setPlotFeatureRecursive(openMBV, false);
    // initialize dss
    cout<<"Initialize the model."<<endl;
    dss->initialize();

    if(cosim) {
      try {
        integrator->setSystem(dss.get());
        integrator->preIntegrate();
      }
      catch(MBSimError &ex) {
        auto &integratorRef=*integrator;
        throw runtime_error("The used integrator "+boost::core::demangle(typeid(integratorRef).name())+
          " failed to initialize the co-simulation interface. Error message was:\n"+
          ex.what()+"\n"+
          "The model may be wrong or this integrator cannot be used for cosim FMUs.");
      }
    }

    // build list of value references
    cout<<"Create model input/output variables."<<endl;
    addModelInputOutputs(var, dss.get());

    // create DOM of modelDescription.xml
    std::shared_ptr<DOMDocument> modelDescDoc(parserNoneVali->createDocument());

    // root element fmiModelDescription and its attributes
    cout<<"Create the modelDescription.xml file."<<endl;
    DOMElement *modelDesc=D(modelDescDoc)->createElement("fmiModelDescription");
    modelDescDoc->appendChild(modelDesc);
    E(modelDesc)->setAttribute("author", getenv(USERENVVAR.c_str()));
    E(modelDesc)->setAttribute("description", "FMI export of a MBSim-XML model");
    E(modelDesc)->setAttribute("fmiVersion", "1.0");
    E(modelDesc)->setAttribute("generationDateAndTime",
      boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time())+"Z");
    E(modelDesc)->setAttribute("generationTool", string("MBSimFMI Version ")+VERSION);
    E(modelDesc)->setAttribute("version", "1.0");
    E(modelDesc)->setAttribute("guid", "mbsimfmi_guid");
    E(modelDesc)->setAttribute("modelIdentifier", "mbsim");
    path desc=inputFilename.filename();
    desc.replace_extension();
    desc.replace_extension();
    E(modelDesc)->setAttribute("modelName", desc.string());
    E(modelDesc)->setAttribute("numberOfContinuousStates", cosim?0:dss->getzSize());
    E(modelDesc)->setAttribute("numberOfEventIndicators", cosim?0:dss->getsvSize());
    E(modelDesc)->setAttribute("variableNamingConvention", "structured");

      // Type definition
      // get a unique list of all enumeration types
      set<Variable::EnumList> enumType;
      for(auto & vr : var)
        if(vr->getEnumerationList())
          enumType.insert(vr->getEnumerationList());
      // write all enumeration type to xml file
      DOMElement *typeDef=D(modelDescDoc)->createElement("TypeDefinitions");
      modelDesc->appendChild(typeDef);
        for(const auto & it : enumType) {
          DOMElement *type=D(modelDescDoc)->createElement("Type");
          typeDef->appendChild(type);
          E(type)->setAttribute("name", "EnumType_"+boost::lexical_cast<string>(it));
            DOMElement *enumEle=D(modelDescDoc)->createElement("EnumerationType");
            type->appendChild(enumEle);
            E(enumEle)->setAttribute("min", "1");
            E(enumEle)->setAttribute("max", it->size());
            for(size_t id=0; id<it->size(); ++id) {
              DOMElement *item=D(modelDescDoc)->createElement("Item");
              enumEle->appendChild(item);
              E(item)->setAttribute("name", (*it)[id].second);
              E(item)->setAttribute("description", (*it)[id].second);
            }
        }

      // DefaultExperiment element and its attributes (only if a integrator object exists)
      if(integrator) {
        DOMElement *defaultExp=D(modelDescDoc)->createElement("DefaultExperiment");
        modelDesc->appendChild(defaultExp);
        E(defaultExp)->setAttribute("startTime", integrator->getStartTime());
        E(defaultExp)->setAttribute("stopTime", integrator->getEndTime());
        E(defaultExp)->setAttribute("tolerance", 1e-5);
      }

      // ModelVariables element
      DOMElement *modelVars=D(modelDescDoc)->createElement("ModelVariables");
      modelDesc->appendChild(modelVars);

        // loop over all FMI variables
        for(size_t vr=0; vr<var.size(); ++vr) {
          // create ScalarVariable element
          DOMElement *scalarVar=D(modelDescDoc)->createElement("ScalarVariable");
          modelVars->appendChild(scalarVar);

            // create datatype element
            string datatypeEleName;
            switch(var[vr]->getDatatypeChar()) {
              case 'r': datatypeEleName="Real"; break;
              case 'i': datatypeEleName= var[vr]->getEnumerationList() ? "Enumeration" : "Integer"; break;
              case 'b': datatypeEleName="Boolean"; break;
              case 's': datatypeEleName="String"; break;
            }
            DOMElement *varType=D(modelDescDoc)->createElement(datatypeEleName);
            scalarVar->appendChild(varType);

            // handle enumeration
            if(var[vr]->getEnumerationList()) {
              E(varType)->setAttribute("declaredType", "EnumType_"+boost::lexical_cast<string>(var[vr]->getEnumerationList()));
              E(varType)->setAttribute("min", "1");
              E(varType)->setAttribute("max", var[vr]->getEnumerationList()->size());
            }

          // attributes on ScalarVariable element
          E(scalarVar)->setAttribute("name", var[vr]->getName());
          E(scalarVar)->setAttribute("description", var[vr]->getDescription());
          E(scalarVar)->setAttribute("valueReference", vr);
          switch(var[vr]->getType()) {
            case Parameter:
              E(scalarVar)->setAttribute("causality", "internal");
              E(scalarVar)->setAttribute("variability", "parameter");
              E(varType)->setAttribute("start", var[vr]->getValueAsString());
              E(varType)->setAttribute("fixed", "true");
              break;
            case Input:
              E(scalarVar)->setAttribute("causality", "input");
              E(scalarVar)->setAttribute("variability", "continuous");
              E(varType)->setAttribute("start", var[vr]->getValueAsString());
              E(varType)->setAttribute("fixed", "true");
              break;
            case Output:
              E(scalarVar)->setAttribute("causality", "output");
              E(scalarVar)->setAttribute("variability", "continuous");
              break;
          }
        }

      if(cosim) {
        // Implementation element
        DOMElement *implementation=D(modelDescDoc)->createElement("Implementation");
        modelDesc->appendChild(implementation);
          DOMElement *cosimStandalone=D(modelDescDoc)->createElement("CoSimulation_StandAlone");
          implementation->appendChild(cosimStandalone);
            DOMElement *capabilities=D(modelDescDoc)->createElement("Capabilities");
            cosimStandalone->appendChild(capabilities);
            E(capabilities)->setAttribute("canHandleVariableCommunicationStepSize", true);
            E(capabilities)->setAttribute("canInterpolateInputs", false);
            E(capabilities)->setAttribute("canNotUseMemoryManagementFunctions", true);
      }

    // add modelDescription.xml file to FMU
    cout<<"Copy the modelDescription.xml file to FMU."<<endl;
    string modelDescriptionStr;
    DOMParser::serializeToString(modelDescDoc.get(), modelDescriptionStr);
    fmuFile.add("modelDescription.xml", modelDescriptionStr);

    // add main FMU binary (and other required files) to FMU file
    if(xmlFile) {
      if(xmlParam.empty()) {
        // xml with no parameters -> save libmbsimxml_fmi.so to FMU
        cout<<"Copy MBSim FMI library for preprocessed XML models and dependencies to FMU."<<endl;
        copyShLibToFMU(parserNoneVali, fmuFile, path("resources")/"local"/LIBDIR/("libmbsimXXX_fmi"+SHEXT), path("resources")/"local"/LIBDIR,
                       installPath/LIBDIR/("libmbsimxml_fmi"+SHEXT));
        cout<<endl;
      }
      else {
        // xml with parameters -> save libmbsimppxml_fmi.so to FMU
        cout<<"Copy MBSim FMI library for (normal) XML models and dependencies to FMU."<<endl;
        copyShLibToFMU(parserNoneVali, fmuFile, path("resources")/"local"/LIBDIR/("libmbsimXXX_fmi"+SHEXT), path("resources")/"local"/LIBDIR,
                       installPath/LIBDIR/("libmbsimppxml_fmi"+SHEXT));
        cout<<endl;

        cout<<"Copy XML schema files to FMU."<<endl;
        path schemaDir=installPath/"share"/"mbxmlutils"/"schema";
        size_t depth=distance(schemaDir.begin(), schemaDir.end());
        for(recursive_directory_iterator srcIt=recursive_directory_iterator(schemaDir);
            srcIt!=recursive_directory_iterator(); ++srcIt) {
          if(is_directory(*srcIt)) // skip directories
            continue;
          path::iterator dstIt=srcIt->path().begin();
          for(int i=0; i<depth; ++i) ++dstIt;
          path dst;
          for(; dstIt!=srcIt->path().end(); ++dstIt)
            dst/=*dstIt;
          cout<<"."<<flush;
          fmuFile.add(path("resources")/"local"/"share"/"mbxmlutils"/"schema"/dst, srcIt->path());
        }
        cout<<endl;

        cout<<"Copy MBXMLUtils measurement.xml file to FMU."<<endl;
        fmuFile.add(path("resources")/"local"/"share"/"mbxmlutils"/"xml"/"measurement.xml",
          installPath/"share"/"mbxmlutils"/"xml"/"measurement.xml");

        map<path, pair<path, bool> > &files=eval->requiredFiles();
        cout<<"Copy files required by the evaluator and dependencies to FMU."<<endl;
        string evalFile="libmbxmlutils-eval-global-"+evalName+SHEXT;
        if(!exists(installPath/LIBDIR/evalFile))
          evalFile="libmbxmlutils-eval-"+evalName+SHEXT;
        copyShLibToFMU(parserNoneVali, fmuFile, path("resources")/"local"/LIBDIR/evalFile,
                       path("resources")/"local"/LIBDIR, installPath/LIBDIR/evalFile);
        for(auto & file : files) {
          cout<<"."<<flush;
          if(!file.second.second) {
            if(file.first.filename()=="PKG_ADD") {
              // a special hack for octave: fix PKG_ADD file before adding to the archive
              // see also the fixes in the Dockerfile's in mbsim-env/build
              boost::filesystem::ifstream pkgAdd(file.first);
              string content;
              for(string line; getline(pkgAdd, line);) {
                if(line.find("gnuplot")!=string::npos) continue;
                content+=line+"\n";
              }
              fmuFile.add(path("resources")/"local"/file.second.first/file.first.filename(), content);
            }
            else
              fmuFile.add(path("resources")/"local"/file.second.first/file.first.filename(), file.first);
          }
          else
            copyShLibToFMU(parserNoneVali, fmuFile, path("resources")/"local"/file.second.first/file.first.filename(),
                           path("resources")/"local"/LIBDIR, file.first);
        }
        cout<<endl;
      }

      cout<<"Copy MBSim module files to FMU."<<endl;
      for(directory_iterator srcIt=directory_iterator(installPath/"share"/"mbsimmodules");
        srcIt!=directory_iterator(); ++srcIt) {
        cout<<"."<<flush;
        fmuFile.add(path("resources")/"local"/"share"/"mbsimmodules"/srcIt->path().filename(), srcIt->path());
      }
      cout<<endl;
      for(const auto & moduleLib : moduleLibs) {
        cout<<"Copy MBSim module "<<moduleLib.filename()<<" and dependencies to FMU."<<endl;
        copyShLibToFMU(parserNoneVali, fmuFile, path("resources")/"local"/LIBDIR/moduleLib.filename(), path("resources")/"local"/LIBDIR,
                       moduleLib);
        cout<<endl;
      }
    }
    else {
      // source model (always without parameters) -> save libmbsimppxml_fmi.so to FMU
      cout<<"Copy MBSim FMI library for source code models and dependencies to FMU."<<endl;
      copyShLibToFMU(parserNoneVali, fmuFile, path("resources")/"local"/LIBDIR/("libmbsimXXX_fmi"+SHEXT), path("resources")/"local"/LIBDIR,
                     installPath/LIBDIR/("libmbsimsrc_fmi"+SHEXT));
      cout<<endl;
    }

    cout<<"Copy MBSim FMI wrapper library and dependencies to FMU."<<endl;
    string fmuLibName(cosim?"mbsim_cosim":"mbsim_me");
    copyShLibToFMU(parserNoneVali, fmuFile, path("binaries")/FMIOS/("mbsim"+SHEXT), path("binaries")/FMIOS,
                   installPath/"lib"/(fmuLibName+SHEXT));
    cout<<endl;

    fmuFile.close();

    return 0;
  }
  catch(const exception &ex) {
    cerr<<ex.what()<<endl;
    return 1;
  }
  catch(...) {
    cerr<<"Unknwon exception."<<endl;
    return 1;
  }
}

namespace {

  path addDebugExtension(const path &p) {
    path debug=p;
    return debug.replace_extension(debug.extension().string()+".debug");
  }

  void copyShLibToFMU(const std::shared_ptr<DOMParser> &parser,
                      CreateZip &fmuFile, const path &dst, const path &depdstdir, const path &src) {
    static boost::filesystem::path installPath(boost::dll::program_location().parent_path().parent_path());

    // copy src to FMU
    cout<<"."<<flush;
    fmuFile.add(dst, src);
    if(exists(addDebugExtension(src)))
      fmuFile.add(addDebugExtension(dst), addDebugExtension(src));

    // check if *.deplibs file exits
    path depFile=src.parent_path()/(src.filename().string()+".deplibs");
    if(!exists(depFile)) {
      depFile=installPath/"share"/"deplibs"/(src.filename().string()+".deplibs");
      if(!exists(depFile)) {
        cerr<<endl<<"Warning: No *.deplibs file found for library "<<src<<"."<<endl<<"Some dependent libraries may be missing in the FMU."<<endl;
        return;
      }
    }

    // read *.deplibs file
    std::shared_ptr<DOMDocument> depDoc=parser->parse(depFile);
    for(DOMElement *e=depDoc->getDocumentElement()->getFirstElementChild(); e!=nullptr; e=e->getNextElementSibling()) {
      string file=X()%E(e)->getFirstTextChild()->getData();

      // check for file in reldir and copy it to FMU
      path reldir=E(e)->getAttribute("reldir");
      if(exists(installPath/reldir/file)) {
        cout<<"."<<flush;
        fmuFile.add(depdstdir/file, installPath/reldir/file);
        if(exists(addDebugExtension(installPath/reldir/file)))
          fmuFile.add(addDebugExtension(depdstdir/file), addDebugExtension(installPath/reldir/file));
        continue;
      }

      // check for file in orgdir and copy it to FMU
      path orgdir=E(e)->getAttribute("orgdir");
      if(exists(orgdir/file)) {
        cout<<"."<<flush;
        fmuFile.add(depdstdir/file, orgdir/file);
        if(exists(addDebugExtension(orgdir/file)))
          fmuFile.add(addDebugExtension(depdstdir/file), addDebugExtension(orgdir/file));
        continue;
      }

      // not found
      cerr<<endl<<"Warning: Dependent library "<<file<<" not found."<<endl<<"This dependent libraries will be missing in the FMU."<<endl;
    }
  }

}
