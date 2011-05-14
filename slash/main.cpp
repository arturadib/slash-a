/*
 *
 *  slash - A command-line Slash/A interpreter illustrating the use of the Default Instruction Set (DIS). 
 *
 *  Copyright (C) 2004-2011 Artur B Adib
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include "SlashA.h"

using namespace std;

int main(int argc, char** argv)
{  
  cout << "slash -- An interpreter for the Slash/A language" << endl;
  cout << SlashA::getHeader() << endl << endl;

  if (argc<2) {
    cout << "Usage:\n";
    cout << "  slash <file.sla>\n\n";
    exit(1);
  }

  string source = "";
  ifstream f(argv[1]);

  if (!f) {
    cout << "Cannot open file " << argv[1] << ".\n\n";
    exit(1);
  }
  
  while (!f.eof())
    source += f.get();

  f.close();

  try 
  {
    vector<double> input, output;
    SlashA::ByteCode bc;
    SlashA::InstructionSet iset(32768); // initializes the Default Instruction Set with 32768 numeric instructions
    SlashA::MemCore memcore(10, // length of the data tape D[]
                            10, // length of the label tape L[]
                            input, // input buffer (will use keyboard if empty)
                            output); // output buffer

    iset.insert_DIS_full();

    SlashA::source2ByteCode(source, bc, iset); // Translates "source" into "bc" using the instruction set "iset"

    bool failed = SlashA::runByteCode(iset, // instruction set pointer
                                      memcore, // memory core pointer
                                      bc, // ByteCoded program to be run (pointer)
                                      -2237, // random seed for random number instructions
                                      0, // max run time in seconds (0 for no limit)
                                      -1); // max loop depth

    if (failed)
      cout << "Program failed (time-out, loop depth, etc)!" << endl;
      
    cout << endl;
    cout << "Total number of operations: " << iset.getTotalOps() << endl;
    cout << "Total number of invalid operations: " << iset.getTotalInvops() << endl;
    cout << "Total number of inputs before an output: " << iset.getTotalInputsBFOutput() << endl;
  }
  catch(string& s)
  {
    cout << s << endl << endl;
    exit(1);
  }
  
  cout << endl;
  
  return 0;
}
