#include "../config.h"
#include <fmiinstance.h>
#include <stdexcept>
#include <mbxmlutilshelper/dom.h>
#include <xercesc/dom/DOMDocument.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>
#include <mbsim/dynamic_system_solver.h>
#include <mbsim/integrators/integrator.h>
#include <mbxmlutilshelper/thislinelocation.h>

// rethrow a catched exception after prefixing the what() string with the FMI variable name
#define RETHROW_VR(vr) \
  catch(const std::exception &ex) { \
    rethrowVR(vr, ex); \
  } \
  catch(...) { \
    rethrowVR(vr); \
  }

using namespace std;
using namespace boost::filesystem;
using namespace MBSim;
using namespace MBSimControl;
using namespace MBXMLUtils;

namespace {

  template<class Datatype>
  void addPreInitVariable(const xercesc::DOMElement *scalarVar, vector<shared_ptr<MBSimFMI::Variable> > &var) {
    // get type
    MBSimFMI::Type type;
    if     (E(scalarVar)->getAttribute("causality")=="internal" && E(scalarVar)->getAttribute("variability")=="parameter")
      type=MBSimFMI::Parameter;
    else if(E(scalarVar)->getAttribute("causality")=="input"    && E(scalarVar)->getAttribute("variability")=="continuous")
      type=MBSimFMI::Input;
    else if(E(scalarVar)->getAttribute("causality")=="output"   && E(scalarVar)->getAttribute("variability")=="continuous")
      type=MBSimFMI::Output;
    else
      throw runtime_error("Internal error: Unknwon variable type.");
    // get default
    Datatype defaultValue{};
    if(E(scalarVar->getFirstElementChild())->hasAttribute("start"))
      defaultValue=boost::lexical_cast<Datatype>(E(scalarVar->getFirstElementChild())->getAttribute("start"));
    // create preprocessing variable
    var.push_back(make_shared<MBSimFMI::VariableStore<Datatype> >(E(scalarVar)->getAttribute("name"), type, defaultValue));
  }

}

namespace MBSimFMI {

  ThisLineLocation fmuLoc;

  shared_ptr<FMIInstanceBase> fmiInstanceCreate(bool cosim, fmiString instanceName_, fmiString GUID,
                                                fmiCallbackLogger logger, fmiBoolean loggingOn) {
    return shared_ptr<FMIInstance>(new FMIInstance(cosim, instanceName_, GUID, logger, loggingOn));
  }

  // A MBSim FMI instance. Called by fmiInstantiateModel
  FMIInstance::FMIInstance(bool cosim_, fmiString instanceName_, fmiString GUID, fmiCallbackLogger logger_, fmiBoolean loggingOn) :
    cosim(cosim_),
    instanceName(instanceName_),
    logger(logger_),
    time(timeStore),
    z(zStore) {

    driftCompensation=none; // only needed for ME

    // use the per FMIInstance provided buffers for all subsequent fmatvec::Atom objects
    auto f=[this](const string &s, fmiStatus status, const string &category){
      logger(this, instanceName.c_str(), status, category.c_str(), s.c_str());
    };
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom:: Info,       make_shared<bool>(true ),
      make_shared<fmatvec::PrePostfixedStream>("", "", bind(f, placeholders::_1, fmiOK     , "info"      )));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom:: Warn,       make_shared<bool>(true ),
      make_shared<fmatvec::PrePostfixedStream>("", "", bind(f, placeholders::_1, fmiWarning, "warning"   )));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom:: Debug,      make_shared<bool>(true ),
      make_shared<fmatvec::PrePostfixedStream>("", "", bind(f, placeholders::_1, fmiOK     , "debug"     )));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom:: Error,      make_shared<bool>(true ),
      make_shared<fmatvec::PrePostfixedStream>("", "", bind(f, placeholders::_1, fmiError  , "error"     )));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom:: Deprecated, make_shared<bool>(true ),
      make_shared<fmatvec::PrePostfixedStream>("", "", bind(f, placeholders::_1, fmiWarning, "deprecated")));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom:: Status,     make_shared<bool>(false));
    // also use these streams for this object.
    // Note: we can not create a FMIInstance object with the correct streams but we can adopt the streams now!
    adoptMessageStreams(); // note: no arg means adopt the current (static) message streams (set above)
    // Now we can use msg(...)<< to print messages using the FMI logger

    // set debug stream according loggingOn
    setMessageStreamActive(Debug, loggingOn);
    msg(Debug)<<"Enabling debug logging."<<endl;

    // check GUID: we use currently a constant GUID
    if(string(GUID)!="mbsimfmi_guid")
      throw runtime_error("GUID provided by caller and internal GUID does not match.");

    // load modelDescription XML file
    parser=DOMParser::create();
    msg(Debug)<<"Read modelDescription file."<<endl;
    path modelDescriptionXMLFile=path(fmuLoc()).lexically_normal().parent_path().parent_path().parent_path().parent_path()/
      "modelDescription.xml";
    shared_ptr<xercesc::DOMDocument> doc=parser->parse(modelDescriptionXMLFile, nullptr, false);

    if(!cosim) {
      // init state vector size (just to be usable before initialize is called)
      z.get().resize(boost::lexical_cast<size_t>(E(doc->getDocumentElement())->getAttribute("numberOfContinuousStates")));
      svLast.resize(boost::lexical_cast<size_t>(E(doc->getDocumentElement())->getAttribute("numberOfEventIndicators")));
    }

    // add all predefined parameters
    addPredefinedParameters(cosim, var, predefinedParameterStruct, true);
    size_t numPredefParam=var.size();
    // create FMI variables from modelDescription.xml file
    msg(Debug)<<"Generate call variables as VariableStore objects. Used until fmiInitialize is called."<<endl;
    size_t vr=0;
    for(xercesc::DOMElement *scalarVar=E(doc->getDocumentElement())->getFirstElementChildNamed("ModelVariables")->getFirstElementChild();
        scalarVar; scalarVar=scalarVar->getNextElementSibling(), ++vr) {
      // skip all predefined parameters which are already added by addPredefinedParameters above
      if(vr<numPredefParam)
        continue;

      // now add all other parameters
      msg(Debug)<<"Generate variable '"<<E(scalarVar)->getAttribute("name")<<"'"<<endl;
      if(vr!=boost::lexical_cast<size_t>(E(scalarVar)->getAttribute("valueReference")))
        throw runtime_error("Internal error: valueReference missmatch!");
      // add variable
      if(E(scalarVar)->getFirstElementChildNamed("Real"))
        addPreInitVariable<double>(scalarVar, var);
      else if(E(scalarVar)->getFirstElementChildNamed("Integer") ||
              E(scalarVar)->getFirstElementChildNamed("Enumeration"))
        addPreInitVariable<int>(scalarVar, var);
      else if(E(scalarVar)->getFirstElementChildNamed("Boolean"))
        addPreInitVariable<bool>(scalarVar, var);
      else if(E(scalarVar)->getFirstElementChildNamed("String"))
        addPreInitVariable<string>(scalarVar, var);
      else
        throw runtime_error("Internal error: Unknown variable datatype.");
    }
  }

  // destroy a MBSim FMI instance. Called by fmiFreeModelInstance
  FMIInstance::~FMIInstance() = default;

  // FMI wrapper functions, except fmiInstantiateModel and fmiFreeModelInstance see above

  void FMIInstance::setDebugLogging(fmiBoolean loggingOn) {
    if(!loggingOn)
      msg(Debug)<<"Disabling debug logging."<<endl;
    // set debug stream according loggingOn
    setMessageStreamActive(Debug, loggingOn);
    if(loggingOn)
      msg(Debug)<<"Enabling debug logging."<<endl;
  }

  // set a real/integer/boolean/string variable
  template<typename CppDatatype, typename FMIDatatype>
  void FMIInstance::setValue(const fmiValueReference vr[], size_t nvr, const FMIDatatype value[]) {
    for(size_t i=0; i<nvr; ++i) {
      if(vr[i]>=var.size())
        throw runtime_error("No such value reference "+fmatvec::toString(vr[i]));
      try { var[vr[i]]->setValue(CppDatatype(value[i])); } RETHROW_VR(vr[i])
    }
    if(dss)
      dss->resetUpToDate();
  }
  // explicitly instantiate all four FMI types
  template void FMIInstance::setValue<double, fmiReal   >(const fmiValueReference vr[], size_t nvr, const fmiReal    value[]);
  template void FMIInstance::setValue<int,    fmiInteger>(const fmiValueReference vr[], size_t nvr, const fmiInteger value[]);
  template void FMIInstance::setValue<bool,   fmiBoolean>(const fmiValueReference vr[], size_t nvr, const fmiBoolean value[]);
  template void FMIInstance::setValue<string, fmiString >(const fmiValueReference vr[], size_t nvr, const fmiString  value[]);

  namespace {
    // convert a CppDatatype to FMIDatatype: default implementaion: just use implicit conversion.
    template<class RetType, class ArgType>
    RetType cppDatatypeToFMIDatatype(const ArgType &a) { return a; }
    // convert a CppDatatype to FMIDatatype: specialization for string: return the c string part of the reference string.
    template<>
    fmiString cppDatatypeToFMIDatatype<fmiString, string>(const string &a) { return a.c_str(); }
  }

  // get a real/integer/boolean/string variable
  template<typename CppDatatype, typename FMIDatatype>
  void FMIInstance::getValue(const fmiValueReference vr[], size_t nvr, FMIDatatype value[]) {
    for(size_t i=0; i<nvr; ++i) {
      if(vr[i]>=var.size())
        throw runtime_error("No such value reference "+fmatvec::toString(vr[i]));
      try { value[i]=cppDatatypeToFMIDatatype<FMIDatatype, CppDatatype>(var[vr[i]]->getValue(CppDatatype())); } RETHROW_VR(vr[i])
    }
  }
  // explicitly instantiate all four FMI types
  template void FMIInstance::getValue<double, fmiReal   >(const fmiValueReference vr[], size_t nvr, fmiReal value[]);
  template void FMIInstance::getValue<int,    fmiInteger>(const fmiValueReference vr[], size_t nvr, fmiInteger value[]);
  template void FMIInstance::getValue<bool,   fmiBoolean>(const fmiValueReference vr[], size_t nvr, fmiBoolean value[]);
  template void FMIInstance::getValue<string, fmiString> (const fmiValueReference vr[], size_t nvr, fmiString value[]);

  void FMIInstance::terminate() {
    if(dss) {
      if(cosim) {
        // finish cosim integration
        integrator->setSystem(dss.get());
        integrator->postIntegrate();
      }
      else
        // plot end state
        dss->plot();
    }

    // delete DynamicSystemSolver (requried here since after terminate a call to initialize is allowed without
    // calls to fmiFreeModelInstance and fmiInstantiateModel)
    time=std::ref(timeStore);
    z=std::ref(zStore); // only used for ME
    dss.reset();
  }

  // FMI helper functions

  // print exceptions using the FMI logger
  void FMIInstance::logException(const std::exception &ex) {
    logger(this, instanceName.c_str(), fmiError, "error", ex.what());
  }

  // rethrow a exception thrown during a operation on a valueReference: prefix the exception text with the variable name.
  void FMIInstance::rethrowVR(size_t vr, const std::exception &ex) {
    throw runtime_error(string("In variable '#")+var[vr]->getDatatypeChar()+fmatvec::toString(vr)+"#': "+ex.what());
  }

  void FMIInstance::initialize() {
    // after the ctor call another FMIInstance ctor may be called, hence we need to reset the message streams here
    // use the per FMIInstance provided buffers for all subsequent fmatvec::Atom objects
    shared_ptr<bool> a;
    shared_ptr<ostream> s;
    getMessageStream(fmatvec::Atom::Info,  a, s); fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Info,  a, s);
    getMessageStream(fmatvec::Atom::Warn,  a, s); fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Warn,  a, s);
    getMessageStream(fmatvec::Atom::Debug, a, s); fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Debug, a, s);

    // predefined variables used during simulation
    msg(Debug)<<"Create predefined parameters."<<endl;
    vector<shared_ptr<Variable>> varSim;
    addPredefinedParameters(cosim, varSim, predefinedParameterStruct, false);
    
    // create output directory
    create_directories(absolute(predefinedParameterStruct.outputDir));

    // add model parmeters to varSim and create the DynamicSystemSolver (and the Integrator for cosim FMUs)
    addModelParametersAndCreateSystem(varSim);

    // system time
    dss->setTime(time);
    time=std::ref(dss->getTime());

    // save the current dir and change to outputDir -> MBSim will create output files in the current dir
    // this must be done before the dss is initialized since dss->initialize creates files in the current dir)
    msg(Debug)<<"Write MBSim output files to "<<predefinedParameterStruct.outputDir<<endl;
    path savedCurDir=current_path();
    // restore current dir on scope exit
    BOOST_SCOPE_EXIT((&savedCurDir)) { current_path(savedCurDir); } BOOST_SCOPE_EXIT_END
    current_path(predefinedParameterStruct.outputDir);

    // initialize dss (must be done before addModelInputOutputs because references in MBSim may be resolved for this;
    // must be done after the current dir is set (temporarily) to the output dir)
    msg(Debug)<<"Initialize DynamicSystemSolver."<<endl;
    dss->initialize();

    // create model IO vars (before this call the dss must be initialized)
    msg(Debug)<<"Create model input/output variables."<<endl;
    addModelInputOutputs(varSim, dss.get());

    // Till now (between fmiInstantiateModel and fmiInitialize) we have only used objects of type VariableStore's in var.
    // Now we copy all values from var to varSim (varSim is generated above).

    /***** first check for interface changes (and copy variable values) *****/

    if(!cosim) {
      int zDim = z.get().size();
      if(zDim!=dss->getzSize())
        throw runtime_error("The number of continuous states in modelDescription.xml and the current model differ: "+
                            fmatvec::toString(zDim)+", "+fmatvec::toString(dss->getzSize())+". "+
                            "Maybe the model topologie has changed due to a parameter change but this is not allowed.");
       
      int svDim = svLast.size();
      if(svDim!=dss->getsvSize())
        throw runtime_error("The number of event indicators in modelDescription.xml and the current model differ: "+
                            fmatvec::toString(svDim)+", "+fmatvec::toString(dss->getsvSize())+". "+
                            "Maybe the model topologie has changed due to a parameter change but this is not allowed.");
    }

    if(var.size()!=varSim.size())
      throw runtime_error("The number of parameters in modelDescription.xml and the current model differ: "
                          +fmatvec::toString(var.size())+", "+fmatvec::toString(varSim.size())+". "+
                          "Maybe the model topologie has changed due to a parameter change but this is not allowed.");

    auto varSimIt=varSim.begin();
    size_t vr=0;
    for(auto varIt=var.begin(); varIt!=var.end(); ++varIt, ++varSimIt, ++vr) {
      try {
        // check for a change of the model topologie
        if((*varSimIt)->getName()!=(*varIt)->getName())
          throw runtime_error("Variable names in modelDescription.xml and the current model does not match: "
                              +(*varIt)->getName()+", "+(*varSimIt)->getName()+". "+
                              "Maybe the model topologie has changed due to a parameter change but this is not allowed.");
        if((*varSimIt)->getType()!=(*varIt)->getType())
          throw runtime_error("Variable type (parameter, input, output) in modelDescription.xml and the current model does not match: "
                              +fmatvec::toString((*varIt)->getType())+", "
                              +fmatvec::toString((*varSimIt)->getType())+". "+
                              "Maybe the model topologie has changed due to a parameter change but this is not allowed.");
        if((*varSimIt)->getDatatypeChar()!=(*varIt)->getDatatypeChar())
          throw runtime_error(string("Variable datatype in modelDescription.xml and the current model does not match: ")
                              +(*varIt)->getDatatypeChar()+", "+(*varSimIt)->getDatatypeChar()+". "+
                              "Maybe the model topologie has changed due to a parameter change but this is not allowed.");
        // copy variable values
        if((*varIt)->getType()==Output) // outputs are not allowed to be set -> skip
          continue;
        msg(Debug)<<"Copy variable '"<<(*varSimIt)->getName()<<"' from VariableStore object to the \"real\" varaible object."<<endl;
        switch((*varIt)->getDatatypeChar()) {
          case 'r': (*varSimIt)->setValue((*varIt)->getValue(double())); break;
          case 'i': (*varSimIt)->setValue((*varIt)->getValue(int())); break;
          case 'b': (*varSimIt)->setValue((*varIt)->getValue(bool())); break;
          case 's': (*varSimIt)->setValue((*varIt)->getValue(string())); break;
        }
      }
      RETHROW_VR(vr)
    }

    /***** now use the new variables *****/

    // var is now no longer needed since we use varSim now.
    var=varSim;

    // initialize state
    msg(Debug)<<"Initialize initial conditions of the DynamicSystemSolver."<<endl;
    dss->evalz0();

    // initialize last stop vector with initial stop vector state
    dss->computeInitialCondition();

    // plot initial state
    dss->plot();
  }



  /* me special functions */

  void FMIInstance::setTime(fmiReal time_) {
    assert(!cosim);
    time.get()=time_;
    if(dss)
      dss->resetUpToDate();
  }

  void FMIInstance::setContinuousStates(const fmiReal x[], size_t nx) {
    assert(!cosim);
    copy(&x[0], &x[0]+nx, &z.get()(0));
    if(dss)
      dss->resetUpToDate();
  }

  // is called when the current state is a valid/accepted integrator step.
  // (e.g. not a intermediate Runge-Kutta step or a step which will be rejected due to
  // the error tolerances of the integrator)
  void FMIInstance::completedIntegratorStep(fmiBoolean* callEventUpdate) {
    assert(!cosim);
    // update internal system state (we are here at a valid step)
    dss->updateInternalState();

    // plot the current system state dependent on plotMode
    switch(predefinedParameterStruct.plotMode) {
      // plot at each n-th completed step
      case EverynthCompletedStep:
        completedStepCounter++;
        if(completedStepCounter==predefinedParameterStruct.plotEachNStep) {
          completedStepCounter=0;
          dss->plot();
        }
        break;
      // plot if this is the first completed step after nextPlotTime
      case NextCompletedStepAfterSampleTime:
        if(time>=nextPlotTime) {
          nextPlotTime += predefinedParameterStruct.plotStepSize * (floor((time-nextPlotTime)/predefinedParameterStruct.plotStepSize)+1);
          dss->plot();
        }
        break;
      // do not plot at completed steps -> plot at discrete sample times
      case SampleTime:
        break;
    }

    // check drift
    if(dss->positionDriftCompensationNeeded(predefinedParameterStruct.gMax)) {
      driftCompensation=positionLevel;
      *callEventUpdate=true;
    }
    else if(dss->velocityDriftCompensationNeeded(predefinedParameterStruct.gdMax)) {
      driftCompensation=velocityLevel;
      *callEventUpdate=true;
    }
    else
      *callEventUpdate=false;
  }

  void FMIInstance::initialize_me(fmiBoolean toleranceControlled, fmiReal relativeTolerance, fmiEventInfo* eventInfo) {
    assert(!cosim);

    // set eventInfo (except next event time)
    eventInfo->iterationConverged=true;
    eventInfo->stateValueReferencesChanged=false;
    eventInfo->stateValuesChanged=false;
    eventInfo->terminateSimulation=false;

    initialize();

    // initialize stop vector
    svLast=dss->evalsv();

    // handling of plot mode
    switch(predefinedParameterStruct.plotMode) {
      case EverynthCompletedStep:
        // init
        completedStepCounter=0;
        // no next time event
        eventInfo->upcomingTimeEvent=false;
        break;
      case NextCompletedStepAfterSampleTime:
        // init
        nextPlotTime=time+predefinedParameterStruct.plotStepSize;
        // next time event
        eventInfo->upcomingTimeEvent=false;
        break;
      case SampleTime:
        // init
        nextPlotTime=time+predefinedParameterStruct.plotStepSize;
        // next time event
        eventInfo->upcomingTimeEvent=true;
        eventInfo->nextEventTime=nextPlotTime;
        break;
    }

    z=std::ref(dss->getState());
  }

  void FMIInstance::getDerivatives(fmiReal derivatives[], size_t nx) {
    assert(!cosim);
    const fmatvec::Vec &zd=dss->evalzd();
    copy(&zd(0), &zd(0)+nx, &derivatives[0]);
  }

  void FMIInstance::getEventIndicators(fmiReal eventIndicators[], size_t ni) {
    assert(!cosim);
    const fmatvec::Vec &sv=dss->evalsv();
    copy(&sv(0), &sv(0)+ni, &eventIndicators[0]);
  }

  // shift point
  void FMIInstance::eventUpdate(fmiBoolean intermediateResults, fmiEventInfo* eventInfo) {
    assert(!cosim);
    // initialize eventInfo fields (except next time event)
    eventInfo->iterationConverged=true;
    eventInfo->stateValueReferencesChanged=false;
    eventInfo->stateValuesChanged=false;
    eventInfo->terminateSimulation=false;
    eventInfo->upcomingTimeEvent=false;

    // ***** state event = root = stop vector event *****

    // Note: The FMI interface does not provide a jsv integer vector as e.g. the LSODAR integrator does.
    // Since MBSim requires this information we have to generate it here. For this we:
    // * store the stop vector after the last event (shift point) in the variable svLast
    // * compare the current sv with svLast using the FMI state event condition (see below)
    // * set all entries in jsv to 1 if the this condition matches

    // get current stop vector
    const fmatvec::Vec &sv=dss->evalsv();
    // compare last (svLast) and current (sv) stop vector, based on the FMI standard
    // shift equation: (sv(i)>0) != (svLast(i)>0): build jsv and set shiftRequired
    bool shiftRequired=false;
    fmatvec::VecInt &jsv=dss->getjsv();
    for(int i=0; i<sv.size(); ++i)
      if((svLast(i)>0) != (sv(i)>0)) { // use 0 to check for ==0
        jsv(i)=1;
        shiftRequired=true; // set shiftRequired: a MBSim shift call is required (at least 1 entry in jsv is 1)
      }
      else
        jsv(i)=0;
    if(shiftRequired) {
      // shift MBSim system
      dss->resetUpToDate();
      dss->shift();
      // A MBSim shift always changes state values (at least by a minimal projection)
      // This must be reported to the environment (the integrator must be resetted in this case).
      eventInfo->stateValuesChanged=true;

      // get current stop vector with is now also the last stop vector
      svLast=dss->evalsv();
    }
    else
      dss->updateStopVectorParameters();

    // ***** time event (currently only for plotting) *****

    switch(predefinedParameterStruct.plotMode) {
      case EverynthCompletedStep:
      case NextCompletedStepAfterSampleTime:
        // no next time event
        break;
      case SampleTime:
        // next time event
        eventInfo->upcomingTimeEvent=true;

        // next event wenn plotting with sample time and we currently match that time
        if(fabs(time-nextPlotTime)<1e4*numeric_limits<double>::epsilon()*time+1e4*numeric_limits<double>::epsilon()) {
          // plot
          dss->plot();
          // next time event
          nextPlotTime=time+predefinedParameterStruct.plotStepSize;
          eventInfo->nextEventTime=nextPlotTime;
        }
        else
          eventInfo->nextEventTime=nextPlotTime;
        break;
    }

    // ***** step event (drift compensation) *****
    if(driftCompensation==positionLevel) {
      // compensate drift on position and velocity level
      dss->projectGeneralizedPositions(3);
      dss->projectGeneralizedVelocities(3);
      // make changed state values and reset driftCompensation flag
      eventInfo->stateValuesChanged=true;
      driftCompensation=none;
    }
    else if(driftCompensation==velocityLevel) {
      // compensate drift on velocity level
      dss->projectGeneralizedVelocities(3);
      // make changed state values and reset driftCompensation flag
      eventInfo->stateValuesChanged=true;
      driftCompensation=none;
    }
  }

  void FMIInstance::getContinuousStates(fmiReal states[], size_t nx) {
    assert(!cosim);
    copy(&z.get()(0), &z.get()(0)+nx, &states[0]);
  }

  void FMIInstance::getNominalContinuousStates(fmiReal x_nominal[], size_t nx) {
    assert(!cosim);
    // we do not proved a nominal value for states in MBSim -> just return 1 as nominal value.
    fill(&x_nominal[0], &x_nominal[0]+nx, 1);
  }

  void FMIInstance::getStateValueReferences(fmiValueReference vrx[], size_t nx) {
    assert(!cosim);
    // we do not assign any MBSim state to a FMI valueReference -> return fmiUndefinedValueReference for all states
    fill(&vrx[0], &vrx[0]+nx, fmiUndefinedValueReference);
  }





  /* cosim special functions */

  void FMIInstance::initialize_cosim(fmiReal tStart, fmiBoolean StopTimeDefined, fmiReal tStop) {
    assert(cosim);

    time=tStart;
    initialize();

    integrator->setStartTime(time);
    integrator->setEndTime(StopTimeDefined ? tStop : 1.0e30);
    integrator->setSystem(dss.get());
    integrator->preIntegrate();
  }

  void FMIInstance::resetSlave() {
    assert(cosim);
    // finish cosim integration
    integrator->setSystem(dss.get());
    integrator->postIntegrate();
    integrator.reset();

    // delete DynamicSystemSolver (requried here since after terminate a call to initialize is allowed without
    // calls to fmiFreeModelInstance and fmiInstantiateModel)
    time=std::ref(timeStore);
    z=std::ref(zStore); // only used for ME
    dss.reset();
  }

  void FMIInstance::setRealInputDerivatives(const fmiValueReference vr[], size_t nvr, const fmiInteger order[], const fmiReal value[]) {
    assert(cosim);
    throw std::runtime_error("This call is not allowed accoring the cabability flags of this FMU.");
  }

  void FMIInstance::getRealOutputDerivatives(const fmiValueReference vr[], size_t nvr, const fmiInteger order[], fmiReal value[]) {
    assert(cosim);
    throw std::runtime_error("This call is not allowed according the capability flags of this FMU.");
  }

  void FMIInstance::cancelStep() {
    assert(cosim);
    throw std::runtime_error("This call is not allowed according the capability flags of this FMU.");
  }

  void FMIInstance::doStep(fmiReal currentCommunicationPoint, fmiReal communicationStepSize, fmiBoolean newStep) {
    assert(cosim);
    assert(newStep);

    // some checks
    if(abs(time-currentCommunicationPoint)>1e-13)
      throw std::runtime_error("Internal time "+fmatvec::toString(time)+" and the time got by fmiDoStep "+
                               fmatvec::toString(currentCommunicationPoint)+" differs.");
    if(communicationStepSize<1e-13) {
      msg(Debug)<<"The cosim master is performing a event iteration. This FMU does nothing for this step."<<endl;
      return;
    }

    // carry out a macro step in MBSim, this will also increate time
    integrator->subIntegrate(currentCommunicationPoint+communicationStepSize);
  }

  void FMIInstance::getStatus(const fmiStatusKind s, fmiStatus* value) {
    assert(cosim);
    throw std::runtime_error("This call is not allowed according the capability flags of this FMU.");
  }

  void FMIInstance::getDoubleStatus(const fmiStatusKind s, fmiReal* value) {
    assert(cosim);
    throw std::runtime_error("This call is not allowed according the capability flags of this FMU.");
  }

  void FMIInstance::getIntStatus(const fmiStatusKind s, fmiInteger* value) {
    assert(cosim);
    throw std::runtime_error("This call is not allowed according the capability flags of this FMU.");
  }

  void FMIInstance::getBoolStatus(const fmiStatusKind s, fmiBoolean* value) {
    assert(cosim);
    throw std::runtime_error("This call is not allowed according the capability flags of this FMU.");
  }

  void FMIInstance::getStringStatus(const fmiStatusKind s, fmiString* value) {
    assert(cosim);
    throw std::runtime_error("This call is not allowed according the capability flags of this FMU.");
  }

}
