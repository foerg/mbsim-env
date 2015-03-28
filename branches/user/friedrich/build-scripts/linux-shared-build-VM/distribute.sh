#! /bin/bash

set -e 
set -o pipefail

# Usage:
# Call this script from everywhere!
# use argument "noclean" to disable cleaning the dist dir before starting
# use argument "noarchive" to disalbe the creation of a tar.bz2 archive at end



##############################################################################
# MBSIM
##############################################################################

DISTBASEDIR=/home/user/MBSimLinux/dist_mbsim

PREFIX=/home/user/MBSimLinux/local

BINFILES="
$PREFIX/bin/h5dumpserie
$PREFIX/bin/h5lsserie
$PREFIX/bin/h5plotserie
$PREFIX/bin/mbsimflatxml
$PREFIX/bin/mbsimxml
$PREFIX/bin/mbsimgui
$PREFIX/bin/mbxmlutilspp
$PREFIX/bin/openmbv
$PREFIX/bin/casadi_interface.oct
$PREFIX/bin/mbsimCreateFMU
$PREFIX/bin/fmuCheck.*
$PREFIX/lib/mbsimsrc_fmi.so
$PREFIX/lib/mbsimppxml_fmi.so
$PREFIX/lib/mbsimxml_fmi.so
/usr/bin/h5copy
/usr/bin/h5diff
/usr/bin/h5dump
/usr/bin/h5import
/usr/bin/h5ls
/usr/bin/h5mkgrp
/usr/bin/h5repack
/usr/bin/h5repart
/usr/bin/h5stat
/usr/bin/octave
/usr/lib/libgtk-x11-2.0.so.0
/usr/lib/libgnomeui-2.so.0
/usr/lib/libgnomevfs-2.so.0
/usr/lib/libgconf-2.so.4
"
# Note: libgtk-x11, libgnomeui, libgnomevfs and libgconf are loaded at runtime by QGtkStyle, hence we must
# include these to the release including all dependencies

SHAREDIRS="
doc
hdf5serie
mbxmlutils
openmbv
mbsimgui
mbsimxml
"

OCTAVEMDIR="/usr/share/octave/$(octave-config --version)/m"
OCTAVEOCTDIR="/usr/lib/octave/$(octave-config --version)/oct"





# check args
NOCLEAN=0
NOARCHIVE=0
while [ $# -gt 0 ]; do
  case $1 in
    noclean)   NOCLEAN=1 ;;
    noarchive) NOARCHIVE=1 ;;
  esac
  shift
done

# dist dir
DISTDIR=$DISTBASEDIR/mbsim

# PKG config
export PKG_CONFIG_PATH=/home/user/MBSimLinux/local/lib/pkgconfig
# get includes and libs of all packages required for compiling mbsim source examples
SRCINC=$(pkg-config --cflags mbsim mbsimControl mbsimElectronics mbsimFlexibleBody mbsimHydraulics mbsimInterface mbsimPowertrain)
SRCLIB=$(pkg-config --libs   mbsim mbsimControl mbsimElectronics mbsimFlexibleBody mbsimHydraulics mbsimInterface mbsimPowertrain)

# clear previout dist dir
if [ $NOCLEAN -eq 0 ]; then
  rm -rf $DISTDIR
fi

# copy libs
mkdir -p $DISTDIR/lib
cp -ru $PREFIX/lib/* $DISTDIR/lib

# copy bin and get dependent libs
TMPSOFILE=$DISTBASEDIR/distribute.sh.sofile
rm -f $TMPSOFILE
mkdir -p $DISTDIR/bin
for F in $BINFILES; do
  cp -uL $F $DISTDIR/bin
  ldd $F | sed -rne "/=>/s/^.*=> ([^(]+) .*$/\1/p" >> $TMPSOFILE
done
# get dependent libs for Qt plugins
ldd /usr/lib/qt4/plugins/imageformats/libqsvg.so | sed -rne "/=>/s/^.*=> ([^(]+) .*$/\1/p" >> $TMPSOFILE
ldd /usr/lib/qt4/plugins/iconengines/libqsvgicon.so | sed -rne "/=>/s/^.*=> ([^(]+) .*$/\1/p" >> $TMPSOFILE
# copy dependent libs
sort $TMPSOFILE | uniq > $TMPSOFILE.uniq
for F in $(cat $TMPSOFILE); do
  test -e $DISTDIR/lib/$(basename $F) || cp -uL $F $DISTDIR/lib
done

# check bin in dist for correct rpath
echo "The following executables have not the required rpath set. Creating a wrapper script:"
mkdir -p $DISTDIR/bin/.wrapper
cat << EOF > $DISTDIR/bin/.wrapper/ld_library_path_wrapper.sh
#! /bin/sh
DIRNAME=\$(dirname \$0)
BASENAME=\$(basename \$0)
export LD_LIBRARY_PATH=\$DIRNAME/../lib:\$LD_LIBRARY_PATH
\$DIRNAME/.wrapper/\$BASENAME "\$@"
EOF
chmod +x $DISTDIR/bin/.wrapper/ld_library_path_wrapper.sh
for F in $DISTDIR/bin/*; do
  test "$(basename $F)" == "casadi_interface.oct" && continue
  ldd $F &> /dev/null || continue
  if ! readelf -d $F | grep "Library rpath: \[.*\$ORIGIN/../lib.*" &> /dev/null; then
    echo "$F"
    mv $F $DISTDIR/bin/.wrapper/$(basename $F)
    (cd $DISTDIR/bin; ln -s .wrapper/ld_library_path_wrapper.sh $F)
  fi
done
echo "Done"
# octave needs a special handling
if grep -I LD_LIBRARY_PATH $DISTDIR/bin/octave &> /dev/null; then # is a script
  rm $DISTDIR/bin/octave
  cat << EOF > $DISTDIR/bin/.wrapper/octave_ld_library_path_wrapper.sh
#! /bin/sh
DIRNAME=\$(dirname \$0)
BASENAME=\$(basename \$0)
case $0 in
  /*) THISDIR=\$DIRNAME ;;
  *)  THISDIR=\$PWD/\$DIRNAME ;;
esac
export LD_LIBRARY_PATH=\$DIRNAME/../lib:\$LD_LIBRARY_PATH
export OCTAVE_HOME=\$THISDIR/..
\$DIRNAME/.wrapper/\$BASENAME "\$@"
EOF
  chmod +x $DISTDIR/bin/.wrapper/octave_ld_library_path_wrapper.sh
  (cd $DISTDIR/bin; ln -s .wrapper/octave_ld_library_path_wrapper.sh $DISTDIR/bin/octave)
else # is not a script
  echo "$DISTDIR/bin/octave is not a script. This is not implemented!"
fi

# copy includes
TMPINCFILE=$DISTBASEDIR/distribute.inc.cc
rm -f $TMPINCFILE
for F in $(find $PREFIX/include -type f | grep "/fmatvec/\|/hdf5serie/\|/mbsim/\|/mbsimControl/\|/mbsimElectronics/\|/mbsimFlexibleBody/\|/mbsimHydraulics/\|/mbsimPowertrain/\|/mbsimInterface/\|/mbsimtinyxml/\|/mbsimxml/\|/openmbvcppinterface/\|/mbsimfmi/"); do
  echo "#include <$F>" >> $TMPINCFILE
done
TMPDEPFILE=$DISTBASEDIR/distribute.dep
rm -f $TMPDEPFILE
g++ -M -MT 'DUMMY' $TMPINCFILE $SRCINC | sed -re "s+^ *DUMMY *: *$TMPINCFILE *++;s+\\\++" > $TMPDEPFILE
for FSRC in $(cat $TMPDEPFILE); do
  FDST=$(echo $FSRC | sed -re "s+^.*/include/++")
  
  if echo $FDST | grep "^/" > /dev/null; then
    echo "WARNING: not copying $FSRC (no '/include/' in path)"
  else
    mkdir -p $(dirname $DISTDIR/include/$FDST)
    cp -uL $FSRC $DISTDIR/include/$FDST
  fi
done







# copy shares
mkdir -p $DISTDIR/share
for D in $SHAREDIRS; do
  cp -ruL $PREFIX/share/$D $DISTDIR/share
done

# copy octave m
mkdir -p $DISTDIR/share/octave/$(octave-config --version)/m
cp -ruL $OCTAVEMDIR/* $DISTDIR/share/octave/$(octave-config --version)/m

# copy octave oct
mkdir -p $DISTDIR/lib/octave/$(octave-config --version)/oct
cp -ruL $OCTAVEOCTDIR/* $DISTDIR/lib/octave/$(octave-config --version)/oct

# Remove all libs being hardware or OS dependent (here all libs from the packages glibc-* and mesa-*)
for F in $(rpm -qa | grep -e "^glibc-" -e "^mesa-" | xargs rpm -ql | grep -e "\.so$" -e "\.so\." | sed -re "s+^.*/++"); do
  rm -f $DISTDIR/lib/$F &> /dev/null
done
# SPECIAL ACTIONS
cp -uL /usr/lib/libQtXml.so.4 $DISTDIR/lib/. # required by QSvg-plugin
rm -f $DISTDIR/include/sys/select.h
rm -f $DISTDIR/include/features.h
(cd $DISTDIR/lib; ln -s liblapack.so.3 liblapack.so)
(cd $DISTDIR/lib; ln -s libblas.so.3 libblas.so)
(cd $DISTDIR/lib; ln -s libgfortran.so.3 libgfortran.so)
(cd $DISTDIR/lib; ln -s libquadmath.so.0 libquadmath.so)
(cd $DISTDIR/lib; ln -s libhdf5_cpp.so.7 libhdf5_cpp.so)
(cd $DISTDIR/lib; ln -s libhdf5.so.7 libhdf5.so)
(cd $DISTDIR/lib; ln -s libz.so.1 libz.so)
(cd $DISTDIR/lib; ln -s libf77blas.so.3 libf77blas.so)
(cd $DISTDIR/lib; ln -s libcblas.so.3 libcblas.so)
(cd $DISTDIR/lib; ln -s libatlas.so.3 libatlas.so)
(cd $DISTDIR/lib; ln -s libstdc++.so.6 libstdc++.so)
(cd $DISTDIR/lib; ln -s libboost_system.so.1.48.0 libboost_system.so)
(cd $DISTDIR/lib; ln -s libboost_filesystem.so.1.48.0 libboost_filesystem.so)
(cd $DISTDIR/lib; ln -s libxerces-c-3.1.so libxerces-c.so)
# copy openmbvcppinterface SWIG files
cp -uL $PREFIX/bin/OpenMBV.oct $DISTDIR/bin
cp -uL $PREFIX/bin/OpenMBV.py $DISTDIR/bin
cp -uL $PREFIX/bin/_OpenMBV.so $DISTDIR/bin
cp -uL $PREFIX/bin/openmbv.jar $DISTDIR/bin
cp -uL $PREFIX/bin/libopenmbvjava.jni $DISTDIR/bin
cp -uL $PREFIX/bin/libopenmbvjavaloadJNI.jni $DISTDIR/bin
# copy openmbvcppinterface SWIG example files
mkdir -p $DISTDIR/examples/openmbvcppinterface_swig
cp -uL $PREFIX/share/openmbvcppinterface/examples/swig/* $DISTDIR/examples/openmbvcppinterface_swig
# copy casadi SWIG files for octave
cp -uL $PREFIX/bin/casadi.m $DISTDIR/bin
cp -ruL $PREFIX/bin/@swig_ref $DISTDIR/bin
# modifie all ELF rpath in lib/*.so*
for F in $DISTDIR/lib/*.so $DISTDIR/lib/*.so.*; do
  chrpath -r '$ORIGIN/../lib' $F &> /dev/null || chrpath -d $F &> /dev/null || DUMMYVAR=0
done

# create mbsim-config
cat << EOF > $DISTDIR/bin/mbsim-config
#! /bin/sh

if [ \$# -ne 1 ]; then
  echo "Usage: \$0 [--cflags|--libs]"
  echo "  --cflags:  outputs the compile flags"
  echo "  --libs:    outputs the link flags"
  exit
fi

INSTDIR="\$(readlink -f \$(dirname \$0)/..)"

CFLAGS="$(echo $SRCINC | sed -re "s|$PREFIX|\$INSTDIR|g")"
LIBS="$(echo $SRCLIB | sed -re "s|$PREFIX|\$INSTDIR|g")"

if [ "_\$1" = "_--cflags" ]; then
  echo "\$CFLAGS"
elif [ "_\$1" = "_--libs" ]; then
  echo "\$LIBS"
fi
EOF
chmod +x $DISTDIR/bin/mbsim-config

# Qt plugins
mkdir -p $DISTDIR/bin/imageformats
mkdir -p $DISTDIR/bin/iconengines
cp /usr/lib/qt4/plugins/imageformats/libqsvg.so $DISTDIR/bin/imageformats
cp /usr/lib/qt4/plugins/iconengines/libqsvgicon.so $DISTDIR/bin/iconengines

# README.txt
cat << EOF > $DISTDIR/README.txt
Using of the MBSim and Co. Package:
===================================

NOTE
This binary Linux build requires a Linux distribution with glibc >= 2.15.

- Unpack the archive to an arbitary directory (already done)
  (Note: It is recommended, that the full directory path where the archive
  is unpacked does not contain any spaces.)
- Test the installation:
  1)Run the program <install-dir>/mbsim/bin/mbsim-test to check the
    installation. This will run the MBSim examples xmlflat/hierachical_modelling,
    xml/hydraulics_ballcheckvalve and xml/hierachical_modelling as well as the
    program h5plotserie, openmbv and mbsimgui.
  2)If you have a compiler (GNU gcc) installed you can also run
    <install-dir>/mbsim/bin/mbsim-test <path-to-my-c++-compiler>.
    This will first try to compile a simple MBSim test program including all
    MBSim modules. Afterwards the mechanics/basics/hierachical_modelling
    example will be compiled and executed. At least the same as in 1) is run.
- Try any of the programs in <install-dir>/mbsim/bin
- Build your own models using XML and run it with
  <install-dir>/mbsim/bin/mbsimxml <mbsim-project-file.xml>
  View the plots with h5plotserie and view the animation with openmbv.
- Build your own models using the GUI: mbsimgui
- Try to compile and run your own source code models. Use the output of
  <install-dir>/mbsim/bin/mbsim-config --cflags and 
  <install-dir>/mbsim/bin/mbsim-config --libs as compiler and linker flags.

Have fun!
EOF

# Add some examples
mkdir -p $DISTDIR/examples
(cd $DISTDIR/examples; svn checkout https://mbsim-env.googlecode.com/svn/trunk/examples/mechanics/basics/hierachical_modelling mechanics/basics/hierachical_modelling)
(cd $DISTDIR/examples; svn checkout https://mbsim-env.googlecode.com/svn/trunk/examples/xmlflat/hierachical_modelling xmlflat/hierachical_modelling)
(cd $DISTDIR/examples; svn checkout https://mbsim-env.googlecode.com/svn/trunk/examples/xml/hierachical_modelling xml/hierachical_modelling)
(cd $DISTDIR/examples; svn checkout https://mbsim-env.googlecode.com/svn/trunk/examples/xml/hydraulics_ballcheckvalve xml/hydraulics_ballcheckvalve)
(cd $DISTDIR/examples; svn checkout https://mbsim-env.googlecode.com/svn/trunk/examples/fmi fmi)
mkdir -p $DISTDIR/examples/compile_test_all
cat << EOF > $DISTDIR/examples/compile_test_all/main.cc
#include <openmbvcppinterface/cube.h>
#include <mbsim/rigid_body.h>
#include <mbsim/integrators/lsode_integrator.h>
#include <mbsimControl/actuator.h>
#include <mbsimElectronics/simulation_classes.h>
#include <mbsimFlexibleBody/contours/flexible_band.h>
#include <mbsimHydraulics/hnode.h>
#include <mbsimPowertrain/cardan_shaft.h>

int main() {
  boost::shared_ptr<OpenMBV::Cube> cube=OpenMBV::ObjectFactory::create<OpenMBV::Cube>();
  MBSim::RigidBody *rb=new MBSim::RigidBody("RB");
  MBSimIntegrator::LSODEIntegrator *integ=new MBSimIntegrator::LSODEIntegrator;
  MBSimControl::Actuator *act=new MBSimControl::Actuator("ACT");
  MBSimElectronics::Diode *di=new MBSimElectronics::Diode("DI");
  MBSimFlexibleBody::FlexibleBand *band=new MBSimFlexibleBody::FlexibleBand("FLEX");
  MBSimHydraulics::ElasticNode *node=new MBSimHydraulics::ElasticNode("NODE");
  MBSimPowertrain::CardanShaft *shaft=new MBSimPowertrain::CardanShaft("SHAFT2");
}
EOF
# Add test script
cat << EOF > $DISTDIR/bin/mbsim-test
#! /bin/sh

INSTDIR="\$(readlink -f \$(dirname \$0)/..)"

CXX=\$1

cd \$INSTDIR/examples

if [ "_\$CXX" != "_" ]; then
  LD_LIBRARY_PATH_SAVE=\$LD_LIBRARY_PATH
  export LD_LIBRARY_PATH=\$INSTDIR/lib

  echo "COMPILE_TEST_ALL"
  cd compile_test_all
  \$CXX -c -o main.o main.cc \$(\$INSTDIR/bin/mbsim-config --cflags) || exit
  \$CXX -o main main.o \$(\$INSTDIR/bin/mbsim-config --libs) || exit
  ./main || exit
  cd ..
  echo "DONE"
  
  echo "MECHANICS_BASICS_HIERACHICAL_MODELLING"
  cd mechanics/basics/hierachical_modelling
  \$CXX -c -o group1.o group1.cc \$(\$INSTDIR/bin/mbsim-config --cflags) || exit
  \$CXX -c -o group2.o group2.cc \$(\$INSTDIR/bin/mbsim-config --cflags) || exit
  \$CXX -c -o system.o system.cc \$(\$INSTDIR/bin/mbsim-config --cflags) || exit
  \$CXX -c -o main.o main.cc \$(\$INSTDIR/bin/mbsim-config --cflags) || exit
  \$CXX -o main main.o system.o group1.o group2.o \$(\$INSTDIR/bin/mbsim-config --libs) || exit
  ./main || exit
  cd ../../..
  echo "DONE"

  echo "FMI_SPHERE_ON_PLANE"
  cd fmi/sphere_on_plane
  \$CXX -fPIC -c -o fmi.o fmi.cc \$(\$INSTDIR/bin/mbsim-config --cflags) || exit
  \$CXX -fPIC -c -o system.o system.cc \$(\$INSTDIR/bin/mbsim-config --cflags) || exit
  \$CXX -shared -o mbsimfmi_model.so system.o fmi.o \$(\$INSTDIR/bin/mbsim-config --libs) || exit
  \$INSTDIR/bin/mbsimCreateFMU mbsimfmi_model.so || exit
  \$INSTDIR/bin/fmuCheck.* mbsim.fmu || exit
  cd ../..
  echo "DONE"

  export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH_SAVE
fi

echo "XMLFLAT_HIERACHICAL_MODELLING"
cd xmlflat/hierachical_modelling
\$INSTDIR/bin/mbsimflatxml MBS.mbsimprj.flat.xml || exit
cd ../..
echo "DONE"

echo "XML_HIERACHICAL_MODELLING"
cd xml/hierachical_modelling
\$INSTDIR/bin/mbsimxml MBS.mbsimprj.xml || exit
cd ../..
echo "DONE"

echo "XML_HYDRAULICS_BALLCHECKVALVE"
cd xml/hydraulics_ballcheckvalve
\$INSTDIR/bin/mbsimxml MBS.mbsimprj.xml || exit
cd ../..
echo "DONE"

echo "FMI_SIMPLE_TEST"
cd fmi/simple_test
\$INSTDIR/bin/mbsimCreateFMU FMI.mbsimprj.xml || exit
\$INSTDIR/bin/fmuCheck.* mbsim.fmu || exit
cd ../..
echo "DONE"

#echo "FMI_HIERACHICAL_MODELLING"
#cd fmi/hierachical_modelling
#\$INSTDIR/bin/mbsimCreateFMU FMI.mbsimprj.xml || exit
#\$INSTDIR/bin/fmuCheck.* mbsim.fmu || exit
#cd ../..
#echo "DONE"

echo "STARTING H5PLOTSERIE"
\$INSTDIR/bin/h5plotserie xml/hierachical_modelling/TS.mbsim.h5 || exit
echo "DONE"

echo "STARTING OPENMBV"
\$INSTDIR/bin/openmbv xml/hierachical_modelling/TS.ombv.xml || exit
echo "DONE"

echo "STARTING MBSIMGUI"
\$INSTDIR/bin/mbsimgui || exit
echo "DONE"

echo "ALL TESTS DONE"
EOF
chmod +x $DISTDIR/bin/mbsim-test
     
# archive dist dir
if [ $NOARCHIVE -eq 0 ]; then
  rm -f $DISTBASEDIR/mbsim-linux-shared-build-xxx.tar.bz2
  (cd $DISTBASEDIR; tar -cvjf $DISTBASEDIR/mbsim-linux-shared-build-xxx.tar.bz2 mbsim)
  echo "Create MBSim archive at $DISTBASEDIR/mbsim-linux-shared-build-xxx.tar.bz2"
fi
