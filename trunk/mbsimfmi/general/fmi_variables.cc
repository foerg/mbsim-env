#include "fmi_variables.h"
#include <mbsim/dynamic_system_solver.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/ref.hpp>

namespace MBSimFMI {

// create predefined parameters
void addPredefinedParameters(std::vector<boost::shared_ptr<Variable> > &var,
                             PredefinedParameterStruct &predefinedParameterStruct) {
  // output directory
  var.push_back(boost::make_shared<PredefinedParameter<std::string> >("Output directory",
    "MBSim output directory for all files: *.mbsim.h5, *.ombv.h5, *.ombv.xml, ...", boost::ref(predefinedParameterStruct.outputDir)));
  // default value: current dir
  (*--var.end())->setValue(std::string("."));

  // plot mode
  // generate enumeration list
  Variable::EnumList plotModeList=boost::make_shared<Variable::EnumListCont>();
  // PlotMode::EverynthCompletedStep
  plotModeList->push_back(std::make_pair<std::string, std::string>("Every n-th completed step",
    "Plot each n-th completed integrator step, with n = 'Plot.each n-th step'."));
  // PlotMode::NextCompletedStepAfterSampleTime
  plotModeList->push_back(std::make_pair<std::string, std::string>("Next completed step after sample time",
    "Plot at the first completed step after sample time, with sample time = 'Plot.sample time'."));
  // PlotMode::SampleTime
  plotModeList->push_back(std::make_pair<std::string, std::string>("Constant sample time",
    "Plot in equidistant sample times, with sample time = 'Plot.sample time'."));
  // add variable
  var.push_back(boost::make_shared<PredefinedParameter<int> >("Plot.mode",
    "Write to *.mbsim.h5 and *.ombv.h5 files at every ...", boost::ref(predefinedParameterStruct.plotMode), plotModeList));
  // default value
  (*--var.end())->setValue(int(NextCompletedStepAfterSampleTime));

  // plot at each n-th integrator step
  var.push_back(boost::make_shared<PredefinedParameter<int> >("Plot.each n-th step",
    "... n-th completed integrator step", boost::ref(predefinedParameterStruct.plotEachNStep)));
  // default value: every 5-th step
  (*--var.end())->setValue(int(5));

  // plot every dt
  var.push_back(boost::make_shared<PredefinedParameter<double> >("Plot.sample time",
    "... sample point with this sample time", boost::ref(predefinedParameterStruct.plotStepSize)));
  // default value: every 1ms
  (*--var.end())->setValue(double(0.001));

  // ADD HERE MORE PREDEFINED PARAMETERS
}

void addModelInputOutputs(std::vector<boost::shared_ptr<Variable> > &var,
                          const MBSim::DynamicSystemSolver *dss) {
  // create all input/output variables for links in the dss
  std::vector<MBSim::Link*> l;
  getAllLinks(dss, l);
  for(std::vector<MBSim::Link*>::const_iterator it=l.begin(); it!=l.end(); ++it) {
    // for ExternGeneralizedIO create three variables: force input, position output and velocity output
    MBSim::ExternGeneralizedIO *genIO=dynamic_cast<MBSim::ExternGeneralizedIO*>(*it);
    if(genIO) {
      var.push_back(boost::make_shared<ExternGeneralizedIOForceInput>(genIO));
      (*--var.end())->setValue(double(0.0)); // default value
      var.push_back(boost::make_shared<ExternGeneralizedIOPositionOutput>(genIO));
      var.push_back(boost::make_shared<ExternGeneralizedIOVelocityOutput>(genIO));
    }
    // for ExternSignalSource create one input variable
    MBSimControl::ExternSignalSource *sigSource=dynamic_cast<MBSimControl::ExternSignalSource*>(*it);
    if(sigSource) {
      var.push_back(boost::make_shared<ExternSignalSourceInput>(sigSource));
      (*--var.end())->setValue(double(0.0)); // default value
    }
    // for ExternSignalSink create one output variable
    MBSimControl::ExternSignalSink *sigSink=dynamic_cast<MBSimControl::ExternSignalSink*>(*it);
    if(sigSink) {
      var.push_back(boost::make_shared<ExternSignalSinkOutput>(sigSink));
    }
    // ADD HERE MORE MBSIM TYPES WHICH SHOULD BECOME FMI INPUTS/OUTPUTS
  }
}

}

namespace {

  std::string mbsimPathToFMIName(std::string path) {
    boost::replace_all(path, "/", "."); // replace the MBSim separator / by the FMI seperator .
    return path.substr(1); // skip the starting spearotor character
  }

  void getAllLinks(const MBSim::DynamicSystem *sys, std::vector<MBSim::Link*> &link) {
    // add all link of dss
    const std::vector<MBSim::Link*> &l=sys->getLinks();
    link.insert(link.end(), l.begin(), l.end());
  
    // call recursively for all DynamicSystem
    const std::vector<MBSim::DynamicSystem*> &s=sys->getDynamicSystems();
    for(std::vector<MBSim::DynamicSystem*>::const_iterator it=s.begin(); it!=s.end(); ++it)
      getAllLinks(*it, link);
  }

}
