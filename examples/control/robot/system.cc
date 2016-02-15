#include "system.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/kinetic_excitation.h"
#include "mbsim/environment.h"
#include "mbsim/functions/kinematic_functions.h"
#include "mbsim/functions/tabular_functions.h"
#include "mbsim/functions/symbolic_functions.h"

#include "mbsimControl/linear_transfer_system.h"
#include "mbsimControl/object_sensors.h"
#include "mbsimControl/function_sensor.h"
#include "mbsimControl/signal_manipulation.h"
#include "mbsimControl/signal_function.h"

#include "tools/file_to_fmatvecstring.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include "openmbvcppinterface/ivbody.h"
#endif

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBSimControl;
using namespace casadi;

Robot::Robot(const string &projectName) : DynamicSystemSolver(projectName) {
  // Gravitation
  Vec grav(3);
  grav(1)=-1;
  grav(2)=-1;
  grav=grav/nrm2(grav)*9.81;
  MBSimEnvironment::getInstance()->setAccelerationOfGravity(grav);

  // Data
  double mB = 20;
  double mA = 10;
  double mS = 5;
  double hB= 1;
  double lA= 1;
  double rB= 0.2;
  double rA= 0.1;
  double rS= 0.05;

  // --------------------------- Setup MBS ----------------------------
  // System with tree-structure
  RigidBody *basis = new RigidBody("Basis");
  this->addObject(basis);
  basis->setMass(mB);
  SymMat Theta(3);
  Theta(0,0) = mB*rB*rB;
  Theta(1,1) = 1./2.*mB*rB*rB;
  Theta(2,2) = mB*rB*rB;
  basis->setInertiaTensor(Theta);

  SqrMat A(3);
  for(int i=0; i<3; i++)
    A(i,i) = 1;

  Vec KrKS(3);
  KrKS(1) = hB/2;
  basis->setRotation(new RotationAboutYAxis<VecV>);
  Vec KrSP(3);
  KrSP(1) = hB/2;
  basis->addFrame(new FixedRelativeFrame("R",-KrKS,A));
  basis->setFrameOfReference(getFrame("I"));
  basis->setFrameForKinematics(basis->getFrame("R"));

  RigidBody *arm = new RigidBody("Arm");
  this->addObject(arm);
  Vec PrPK0(3);
  PrPK0(1) = hB;
  basis->addFrame(new FixedRelativeFrame("P",PrPK0,A,basis->getFrame("R")));
  KrKS.init(0);
  KrKS(1) = lA/2;
  arm->addFrame(new FixedRelativeFrame("R",-KrKS,A));
  arm->setFrameOfReference(basis->getFrame("P"));
  arm->setFrameForKinematics(arm->getFrame("R"));

  arm->setMass(mA);
  Theta(0,0) = mA*rA*rA;
  Theta(1,1) = 1./2.*mA*rA*rA;
  Theta(2,2) = mA*rA*rA;
  arm->setInertiaTensor(Theta);
  arm->setRotation(new RotationAboutZAxis<VecV>);
  KrSP(1) = -lA/2;
  KrSP(1) = lA/2;

  RigidBody *spitze = new RigidBody("Spitze");
  this->addObject(spitze);
  spitze->setMass(mS);
  Theta(0,0) = mS*rS*rS;
  Theta(1,1) = 1./2.*mS*rS*rS;
  Theta(2,2) = mS*rS*rS;
  PrPK0(1) = lA;
  arm->addFrame(new FixedRelativeFrame("Q",PrPK0,SqrMat(3,EYE),arm->getFrame("R")));
  spitze->setInertiaTensor(Theta);
  spitze->setTranslation(new TranslationAlongYAxis<VecV>);
  spitze->setFrameOfReference(arm->getFrame("Q"));
  spitze->setFrameForKinematics(spitze->getFrame("C"));

  // --------------------------- Setup Control ----------------------------
//  RelativeAngularPositionSensor * basePosition = new RelativeAngularPositionSensor("BasePositionIst");
//  addLink(basePosition);
//  basePosition->setReferenceFrame(getFrame("I"));
//  basePosition->setRelativeFrame(basis->getFrame("C"));
//  basePosition->setDirection("[0;1;0]");
  GeneralizedPositionSensor * basePosition = new GeneralizedPositionSensor("BasePositionIst");
  addLink(basePosition);
  basePosition->setObject(basis);
  basePosition->setIndex(0);

  Mat bPT(FileTofmatvecString("./Soll_Basis.tab").c_str());
  TabularFunction<VecV(double)> * basePositionSollFunction = new TabularFunction<VecV(double)>(bPT.col(0), bPT.col(1));
  FunctionSensor * basePositionSoll = new FunctionSensor("BasePositionSoll");
  addLink(basePositionSoll);
  basePositionSoll->setFunction(basePositionSollFunction);

  SX s1 = SX::sym("s1",1);
  SX s2 = SX::sym("s2",1);
  SX y = s1-s2;
  vector<SX> input(2);
  input[0] = s1;
  input[1] = s2;
  SXFunction f(input,y);

  BinarySignalOperation * basePositionDiff = new BinarySignalOperation("BasePositionDiff");
  addLink(basePositionDiff);
  basePositionDiff->setFirstInputSignal(basePositionSoll);
  basePositionDiff->setSecondInputSignal(basePosition);
  basePositionDiff->setFunction(new SymbolicFunction<VecV(VecV,VecV)>(f));

  LinearTransferSystem * basisControl = new LinearTransferSystem("ReglerBasis");
  addLink(basisControl);
  basisControl->setInputSignal(basePositionDiff);
  basisControl->setPID(4000., 0., 200.);

//  SignalProcessingSystemSensor * basisControlOut = new SignalProcessingSystemSensor("BaseControlOut");
//  addLink(basisControlOut);
//  basisControlOut->setSignalProcessingSystem(basisControl);

  SignalTimeDiscretization * bDis = new SignalTimeDiscretization("BaseDis");
  addLink(bDis);
  bDis->setInputSignal(basisControl);

  KineticExcitation *motorBasis = new KineticExcitation("MotorBasis");
  addLink(motorBasis);
  motorBasis->setMomentDirection("[0;1;0]");
  motorBasis->setMomentFunction(new SignalFunction<VecV(double)>(bDis));
  motorBasis->connect(getFrame("I"),basis->getFrame("R"));

//  RelativeAngularPositionSensor * armPosition = new RelativeAngularPositionSensor("ArmPositionIst");
//  addLink(armPosition);
//  armPosition->setReferenceFrame(basis->getFrame("C"));
//  armPosition->setRelativeFrame(arm->getFrame("C"));
//  armPosition->setDirection("[0;0;1]");
  GeneralizedPositionSensor * armPosition = new GeneralizedPositionSensor("ArmPositionIst");
  addLink(armPosition);
  armPosition->setObject(arm);
  armPosition->setIndex(0);

  Mat aPT(FileTofmatvecString("./Soll_Arm.tab").c_str());
  TabularFunction<VecV(double)> * armPositionSollFunction = new TabularFunction<VecV(double)>(aPT.col(0), aPT.col(1));
  FunctionSensor * armPositionSoll = new FunctionSensor("ArmPositionSoll");
  addLink(armPositionSoll);
  armPositionSoll->setFunction(armPositionSollFunction);

  BinarySignalOperation * armPositionDiff = new BinarySignalOperation("ArmPositionDiff");
  addLink(armPositionDiff);
  armPositionDiff->setFirstInputSignal(armPositionSoll);
  armPositionDiff->setSecondInputSignal(armPosition);
  armPositionDiff->setFunction(new SymbolicFunction<VecV(VecV,VecV)>(f));

  LinearTransferSystem * armControl = new LinearTransferSystem("ReglerArm");
  addLink(armControl);
  armControl->setInputSignal(armPositionDiff);
  armControl->setPID(4000., 0., 200.);

//  SignalProcessingSystemSensor * armControlOut = new SignalProcessingSystemSensor("ArmControlOut");
//  addLink(armControlOut);
//  armControlOut->setSignalProcessingSystem(armControl);

  SignalTimeDiscretization * aDis = new SignalTimeDiscretization("ArmDis");
  addLink(aDis);
  aDis->setInputSignal(armControl);

  KineticExcitation *motorArm = new KineticExcitation("MotorArm");
  addLink(motorArm);
  motorArm->setMomentDirection("[0;0;1]");
  motorArm->setMomentFunction(new SignalFunction<VecV(double)>(aDis));
  motorArm->connect(basis->getFrame("P"),arm->getFrame("R"));

//  RelativePositionSensor * spitzePosition = new RelativePositionSensor("SpitzePositionIst");
//  addLink(spitzePosition);
//  spitzePosition->setReferenceFrame(arm->getFrame("Q"));
//  spitzePosition->setRelativeFrame(spitze->getFrame("C"));
//  spitzePosition->setDirection("[0;1;0]");
  GeneralizedPositionSensor * spitzePosition = new GeneralizedPositionSensor("SpitzePositionIst");
  addLink(spitzePosition);
  spitzePosition->setObject(spitze);
  spitzePosition->setIndex(0);

  Mat sPT(FileTofmatvecString("./Soll_Spitze.tab").c_str());
  TabularFunction<VecV(double)> * spitzePositionSollFunction = new TabularFunction<VecV(double)>(sPT.col(0), sPT.col(1));
  FunctionSensor * spitzePositionSoll = new FunctionSensor("SpitzePositionSoll");
  addLink(spitzePositionSoll);
  spitzePositionSoll->setFunction(spitzePositionSollFunction);

  BinarySignalOperation * spitzePositionDiff = new BinarySignalOperation("SpitzePositionDiff");
  addLink(spitzePositionDiff);
  spitzePositionDiff->setFirstInputSignal(spitzePositionSoll);
  spitzePositionDiff->setSecondInputSignal(spitzePosition);
  spitzePositionDiff->setFunction(new SymbolicFunction<VecV(VecV,VecV)>(f));

  LinearTransferSystem * spitzeControl = new LinearTransferSystem("ReglerSpitze");
  addLink(spitzeControl);
  spitzeControl->setInputSignal(spitzePositionDiff);
  spitzeControl->setPID(4000., 0., 200.);

//  SignalProcessingSystemSensor * spitzeControlOut = new SignalProcessingSystemSensor("SpitzeControlOut");
//  addLink(spitzeControlOut);
//  spitzeControlOut->setSignalProcessingSystem(spitzeControl);

  SignalTimeDiscretization * sDis = new SignalTimeDiscretization("SpitzeDis");
  addLink(sDis);
  sDis->setInputSignal(spitzeControl);

  KineticExcitation *motorSpitze = new KineticExcitation("MotorSpitze");
  addLink(motorSpitze);
  motorSpitze->setForceDirection("[0;1;0]");
  motorSpitze->setForceFunction(new SignalFunction<VecV(double)>(sDis));
  motorSpitze->connect(arm->getFrame("Q"),spitze->getFrame("C"));

#ifdef HAVE_OPENMBVCPPINTERFACE
  boost::shared_ptr<OpenMBV::IvBody> obj=OpenMBV::ObjectFactory::create<OpenMBV::IvBody>();
  obj->setIvFileName("wrl/basis.wrl");
  obj->setScaleFactor(0.2);
  obj->setInitialRotation(M_PI,0,0);
  obj->setInitialTranslation(0,0.25,0);
  basis->setOpenMBVRigidBody(obj);

  obj=OpenMBV::ObjectFactory::create<OpenMBV::IvBody>();
  obj->setIvFileName("wrl/arm.wrl");
  obj->setScaleFactor(0.2);
  obj->setInitialRotation(M_PI,0,0);
  obj->setInitialTranslation(0,0.08,0);
  arm->setOpenMBVRigidBody(obj);

  obj=OpenMBV::ObjectFactory::create<OpenMBV::IvBody>();
  obj->setIvFileName("wrl/spitze.wrl");
  obj->setScaleFactor(0.2);
  obj->setInitialTranslation(0,-0.3,0);
  obj->setInitialRotation(M_PI,0,0);
  spitze->setOpenMBVRigidBody(obj);
#endif
}
