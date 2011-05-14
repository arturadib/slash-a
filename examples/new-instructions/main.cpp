/*
 *
 *  new-slash
 *
 *  A command-line utility illustrating the use of the Slash/A library with user-defined instructions. 
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

using namespace std;

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include "SlashA.h"

inline double p2(double x) { return x*x; }

//
// Defines new instruction
//
class Dist : public SlashA::Instruction
{
  public:
    Dist() : Instruction() { name="DIST"; }
    ~Dist() {}
    inline void code(SlashA::MemCore& core, SlashA::InstructionSet& iset) 
    {
      core.setF( sqrt(p2(core.getF()) + p2(core.D[core.I])) ); // returns the distance from the origin; with x=F, y=D[I].
    }
};


int main(int argc, char** argv)
{
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
    Dist* dist = new Dist(); // allocates the new instruction
    iset.insert(dist); // inserts the new instruction in the instruction set iset

    source2ByteCode(source, bc, iset); // translates "source" into "bc" using the instruction set "iset"

    bool failed = SlashA::runByteCode(iset, // instruction set pointer
                                      memcore, // memory core pointer
                                      bc, // ByteCoded program to be run (pointer)
                                      2237, // random seed for random number instructions
                                      0, // max run time in seconds (0 for no limit)
                                      -1); // loop depth limit

    if (failed)
      cout << "Program failed (time-out, max loop depth, etc)!" << endl;
      
    cout << endl;
    cout << "Total number of invalid operations: " << iset.getTotalInvops() << endl;
  }
  catch(string& s)
  {
    cout << s << endl << endl;
    exit(1);
  }
  
  cout << endl;
  
  return 0;
}
