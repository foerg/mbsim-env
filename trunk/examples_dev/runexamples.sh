#! /bin/sh

if [ $# -eq 1 -a "$1" = "-h" ]; then
  echo "Usage:"
  echo ""
  echo "This script must be executed in the current directory!"
  echo ""
  echo "runexamples.sh (run all examples with default RTOL=1e-6)"
  echo "runexamples.sh 1e-3 (run all examples with RTOL=1e-3)"
  echo "runexamples.sh robot (run robot example with default RTOL=1e-6)"
  echo "runexamples.sh robot 1e-3 (run robot example with RTOL=1e-3)"
  echo "runexamples.sh dist (make a ./reference.tar.bz2 reference distribution using"
  echo "        the data stored in the reference directory of the respective example)"
  echo "runexamples.sh install (install the reference.tar.bz2 reference file from"
  echo "                        the berlios server)"
  echo "runexamples.sh install ../myref/ref.tar.bz2 (install the ../myref/ref.tar.bz2"
  echo "                                            reference file)"
  exit
fi

RTOL=1e-6
EXAMPLES=$(find -maxdepth 1 -type d | grep -v "^\.$" | grep -v "^\./\.")
if [ $# -eq 1 ]; then
  if [ "$1" = "dist" ]; then
    echo "Making a distribution of reference files: reference.tar.bz2"
    tar -cjf reference.tar.bz2 $(find -name "reference")
    exit
  fi
  if [ "$1" = "install" ]; then
    echo "Download reference file"
    rm reference.tar.bz2
    wget http://download.berlios.de/mbsim/reference.tar.bz2
    echo "Install the reference file"
    tar -xjf reference.tar.bz2
    exit
  fi
  if cd $1; then
    EXAMPLES=$1
  else
    RTOL=$1
  fi
fi
if [ $# -eq 2 ]; then
  if [ "$1" = "install" ]; then
    echo "Install the reference file: $2"
    tar -xjf $2
    exit
  fi
  EXAMPLES=$1
  RTOL=$2
fi


# RUN EXAMPLES

FAILED=""
DIFF=""
find -name "*.d" -exec rm -f {} \;

for D in $EXAMPLES; do
  echo "RUNNING EXAMPLE $D"
  cd $D

  ERROR=1
  make clean && \
  make && \
  ./main && \
  ERROR=0

  if [ $ERROR -eq 0 ]; then
    echo "EXAMPLE $D PASSED COMPILING AND RUNNING"
  else
    echo "EXAMPLE $D FAILED COMPILING OR RUNNING"
    FAILED="$FAILED\n$D"
  fi

  if test -d reference; then
    for H5F in $(cd reference && find -name "*.h5"); do
      for DS in $($(pkg-config hdf5serie --variable=bindir)/h5lsserie $H5F | sed -nre "s|^.*\(Path: (.*)\)|\1|p"); do
        P=$(echo $DS | sed -re "s|^.*\.h5/(.*)|\1|")
        $(pkg-config hdf5serie --variable=hdf5_prefix)/bin/h5diff --relative=$RTOL $H5F reference/$H5F $P $P
        RET=$?
        if [ $RET -ne 0 ]; then
          echo "EXAMPLE $DS FAILED DIFF WITH REFERENCE SOLUTION"
          DIFF="$DIFF\n$D/$DS"
        else
          echo "EXAMPLE $DS PASSED DIFF WITH REFERENCE SOLUTION"
        fi
      done
    done
  fi

  cd ..
done

echo -e "\n\n\n\n\n\n\n\n\n\n"
echo -e "EXAMPLES FAILED COMPILING OR RUNNING:$FAILED"
echo -e "\n"
echo -e "EXAMPLES FAILED DIFF WITH REFERENCE SOLUTION:$DIFF"
