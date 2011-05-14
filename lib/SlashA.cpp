/*
 *
 *  SlashA.cpp
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
#include <string>
#include <ctime>
#include <unistd.h> // contains the alarm() function used for timing-out the interpreter
#include <signal.h> // contains the signal() function to handle the alarm
#include "SlashA.h"
#include "SlashA_DIS.h"

using namespace std;

namespace SlashA
{


/*
 *
 * Global variables and functions
 *
 */
 
volatile bool timedout; // volatile tells the compiler the variable might be changed from outside the normal program flow
void alarm_handler(int sign) { timedout=true; }; // alarm for the runByteCode() function


/*
 *
 * Class methods
 *
 */

//
//  Class: InstructionSet
//


void InstructionSet::insert_DIS_IO()
{
  Instruction* setiptr;
  // input/output
  setiptr = new DIS::Input(); insert(setiptr);
  setiptr = new DIS::Output(); insert(setiptr);
}

void InstructionSet::insert_DIS_memreg()
{
  Instruction* setiptr;

  // memory-register commands
  setiptr = new DIS::Load(); insert(setiptr);
  setiptr = new DIS::Save(); insert(setiptr);
  setiptr = new DIS::Swap(); insert(setiptr);
  setiptr = new DIS::Cmp(); insert(setiptr);
};

void InstructionSet::insert_DIS_regreg()
{
  Instruction* setiptr;
  // register-register commands
  setiptr = new DIS::Inc(); insert(setiptr);
  setiptr = new DIS::Dec(); insert(setiptr);
  setiptr = new DIS::ItoF(); insert(setiptr);
  setiptr = new DIS::FtoI(); insert(setiptr);
}

void InstructionSet::insert_DIS_gotos()
{
  Instruction* setiptr;
  // flow control: gotos
  setiptr = new DIS::Label(); insert(setiptr);
  setiptr = new DIS::GotoIfP(); insert(setiptr);
}

void InstructionSet::insert_DIS_jumps()
{
  Instruction* setiptr;
  // flow control: jumps
  setiptr = new DIS::JumpIfN(); insert(setiptr);
  setiptr = new DIS::JumpHere(); insert(setiptr);
}

void InstructionSet::insert_DIS_loops() 
{
  Instruction* setiptr;
  // flow control: loops
  setiptr = new DIS::Loop(); insert(setiptr);
  setiptr = new DIS::EndLoop(); insert(setiptr);
}

void InstructionSet::insert_DIS_basicmath()
{
  Instruction* setiptr;
  // basic math
  setiptr = new DIS::Add(); insert(setiptr);
  setiptr = new DIS::Sub(); insert(setiptr);
  setiptr = new DIS::Mul(); insert(setiptr);
  setiptr = new DIS::Div(); insert(setiptr);
}

void InstructionSet::insert_DIS_advmath()
{
  Instruction* setiptr;
  // advanced math
  setiptr = new DIS::Abs(); insert(setiptr);
  setiptr = new DIS::Sign(); insert(setiptr);
  setiptr = new DIS::Exp(); insert(setiptr);
  setiptr = new DIS::Log(); insert(setiptr);
  setiptr = new DIS::Sin(); insert(setiptr);
  setiptr = new DIS::Pow(); insert(setiptr);
  setiptr = new DIS::Ran(); insert(setiptr);
}

void InstructionSet::insert_DIS_misc()
{
  Instruction* setiptr;
  // misc
  setiptr = new DIS::Nop(); insert(setiptr);
}

void InstructionSet::insert_DIS_numeric(ByteCode_Type n_num)
{
  Instruction* setiptr;
  
  for (ByteCode_Type i=0;i<n_num;i++) {
    setiptr = new DIS::SetI(i);
    insert(setiptr);
  }
}

void InstructionSet::insert_DIS_full()
{
  insert_DIS_IO(); // input/output commands
  insert_DIS_memreg(); // memory-register commands
  insert_DIS_regreg(); // register-register commands
  insert_DIS_gotos(); // flow-control: goto commands
  insert_DIS_jumps(); // flow-control: jump commands
  insert_DIS_loops(); // flow-control: loop commands
  insert_DIS_basicmath(); // basic math operations
  insert_DIS_advmath(); // advanced math functions
  insert_DIS_misc(); // everything else
}

void InstructionSet::insert_DIS_full_minus_Gotos() // avoids infinite loops
{
  insert_DIS_IO(); // input/output commands
  insert_DIS_memreg(); // memory-register commands
  insert_DIS_regreg(); // register-register commands
  insert_DIS_jumps(); // flow-control: jump commands
  insert_DIS_loops(); // flow-control: loop commands
  insert_DIS_basicmath(); // basic math operations
  insert_DIS_advmath(); // advanced math functions
  insert_DIS_misc(); // everything else
}

void InstructionSet::remove_DIS()
{
  for (unsigned i=0;i<set.size();i++)
    if (set[i]->isDIS()) // only deallocates memory if it's a DIS instruction (users must deallocate their own instructions!!)
      delete set[i]; 
}

//
//  Class: MemCore
//

// Constructor
MemCore::MemCore(const unsigned _Dsize, 
                 const unsigned _Lsize,
                 vector<double>& _input,
                 vector<double>& _output)
{
  D_size = _Dsize;
  L_size = _Lsize;
  input = &_input;
  output = &_output;
  output_executed = false;

  F = I = c = 0; 
  D = new double[D_size];
  D_saved = new bool[D_size];
  L = new unsigned[L_size];
  L_saved = new bool[L_size];
  ran_ptr = new long;

  for (unsigned i=0;i<D_size;i++) {
    D[i] = 0;
    D_saved[i] = false;
  }

  for (unsigned i=0;i<L_size;i++) {
    L[i] = 0;
    L_saved[i] = false;
  }
};

// Destructor
MemCore::~MemCore()
{
  delete[] D; 
  delete[] D_saved; 
  delete[] L; 
  delete[] L_saved; 
  delete ran_ptr;
}


/* 
 *
 * Functions
 *
 */

string getHeader()
{
#ifndef DEBUG
  return "Slash/A Revision " + revNumber + ", Copyright (C) Artur B. Adib";
#else
  return "Slash/A Revision " + revNumber + " *** DEBUG MODE ***, Copyright (C) Artur B. Adib";
#endif
}


ByteCode_Type instruction2ByteCode( string inst, 
                                            InstructionSet& iset )
{
  for (ByteCode_Type i=0;i<iset.size();i++)
    if (inst == iset.getName(i) )
      return i;

  throw (string)"Instruction not recognized: " + inst;
}


void source2ByteCode( string src,
                      ByteCode& bc,
                      InstructionSet& iset )
{
  bool seeking_next_line = false;
  string instr;

  bc.clear();

  for (unsigned c=0;c<src.size();c++) {

    if ( seeking_next_line ) {
      if (src[c]==10) // line-feed found?
        seeking_next_line = false;
      continue;
    }

    else if (src[c] == '.') // a dot signals end of program
      break;

    else if (src[c]=='/') { // instruction reading is done
      try 
      {
        bc.push_back( instruction2ByteCode(instr, iset) );
        instr.clear();
      }
      catch(string& err)
      {
        throw err;
      }
    }

    else if (src[c] == ' ') {} // spaces are ignored altogether

    else if (src[c] == 10) {} // line-feeds are ignored

    else if (src[c] == 9) {} // tabs are ignored as well

    else if (src[c] == '#') // interpreter will ignore rest of the line
      seeking_next_line = true;

    else {
      instr += src[c];
      if (instr.size() > maxWordLen)
        throw (string)"Instruction word is too large: " + instr;
    } // ifs

  } // for

} // Source2ByteCode()


void bytecode2Source( ByteCode& bc,
                      string& src,
                      InstructionSet& iset )
{
  src.clear();
  for (unsigned i=0;i<bc.size();i++)
    src += iset.getName(bc[i]) + "/";
  src += ".";
}


// Runs a given ByteCode, returns true if timed-out.
bool runByteCode(InstructionSet& iset,
                 MemCore& core,
                 ByteCode& bc,
                 long randseed,
                 long max_rtime,
                 int max_loop_depth)
{
  if (!max_rtime)
    max_rtime = 3600*24*7; // that's a week's worth of runtime!

  core.C = &bc;
  core.c = 0;  
  iset.clear();

#ifndef DEBUG
  /* Setup the time-out handler */
  timedout=false;
  signal(SIGALRM, alarm_handler);
  alarm(max_rtime);
#endif
  
  iset.setMaxLoopDepth(max_loop_depth);

  try
  {
    do {
#ifdef DEBUG
      cout << " [I]=" << core.I << ", [F]=" << core.F << ", D[I]=" << core.D[core.I] << endl;
      cout << " Next instruction: " << iset.getName((*core.C)[core.c]) << ". (hit enter)";
      cin.get();
      cout << endl;
#endif
      iset.exec((*core.C)[core.c], core);
      core.c++;
    } while ( (core.c<(*core.C).size()) && (!timedout) );
  }
  catch(int whatever)
  {
    return true; // program failed 
  }
  
#ifndef DEBUG
  alarm(0); // turns off alarm
#endif

  if (timedout)
    return true; // interpreter timed-out
  else
    return false;
} // runByteCode


}; //namespace SlashA
