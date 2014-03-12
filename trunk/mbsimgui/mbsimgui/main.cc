/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2012 Martin Förg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <config.h>
#include <QApplication>
#include "mainwindow.h"
#include <H5Cpp.h>
#include <QLocale>

using namespace std;
using namespace MBSimGUI;

int main(int argc, char *argv[]) {
  // environment variables
  // Disalbe COIN VBO per default (see --help)
  char COIN_VBO[strlen("COIN_VBO=0")+1];
  if(getenv("COIN_VBO")==NULL) putenv(strcpy(COIN_VBO, "COIN_VBO=0"));

  QStringList arg;
  for(int i=1; i<argc; i++)
    arg << argv[i];

  // help
  if(arg.contains("-h") || arg.contains("--help")) {
    cout<<"MBSimGUI - A Graphical User Interface for MBSim"<<endl;
    cout    <<""<<endl;
    //cout    <<"Version "SVNVERSION<<endl;
    //cout    <<""<<endl;
            // 12345678901234567890123456789012345678901234567890123456789012345678901234567890
    cout    <<"Copyright (C) 2013 Martin Foerg <martin.o.foerg@googlemail.com"<<endl;
    cout    <<"This is free software; see the source for copying conditions. There is NO"<<endl;
    cout    <<"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."<<endl;
    cout    <<""<<endl;
    cout    <<"Licensed under the GNU General Public License (GPL)"<<endl;
    cout    <<""<<endl;
    cout    <<"Usage: mbsimgui [-h|--help]"<<endl;
    cout    <<"                [<dir>] [<mbsimfile>] [<parameterfile>] [<integratorfile>]"<<endl;
    cout    <<""<<endl;
    cout    <<"-h|--help          Shows this help"<<endl;
    cout    <<"<dir>              Load first *.mbsim.xml, *.mbsimparam.xml"<<endl;
    cout    <<"                   and *.mbsimint.xml file in <dir>."<<endl;
    cout    <<"<mbsimfile>        Load <mbsimfile>"<<endl;
    cout    <<"<parameterfile>    Load <parameterfile>"<<endl;
    cout    <<"<integratorfile>   Load <integratorfile>"<<endl;
    cout    <<""<<endl;
    arg.removeAll("-h"); 
    arg.removeAll("--help"); 
    return 0;
  }
  // disalbe HDF5 error message print
  H5::Exception::dontPrint();

  QApplication app(argc, argv);
  app.setOrganizationName("MBSimGUI");
  QLocale::setDefault(QLocale::C);
  setlocale(LC_ALL, "C");
  MainWindow mainwindow(arg);
  mainwindow.show();
  //mainwindow.showMaximized();
  //mainwindow->resize(1400, 900);
  //mainwindow->resize(1100, 700);
  int ret=app.exec();
  return ret;
}
