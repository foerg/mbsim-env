/* Copyright (C) 2004-2009 MBSim Development Team
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 *  
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *  
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: schneidm@users.berlios.de
 */

#include "mbsimHydraulics/obsolet_hint.h"
#include <string>
#include <iostream>

using namespace std;

namespace MBSimHydraulics {

  string process_signal_string(string path) {
    if (path.find("Signal[")!=string::npos) {
      const size_t pos=path.find("Signal[");
      cout << "WARNING! Signal-Container is obsolete, use Link-Container instead! (" << path << ")" << endl;
      path.erase(pos, 7);
      path.insert(pos, "Link[");
    }
    return path;
  }

  string process_hline_string(string path) {
    if (path.find("HLine[")!=string::npos) {
      const size_t pos=path.find("HLine[");
      cout << "WARNING! HLine-Container is obsolete, use Object-Container instead! (" << path << ")" << endl;
      path.erase(pos, 6);
      path.insert(pos, "Object[");
    }
    return path;
  }

}
