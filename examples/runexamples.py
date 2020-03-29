#! /usr/bin/python3

# imports
import sys
import argparse
import fnmatch
import os
import stat
from os.path import join as pj
import subprocess
import datetime
import fileinput
import glob
import shutil
import functools
import multiprocessing
import math
import traceback
import tarfile
import re
import hashlib
import codecs
import threading
import time
import json
import fcntl
import zipfile
import tempfile
import urllib.request
import urllib.parse

# global variables
scriptDir=os.path.dirname(os.path.realpath(__file__))
mbsimBinDir=None
canCompare=True # True if numpy and h5py are found
mbxmlutilsvalidate=None
ombvSchema=None
mbsimXMLSchemas=None
timeID=None
directories=list() # a list of all examples sorted in descending order (filled recursively (using the filter) by --directories)

# MBSim Modules
mbsimModules=["mbsimControl", "mbsimElectronics", "mbsimFlexibleBody",
              "mbsimHydraulics", "mbsimInterface", "mbsimPowertrain"]

# command line option definition
argparser = argparse.ArgumentParser(
  formatter_class=argparse.RawTextHelpFormatter,
  description='''
Run MBSim examples.
This script runs the action given by --action on all specified directories recursively.
However only examples of the type matching --filter are executed. The specified directories are
processed from left to right.
The type of an example is defined dependent on some key files in the corrosponding example directory:
- If a file named 'Makefile' exists, than it is treated as a SRC example.
- If a file named 'MBS.mbsimprj.flat.xml' exists, then it is treated as a FLATXML example.
- If a file named 'MBS.mbsimprj.xml' exists, then it is treated as a XML example
  which run throught the MBXMLUtils preprocessor first.
- If a file named 'FMI.mbsimprj.xml' exists, then it is treated as a FMI ME XML export example. Beside running the file
  by mbsimxml also mbsimCreateFMU is run to export the model as a FMU and the FMU is run by fmuCheck.<PLATFORM>.
- If a file named 'Makefile_FMI' exists, then it is treated as a FMI ME source export example. Beside compiling the
  source examples also mbsimCreateFMU is run to export the model as a FMU and the FMU is run by fmuCheck.<PLATFORM>.
- If a file named 'FMI_cosim.mbsimprj.xml' exists, then it is treated as a FMI Cosim XML export example. Beside running the file
  by mbsimxml also mbsimCreateFMU is run to export the model as a FMU and the FMU is run by fmuCheck.<PLATFORM>.
- If a file named 'Makefile_FMI_cosim' exists, then it is treated as a FMI Cosim source export example. Beside compiling the
  source examples also mbsimCreateFMU is run to export the model as a FMU and the FMU is run by fmuCheck.<PLATFORM>.
If more then one of these files exist the behaviour is undefined.
The 'Makefile' of a SRC example must build the example and must create an executable named 'main'.
'''
)

mainOpts=argparser.add_argument_group('Main Options')
mainOpts.add_argument("directories", nargs="*", default=os.curdir,
  help="A directory to run (recursively). If prefixed with '^' remove the directory form the current list [default: %(default)s]")
mainOpts.add_argument("--action", default="report", type=str,
  help='''The action of this script: [default: %(default)s]
- 'report'                     Run examples and report results
- 'copyToReference'            Copy current results to reference directory
- 'updateReference[=URL|DIR]'  Update references from URL or DIR, use the build system if not given
- 'pushReference=DIR'          Push references to DIR
- 'list'                       List directories to be run''')
mainOpts.add_argument("-j", default=1, type=int,
  help="Number of jobs to run in parallel (applies only to the action 'report') [default: %(default)s]")
mainOpts.add_argument("--filter", default="'nightly' in labels", type=str,
  help='''Filter the specifed directories using the given Python code. If not given all directories with the
label 'nightly' are used [default: %(default)s]
A directory is processed if the provided Python code evaluates to True where the following variables are defined:
- src      Is True if the directory is a source code example
- flatxml  Is True if the directory is a xml flat example
- ppxml    Is True if the directory is a preprocessing xml example
- xml      Is True if the directory is a flat or preprocessing xml example
- fmi      Is True if the directory is a FMI export example (source or XML)
- labels   A list of labels (defined by the 'labels' file, being a space separated list of labels).
           The labels defined in the 'labels' file are extended automatically by the MBSim module
           labels: '''+str(mbsimModules)+'''
           The special label 'willfail' defines examples which are not reported as errors if they fail.
           If no labels file exists in a directory a labels file with the content "nightly" is assumed.
Example: --filter "xml and 'mbsimControl' not in labels or 'basic' in labels"
         run xml examples not requiring mbsimControl or all examples having the label "basic"''')

cfgOpts=argparser.add_argument_group('Configuration Options')
cfgOpts.add_argument("--atol", default=2e-5, type=float,
  help="Absolute tolerance. Channel comparing failed if for at least ONE datapoint the abs. AND rel. toleranz is violated [default: %(default)s]")
cfgOpts.add_argument("--rtol", default=2e-5, type=float,
  help="Relative tolerance. Channel comparing failed if for at least ONE datapoint the abs. AND rel. toleranz is violated [default: %(default)s]")
cfgOpts.add_argument("--disableRun", action="store_true", help="disable running the example on action 'report'")
cfgOpts.add_argument("--disableMakeClean", action="store_true", help="disable make clean on action 'report'")
cfgOpts.add_argument("--checkGUIs", action="store_true", help="Try to check/start the GUIs and exit after a short time.")
cfgOpts.add_argument("--disableCompare", action="store_true", help="disable comparing the results on action 'report'")
cfgOpts.add_argument("--disableValidate", action="store_true", help="disable validating the XML files on action 'report'")
cfgOpts.add_argument("--printToConsole", action='store_const', const=sys.stdout, help="print all output also to the console")
cfgOpts.add_argument("--buildType", default="local", type=str, help="Description of the build type (e.g: linux64-dailydebug) [default: %(default)s]")
cfgOpts.add_argument("--prefixSimulation", default=None, type=str,
  help="prefix the simulation command (./main, mbsimflatxml, mbsimxml) with this string: e.g. 'valgrind --tool=callgrind'")
cfgOpts.add_argument("--prefixSimulationKeyword", default=None, type=str,
  help="VALGRIND: add special arguments and handling for valgrind")
cfgOpts.add_argument("--exeExt", default="", type=str, help="File extension of cross compiled executables (wine is used if set)")
cfgOpts.add_argument("--maxExecutionTime", default=30, type=float, help="The time in minutes after started program timed out [default: %(default)s]")
cfgOpts.add_argument("--maxCompareFailure", default=200, type=float, help="Maximal number of compare failures to report. Use 0 for unlimited [default: %(default)s]")
cfgOpts.add_argument("--coverage", default=None, type=str, help='Enable coverage analyzis using gcov/lcov; The arg must be: <sourceDir>:<binSuffix>:<prefix>:<baseExamplesDir>')

outOpts=argparser.add_argument_group('Output Options')
outOpts.add_argument("--reportOutDir", default="runexamples_report", type=str, help="the output directory of the report [default: %(default)s]")
outOpts.add_argument("--url", type=str,
  help="the URL where the report output is accessible (without the trailing '/index.html'. Only used for the Atom feed")
outOpts.add_argument("--rotate", default=3, type=int, help="keep last n results and rotate them [default: %(default)s]")

debugOpts=argparser.add_argument_group('Debugging and other Options')
debugOpts.add_argument("--debugDisableMultiprocessing", action="store_true",
  help="disable the -j option and run always in a single process/thread")
debugOpts.add_argument("--currentID", default=0, type=int, help="Internal option used in combination with build.py")
debugOpts.add_argument("--timeID", default="", type=str, help="Internal option used in combination with build.py")
debugOpts.add_argument("--buildSystemRun", action="store_true", help="Run in build system mode: generate build system state.")
debugOpts.add_argument("--webapp", action="store_true", help="Add buttons for mbsimwebapp.")

# parse command line options
args = argparser.parse_args()

# A file object which prints to multiple files
class MultiFile(object):
  def __init__(self, file1, second=None):
    self.filelist=[file1]
    if second!=None:
      self.filelist.append(second)
  def write(self, str):
    for f in self.filelist:
      f.write(str)
  def flush(self):
    for f in self.filelist:
      f.flush()
  def close(self):
    for f in self.filelist:
      if f!=sys.stdout and f!=sys.stderr:
        f.close()
# kill the called subprocess
def killSubprocessCall(proc, f, killed, timeout):
  killed.set()
  f.write("\n\n\n******************** START: MESSAGE FROM runexamples.py ********************\n")
  f.write("The maximal execution time (%d min) has reached (option --maxExecutionTime),\n"%(timeout))
  f.write("but the program is still running. Terminating the program now.\n")
  f.write("******************** END: MESSAGE FROM runexamples.py **********************\n\n\n\n")
  proc.terminate()
  time.sleep(30)
  # if proc has not terminated after 30 seconds kill it
  if proc.poll()==None:
    f.write("\n\n\n******************** START: MESSAGE FROM runexamples.py ********************\n")
    f.write("Program has not terminated after 30 seconds, killing the program now.\n")
    f.write("******************** END: MESSAGE FROM runexamples.py **********************\n\n\n\n")
    proc.kill()
# subprocess call with MultiFile output
def subprocessCall(args, f, env=os.environ, maxExecutionTime=0):
  # remove core dumps from previous runs
  for coreFile in glob.glob("*core*"):
    if "LSB core file" in subprocess.check_output(["file", coreFile]).decode('utf-8'):
      os.remove(coreFile)
  # start the program to execute
  proc=subprocess.Popen(args, stderr=subprocess.STDOUT, stdout=subprocess.PIPE, bufsize=-1, env=env)
  # a guard for the maximal execution time for the starte program
  guard=None
  killed=threading.Event()
  if maxExecutionTime>0:
    guard=threading.Timer(maxExecutionTime*60, killSubprocessCall, args=(proc, f, killed, maxExecutionTime))
    guard.start()
  # make stdout none blocking
  fd=proc.stdout.fileno()
  fcntl.fcntl(fd, fcntl.F_SETFL, fcntl.fcntl(fd, fcntl.F_GETFL) | os.O_NONBLOCK)
  # read all output
  lineNP=b'' # not already processed bytes (required since we read 100 bytes which may break a unicode multi byte character)
  while proc.poll()==None:
    time.sleep(0.5)
    try:
      line=lineNP+proc.stdout.read()
    except:
      continue
    lineNP=b''
    try:
      print(line.decode("utf-8"), end="", file=f)
    except UnicodeDecodeError as ex: # catch broken multibyte unicode characters and append it to next line
      print(line[0:ex.start].decode("utf-8"), end="", file=f) # print up to first broken character
      lineNP=ex.object[ex.start:] # add broken characters to next line
  # wait for the call program to exit
  ret=proc.wait()
  # stop the execution time guard thread
  if maxExecutionTime>0:
    if killed.isSet():
      return subprocessCall.timedOutErrorCode # return to indicate that the program was terminated/killed
    else:
      guard.cancel()
  # check for core dump file
  exeRE=re.compile("^.*LSB core file.*, *from '([^']*)' *,.*$")
  for coreFile in glob.glob("*core*"):
    m=exeRE.match(subprocess.check_output(["file", coreFile]).decode('utf-8'))
    if m==None: continue
    exe=m.group(1).split(" ")[0]
    out=subprocess.check_output(["gdb", "-q", "-n", "-ex", "bt", "-batch", exe, coreFile]).decode('utf-8')
    f.write("\n\n\n******************** START: CORE DUMP BACKTRACE OF "+exe+" ********************\n\n\n")
    f.write(out)
    f.write("\n\n\n******************** END: CORE DUMP BACKTRACE ********************\n\n\n")
  # return the return value ot the called programm
  return ret
subprocessCall.timedOutErrorCode=1000000

# rotate
def rotateOutput():
  # create output dir
  if not os.path.isdir(args.reportOutDir): os.makedirs(args.reportOutDir)

  if args.currentID==0:
    # get result IDs of last runs
    resultID=[]
    for curdir in glob.glob(pj(args.reportOutDir, "result_*")):
      currentID=1
      # skip all except result_[0-9]+
      try: currentID=int(curdir[len(pj(args.reportOutDir, "result_")):])
      except ValueError: continue
      # skip symbolic links
      if os.path.islink(curdir):
        os.remove(curdir)
        continue
      # add to resultID
      resultID.append(currentID);
    # sort resultID
    resultID=sorted(resultID)

    # calculate ID for this run
    if len(resultID)>0:
      currentID=resultID[-1]+1
    else:
      currentID=1

    # only keep args.rotate old results
    delFirstN=len(resultID)-args.rotate
    if delFirstN>0:
      for delID in resultID[0:delFirstN]:
        shutil.rmtree(pj(args.reportOutDir, "result_%010d"%(delID)))
      resultID=resultID[delFirstN:]

    # create link for very last result
    lastLinkID=1
    if len(resultID)>0:
      lastLinkID=resultID[0]
    try: os.remove(pj(args.reportOutDir, "result_%010d"%(lastLinkID-1)))
    except OSError: pass
    os.symlink("result_%010d"%(lastLinkID), pj(args.reportOutDir, "result_%010d"%(lastLinkID-1)))
    # create link for very first result
    try: os.remove(pj(args.reportOutDir, "result_%010d"%(currentID+1)))
    except OSError: pass
    os.symlink("result_%010d"%(currentID), pj(args.reportOutDir, "result_%010d"%(currentID+1)))
  else:
    currentID=args.currentID
  # create link for current result
  try: os.remove(pj(args.reportOutDir, "result_current"))
  except OSError: pass
  os.symlink("result_%010d"%(currentID), pj(args.reportOutDir, "result_current"))

  # fix reportOutDir, create and clean output dir
  args.reportOutDir=pj(args.reportOutDir, "result_%010d"%(currentID))
  if os.path.isdir(args.reportOutDir): shutil.rmtree(args.reportOutDir)
  os.makedirs(args.reportOutDir)

# the main routine being called ones
def main():
  # check arguments
  if not (args.action=="report" or args.action=="copyToReference" or
          args.action=="updateReference" or args.action.startswith("updateReference=") or
          args.action.startswith("pushReference=") or
          args.action=="list"):
    argparser.print_usage()
    print("error: unknown argument --action "+args.action+" (see -h)")
    return 1
  args.updateURL="https://www.mbsim-env.de/mbsim/linux64-dailydebug/references" # default value
  args.pushDIR=None # no default value (use /var/www/html/mbsim-env/MBSimDailyBuild/references for the build system)
  if args.action.startswith("updateReference="):
    if os.path.isdir(args.action[16:]):
      args.updateURL="file://"+urllib.request.pathname2url(os.path.abspath(args.action[16:]))
    else:
      args.updateURL=args.action[16:]
    args.action="updateReference"
  if args.action.startswith("pushReference="):
    args.pushDIR=args.action[14:]
    args.action="pushReference"

  if args.buildSystemRun:
    sys.path.append("/context")
    import buildSystemState
    if args.coverage!=None:
      buildSystemState.createStateSVGFile("/mbsim-state/"+args.buildType+"-coverage.svg", "...", "#777")
    buildSystemState.createStateSVGFile("/mbsim-state/"+args.buildType+"-examples.nrFailed.svg", "...", "#777")
    buildSystemState.createStateSVGFile("/mbsim-state/"+args.buildType+"-examples.nrAll.svg", "...", "#777")

  # fix arguments
  args.reportOutDir=os.path.abspath(args.reportOutDir)
  if args.prefixSimulation!=None:
    args.prefixSimulation=args.prefixSimulation.split(' ')
  else:
    args.prefixSimulation=[]

  # rotate (modifies args.reportOutDir)
  rotateOutput()
  os.makedirs(pj(args.reportOutDir, "tmp"))

  # check if the numpy and h5py modules exists. If not disable compare
  try: 
    import numpy
    import h5py
  except ImportError: 
    print("WARNING!")
    print("The python module numpy and h5py is required for full functionallity of this script.")
    print("However at least one of these modules are not found. Hence comparing the results will be disabled.\n")
    global canCompare
    canCompare=False
  # get mbxmlutilsvalidate program
  global mbxmlutilsvalidate
  mbxmlutilsvalidate=pj(pkgconfig("mbxmlutils", ["--variable=BINDIR"]), "mbxmlutilsvalidate"+args.exeExt)
  if not os.path.isfile(mbxmlutilsvalidate):
    mbxmlutilsvalidate="mbxmlutilsvalidate"+args.exeExt
  # set global dirs
  global mbsimBinDir
  mbsimBinDir=pkgconfig("mbsim", ["--variable=bindir"])
  # get schema files
  schemaDir=pkgconfig("mbxmlutils", ["--variable=SCHEMADIR"])
  global ombvSchema, mbsimXMLSchemas
  # create mbsimxml schema
  mbsimXMLSchemas=subprocess.check_output(exePrefix()+[pj(mbsimBinDir, "mbsimxml"+args.exeExt), "--onlyListSchemas"]).\
    decode("utf-8").split()
  ombvSchemaRE=re.compile(".http___www_mbsim-env_de_OpenMBV.openmbv.xsd$")
  ombvSchema=list(filter(lambda x: ombvSchemaRE.search(x)!=None, mbsimXMLSchemas))[0]

  # check args.directories
  for d in args.directories:
    if not os.path.isdir(d):
      print("The positional argument (directory) "+d+" does not exist.")
      exit(1)

  # if no directory is specified use the current dir (all examples) filter by --filter
  if len(args.directories)==0:
    dirs=[os.curdir]
  else:
    dirs=args.directories
  # loop over all directories on command line and add subdir which match the filter
  directoriesSet=set()
  for d in dirs:
    addExamplesByFilter(d, directoriesSet)

  # sort directories in descending order of simulation time (reference time)
  sortDirectories(directoriesSet, directories)

  # copy the current solution to the reference directory
  if args.action=="copyToReference":
    copyToReference()
    return 0

  # apply (unpack) a reference archive
  if args.action=="pushReference":
    pushReference()
    return 0

  # apply (unpack) a reference archive
  if args.action=="updateReference":
    updateReference()
    return 0

  # list directires to run
  if args.action=="list":
    listExamples()
    return 0

  # create index.html
  mainFD=codecs.open(pj(args.reportOutDir, "index.html"), "w", encoding="utf-8")
  print('''<!DOCTYPE html>
  <html lang="en">
  <head>
    <META http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>MBSim runexamples Results: %s</title>
    <link rel="stylesheet" type="text/css" href="https://cdn.datatables.net/s/bs-3.3.5/jq-2.1.4,dt-1.10.10/datatables.min.css"/>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/octicons/3.5.0/octicons.min.css"/>
    <link rel="shortcut icon" href="/mbsim/html/mbsimenv.ico" type="image/x-icon"/>
    <link rel="icon" href="/mbsim/html/mbsimenv.ico" type="image/x-icon"/>
  </head>
  <body style="margin:0.5em">
  <script src="https://cdn.datatables.net/s/bs-3.3.5/jq-2.1.4,dt-1.10.10/datatables.min.js"> </script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.22.2/moment.min.js"> </script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/moment-timezone/0.5.23/moment-timezone-with-data-2012-2022.min.js"> </script>
  <script src="/mbsim/html/cookiewarning.js"> </script>
  <script>
    $(document).ready(function() {
      $('.DATETIME').each(function() {
        $(this).text(moment($(this).text()).tz(moment.tz.guess()).format("ddd, YYYY-MM-DD - HH:mm:ss z"));
      }); 
      // init table
      $('#SortThisTable').dataTable({'lengthMenu': [ [10, 25, 50, 100, -1], [10, 25, 50, 100, 'All'] ],
                                     'pageLength': 25, 'aaSorting': [], stateSave: true});

      // when the save button is clicked
      $("#SAVEBUTTON").click(function() {
        statusCommunicating();
        // collect all data
        var checkedExamples=[];
        $("#SortThisTable").DataTable().$("._EXAMPLE").each(function() {
          if($(this).prop("checked")) {
            checkedExamples.push($(this).attr("name"));
          }
        });
        // save current checked examples
        var data={checkedExamples: checkedExamples}
        $.ajax({url: serverScriptURI+"/setcheck", xhrFields: {withCredentials: true}, dataType: "json",
                type: "POST", data: JSON.stringify(data)}).done(function(response) {
          statusMessage(response);
        });
      });

      // if this is the current example table from the build server and is finished than enable the reference update
      if($(location).attr('href').search("/mbsim/linux64-dailydebug/report/result_current/runexamples_report/result_current")>=0 &&
          $("#FINISHED").length>0) {
        // load mbsimBuildServiceClient.js
        $.getScript("/mbsim/html/mbsimBuildServiceClient.js", function() {
          // show reference update and status
          $("#UPDATEREFERENCES").css("display", "block");
          $("#STATUSPANEL").css("display", "block");
    
          // update checked examples using server data
          statusCommunicating();
          $.ajax({url: serverScriptURI+"/getcheck", xhrFields: {withCredentials: true}, dataType: "json", type: "GET"}).done(function(response) {
            if(!response.success)
              statusMessage(response);
            else {
              // "check" and enable these
              $("#SortThisTable").DataTable().$("._EXAMPLE").each(function() {
                $(this).prop("checked", $.inArray($(this).attr("name"), response.checkedExamples)>=0);
                $(this).prop("disabled", false);
              });
              statusMessage(response);
            }
          });
        });
      }

      // if this is the current result and the sessionid cookie exists then enable the webapp buttons
      if($(location).attr('href').search("/report/result_current/runexamples_report/result_current")>=0) {
        var c=document.cookie.split(';');
        for(var i=0; i<c.length; i++)
          if(c[i].split('=')[0].trim()=="mbsimenvsessionid_js") {
        $("#SortThisTable").DataTable().$("._WEBAPP").each(function() {
          $(this).prop("disabled", false);
        });
            break;
          }
      }
    });
  </script>'''%(args.buildType), file=mainFD)

  print('<h1>MBSim runexamples Results: <small>%s</small></h1>'%(args.buildType), file=mainFD)
  print('<dl class="dl-horizontal">', file=mainFD)
  print('''<dt>Called Command</dt><dd><div class="dropdown">
  <button class="btn btn-default btn-xs" id="calledCommandID" data-toggle="dropdown">show <span class="caret"></span>
  </button>
  <code class="dropdown-menu" style="padding-left: 0.5em; padding-right: 0.5em;" aria-labelledby="calledCommandID">''', file=mainFD)
  for argv in sys.argv: print(argv.replace('/', u'/\u200B')+' ', file=mainFD)
  print('</code></div></dd>', file=mainFD)
  global timeID
  timeID=datetime.datetime.utcnow()
  timeID=datetime.datetime(timeID.year, timeID.month, timeID.day, timeID.hour, timeID.minute, timeID.second)
  if args.timeID!="":
    timeID=datetime.datetime.strptime(args.timeID, "%Y-%m-%dT%H:%M:%SZ")
  print('  <dt>Time ID</dt><dd class="DATETIME">'+timeID.isoformat()+'Z</dd>', file=mainFD)
  print('  <dt>End time</dt><dd><!--S_ENDTIME--><span class="text-danger"><b>still running or aborted</b></span><!--E_ENDTIME--></dd>', file=mainFD)
  currentID=int(os.path.basename(args.reportOutDir)[len("result_"):])
  navA=""
  navB=""
  if args.currentID!=0:
    navA="/../.."
    navB="/runexamples_report/result_current"
  print('  <dt>Navigate</dt><dd><a class="btn btn-info btn-xs" href="..%s/result_%010d%s/index.html"><span class="glyphicon glyphicon-step-backward"> </span> previous</a>'%(navA, currentID-1, navB), file=mainFD)
  print('                    <a class="btn btn-info btn-xs" href="..%s/result_%010d%s/index.html"><span class="glyphicon glyphicon-step-forward"> </span> next</a>'%(navA, currentID+1, navB), file=mainFD)
  print('                    <a class="btn btn-info btn-xs" href="..%s/result_current%s/index.html"><span class="glyphicon glyphicon-fast-forward"> </span> newest</a>'%(navA, navB), file=mainFD)
  if args.currentID!=0:
    print('                  <a class="btn btn-info btn-xs" href="../../index.html"><span class="glyphicon glyphicon-eject"> </span> parent</a>', file=mainFD)
  print('                    </dd>', file=mainFD)
  print('</dl>', file=mainFD)
  print('<hr/><p><span class="glyphicon glyphicon-info-sign"> </span> A example with grey text is a example which may fail and is therefore not reported as an error in the Atom feed.</p>', file=mainFD)

  print('<table id="SortThisTable" class="table table-striped table-hover table-bordered table-condensed">', file=mainFD)
  print('<thead><tr>', file=mainFD)
  print('<th><span class="glyphicon glyphicon-folder-open"></span>&nbsp;Example</th>', file=mainFD)
  if not args.disableRun:
    print('<th><span class="glyphicon glyphicon-repeat"></span>&nbsp;Run</th>', file=mainFD)
    print('<th><span class="glyphicon glyphicon-time"></span>&nbsp;Time</th>', file=mainFD)
    print('<th><span class="glyphicon glyphicon-time"></span>&nbsp;Ref. Time</th>', file=mainFD)
  if args.checkGUIs:
    print('<th><span class="glyphicon glyphicon-modal-window"></span>&nbsp;GUI Test</th>', file=mainFD)
  if not args.disableCompare:
    print('<th><div class="pull-left"><span class="glyphicon glyphicon-search"></span>&nbsp;Ref.</div>'+\
          '<div class="pull-right" style="padding-right:0.75em;">[update]</div></th>', file=mainFD)
  if not args.disableRun and args.buildSystemRun and args.webapp:
    print('<th><span class="glyphicon glyphicon-picture"></span>&nbsp;Webapp</th>', file=mainFD)
  if not args.disableRun:
    print('<th><span class="glyphicon glyphicon-warning-sign"></span>&nbsp;Depr.</th>', file=mainFD)
  if not args.disableValidate:
    print('<th><span class="glyphicon glyphicon-ok"></span>&nbsp;XML out.</th>', file=mainFD)
  print('</tr></thead><tbody>', file=mainFD)
  mainFD.flush()
  mainRet=0
  failedExamples=[]

  # run examples in parallel
  print("Started running examples. Each example will print a message if finished.")
  print("See the log file "+pj(os.path.dirname(args.reportOutDir), "result_current", "index.html")+" for detailed results.\n")

  if args.coverage!=None:
    # backup the coverage files in the build directories
    coverageBackupRestore('backup')
    # remove all "*.gcno", "*.gcda" files in ALL the examples
    for d,_,files in os.walk(args.coverage.split(":")[3]):
      for f in files:
        if os.path.splitext(f)[1]==".gcno": os.remove(pj(d, f))
        if os.path.splitext(f)[1]==".gcda": os.remove(pj(d, f))

  if args.checkGUIs:
    # start vnc server on a free display
    global displayNR
    displayNR=3
    while subprocess.call(["vncserver", ":"+str(displayNR), "-noxstartup", "-SecurityTypes", "None"], stdout=open(os.devnull, 'wb'), stderr=open(os.devnull, 'wb'))!=0:
      displayNR=displayNR+1
      if displayNR>100:
        raise RuntimeError("Cannot find a free DISPLAY for vnc server.")

  try:

    if not args.debugDisableMultiprocessing:
      # init mulitprocessing handling and run in parallel
      resultQueue = multiprocessing.Manager().Queue()
      poolResult=multiprocessing.Pool(args.j).map_async(functools.partial(runExample, resultQueue), directories, 1)
    else: # debugging
      import queue
      resultQueue = queue.Queue()
      poolResult=queue.Queue(); poolResult.put(list(map(functools.partial(runExample, resultQueue), directories)))

    # get the queue values = html table row
    missingDirectories=list(directories)
    for dummy in directories:
      result=resultQueue.get()
      print(result[1], file=mainFD)
      mainFD.flush()
      printFinishedMessage(missingDirectories, result)
    # wait for pool to finish and get result
    retAll=poolResult.get()

  finally:
    if args.checkGUIs:
      # kill vnc server
      if subprocess.call(["vncserver", "-kill", ":"+str(displayNR)], stdout=open(os.devnull, 'wb'), stderr=open(os.devnull, 'wb'))!=0:
        print("Cannot close vnc server on :%d but continue."%(displayNR))

  # set global result and add failedExamples
  for index in range(len(retAll)):
    willFail=False
    if os.path.isfile(pj(directories[index][0], "labels")):
      willFail='willfail' in codecs.open(pj(directories[index][0], "labels"), "r", encoding="utf-8").read().rstrip().split(' ')
    if retAll[index]!=0 and not willFail:
      mainRet=1
      failedExamples.append(directories[index][0])

  # coverage analyzis (postpare)
  coverageAll=0
  coverageFailed=0
  if args.coverage!=None:
    coverageAll=1
    print("Create coverage analyzis"); sys.stdout.flush()
    coverageFailed=coverage(mainFD)
    # restore the coverage files in the build directories
    coverageBackupRestore('restore')

  print('</tbody></table><hr/>', file=mainFD)

  if len(failedExamples)>0:
    print('<div class="panel panel-info">', file=mainFD)
    print('  <div class="panel-heading"><span class="glyphicon glyphicon-refresh"></span>&nbsp;<a data-toggle="collapse" href="#collapseRerunFailedExamples">'+\
            'Rerun all failed examples<span class="caret"> </span>'+\
            '</a></div>', file=mainFD)
    print('  <div class="panel-body panel-collapse collapse" id="collapseRerunFailedExamples">', file=mainFD)
    print('<code>'+sys.argv[0], end=" ", file=mainFD)
    for arg in sys.argv[1:]:
      if not arg in set(args.directories):
        print(arg, end=" ", file=mainFD)
    for failedEx in failedExamples:
      print(failedEx, end=" ", file=mainFD)
    print('</code>', file=mainFD)
    print('  </div>', file=mainFD)
    print('</div>', file=mainFD)

  print('''<div id="UPDATEREFERENCES" class="panel panel-warning" style="display:none">
  <div class="panel-heading"><span class="glyphicon glyphicon-pencil">
    </span>&nbsp;<a data-toggle="collapse" href="#collapseUpdateReferences">
 Update references<span class="caret"> </span></a></div>
  <div class="panel-body panel-collapse collapse" id="collapseUpdateReferences">
    <p>Update the references of the selected examples before next build.</p>
    <p>
      <button id="SAVEBUTTON" disabled="disabled" type="button" class="_DISABLEONCOMM btn btn-default btn-sm"><span class="glyphicon glyphicon-ok"></span>&nbsp;Save changes</button>
    </p>
  </div>
</div>
<div id="STATUSPANEL" class="panel panel-info" style="display:none">
  <div class="panel-heading"><span class="glyphicon glyphicon-info-sign">
    </span>&nbsp;<span class="glyphicon glyphicon-exclamation-sign"></span>&nbsp;Status message</div>
  <div class="panel-body">
    <pre style="border: 0; background-color: transparent;" id="STATUSMSG">Communicating with server, please wait. (reload page if hanging)</pre>
  </div>
</div>
<hr/>
<span class="pull-left small">
  <a href="/mbsim/html/impressum_disclaimer_datenschutz.html#impressum">Impressum</a> /
  <a href="/mbsim/html/impressum_disclaimer_datenschutz.html#disclaimer">Disclaimer</a> /
  <a href="/mbsim/html/impressum_disclaimer_datenschutz.html#datenschutz">Datenschutz</a>
</span>
<span class="pull-right small">
  Generated on <span class="DATETIME">%s</span> by runexamples.py
  <a href="/">Home</a>
</span>
<span id="FINISHED" style="display:none"> </span>
</body>
</html>'''%(timeID.isoformat()+"Z"), file=mainFD)

  mainFD.close()
  # replace end time in index.html
  endTime=datetime.datetime.now()
  endTime=datetime.datetime(endTime.year, endTime.month, endTime.day, endTime.hour, endTime.minute, endTime.second)
  with codecs.open(pj(args.reportOutDir, "index.html"), "r", encoding="UTF-8") as f:
    s=f.read()
  s=re.sub('<!--S_ENDTIME-->.*?<!--E_ENDTIME-->', '<span class="DATETIME">'+endTime.isoformat()+"Z</span>", s)
  with codecs.open(pj(args.reportOutDir, "index.html"), "w", encoding="UTF-8") as f:
    f.write(s)


  # write RSS feed
  writeAtomFeed(currentID, len(failedExamples)+coverageFailed, len(retAll)+coverageAll)

  # print result summary to console
  if len(failedExamples)>0:
    print('\nERROR: '+str(len(failedExamples))+' of '+str(len(retAll))+' examples have failed.')
  if coverageFailed!=0:
    mainRet=1
    print('\nERROR: Coverage analyzis generation failed.')

  return mainRet



#####################################################################################
# from now on only functions follow and at the end main is called
#####################################################################################



def pkgconfig(module, options):
  comm=["pkg-config", module]
  comm.extend(options)
  try:
    output=subprocess.check_output(comm).decode("utf-8")
  except subprocess.CalledProcessError as ex:
    if ex.returncode==0:
      raise
    else:
      print("Error: pkg-config module "+module+" not found. Trying to continue.", file=sys.stderr)
      output="pkg_config_"+module+"_not_found"
  return output.rstrip()



def printFinishedMessage(missingDirectories, result):
  missingDirectories.remove(result[0])
  lenDirs=len(directories)
  curNumber=lenDirs-len(missingDirectories)
  eta=0
  for example in missingDirectories:
    eta+=example[1]
  if not math.isinf(eta):
    etaStr=datetime.timedelta(0, round(eta/min(args.j, multiprocessing.cpu_count())))
  else:
    etaStr="unknown"
  print("Finished example %03d/%03d; %5.1f%%; ETA %s; %s; %s"%(curNumber, lenDirs, curNumber/lenDirs*100,
    etaStr, result[0][0], "passed" if result[2]==0 else "FAILED!!!"))



def sortDirectories(directoriesSet, dirs):
  unsortedDir=[]
  for example in directoriesSet:
    if os.path.isfile(pj(example, "reference", "time.dat")):
      refTimeFD=codecs.open(pj(example, "reference", "time.dat"), "r", encoding="utf-8")
      refTime=float(refTimeFD.read())
      refTimeFD.close()
    else:
      refTime=float("inf") # use very long time if unknown to force it to run early
    unsortedDir.append([example, refTime])
  dirs.extend(sorted(unsortedDir, key=lambda x: x[1], reverse=True))



# the labels: returns all labels defined in the labels file and append the special labels detected automatically
def getLabels(directory):
  labels=[]
  if os.path.isfile(pj(directory, "labels")):
    labels=codecs.open(pj(directory, "labels"), "r", encoding="utf-8").read().rstrip().split(' ')
  else:
    labels=['nightly'] # nightly is the default label if no labels file exist
  # check for MBSim modules in src examples
  src=os.path.isfile(pj(directory, "Makefile")) or os.path.isfile(pj(directory, "Makefile_FMI")) or os.path.isfile(pj(directory, "Makefile_FMI_cosim"))
  if src:
    makefile="Makefile" if os.path.isfile(pj(directory, "Makefile")) else \
             ("Makefile_FMI" if os.path.isfile(pj(directory, "Makefile_FMI")) else "Makefile_FMI_cosim")
    filecont=codecs.open(pj(directory, makefile), "r", encoding="utf-8").read()
    for m in mbsimModules:
      if re.search("\\b"+m+"\\b", filecont): labels.append(m)
  # check for MBSim modules in xml and flatxml examples
  else:
    for filedir, _, filenames in os.walk(directory):
      if "tmp_fmuCheck" in filedir.split('/') or "tmp_mbsimTestFMU" in filedir.split('/'): # skip temp fmu directories
        continue
      for filename in fnmatch.filter(filenames, "*.xml"):
        if filename[0:4]==".pp.": continue # skip generated .pp.* files
        filecont=codecs.open(pj(filedir, filename), "r", encoding="utf-8").read()
        for m in mbsimModules:
          if re.search('=\\s*"http://[^"]*'+m+'"', filecont, re.I): labels.append(m)
  return labels



# handle the --filter option: add/remove to directoriesSet
def addExamplesByFilter(baseDir, directoriesSet):
  if baseDir[0]!="^": # add dir
    addOrDiscard=directoriesSet.add
  else: # remove dir
    baseDir=baseDir[1:] # remove the leading "^"
    addOrDiscard=directoriesSet.discard
  # make baseDir a relative path
  baseDir=os.path.relpath(baseDir)
  for root, dirs, _ in os.walk(baseDir):
    ppxml=os.path.isfile(pj(root, "MBS.mbsimprj.xml")) 
    flatxml=os.path.isfile(pj(root, "MBS.mbsimprj.flat.xml"))
    xml=ppxml or flatxml
    src=os.path.isfile(pj(root, "Makefile")) or os.path.isfile(pj(root, "Makefile_FMI")) or os.path.isfile(pj(root, "Makefile_FMI_cosim"))
    fmi=os.path.isfile(pj(root, "FMI.mbsimprj.xml")) or os.path.isfile(pj(root, "Makefile_FMI")) or \
        os.path.isfile(pj(root, "FMI_cosim.mbsimprj.xml")) or os.path.isfile(pj(root, "Makefile_FMI_cosim"))
    # skip none examples directires
    if(not ppxml and not flatxml and not src and not fmi):
      continue
    dirs=[]

    labels=getLabels(root)
    # evaluate filter
    try:
      filterResult=eval(args.filter,
        {'ppxml': ppxml, 'flatxml': flatxml, 'xml': xml, 'src': src, 'fmi': fmi, 'labels': labels})
    except:
      print("Unable to evaluate the filter:\n"+args.filter)
      exit(1)
    if type(filterResult)!=bool:
      print("The filter does not return a bool value:\n"+args.filter)
      exit(1)
    path=os.path.normpath(root)
    if filterResult and not "tmp_mbsimTestFMU" in path.split(os.sep) and not "tmp_fmuCheck" in path.split(os.sep):
      addOrDiscard(path)



# run the given example
def runExample(resultQueue, example):
  savedDir=os.getcwd()
  try:
    resultStr=""
    os.chdir(example[0])

    runExampleRet=0 # run ok
    # execute the example[0]
    if not os.path.isdir(pj(args.reportOutDir, example[0])): os.makedirs(pj(args.reportOutDir, example[0]))
    executeFN=pj(example[0], "execute.txt")
    executeRet=0
    if not args.disableRun:
      # clean output of previous run
      if os.path.isfile("time.dat"): os.remove("time.dat")
      list(map(os.remove, glob.glob("*.h5")))

      executeFD=MultiFile(codecs.open(pj(args.reportOutDir, executeFN), "w", encoding="utf-8"), args.printToConsole)
      dt=0
      if os.path.isfile("Makefile"):
        executeRet, dt, outfiles=executeSrcExample(executeFD, example)
      elif os.path.isfile("MBS.mbsimprj.xml") or os.path.isfile("MBS.mbsimprj.alpha_py.xml"):
        executeRet, dt, outfiles=executeXMLExample(executeFD, example)
      elif os.path.isfile("MBS.mbsimprj.flat.xml"):
        executeRet, dt, outfiles=executeFlatXMLExample(executeFD, example)
      elif os.path.isfile("FMI.mbsimprj.xml") or os.path.isfile("FMI_cosim.mbsimprj.xml"):
        executeRet, dt, outfiles=executeFMIXMLExample(executeFD, example)
      elif os.path.isfile("Makefile_FMI") or os.path.isfile("Makefile_FMI_cosim"):
        executeRet, dt, outfiles=executeFMISrcExample(executeFD, example)
      else:
        print("Unknown example type in directory "+example[0]+" found.", file=executeFD)
        executeRet=1
        dt=0
        outfiles=[]
      executeFD.close()
    if executeRet!=0: runExampleRet=1
    # get reference time
    refTime=example[1]
    # print result to resultStr
    willFail=False
    if os.path.isfile("labels"):
      willFail='willfail' in codecs.open("labels", "r", encoding="utf-8").read().rstrip().split(' ')
    if not willFail:
      resultStr+='<tr>'
    else:
      resultStr+='<tr class="text-muted">'
    resultStr+='<td>'+example[0].replace('/', u'/\u200B')+'</td>'
    if not args.disableRun:
      if executeRet==subprocessCall.timedOutErrorCode:
        text='timed out'
        order=3
      elif executeRet!=0:
        text='failed'
        order=1 if willFail else 2
      else:
        text='passed'
        order=0
      if executeRet!=0:
        resultStr+='<td data-order="%d" class="'%(order)+('danger' if not willFail else 'success')+\
          '"><span class="glyphicon glyphicon-exclamation-sign alert-danger"></span>&nbsp;<a href="'+\
          urllib.request.pathname2url(executeFN)+'">'
      else:
        resultStr+='<td data-order="%d" class="'%(order)+('success' if not willFail else 'danger')+\
        '"><span class="glyphicon glyphicon-ok-sign alert-success"></span>&nbsp;<a href="'+\
          urllib.request.pathname2url(executeFN)+'">'
      resultStr+=text+'</a>'
      # add all additional output files
      if len(outfiles)>0:
         resultStr+=' <span class="dropdown"><button class="btn btn-default btn-xs" data-toggle="dropdown">more <span class="caret"></span>'+\
                    '</button><div class="dropdown-menu" style="padding-left: 0.5em; padding-right: 0.5em;">'
         for outfile in outfiles:
            resultStr+='<a href="'+urllib.request.pathname2url(pj(example[0], outfile))+'">'+os.path.splitext(outfile)[0]+'</a><br/>'
         resultStr+='</div></span>'
      resultStr+='</td>'
    if not args.disableRun:
      # if not reference time or time is nearly equal refTime => display time in black color
      if math.isinf(refTime) or abs(dt-refTime)<0.1*refTime:
        resultStr+='<td>%.3f</td>'%dt
      # dt differs more then 10% from refTime => display in yellow color
      else:
        resultStr+='<td class="%s">%.3f</td>'%("success" if dt<refTime else "warning", dt)
      if not math.isinf(refTime):
        resultStr+='<td>%.3f</td>'%refTime
      else:
        resultStr+='<td class="warning">no reference</td>'

    if args.checkGUIs:
      outfiles1=[]
      outfiles2=[]
      outfiles3=[]
      # get files to load
      ombvFiles=mainFiles(glob.glob("*.ombv.xml"), ".", ".ombv.xml")
      h5pFiles=mainFiles(glob.glob("*.mbsim.h5"), ".", ".mbsim.h5")
      guiFile=None
      if os.path.exists("MBS.mbsimprj.xml"):
        guiFile='./MBS.mbsimprj.xml'
      elif os.path.exists("FMI.mbsimprj.xml"):
        guiFile='./FMI.mbsimprj.xml'
      elif os.path.exists("FMI_cosim.mbsimprj.xml"):
        guiFile='./FMI_cosim.mbsimprj.xml'
      # run gui tests
      denv=os.environ
      denv["DISPLAY"]=":"+str(displayNR)
      denv["COIN_FULL_INDIRECT_RENDERING"]="1"
      denv["QT_X11_NO_MITSHM"]="1"
      def runGUI(files, tool):
        if len(files)==0:
          return 0, []
        # at least on Windows (wine) the DISPLAY is not found sometimes (unknown why). Hence, try this number of times before reporting an error
        tries=5 if exePrefix()==["wine"] else 1
        outFD=MultiFile(codecs.open(pj(args.reportOutDir, example[0], "gui_"+tool+".txt"), "w", encoding="utf-8"), args.printToConsole)
        comm=prefixSimulation(example, tool)+exePrefix()+[pj(mbsimBinDir, tool+args.exeExt), "--autoExit"]+files
        allTimedOut=True
        for t in range(0, tries):
          print("Starting (try %d/%d):\n"%(t+1, tries)+str(comm)+"\n\n", file=outFD)
          ret=subprocessCall(comm, outFD, env=denv, maxExecutionTime=(10 if args.prefixSimulationKeyword=='VALGRIND' else 5))
          print("\n\nReturned with "+str(ret), file=outFD)
          if ret!=subprocessCall.timedOutErrorCode: allTimedOut=False
          if ret==0: break
          if t+1<tries: time.sleep(60) # wait some time, a direct next test will likely also fail (see above)
        if ret!=0 and not allTimedOut: ret=1 # if at least one example failed return with error (not with subprocessCall.timedOutErrorCode)
        if exePrefix()!=["wine"] and allTimedOut: ret=1 # on none wine treat allTimedOut as error (allTimedOut is just a hack for Windows)
        retl=[ret]; outfiles=getOutFilesAndAdaptRet(example, retl); ret=retl[0]
        outFD.close()
        return ret, outfiles
      ombvRet, outfiles1=runGUI(ombvFiles, "openmbv")
      h5pRet,  outfiles2=runGUI(h5pFiles, "h5plotserie")
      guiRet,  outfiles3=runGUI([guiFile] if guiFile else [], "mbsimgui")
      outfiles=outfiles1+outfiles2+outfiles3
      # result
      resultStr+='<td data-order="%d%d%d%d">'%(0 if abs(ombvRet)+abs(h5pRet)+abs(guiRet)==0 else (1 if willFail else 2),
                                               int(len(ombvFiles)>0), int(len(h5pFiles)>0), int(guiFile!=None))+\
        '<a href="%s" style="visibility:%s;" class="label bg-%s">'%(urllib.request.pathname2url(pj(example[0], "gui_openmbv.txt")),
          "visible" if len(ombvFiles)>0 else "hidden", "success" if ombvRet==0 else ("danger" if not willFail else "warning"))+\
          '<img src="/mbsim/html/openmbv.svg" alt="ombv"/></a>'+\
        '<a href="%s" style="visibility:%s;" class="label bg-%s">'%(urllib.request.pathname2url(pj(example[0], "gui_h5plotserie.txt")),
          "visible" if len(h5pFiles)>0 else "hidden", "success" if h5pRet==0 else ("danger" if not willFail else "warning"))+\
          '<img src="/mbsim/html/h5plotserie.svg" alt="h5p"/></a>'+\
        '<a href="%s" style="visibility:%s;" class="label bg-%s">'%(urllib.request.pathname2url(pj(example[0], "gui_mbsimgui.txt")),
          "visible" if guiFile!=None else "hidden", "success" if guiRet==0 else ("danger" if not willFail else "warning"))+\
          '<img src="/mbsim/html/mbsimgui.svg" alt="gui"/></a>'
      # add all additional output files
      if len(outfiles)>0:
         resultStr+=' <span class="dropdown"><button class="btn btn-default btn-xs" data-toggle="dropdown">more <span class="caret"></span>'+\
                    '</button><div class="dropdown-menu" style="padding-left: 0.5em; padding-right: 0.5em;">'
         for outfile in outfiles:
            resultStr+='<a href="'+urllib.request.pathname2url(pj(example[0], outfile))+'">'+os.path.splitext(outfile)[0]+'</a><br/>'
         resultStr+='</div></span>'
      resultStr+='</td>'
      # treat timedOutErrorCode not as error: note that on Linux timedOutErrorCode can neven happen here, see above
      if (ombvRet!=0 and ombvRet!=subprocessCall.timedOutErrorCode) or \
         (h5pRet!=0 and h5pRet!=subprocessCall.timedOutErrorCode) or \
         (guiRet!=0 and guiRet!=subprocessCall.timedOutErrorCode):
        runExampleRet=1

    compareRet=-1
    compareFN=pj(example[0], "compare.html")
    if not args.disableCompare and canCompare:
      if os.path.isdir("reference"):
        # compare the result with the reference
        compareRet, nrFailed, nrAll=compareExample(example[0], compareFN)
        if compareRet!=0: runExampleRet=1
      else:
        compareRet=-2

    # write time to time.dat for possible later copying it to the reference
    if not args.disableRun:
      refTimeFD=codecs.open("time.dat", "w", encoding="utf-8")
      print('%.3f'%dt, file=refTimeFD)
      refTimeFD.close()
    # print result to resultStr
    if not args.disableCompare:
      if compareRet==-1:
        resultStr+='<td data-order="2" class="warning"><div class="pull-left"><span class="glyphicon glyphicon-warning-sign alert-warning"></span>&nbsp;not run</div>'+\
                   '<div class="pull-right">[<input type="checkbox" disabled="disabled"/>]</div></td>'
      elif compareRet==-2:
        resultStr+='<td data-order="1" class="warning"><div class="pull-left"><span class="glyphicon glyphicon-warning-sign alert-warning"></span>&nbsp;no reference</div>'+\
                   '<div class="pull-right">[<input class="_EXAMPLE'+\
                   '" type="checkbox" name="'+example[0]+'" disabled="disabled"/>]</div></td>'
        nrAll=0
        nrFailed=0
      else:
        if nrFailed==0:
          resultStr+='<td data-order="0" class="success"><div class="pull-left"><span class="glyphicon glyphicon-ok-sign alert-success"></span>&nbsp;<a href="'+urllib.request.pathname2url(compareFN)+\
                     '">passed <span class="badge">'+str(nrAll)+'</span></a></div>'+\
                     '<div class="pull-right">[<input type="checkbox" disabled="disabled"/>]</div></td>'
        else:
          resultStr+='<td data-order="3" class="danger"><div class="pull-left"><span class="glyphicon glyphicon-exclamation-sign alert-danger"></span>&nbsp;<a href="'+urllib.request.pathname2url(compareFN)+\
                     '">failed <span class="badge">'+str(nrFailed)+'</span> of <span class="badge">'+str(nrAll)+\
                     '</span></a></div><div class="pull-right">[<input class="_EXAMPLE'+\
                     '" type="checkbox" name="'+example[0]+'" disabled="disabled"/>]</div></td>'

    # check for deprecated features
    if not args.disableRun and args.buildSystemRun and args.webapp:
      resultStr+=webapp(example[0])

    # check for deprecated features
    if not args.disableRun:
      nrDeprecated=0
      for line in fileinput.FileInput(pj(args.reportOutDir, executeFN)):
        match=re.search("Deprecated feature called:", line)
        if match!=None:
          nrDeprecated=nrDeprecated+1
      if nrDeprecated==0:
        resultStr+='<td data-order="0" class="success"><span class="glyphicon glyphicon-ok-sign alert-success"></span>&nbsp;none</td>'
      else:
        resultStr+='<td data-order="1" class="warning"><span class="glyphicon glyphicon-warning-sign alert-warning"></span>&nbsp;<a href="'+urllib.request.pathname2url(executeFN)+'">'+str(nrDeprecated)+' found</a></td>'

    # validate XML
    if not args.disableValidate:
      htmlOutputFN=pj(example[0], "validateXML.html")
      htmlOutputFD=codecs.open(pj(args.reportOutDir, htmlOutputFN), "w", encoding="utf-8")
      # write header
      print('<!DOCTYPE html>', file=htmlOutputFD)
      print('<html lang="en">', file=htmlOutputFD)
      print('<head>', file=htmlOutputFD)
      print('  <META http-equiv="Content-Type" content="text/html; charset=UTF-8">', file=htmlOutputFD)
      print('  <meta name="viewport" content="width=device-width, initial-scale=1.0" />', file=htmlOutputFD)
      print('  <title>Validate XML Files: %s</title>'%(args.buildType), file=htmlOutputFD)
      print('  <link rel="stylesheet" type="text/css" href="https://cdn.datatables.net/s/bs-3.3.5/jq-2.1.4,dt-1.10.10/datatables.min.css"/>', file=htmlOutputFD)
      print('  <link rel="shortcut icon" href="/mbsim/html/mbsimenv.ico" type="image/x-icon"/>', file=htmlOutputFD)
      print('  <link rel="icon" href="/mbsim/html/mbsimenv.ico" type="image/x-icon"/>', file=htmlOutputFD)
      print('</head>', file=htmlOutputFD)
      print('<body style="margin:0.5em">', file=htmlOutputFD)
      print('<script src="https://cdn.datatables.net/s/bs-3.3.5/jq-2.1.4,dt-1.10.10/datatables.min.js"> </script>', file=htmlOutputFD)
      print('<script src="https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.22.2/moment.min.js"> </script>', file=htmlOutputFD)
      print('<script src="https://cdnjs.cloudflare.com/ajax/libs/moment-timezone/0.5.23/moment-timezone-with-data-2012-2022.min.js"> </script>', file=htmlOutputFD)
      print('<script src="/mbsim/html/cookiewarning.js"> </script>', file=htmlOutputFD)
      print('<script>', file=htmlOutputFD)
      print('  $(document).ready(function() {', file=htmlOutputFD)
      print("    $('.DATETIME').each(function() {", file=htmlOutputFD)
      print('      $(this).text(moment($(this).text()).tz(moment.tz.guess()).format("ddd, YYYY-MM-DD - HH:mm:ss z"));', file=htmlOutputFD)
      print('    }); ', file=htmlOutputFD)
      print("    $('#SortThisTable').dataTable({'lengthMenu': [ [10, 25, 50, 100, -1], [10, 25, 50, 100, 'All'] ], 'pageLength': 25, 'aaSorting': [], stateSave: true});", file=htmlOutputFD)
      print('  } );', file=htmlOutputFD)
      print('</script>', file=htmlOutputFD)
      print('<h1>Validate XML Files: <small>%s</small></h1>'%(args.buildType), file=htmlOutputFD)
      print('<dl class="dl-horizontal">', file=htmlOutputFD)
      print('<dt>Example:</dt><dd>'+example[0].replace('/', u'/\u200B')+'</dd>', file=htmlOutputFD)
      print('<dt>Time ID:</dt><dd class="DATETIME">'+(timeID.isoformat()+"Z")+'</dd>', file=htmlOutputFD)
      currentID=int(os.path.basename(args.reportOutDir)[len("result_"):])
      parDirs="/".join(list(map(lambda x: "..", range(0, example[0].count(os.sep)+1))))
      navA=""
      navB=""
      if args.currentID!=0:
        navA="/../.."
        navB="/runexamples_report/result_current"
      print('<dt>Navigate:</dt><dd><a class="btn btn-info btn-xs" href="%s/..%s/result_%010d%s/%s"><span class="glyphicon glyphicon-step-backward"> </span> previous</a>'%
        (parDirs, navA, currentID-1, navB, urllib.request.pathname2url(htmlOutputFN)), file=htmlOutputFD)
      print('                 <a class="btn btn-info btn-xs" href="%s/..%s/result_%010d%s/%s"><span class="glyphicon glyphicon-step-forward"> </span> next</a>'%
        (parDirs, navA, currentID+1, navB, urllib.request.pathname2url(htmlOutputFN)), file=htmlOutputFD)
      print('                 <a class="btn btn-info btn-xs" href="%s/..%s/result_current%s/%s"><span class="glyphicon glyphicon-fast-forward"> </span> newest</a>'%
        (parDirs, navA, navB, urllib.request.pathname2url(htmlOutputFN)), file=htmlOutputFD)
      print('                 <a class="btn btn-info btn-xs" href="%s%s%s/index.html"><span class="glyphicon glyphicon-eject"> </span> parent</a></dd>'%
        (parDirs, navA, navB), file=htmlOutputFD)
      print('</dl>', file=htmlOutputFD)
      print('<hr/><table id="SortThisTable" class="table table-striped table-hover table-bordered table-condensed">', file=htmlOutputFD)
      print('<thead><tr><th><span class="glyphicon glyphicon-folder-open"></span>&nbsp;XML File</th>'+
            '<th><span class="glyphicon glyphicon-search"></span>&nbsp;Result</th></tr></thead><tbody>', file=htmlOutputFD)

      failed, total=validateXML(example, False, htmlOutputFD)
      if failed==0:
        resultStr+='<td data-order="0" class="success"><span class="glyphicon glyphicon-ok-sign alert-success"></span>&nbsp;<a href="'+urllib.request.pathname2url(htmlOutputFN)+'">valid <span class="badge">'+\
                   str(total)+'</span></a></td>'
      else:
        resultStr+='<td data-order="1" class="danger"><span class="glyphicon glyphicon-exclamation-sign alert-danger"></span>&nbsp;<a href="'+urllib.request.pathname2url(htmlOutputFN)+'">'+\
                   'failed <span class="badge">'+str(failed)+'</span> of <span class="badge">'+str(total)+'</span></a></td>'
        runExampleRet=1
      # write footer
      print('</tbody></table>', file=htmlOutputFD)
      print('<hr/>',  file=htmlOutputFD)
      print('<span class="pull-left small">',  file=htmlOutputFD)
      print('  <a href="/mbsim/html/impressum_disclaimer_datenschutz.html#impressum">Impressum</a> /',  file=htmlOutputFD)
      print('  <a href="/mbsim/html/impressum_disclaimer_datenschutz.html#disclaimer">Disclaimer</a> /',  file=htmlOutputFD)
      print('  <a href="/mbsim/html/impressum_disclaimer_datenschutz.html#datenschutz">Datenschutz</a>',  file=htmlOutputFD)
      print('</span>',  file=htmlOutputFD)
      print('<span class="pull-right small">',  file=htmlOutputFD)
      print('  Generated on <span class="DATETIME">%s</span> by runexamples.py'%(timeID.isoformat()+"Z"), file=htmlOutputFD)
      print('  <a href="/">Home</a>',  file=htmlOutputFD)
      print('</span>',  file=htmlOutputFD)
      print('</body>', file=htmlOutputFD)
      print('</html>', file=htmlOutputFD)

      htmlOutputFD.close()

    resultStr+='</tr>'

  except:
    fatalScriptErrorFN=pj(example[0], "fatalScriptError.txt")
    fatalScriptErrorFD=MultiFile(codecs.open(pj(args.reportOutDir, fatalScriptErrorFN), "w", encoding="utf-8"), args.printToConsole)
    print("Fatal Script Errors should not happen. So this is a bug in runexamples.py which should be fixed.", file=fatalScriptErrorFD)
    print("", file=fatalScriptErrorFD)
    print(traceback.format_exc(), file=fatalScriptErrorFD)
    fatalScriptErrorFD.close()
    resultStr='<tr><td>'+example[0].replace('/', u'/\u200B')+'</td><td class="danger"><a href="'+urllib.request.pathname2url(fatalScriptErrorFN)+'">fatal script error</a></td>%s</tr>' \
      %('<td>-</td>'*(7-sum([args.disableRun, args.disableRun, args.disableRun, not args.checkGUIs, args.disableCompare,
      args.disableRun or (not args.buildSystemRun) or not args.webapp, args.disableRun, args.disableValidate])))
    runExampleRet=1
  finally:
    os.chdir(savedDir)
    resultQueue.put([example, resultStr, runExampleRet])
    return runExampleRet



def mainFiles(fl, example, suffix):
  ret=fl
  for f in fl:
    ret=list(filter(lambda r: not (r.startswith(f[0:-len(suffix)]+'.') and len(r)>len(f)), ret))
  ret=list(map(lambda x: example+'/'+x, ret))
  ret.sort(key=lambda a: os.path.basename(a))
  return ret
def webapp(example):
  ombv={}
  fl=glob.glob("*.ombv.xml")
  if len(fl)>0:
    ombv['buildType']=args.buildType
    ombv['prog']='openmbv'
    ombv['file']=mainFiles(fl, example, ".ombv.xml")
  h5p={}
  for prefix in ['', 'reference/']:
    fl=glob.glob(prefix+"*.mbsim.h5")
    if len(fl)>0:
      h5p['buildType']=args.buildType
      h5p['prog']='h5plotserie'
      if 'file' not in h5p: h5p['file']=[]
      h5p['file'].extend(mainFiles(fl, example, ".mbsim.h5"))
  gui={}
  if os.path.exists("MBS.mbsimprj.xml") or os.path.exists("FMI.mbsimprj.xml") or os.path.exists("FMI_cosim.mbsimprj.xml"):
    gui={'buildType': args.buildType, 'prog': 'mbsimgui'}
    if os.path.exists("MBS.mbsimprj.xml"):
      gui['file']=[example+'/MBS.mbsimprj.xml']
    elif os.path.exists("FMI.mbsimprj.xml"):
      gui['file']=[example+'/FMI.mbsimprj.xml']
    else:
      gui['file']=[example+'/FMI_cosim.mbsimprj.xml']
  return '<td data-order="%03d%03d%03d">'%(len(ombv), len(h5p), len(gui))+\
      ('<button disabled="disabled" type="button" onclick="location.href=\'/mbsim/html/noVNC/mbsimwebapp.html?'+\
       urllib.parse.urlencode(ombv, doseq=True)+'\';" class="_WEBAPP btn btn-default btn-xs" style="visibility:'+\
       ('visible' if len(ombv)>0 else 'hidden')+';">'+\
       '<img src="/mbsim/html/openmbv.svg" alt="ombv"/></button>&nbsp;')+\
      ('<button disabled="disabled" type="button" onclick="location.href=\'/mbsim/html/noVNC/mbsimwebapp.html?'+\
       urllib.parse.urlencode(h5p, doseq=True)+'\';" class="_WEBAPP btn btn-default btn-xs" style="visibility:'+\
       ('visible' if len(h5p)>0 else 'hidden')+';">'+\
       '<img src="/mbsim/html/h5plotserie.svg" alt="h5p"/></button>&nbsp;')+\
      ('<button disabled="disabled" type="button" onclick="location.href=\'/mbsim/html/noVNC/mbsimwebapp.html?'+\
       urllib.parse.urlencode(gui, doseq=True)+'\';" class="_WEBAPP btn btn-default btn-xs" style="visibility:'+\
       ('visible' if len(gui)>0 else 'hidden')+';">'+\
       '<img src="/mbsim/html/mbsimgui.svg" alt="gui"/></button>&nbsp;')+\
    '</td>'

# if args.exeEXt is set we must prefix every command with wine
def exePrefix():
  if args.exeExt=="":
    return []
  else:
    return ["wine"]


# prefix the simultion with this parameter.
# this is normaly just args.prefixSimulation but may be extended by keywords of args.prefixSimulationKeyword.
def prefixSimulation(example, id):
  # handle VALGRIND
  if args.prefixSimulationKeyword=='VALGRIND':
    return args.prefixSimulation+['--xml=yes', '--xml-file=valgrind.%%p.%s.xml'%(id)]
  return args.prefixSimulation

# get additional output files of simulations.
# these are all dependent on the keyword of args.prefixSimulationKeyword.
# additional output files must be placed in the args.reportOutDir and here only the basename must be returned.
def getOutFilesAndAdaptRet(example, ret):
  # handle VALGRIND
  if args.prefixSimulationKeyword=='VALGRIND':
    # get out files
    # and adapt the return value if errors in valgrind outputs are detected
    xmlFiles=glob.glob("valgrind.*.xml")
    outFiles=[]
    i=0
    for xmlFile in xmlFiles:
      # check for errors
      content=codecs.open(xmlFile, encoding="utf-8").read()
      if "</valgrindoutput>" not in content: # incomplete valgrind output -> a skipped trace children
        os.remove(xmlFile)
        continue
      if "<error>" in content and ret[0]!=subprocessCall.timedOutErrorCode:
        ret[0]=1
      # transform xml file to html file (in reportOutDir)
      htmlFile=xmlFile[:-4]+".html"
      i=i+1
      htmlFile=re.sub('^valgrind\.[0-9]+\.', 'valgrind.%d.'%(i), htmlFile) # use small numbers for pid
      outFiles.append(htmlFile)
      # xalan has bugs regarding entity resolver, which lead to unquoted <, > in valgrind suppression output
      # hence, we use xsltproc here
      subprocess.check_call(['xsltproc', '--output', pj(args.reportOutDir, example[0], htmlFile),
                            pj(scriptDir, 'valgrindXMLToHTML.xsl'), xmlFile])
      os.remove(xmlFile)
    return outFiles
  return []


# execute the source code example in the current directory (write everything to fd executeFD)
def executeSrcExample(executeFD, example):
  print("Running commands:", file=executeFD)
  print("make clean && make && "+pj(os.curdir, "main"), file=executeFD)
  print("", file=executeFD)
  executeFD.flush()
  if not args.disableMakeClean:
    if subprocessCall(["make", "clean"], executeFD)!=0: return 1, 0, []
  if subprocessCall(["make"], executeFD)!=0: return 1, 0, []
  # append $prefix/lib to LD_LIBRARY_PATH/PATH to find lib by main of the example
  if os.name=="posix":
    NAME="LD_LIBRARY_PATH"
    SUBDIR="lib"
  elif os.name=="nt":
    NAME="PATH"
    SUBDIR="bin"
  mainEnv=os.environ.copy()
  libDir=pj(mbsimBinDir, os.pardir, SUBDIR)
  if NAME in mainEnv:
    mainEnv[NAME]=mainEnv[NAME]+os.pathsep+libDir
  else:
    mainEnv[NAME]=libDir
  # run main
  t0=datetime.datetime.now()
  ret=[abs(subprocessCall(prefixSimulation(example, 'source')+exePrefix()+[pj(os.curdir, "main"+args.exeExt)], executeFD,
                          env=mainEnv, maxExecutionTime=args.maxExecutionTime))]
  t1=datetime.datetime.now()
  dt=(t1-t0).total_seconds()
  outFiles=getOutFilesAndAdaptRet(example, ret)

  return ret[0], dt, outFiles



# execute the XML example in the current directory (write everything to fd executeFD)
def executeXMLExample(executeFD, example):
  # we handle MBS.mbsimprj.xml, MBS.mbsimprj.alpha_py.xml and FMI.mbsimprj.xml files here
  if   os.path.isfile("MBS.mbsimprj.xml"):          prjFile="MBS.mbsimprj.xml"
  elif os.path.isfile("MBS.mbsimprj.alpha_py.xml"): prjFile="MBS.mbsimprj.alpha_py.xml"
  elif os.path.isfile("FMI.mbsimprj.xml"):          prjFile="FMI.mbsimprj.xml"
  elif os.path.isfile("FMI_cosim.mbsimprj.xml"):    prjFile="FMI_cosim.mbsimprj.xml"
  else: raise RuntimeError("Internal error: Unknown ppxml file.")

  print("Running command:", file=executeFD)
  list(map(lambda x: print(x, end=" ", file=executeFD), [pj(mbsimBinDir, "mbsimxml")]+[prjFile]))
  print("\n", file=executeFD)
  executeFD.flush()
  t0=datetime.datetime.now()
  ret=[abs(subprocessCall(prefixSimulation(example, 'mbsimxml')+exePrefix()+[pj(mbsimBinDir, "mbsimxml"+args.exeExt)]+
                          [prjFile], executeFD, maxExecutionTime=args.maxExecutionTime))]
  t1=datetime.datetime.now()
  dt=(t1-t0).total_seconds()
  outFiles=getOutFilesAndAdaptRet(example, ret)

  return ret[0], dt, outFiles



# execute the flat XML example in the current directory (write everything to fd executeFD)
def executeFlatXMLExample(executeFD, example):
  print("Running command:", file=executeFD)
  list(map(lambda x: print(x, end=" ", file=executeFD), [pj(mbsimBinDir, "mbsimflatxml"), "MBS.mbsimprj.flat.xml"]))
  print("\n", file=executeFD)
  executeFD.flush()
  t0=datetime.datetime.now()
  ret=[abs(subprocessCall(prefixSimulation(example, 'mbsimflatxml')+exePrefix()+[pj(mbsimBinDir, "mbsimflatxml"+args.exeExt), "MBS.mbsimprj.flat.xml"],
                          executeFD, maxExecutionTime=args.maxExecutionTime))]
  t1=datetime.datetime.now()
  dt=(t1-t0).total_seconds()
  outFiles=getOutFilesAndAdaptRet(example, ret)

  return ret[0], dt, outFiles



# helper function for executeFMIXMLExample and executeFMISrcExample
def executeFMIExample(executeFD, example, fmiInputFile, cosim):
  ### create the FMU
  # run mbsimCreateFMU to export the model as a FMU
  # use option --nocompress, just to speed up mbsimCreateFMU
  print("\n\n\n", file=executeFD)
  print("Running command:", file=executeFD)
  labels=getLabels(os.getcwd())
  cosimArg=[]
  if cosim: cosimArg=['--cosim']
  noparamArg=[]
  if "noparam" in labels: noparamArg=['--noparam']
  comm=exePrefix()+[pj(mbsimBinDir, "mbsimCreateFMU"+args.exeExt), '--nocompress']+cosimArg+noparamArg+[fmiInputFile]
  list(map(lambda x: print(x, end=" ", file=executeFD), comm))
  print("\n", file=executeFD)
  executeFD.flush()
  ret1=[abs(subprocessCall(prefixSimulation(example, 'mbsimCreateFMU')+comm, executeFD, maxExecutionTime=args.maxExecutionTime/2))]
  outFiles1=getOutFilesAndAdaptRet(example, ret1)

  ### run using fmuChecker
  # get fmuChecker executable
  fmuCheck=glob.glob(pj(mbsimBinDir, "fmuCheck.*"))
  if len(fmuCheck)!=1:
    raise RuntimeError("None or more than one fmuCheck.* executlabe found.")
  fmuCheck=fmuCheck[0]
  # run fmuChecker
  print("\n\n\n", file=executeFD)
  print("Running command:", file=executeFD)
  # adapt end time if MBSIM_SET_MINIMAL_TEND is set
  endTime=[]
  if 'MBSIM_SET_MINIMAL_TEND' in os.environ:
    endTime=['-s', '0.01']
  if os.path.isdir("tmp_fmuCheck"): shutil.rmtree("tmp_fmuCheck")
  os.mkdir("tmp_fmuCheck")
  comm=exePrefix()+[pj(mbsimBinDir, fmuCheck)]+endTime+["-f", "-l", "5", "-o", "fmuCheck.result.csv", "-z", "tmp_fmuCheck", "mbsim.fmu"]
  list(map(lambda x: print(x, end=" ", file=executeFD), comm))
  print("\n", file=executeFD)
  t0=datetime.datetime.now()
  ret2=[abs(subprocessCall(prefixSimulation(example, 'fmuCheck')+comm, executeFD, maxExecutionTime=args.maxExecutionTime))]
  t1=datetime.datetime.now()
  dt=(t1-t0).total_seconds()
  outFiles2=getOutFilesAndAdaptRet(example, ret2)
  # convert fmuCheck result csv file to h5 format (this is then checked as usual by compareExample)
  if canCompare:
    try:
      print("Convert fmuCheck csv file to h5:\n", file=executeFD)
      import h5py
      import numpy
      data=numpy.genfromtxt("fmuCheck.result.csv", dtype=float, delimiter=",", skip_header=1) # get data from csv
      header=open("fmuCheck.result.csv", "r").readline().rstrip().split(',') # get header from csv
      header=list(map(lambda x: x[1:-1], header)) # remove leading/trailing " form each header
      f=h5py.File("fmuCheck.result.h5", "w") # create h5 file
      d=f.create_dataset("fmuCheckResult", dtype='d', data=data) # create dataset with data
      d.attrs.create("Column Label", dtype=h5py.special_dtype(vlen=bytes), data=header) # create Column Label attr with header
      f.close() # close h5 file
    except:
      print(traceback.format_exc(), file=executeFD)
      print("Failed.\n", file=executeFD)
    else:
      print("Done.\n", file=executeFD)
  # remove unpacked fmu
  if os.path.isdir("tmp_fmuCheck"): shutil.rmtree("tmp_fmuCheck")

  ### run using mbsimTestFMU
  # unpack FMU
  if os.path.isdir("tmp_mbsimTestFMU"): shutil.rmtree("tmp_mbsimTestFMU")
  try:
    print("Unzip mbsim.fmu for mbsimTestFMU:\n", file=executeFD)
    zipfile.ZipFile("mbsim.fmu").extractall("tmp_mbsimTestFMU")
  except:
    print(traceback.format_exc(), file=executeFD)
    print("Failed.\n", file=executeFD)
  else:
    print("Done.\n", file=executeFD)
  # run mbsimTestFMU
  print("\n\n\n", file=executeFD)
  print("Running command:", file=executeFD)
  cosimArg=['--me']
  if cosim: cosimArg=['--cosim']
  comm=exePrefix()+[pj(mbsimBinDir, "mbsimTestFMU"+args.exeExt)]+cosimArg+["tmp_mbsimTestFMU"]
  list(map(lambda x: print(x, end=" ", file=executeFD), comm))
  print("\n", file=executeFD)
  ret3=[abs(subprocessCall(prefixSimulation(example, 'mbsimTestFMU')+comm, executeFD, maxExecutionTime=args.maxExecutionTime/3))]
  outFiles3=getOutFilesAndAdaptRet(example, ret3)
  # remove unpacked fmu
  if os.path.isdir("tmp_mbsimTestFMU"): shutil.rmtree("tmp_mbsimTestFMU")

  # return
  if ret1[0]==subprocessCall.timedOutErrorCode or ret2[0]==subprocessCall.timedOutErrorCode or ret3[0]==subprocessCall.timedOutErrorCode:
    ret=subprocessCall.timedOutErrorCode
  else:
    ret=abs(ret1[0])+abs(ret2[0])+abs(ret3[0])
  outFiles=[]
  outFiles.extend(outFiles1)
  outFiles.extend(outFiles2)
  outFiles.extend(outFiles3)

  return ret, dt, outFiles

# execute the FMI XML export example in the current directory (write everything to fd executeFD)
def executeFMIXMLExample(executeFD, example):
  # first simple run the example as a preprocessing xml example
  ret1, dt, outFiles1=executeXMLExample(executeFD, example)
  # create and run FMU
  basename="FMI.mbsimprj.xml" if os.path.isfile("FMI.mbsimprj.xml") else "FMI_cosim.mbsimprj.xml"
  cosim=False if os.path.isfile("FMI.mbsimprj.xml") else True
  ret2, dt, outFiles2=executeFMIExample(executeFD, example, basename, cosim)
  # return
  if ret1==subprocessCall.timedOutErrorCode or ret2==subprocessCall.timedOutErrorCode:
    ret=subprocessCall.timedOutErrorCode
  else:
    ret=abs(ret1)+abs(ret2)
  outFiles=[]
  outFiles.extend(outFiles1)
  outFiles.extend(outFiles2)
  return ret, dt, outFiles

# execute the FMI source export example in the current directory (write everything to fd executeFD)
def executeFMISrcExample(executeFD, example):
  basename="Makefile_FMI" if os.path.isfile("Makefile_FMI") else "Makefile_FMI_cosim"
  cosim=False if os.path.isfile("Makefile_FMI") else True
  # compile examples
  print("Running commands:", file=executeFD)
  print("make -f "+basename+" clean && make -f "+basename, file=executeFD)
  print("", file=executeFD)
  executeFD.flush()
  if not args.disableMakeClean:
    if subprocessCall(["make", "-f", basename, "clean"], executeFD)!=0: return 1, 0, []
  if subprocessCall(["make", "-f", basename], executeFD)!=0: return 1, 0, []
  # create and run FMU
  if args.exeExt==".exe":
    dllExt=".dll"
  else:
    dllExt=".so"
  return executeFMIExample(executeFD, example, "mbsimfmi_model"+dllExt, cosim)



def createDiffPlot(diffHTMLFileName, example, filename, datasetName, column, label, dataArrayRef, dataArrayCur, gnuplotProcess):
  import numpy

  diffDir=os.path.dirname(diffHTMLFileName)
  if not os.path.isdir(diffDir): os.makedirs(diffDir)

  # create html page
  diffHTMLPlotFD=codecs.open(diffHTMLFileName, "w", encoding="utf-8")
  print('<!DOCTYPE html>', file=diffHTMLPlotFD)
  print('<html lang="en">', file=diffHTMLPlotFD)
  print('<head>', file=diffHTMLPlotFD)
  print('  <META http-equiv="Content-Type" content="text/html; charset=UTF-8">', file=diffHTMLPlotFD)
  print('  <meta name="viewport" content="width=device-width, initial-scale=1.0" />', file=diffHTMLPlotFD)
  print('  <title>Difference Plot: %s</title>'%(args.buildType), file=diffHTMLPlotFD)
  print('  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css"/>', file=diffHTMLPlotFD)
  print('  <link rel="shortcut icon" href="/mbsim/html/mbsimenv.ico" type="image/x-icon"/>', file=diffHTMLPlotFD)
  print('  <link rel="icon" href="/mbsim/html/mbsimenv.ico" type="image/x-icon"/>', file=diffHTMLPlotFD)
  print('</head>', file=diffHTMLPlotFD)
  print('<body style="margin:0.5em">', file=diffHTMLPlotFD)
  print('<script src="https://code.jquery.com/jquery-2.1.4.min.js"> </script>', file=diffHTMLPlotFD)
  print('<script src="https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.22.2/moment.min.js"> </script>', file=diffHTMLPlotFD)
  print('<script src="https://cdnjs.cloudflare.com/ajax/libs/moment-timezone/0.5.23/moment-timezone-with-data-2012-2022.min.js"> </script>', file=diffHTMLPlotFD)
  print('<script src="/mbsim/html/cookiewarning.js"> </script>', file=diffHTMLPlotFD)
  print('<script>', file=diffHTMLPlotFD)
  print('$(document).ready(function() {', file=diffHTMLPlotFD)
  print("  $('.DATETIME').each(function() {", file=diffHTMLPlotFD)
  print('    $(this).text(moment($(this).text()).tz(moment.tz.guess()).format("ddd, YYYY-MM-DD - HH:mm:ss z"));', file=diffHTMLPlotFD)
  print('  }); ', file=diffHTMLPlotFD)
  print('});', file=diffHTMLPlotFD)
  print('</script>', file=diffHTMLPlotFD)
  print('<h1>Difference Plot: <small>%s</small></h1>'%(args.buildType), file=diffHTMLPlotFD)
  print('<dl class="dl-horizontal">', file=diffHTMLPlotFD)
  print('<dt>Example:</dt><dd>'+example.replace('/', u'/\u200B')+'</dd>', file=diffHTMLPlotFD)
  print('<dt>File:</dt><dd>'+filename+'</dd>', file=diffHTMLPlotFD)
  print('<dt>Dataset:</dt><dd>'+datasetName+'</dd>', file=diffHTMLPlotFD)
  print('<dt>Label:</dt><dd>'+label+' (column %d)</dd>'%(column), file=diffHTMLPlotFD)
  print('<dt>Time ID:</dt><dd class="DATETIME">'+timeID.isoformat()+'Z</dd>', file=diffHTMLPlotFD)
  currentID=int(os.path.basename(args.reportOutDir)[len("result_"):])
  parDirs="/".join(list(map(lambda x: "..", range(0, pj(example, filename, datasetName, str(column)).count(os.sep)+1))))
  navA=""
  navB=""
  if args.currentID!=0:
    navA="/../.."
    navB="/runexamples_report/result_current"
  print('<dt>Navigate:</dt><dd><a class="btn btn-info btn-xs" href="%s/..%s/result_%010d%s/%s"><span class="glyphicon glyphicon-step-backward"> </span> previous</a>'%
    (parDirs, navA, currentID-1, navB, example.replace('/', u'/\u200B')+u"/\u200B"+filename+u"/\u200B"+datasetName+u"/\u200B"+str(column)+u"/\u200Bdiffplot.html"), file=diffHTMLPlotFD)
  print('                 <a class="btn btn-info btn-xs" href="%s/..%s/result_%010d%s/%s"><span class="glyphicon glyphicon-step-forward"> </span> next</a>'%
    (parDirs, navA, currentID+1, navB, example.replace('/', u'/\u200B')+u"/\u200B"+filename+u"/\u200B"+datasetName+u"/\u200B"+str(column)+u"/\u200Bdiffplot.html"), file=diffHTMLPlotFD)
  print('                 <a class="btn btn-info btn-xs" href="%s/..%s/result_current%s/%s"><span class="glyphicon glyphicon-fast-forward"> </span> newest</a>'%
    (parDirs, navA, navB, example.replace('/', u'/\u200B')+u"/\u200B"+filename+u"/\u200B"+datasetName+u"/\u200B"+str(column)+u"/\u200Bdiffplot.html"), file=diffHTMLPlotFD)
  print('                 <a class="btn btn-info btn-xs" href="%s/%s%s%s/compare.html"><span class="glyphicon glyphicon-eject"> </span> parent</a></dd>'%
    (parDirs, urllib.request.pathname2url(example), navA, navB), file=diffHTMLPlotFD)
  print('</dl>', file=diffHTMLPlotFD)
  print('<p><span class="glyphicon glyphicon-info-sign"> </span> A result differs if <b>at least at one time point</b> the absolute tolerance <b>and</b> the relative tolerance is larger then the requested.</p>', file=diffHTMLPlotFD)
  if dataArrayRef.shape[0]!=dataArrayCur.shape[0]:
    print('''<div class="alert alert-danger" role="alert">
  <span class="glyphicon glyphicon-exclamation-sign" aria-hidden="true"></span>
  Different number of data points: ref = %d, cur = %d
</div>'''%(dataArrayRef.shape[0], dataArrayCur.shape[0]), file=diffHTMLPlotFD)
  print('<p><object data="plot.svg" type="image/svg+xml"> </object></p>', file=diffHTMLPlotFD)
  print('<hr/>', file=diffHTMLPlotFD)
  print('<span class="pull-left small">', file=diffHTMLPlotFD)
  print('  <a href="/mbsim/html/impressum_disclaimer_datenschutz.html#impressum">Impressum</a> /', file=diffHTMLPlotFD)
  print('  <a href="/mbsim/html/impressum_disclaimer_datenschutz.html#disclaimer">Disclaimer</a> /', file=diffHTMLPlotFD)
  print('  <a href="/mbsim/html/impressum_disclaimer_datenschutz.html#datenschutz">Datenschutz</a>', file=diffHTMLPlotFD)
  print('</span>', file=diffHTMLPlotFD)
  print('<span class="pull-right small">', file=diffHTMLPlotFD)
  print('  Generated on <span class="DATETIME">%s</span> by runexamples.py'%(timeID.isoformat()+"Z"), file=diffHTMLPlotFD)
  print('  <a href="/">Home</a>', file=diffHTMLPlotFD)
  print('</span>', file=diffHTMLPlotFD)
  print('</body>', file=diffHTMLPlotFD)
  print('</html>', file=diffHTMLPlotFD)
  diffHTMLPlotFD.close()

  if gnuplotProcess==None:
    return

  # fix if all values are nan to prevent a gnuplot warning
  if numpy.all(numpy.isnan(dataArrayRef[:,0])) or numpy.all(numpy.isnan(dataArrayRef[:,1])) or \
     numpy.all(numpy.isnan(dataArrayCur[:,0])) or numpy.all(numpy.isnan(dataArrayCur[:,1])):
    add=numpy.array([
      [float("nan"), float("nan")],
      [0, 0],
    ])
    dataArrayCur=numpy.concatenate((dataArrayCur, add), axis=0)
    dataArrayRef=numpy.concatenate((dataArrayRef, add), axis=0)

  # get minimum of all x and y values (and add some space as border)
  (xmin,ymin)=numpy.minimum(numpy.nanmin(dataArrayRef,0), numpy.nanmin(dataArrayCur,0))
  (xmax,ymax)=numpy.maximum(numpy.nanmax(dataArrayRef,0), numpy.nanmax(dataArrayCur,0))
  dx=(xmax-xmin)/50
  dy=(ymax-ymin)/50
  if dx<1e-7: dx=1
  if dy<1e-7: dy=1
  xmin=xmin-dx
  ymin=ymin-dy
  xmax=xmax+dx
  ymax=ymax+dy

  # create datafile
  dataFileName=pj(diffDir, "data.dat")
  nradd=dataArrayRef.shape[0]-dataArrayCur.shape[0]
  add=numpy.empty([abs(nradd), 2])
  add[:]=float("NaN")
  if nradd<0:
    dataArrayRef=numpy.concatenate((dataArrayRef, add), axis=0)
  if nradd>0:
    dataArrayCur=numpy.concatenate((dataArrayCur, add), axis=0)
  dataArrayRefCur=numpy.concatenate((dataArrayRef, dataArrayCur), axis=1)
  dataArrayRefCur.tofile(dataFileName)

  # create gnuplot file
  SVGFileName=pj(diffDir, "plot.svg")
  gnuplotProcess.stdin.write(("set output '"+SVGFileName+"'\n").encode("utf-8"))
  gnuplotProcess.stdin.write(("set multiplot layout 3, 1\n").encode("utf-8"))
  gnuplotProcess.stdin.write(("set title 'Compare'\n").encode("utf-8"))
  gnuplotProcess.stdin.write(("set ylabel 'Value'\n").encode("utf-8"))
  gnuplotProcess.stdin.write(("plot [%g:%g] [%g:%g] \\\n"%(xmin,xmax, ymin,ymax)).encode("utf-8"))
  gnuplotProcess.stdin.write(("  '"+dataFileName+"' u ($1):($2) binary format='%double%double%double%double' title 'ref' w l lw 2, \\\n").encode("utf-8"))
  gnuplotProcess.stdin.write(("  '"+dataFileName+"' u ($3):($4) binary format='%double%double%double%double' title 'cur' w l\n").encode("utf-8"))
  if dataArrayRef.shape==dataArrayCur.shape:
    gnuplotProcess.stdin.write(("set title 'Absolute Tolerance'\n").encode("utf-8"))
    gnuplotProcess.stdin.write(("set ylabel 'cur-ref'\n").encode("utf-8"))
    gnuplotProcess.stdin.write(("plot [%g:%g] [%g:%g] \\\n"%(xmin,xmax, -3*args.atol, 3*args.atol)).encode("utf-8"))
    gnuplotProcess.stdin.write(("  '"+dataFileName+"' u ($1):($4-$2) binary format='%double%double%double%double' title 'cur-ref' w l, \\\n").encode("utf-8"))
    gnuplotProcess.stdin.write(("  %g title 'atol' lt 2 lw 1, \\\n"%(args.atol)).encode("utf-8"))
    gnuplotProcess.stdin.write(("  %g notitle lt 2 lw 1\n"%(-args.atol)).encode("utf-8"))
    gnuplotProcess.stdin.write(("set title 'Relative Tolerance'\n").encode("utf-8"))
    gnuplotProcess.stdin.write(("set ylabel '(cur-ref)/ref'\n").encode("utf-8"))
    gnuplotProcess.stdin.write(("plot [%g:%g] [%g:%g] \\\n"%(xmin,xmax, -3*args.rtol, 3*args.rtol)).encode("utf-8"))
    # prevent division by zero: use 1e-30 instead
    gnuplotProcess.stdin.write(("  '"+dataFileName+"' u ($1):(($4-$2)/($2==0?1e-30:$2)) binary format='%double%double%double%double' title '(cur-ref)/ref' w l, \\\n").encode("utf-8"))
    gnuplotProcess.stdin.write(("  %g title 'rtol' lt 2 lw 1, \\\n"%(args.rtol)).encode("utf-8"))
    gnuplotProcess.stdin.write(("  %g notitle lt 2 lw 1\n"%(-args.rtol)).encode("utf-8"))
  gnuplotProcess.stdin.write(("unset multiplot\n").encode("utf-8"))
  # dataFileName it not removed since gnuplot is running asynchronously

# numpy.isclose(...) is only defined in newer numpy versions.
# This implementation is taken varbatim from numpy 1.9
def numpy_isclose(a, b, rtol=1.e-5, atol=1.e-8, equal_nan=False):
  import numpy
  def within_tol(x, y, atol, rtol):
    with numpy.errstate(invalid='ignore'):
      result = numpy.less_equal(abs(x-y), atol + rtol * abs(y))
    if numpy.isscalar(a) and numpy.isscalar(b):
      result = bool(result)
    return result
  x = numpy.array(a, copy=False, subok=True, ndmin=1)
  y = numpy.array(b, copy=False, subok=True, ndmin=1)
  xfin = numpy.isfinite(x)
  yfin = numpy.isfinite(y)
  if all(xfin) and all(yfin):
    return within_tol(x, y, atol, rtol)
  else:
    finite = xfin & yfin
    cond = numpy.zeros_like(finite, subok=True)
    x = x * numpy.ones_like(cond)
    y = y * numpy.ones_like(cond)
    cond[finite] = within_tol(x[finite], y[finite], atol, rtol)
    cond[~finite] = (x[~finite] == y[~finite])
    if equal_nan:
      both_nan = numpy.isnan(x) & numpy.isnan(y)
      cond[both_nan] = both_nan[both_nan]
    return cond
# return column col from arr as a column Vector if asColumnVector == True or as a row vector
# arr may be of shape vector or a matrix
def getColumn(arr, col, asColumnVector=True):
  if len(arr.shape)==2:
    if asColumnVector:
      return arr[:,col]
    else:
      return arr[:,col:col+1]
  elif len(arr.shape)==1 and col==0:
    if asColumnVector:
      return arr[:]
    else:
      return arr[:][:,None]
  else:
    raise IndexError("Only HDF5 datasets of shape vector and matrix can be handled.")
def compareDatasetVisitor(h5CurFile, data, example, nrAll, nrFailed, refMemberNames, gnuplotProcess, datasetName, refObj):
  import numpy
  import h5py

  if isinstance(refObj, h5py.Dataset):
    # add to refMemberNames
    refMemberNames.add(datasetName)
    # the corresponding curObj to refObj
    try:
      curObj=h5CurFile[datasetName]
    except KeyError:
      data.append([
        (h5CurFile.filename,""),
        (datasetName,""),
        ('in ref. but not in cur.',"d"),
        ('failed',"d")
      ])
      nrAll[0]+=1
      nrFailed[0]+=1
      return
    # get shape
    refObjCols=refObj.shape[1] if len(refObj.shape)==2 else 1
    curObjCols=curObj.shape[1] if len(curObj.shape)==2 else 1
    # get labels from reference
    try:
      refLabels=list(map(lambda x: (x.decode("utf-8"), 's'), refObj.attrs["Column Label"]))
      # append missing dummy labels
      for x in range(len(refLabels), refObjCols):
        refLabels.append(('&lt;no label in ref. for col. '+str(x+1)+'&gt;', 'w'))
    except KeyError:
      refLabels=list(map(
        lambda x: ('&lt;no label for col. '+str(x+1)+'&gt;', 'w'),
        range(refObjCols)))
    # get labels from current
    try:
      curLabels=list(map(lambda x: (x.decode("utf-8"), 'success'), curObj.attrs["Column Label"]))
      # append missing dummy labels
      for x in range(len(curLabels), curObjCols):
        curLabels.append(('&lt;no label in cur. for col. '+str(x+1)+'&gt;', 'warning'))
    except KeyError:
      curLabels=list(map(
        lambda x: ('&lt;no label for col. '+str(x+1)+'&gt;', 'warning'),
        range(refObjCols)))
    # loop over all columns
    for column in range(refObjCols):
      printLabel=refLabels[column]
      diffFilename=pj(h5CurFile.filename, datasetName, str(column), "diffplot.html")
      nrAll[0]+=1
      # if if curObj[:,column] does not exitst
      if column>=curObjCols:
        printLabel=('&lt;label '+printLabel[0]+' not in cur.&gt;', 'd')
        nrFailed[0]+=1
      cell=[]
      cell.append((h5CurFile.filename,""))
      cell.append((datasetName,""))
      if column<curObjCols and refLabels[column][0]==curLabels[column][0]:
        cell.append((printLabel[0],printLabel[1]))
      else:
        cell.append(('&lt;label for col. '+str(column+1)+' differ&gt;',"d"))
        nrFailed[0]+=1
      if column<curObjCols and curObj.shape[0]>0 and curObj.shape[0]>0: # only if curObj and refObj contains data (rows)
        # check for difference
        refObjCol=getColumn(refObj,column)
        curObjCol=getColumn(curObj,column)
        if refObjCol.shape[0]!=curObjCol.shape[0] or not numpy.all(numpy_isclose(refObjCol, curObjCol, rtol=args.rtol,
                         atol=args.atol, equal_nan=True)):
          nrFailed[0]+=1
          if args.maxCompareFailure==0 or nrFailed[0]<=args.maxCompareFailure:
            cell.append(('<a href="'+urllib.request.pathname2url(diffFilename)+'">failed</a>',"d"))
            dataArrayRef=numpy.concatenate((getColumn(refObj, 0, False), getColumn(refObj, column, False)), axis=1)
            dataArrayCur=numpy.concatenate((getColumn(curObj, 0, False), getColumn(curObj, column, False)), axis=1)
            createDiffPlot(pj(args.reportOutDir, example, diffFilename), example, h5CurFile.filename, datasetName,
                           column, refLabels[column][0], dataArrayRef, dataArrayCur, gnuplotProcess)
          else:
            cell.append(('failed (too many failures, skip reporting)',"d"))
        # everything OK
        else:
          cell.append(('passed',"s"))
      else: # not row in curObj or refObj
        cell.append(('no data row in cur. or ref.',"w"))
      data.append(cell)
    # check for labels/columns in current but not in reference
    for label in curLabels[len(refLabels):]:
      data.append([
        (h5CurFile.filename,""),
        (datasetName,""),
        ('label '+label[0]+' not in ref.',"d"),
        ('failed',"d")
      ])
      nrAll[0]+=1
      nrFailed[0]+=1

def appendDatasetName(curMemberNames, datasetName, curObj):
  import h5py
  if isinstance(curObj, h5py.Dataset):
    # add to curMemberNames
    curMemberNames.add(datasetName)

# compare the example with the reference solution
def compareExample(example, compareFN):
  import h5py

  compareFD=codecs.open(pj(args.reportOutDir, compareFN), "w", encoding="utf-8")

  # print html header
  print('<!DOCTYPE html>', file=compareFD)
  print('<html lang="en">', file=compareFD)
  print('<head>', file=compareFD)
  print('  <META http-equiv="Content-Type" content="text/html; charset=UTF-8">', file=compareFD)
  print('  <meta name="viewport" content="width=device-width, initial-scale=1.0" />', file=compareFD)
  print('  <title>Compare Results: %s</title>'%(args.buildType), file=compareFD)
  print('  <link rel="stylesheet" type="text/css" href="https://cdn.datatables.net/s/bs-3.3.5/jq-2.1.4,dt-1.10.10/datatables.min.css"/>', file=compareFD)
  print('  <link rel="shortcut icon" href="/mbsim/html/mbsimenv.ico" type="image/x-icon"/>', file=compareFD)
  print('  <link rel="icon" href="/mbsim/html/mbsimenv.ico" type="image/x-icon"/>', file=compareFD)
  print('</head>', file=compareFD)
  print('<body style="margin:0.5em">', file=compareFD)
  print('<script src="https://cdn.datatables.net/s/bs-3.3.5/jq-2.1.4,dt-1.10.10/datatables.min.js"> </script>', file=compareFD)
  print('<script src="https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.22.2/moment.min.js"> </script>', file=compareFD)
  print('<script src="https://cdnjs.cloudflare.com/ajax/libs/moment-timezone/0.5.23/moment-timezone-with-data-2012-2022.min.js"> </script>', file=compareFD)
  print('<script src="/mbsim/html/cookiewarning.js"> </script>', file=compareFD)
  print('''<script>
    $(document).ready(function() {
      $('.DATETIME').each(function() {
        $(this).text(moment($(this).text()).tz(moment.tz.guess()).format("ddd, YYYY-MM-DD - HH:mm:ss z"));
      }); 
      $('#SortThisTable').dataTable({
        'lengthMenu': [ [10, 25, 50, 100, -1], [10, 25, 50, 100, 'All'] ],
        'pageLength': 25,
        'aaSorting': [],
        'stateSave': true,
        // we use javascript source for this table due to performance reasons
        'data': SortThisTable_data,
        'columns': [
          { data: 'd0' },
          { data: 'd1' },
          { data: 'd2' },
          { data: 'd3' }
        ],
        "rowCallback": function(row, data) {
          var alltd=$(row).children("td");
          for(var c=2; c<4; c++) {
            var td=alltd.eq(c);
            var flag=data["c"+c.toString()];
            if(flag=="w") {
              td.addClass("warning");
              td.children("span.glyphicon").remove();
              td.prepend('<span class="glyphicon glyphicon-warning-sign alert-warning"></span> ');
            }
            if(flag=="d") {
              td.addClass("danger");
              td.children("span.glyphicon").remove();
              td.prepend('<span class="glyphicon glyphicon-exclamation-sign alert-danger"></span> ');
            }
            if(flag=="s") {
              td.addClass("success");
              td.children("span.glyphicon").remove();
              td.prepend('<span class="glyphicon glyphicon-ok-sign alert-success"></span> ');
            }
          }
        }
      });
      $('#SortThisTable').DataTable().columns.adjust().draw();
    });
    </script>''', file=compareFD)
  print('<h1>Compare Results: <small>%s</small></h1>'%(args.buildType), file=compareFD)
  print('<dl class="dl-horizontal">', file=compareFD)
  print('<dt>Example:</dt><dd>'+example.replace('/', u'/\u200B')+'</dd>', file=compareFD)
  print('<dt>Time ID:</dt><dd class="DATETIME">'+timeID.isoformat()+'Z</dd>', file=compareFD)
  currentID=int(os.path.basename(args.reportOutDir)[len("result_"):])
  parDirs="/".join(list(map(lambda x: "..", range(0, example.count(os.sep)+1))))
  navA=""
  navB=""
  if args.currentID!=0:
    navA="/../.."
    navB="/runexamples_report/result_current"
  print('<dt>Navigate:</dt><dd><a class="btn btn-info btn-xs" href="%s/..%s/result_%010d%s/%s"><span class="glyphicon glyphicon-step-backward"> </span> previous</a>'%
    (parDirs, navA, currentID-1, navB, urllib.request.pathname2url(pj(example, "compare.html"))), file=compareFD)
  print('                 <a class="btn btn-info btn-xs" href="%s/..%s/result_%010d%s/%s"><span class="glyphicon glyphicon-step-forward"> </span> next</a>'%
    (parDirs, navA, currentID+1, navB, urllib.request.pathname2url(pj(example, "compare.html"))), file=compareFD)
  print('                 <a class="btn btn-info btn-xs" href="%s/..%s/result_current%s/%s"><span class="glyphicon glyphicon-fast-forward"> </span> newest</a>'%
    (parDirs, navA, navB, urllib.request.pathname2url(pj(example, "compare.html"))), file=compareFD)
  print('                 <a class="btn btn-info btn-xs" href="%s%s%s/index.html"><span class="glyphicon glyphicon-eject"> </span> parent</a></dd>'%
    (parDirs, navA, navB), file=compareFD)
  print('</dl>', file=compareFD)
  print('<hr/><table id="SortThisTable" class="table table-striped table-hover table-bordered table-condensed">', file=compareFD)
  print('<thead><tr><th><span class="glyphicon glyphicon-folder-open"></span>&nbsp;H5 File</th>'+
        '<th><span class="glyphicon glyphicon-folder-close"></span>&nbsp;Dataset</th>'+
        '<th><span class="glyphicon glyphicon-search"></span>&nbsp;Label</th>'+
        '<th><span class="glyphicon glyphicon-search"></span>&nbsp;Result</th></tr></thead><tbody>', file=compareFD)

  nrAll=[0]
  nrFailed=[0]
  try:
    gnuplotProcess=subprocess.Popen(["gnuplot"], stdin=subprocess.PIPE)
    gnuplotProcess.stdin.write(("set terminal svg size 900, 1400\n").encode("utf-8"))
    gnuplotProcess.stdin.write(("set xlabel 'Time'\n").encode("utf-8"))
  except OSError:
    gnuplotProcess=None
    print("gnuplot not found. Hence no compare plot will be generated. Add gnuplot to PATH to enable.")
  data=[]
  for h5RefFileName in glob.glob(pj("reference", "*.h5")):
    # open h5 files
    h5RefFile=h5py.File(h5RefFileName, "r")
    try:
      h5CurFile=h5py.File(h5RefFileName[10:], "r")
    except IOError:
      data.append([
        (h5RefFile.filename[10:],""),
        ("no such file in current solution","d"),
        ("failed","d"),
        ("failed","d")
      ])
      nrAll[0]+=1
      nrFailed[0]+=1
    else:
      # process h5 file
      refMemberNames=set()
      # bind arguments h5CurFile, data, example, nrAll, nrFailed in order (nrAll, nrFailed as lists to pass by reference)
      dummyFctPtr = functools.partial(compareDatasetVisitor, h5CurFile, data, example, nrAll,
                                      nrFailed, refMemberNames, gnuplotProcess)
      h5RefFile.visititems(dummyFctPtr) # visit all dataset
      # check for datasets in current but not in reference
      curMemberNames=set()
      h5CurFile.visititems(functools.partial(appendDatasetName, curMemberNames)) # get all dataset names in cur
      for datasetName in curMemberNames-refMemberNames:
        data.append([
          (h5CurFile.filename,""),
          (datasetName,""),
          ('not in ref. but in cur.',"d"),
          ('failed',"d")
        ])
        nrAll[0]+=1
        nrFailed[0]+=1
      # close h5 files
      h5RefFile.close()
      h5CurFile.close()
  if gnuplotProcess!=None:
    gnuplotProcess.stdin.close()
    if gnuplotProcess.wait()!=0:
      raise RuntimeError("Generating the SVG file using gnuplot failed.")
  # files in current but not in reference
  refFiles=glob.glob(pj("reference", "*.h5"))
  for curFile in glob.glob("*.h5"):
    if pj("reference", curFile) not in refFiles:
      data.append([
        (curFile,""),
        ('no such file in reference solution',"d"),
        ('failed',"d"),
        ('failed',"d")
      ])
      nrAll[0]+=1
      nrFailed[0]+=1

  # all table data is now collected in data: print it now as javascript code using by DataTables in the browser
  print('<script>', file=compareFD)
  print('var SortThisTable_data=[', file=compareFD)
  for row in data:
    print('{', end="", file=compareFD)
    for i in range(0,4):
      cont=row[i][0].replace("'", "\\'")
      if i==0 or i==1:
        cont=cont.replace('/', u'/\u200B')
      print("'d%d':'%s',"%(i, cont), end="", file=compareFD) # d<colIndex> == data for column <colIndex>
    for i in range(2,4):
      print("'c%d':'%s',"%(i, row[i][1]), end="", file=compareFD) # c<colIndex> == "", "d", "w" or "s" flag for column <colIndex>
    print('},', file=compareFD)
  print('];', file=compareFD)
  print('</script>', file=compareFD)

  # print html footer
  print('</tbody></table>', file=compareFD)
  print('<hr/>', file=compareFD)
  print('<span class="pull-left small">', file=compareFD)
  print('  <a href="/mbsim/html/impressum_disclaimer_datenschutz.html#impressum">Impressum</a> /', file=compareFD)
  print('  <a href="/mbsim/html/impressum_disclaimer_datenschutz.html#disclaimer">Disclaimer</a> /', file=compareFD)
  print('  <a href="/mbsim/html/impressum_disclaimer_datenschutz.html#datenschutz">Datenschutz</a>', file=compareFD)
  print('</span>', file=compareFD)
  print('<span class="pull-right small">', file=compareFD)
  print('  Generated on <span class="DATETIME">%s</span> by runexamples.py'%(timeID.isoformat()+"Z"), file=compareFD)
  print('  <a href="/">Home</a>', file=compareFD)
  print('</span>', file=compareFD)
  print('</body>', file=compareFD)
  print('</html>', file=compareFD)

  compareFD.close()
  return 1 if nrFailed[0]>0 else 0, nrFailed[0], nrAll[0]



def loopOverReferenceFiles(msg, srcPostfix, dstPrefix, action):
  curNumber=0
  lenDirs=len(directories)
  for example in directories:
    # remove dst dir and recreate it empty
    if os.path.isdir(pj(dstPrefix, example[0], "reference")): shutil.rmtree(pj(dstPrefix, example[0], "reference"))
    os.makedirs(pj(dstPrefix, example[0], "reference"))
    # loop over src
    curNumber+=1
    print("%s: Example %03d/%03d; %5.1f%%; %s"%(msg, curNumber, lenDirs, curNumber/lenDirs*100, example[0]))
    if not os.path.isdir(pj(dstPrefix, example[0], "reference")): os.makedirs(pj(dstPrefix, example[0], "reference"))
    # apply action to all these files in the current dir (example dir)
    for fnglob in ["time.dat", "*.h5"]:
      for fn in glob.glob(pj(example[0], srcPostfix, fnglob)):
        action(fn, pj(dstPrefix, example[0], "reference", os.path.basename(fn)))

def copyToReference():
  loopOverReferenceFiles("Copy to reference", ".", ".", shutil.copyfile)

def copyAndSHA1AndAppendIndex(src, dst):
  # copy src to dst
  shutil.copyfile(src, dst)
  # create sha1 hash of dst (save to <dst>.sha1)
  codecs.open(dst+".sha1", "w", encoding="utf-8").write(hashlib.sha1(codecs.open(dst, "rb").read()).hexdigest())
  # add file to index
  index=codecs.open(pj(os.path.dirname(dst), "index.txt"), "a", encoding="utf-8")
  index.write(os.path.basename(dst)+":")
def pushReference():
  loopOverReferenceFiles("Pushing reference to download dir", "reference", args.pushDIR, copyAndSHA1AndAppendIndex)

def updateReference():
  import requests
  def downloadFileIfDifferent(s, src):
    remoteSHA1Url=args.updateURL+"/"+urllib.request.pathname2url(src+".sha1")
    remoteSHA1=s.get(remoteSHA1Url).content.decode('utf-8')
    try:
      localSHA1=hashlib.sha1(codecs.open(src, "rb").read()).hexdigest()
    except IOError: 
      localSHA1=""
    if remoteSHA1!=localSHA1:
      remoteUrl=args.updateURL+"/"+urllib.request.pathname2url(src)
      print("  Download "+remoteUrl)
      if not os.path.isdir(os.path.dirname(src)): os.makedirs(os.path.dirname(src))
      codecs.open(src, "wb").write(s.get(remoteUrl).content)
  curNumber=0
  lenDirs=len(directories)
  s=requests.Session()
  for example in directories:
    # print message
    curNumber+=1
    print("%s: Example %03d/%03d; %5.1f%%; %s"%("Update reference", curNumber, lenDirs, curNumber/lenDirs*100, example[0]))
    # loop over all file in src/index.txt
    indexUrl=args.updateURL+"/"+urllib.request.pathname2url(pj(example[0], "reference", "index.txt"))
    res=s.get(indexUrl)
    if res.status_code==404:
      continue
    for fileName in res.content.decode("utf-8").rstrip(":").split(":"):
      downloadFileIfDifferent(s, pj(example[0], "reference", fileName))



def listExamples():
  print('The following examples will be run:\n')
  for example in directories:
    print(example[0])



def validateXML(example, consoleOutput, htmlOutputFD):
  nrFailed=0
  nrTotal=0
  types={"*.ombv.xml":               [ombvSchema], # validate openmbv files generated by MBSim
         "*.ombv.env.xml":           [ombvSchema], # validate openmbv environment user files
         "MBS.mbsimprj.flat.xml": mbsimXMLSchemas} # validate user mbsim flat xml files
  for root, _, filenames in os.walk(os.curdir):
    for typesKey, typesValue in types.items():
      for filename in fnmatch.filter(filenames, typesKey):
        outputFN=pj(example[0], filename+".txt")
        outputFD=MultiFile(codecs.open(pj(args.reportOutDir, outputFN), "w", encoding="utf-8"), args.printToConsole)
        print('<tr>', file=htmlOutputFD)
        print('<td>'+filename.replace('/', u'/\u200B')+'</td>', file=htmlOutputFD)
        print("Running command:", file=outputFD)
        list(map(lambda x: print(x, end=" ", file=outputFD), [mbxmlutilsvalidate]+typesValue+[pj(root, filename)]))
        print("\n", file=outputFD)
        outputFD.flush()
        if subprocessCall(exePrefix()+[mbxmlutilsvalidate]+typesValue+[pj(root, filename)],
                          outputFD)!=0:
          nrFailed+=1
          print('<td data-order="1" class="danger"><span class="glyphicon glyphicon-exclamation-sign alert-danger"></span>&nbsp;<a href="'+urllib.request.pathname2url(filename+".txt")+'">failed</a></td>', file=htmlOutputFD)
        else:
          print('<td data-order="0" class="success"><span class="glyphicon glyphicon-ok-sign alert-success"></span>&nbsp;<a href="'+urllib.request.pathname2url(filename+".txt")+'">passed</a></td>', file=htmlOutputFD)
        print('</tr>', file=htmlOutputFD)
        nrTotal+=1
        outputFD.close()
  return nrFailed, nrTotal



def writeAtomFeed(currentID, nrFailed, nrTotal):
  # do not write a feed if --buildSystemRun is not used
  if not args.buildSystemRun:
    return
  # load the add feed module
  sys.path.append("/context")
  import buildSystemState
  # add a new feed if examples have failed
  buildSystemState.update(args.buildType+"-examples", "Examples Failed: "+args.buildType,
    "%d of %d examples failed."%(nrFailed, nrTotal),
    "%s/result_%010d/index.html"%(args.url, currentID),
    nrFailed, nrTotal)



# restore or backup the coverage files in the build directories
def coverageBackupRestore(variant):
  for t in ["fmatvec", "hdf5serie", "openmbv", "mbsim"]:
    d=pj(args.coverage.split(":")[0], t+args.coverage.split(":")[1])
    for root, _, files in os.walk(d):
      for f in sorted(files):
        if variant=="backup":
          if f.endswith(".gcda"):
            shutil.copyfile(pj(root, f), pj(root, f+".beforeRunexamples"))
        if variant=="restore":
          if f.endswith(".gcda"): # is processed before .gcda.beforeRunexamples: files is sotred
            os.remove(pj(root, f))
          if f.endswith(".gcda.beforeRunexamples"): # is processed after .gcda: files is sotred
            shutil.move(pj(root, f), pj(root, os.path.splitext(f)[0]))
def coverage(mainFD):
  print('<tr><td>Coverage analyzis</td>', file=mainFD); mainFD.flush()
  ret=0
  if not os.path.exists(pj(args.reportOutDir, "coverage")): os.makedirs(pj(args.reportOutDir, "coverage"))
  lcovFD=codecs.open(pj(args.reportOutDir, "coverage", "log.txt"), "a", encoding="utf-8")
  # lcov "-d" arguments
  dirs=map(lambda x: ["-d", pj(args.coverage.split(":")[0], x),
                      "-d", pj(args.coverage.split(":")[0], x+args.coverage.split(":")[1])],
                     ["fmatvec", "hdf5serie", "openmbv", "mbsim"])
  dirs=["-d", args.coverage.split(":")[2]]+[v for il in dirs for v in il]

  # run lcov: init counters
  ret=ret+abs(subprocess.call(["lcov", "-c", "--no-external", "-i", "--ignore-errors", "graph", "-o", pj(args.reportOutDir, "coverage", "cov.trace.base")]+dirs, stdout=lcovFD, stderr=lcovFD))
  # run lcov: count
  ret=ret+abs(subprocess.call(["lcov", "-c", "--no-external", "-o", pj(args.reportOutDir, "coverage", "cov.trace.test")]+dirs, stdout=lcovFD, stderr=lcovFD))
  # run lcov: combine counters
  ret=ret+abs(subprocess.call(["lcov", "-a", pj(args.reportOutDir, "coverage", "cov.trace.base"), "-a", pj(args.reportOutDir, "coverage", "cov.trace.test"), "-o", pj(args.reportOutDir, "coverage", "cov.trace.total")], stdout=lcovFD, stderr=lcovFD))
  # run lcov: remove counters
  ret=ret+abs(subprocess.call(["lcov", "-r", pj(args.reportOutDir, "coverage", "cov.trace.total"),
    "*/mbsim*/kernel/swig/*", "*/openmbv*/openmbvcppinterface/swig/java/*", # SWIG generated
    "*/openmbv*/openmbvcppinterface/swig/octave/*", "*/openmbv*/openmbvcppinterface/swig/python/*", # SWIG generated
    "*/openmbv*/mbxmlutils/mbxmlutils/*", # SWIG generated
    "*/mbsim*/thirdparty/nurbs++/*/*", "*/include/nurbs++/*", "*/mbsim*/kernel/mbsim/numerics/csparse.*", # 3rd party
    "*/mbsim*/examples/*", # mbsim examples
    "*.moc.cc", # mbsim generated
    "*/hdf5serie*/h5plotserie/h5plotserie/*", "*/openmbv*/openmbv/openmbv/*", "*/mbsim*/mbsimgui/mbsimgui/*", # GUI (untested)
    "*/mbsim*/modules/mbsimInterface/mbsimInterface/*", # other untested features
    "-o", pj(args.reportOutDir, "coverage", "cov.trace.final")], stdout=lcovFD, stderr=lcovFD))

  # collect all header files in repos and hash it
  repoHeader={}
  for repo in ["fmatvec", "hdf5serie", "openmbv", "mbsim"]:
    for root, _, files in os.walk(pj(args.coverage.split(":")[0], repo)):
      for f in files:
        if f.endswith(".h"):
          repoHeader[hashlib.sha1(codecs.open(pj(root, f), "r", encoding="utf-8").read().encode("utf-8")).hexdigest()]=pj(root, f)
  # loop over all header files in local and create mapping (using the hash)
  headerMap=[]
  for root, _, files in os.walk(pj(args.coverage.split(":")[2], "include")):
    for f in files:
      if f.endswith(".h"):
        h=hashlib.sha1(codecs.open(pj(root, f), "r", encoding="utf-8").read().encode("utf-8")).hexdigest()
        if h in repoHeader:
          headerMap.append((pj(root, f), repoHeader[h]))
  # replace header map in lcov trace file
  for line in fileinput.FileInput(pj(args.reportOutDir, "coverage", "cov.trace.final"), inplace=1):
    if line.startswith("SF:"):
      for hm in headerMap:
        line=line.replace("SF:"+hm[0], "SF:"+hm[1])
    print(line, end="")

  # generate html files
  ret=ret+abs(subprocess.call(["genhtml", "-t", "MBSim-Env Examples (%s)"%(args.buildType), "--prefix", args.coverage.split(":")[0], "--legend",
    "--html-prolog", pj(scriptDir, "lcov-prolog.part.html"), "--html-epilog", pj(scriptDir, "lcov-epilog.part.html"),
    "--demangle-cpp", "-o", pj(args.reportOutDir, "coverage"),
    pj(args.reportOutDir, "coverage", "cov.trace.final")], stdout=lcovFD, stderr=lcovFD))
  lcovFD.close()

  # get coverage rate
  covRate=0
  linesRE=re.compile("^ *lines\.*: *([0-9]+\.[0-9]+)% ")
  for line in fileinput.FileInput(pj(args.reportOutDir, "coverage", "log.txt")):
    m=linesRE.match(line)
    if m!=None:
      covRate=int(float(m.group(1))+0.5)
  covRateStr=str(covRate)+"%" if ret==0 else "ERR"
  # update build state (only if --buildSystemRun is used)
  if args.buildSystemRun:
    # load and add module
    sys.path.append("/context")
    import buildSystemState
    buildSystemState.createStateSVGFile("/mbsim-state/"+args.buildType+"-coverage.svg", covRateStr,
      "#d9534f" if ret!=0 or covRate<70 else ("#f0ad4e" if covRate<90 else "#5cb85c"))

  if ret==0:
    print('<td class="success"><span class="glyphicon glyphicon-ok-sign alert-success"></span>&nbsp;', file=mainFD)
  else:
    print('<td class="danger"><span class="glyphicon glyphicon-exclamation-sign alert-danger"></span>&nbsp;', file=mainFD)
  print('<a href="'+urllib.request.pathname2url(pj("coverage", "log.txt"))+'">%s</a> - '%("done" if ret==0 else "failed")+
        '<a href="'+urllib.request.pathname2url(pj("coverage", "index.html"))+'"><b>Coverage</b> <span class="badge">%s</span></a></td>'%(covRateStr), file=mainFD)
  for i in range(0, 7-sum([args.disableRun, args.disableRun, args.disableRun, not args.checkGUIs, args.disableCompare,
    args.disableRun or (not args.buildSystemRun) or not args.webapp, args.disableRun, args.disableValidate])):
    print('<td>-</td>', file=mainFD)
  print('</tr>', file=mainFD); mainFD.flush()
  return 1 if ret!=0 else 0



#####################################################################################
# call the main routine
#####################################################################################

if __name__=="__main__":
  mainRet=main()
  exit(mainRet)
