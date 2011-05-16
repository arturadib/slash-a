/*
 *
 *  Slash/A's Default Instruction Set (DIS)
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

#include <cmath>
#include <sstream>
#include "NR-ran2.hpp"

namespace SlashA 
{
 
namespace DIS
{

class SetI : public Instruction
{
  private:
    ByteCode_Type num;
  public:
    SetI(ByteCode_Type n) : Instruction() 
    { 
      std::ostringstream nstr;
      nstr << n;
      name=nstr.str(); 
      DIS_flag = true; 
      num = n; 
    };
    ~SetI() {};
    inline void code(MemCore& core, InstructionSet& iset) { core.I = num; n_ops++; } 
};

class ItoF : public Instruction
{
  public:
    ItoF() : Instruction() { name="itof"; DIS_flag = true; };
    ~ItoF() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    { 
      n_ops++;
      if (!core.setF((double)core.I))
        n_invops++;  
    }
};

class FtoI : public Instruction
{
  public:
    FtoI() : Instruction() { name="ftoi"; DIS_flag = true; };
    ~FtoI() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    { 
      n_ops++; 
      core.I = (unsigned)rint(core.getF());
    }
};

class Inc : public Instruction
{
  public:
    Inc() : Instruction() { name="inc"; DIS_flag = true; };
    ~Inc() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    { 
      n_ops++; 
      if ( !core.setF(core.getF()+1.0) )
        n_invops++;
    }
};

class Dec : public Instruction
{
  public:
    Dec() : Instruction() { name="dec"; DIS_flag = true; };
    ~Dec() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    { 
      n_ops++;
      if ( !core.setF(core.getF()-1.0) )
        n_invops++;
    }
};

class Cmp : public Instruction
{
  public:
    Cmp() : Instruction() { name="cmp"; DIS_flag = true; };
    ~Cmp() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    { 
      double retvalue=0;

      n_ops++;
      if (core.I<core.D_size) 
      {
        if (core.D_saved[core.I]) 
        {
          if ( core.getF() != core.D[core.I] )
            retvalue = -1;

          if ( !core.setF(retvalue) )
            n_invops++;
        }
        else
          n_invops++;
      }
      else
        n_invops++;
    }
};

class Load : public Instruction
{
  public:
    Load() : Instruction() { name="load"; DIS_flag = true; };
    ~Load() {};
    inline void code(MemCore& core, InstructionSet& iset) {
      n_ops++;
      if (core.I<core.D_size) 
      {
        if (core.D_saved[core.I]) 
        {
          if ( !core.setF(core.D[core.I]) )
            n_invops++;
        }
        else
          n_invops++;
      }
      else
        n_invops++;
    };
};

class Save : public Instruction
{
  public:
    Save() : Instruction() { name="save"; DIS_flag = true; };
    ~Save() {};
    inline void code(MemCore& core, InstructionSet& iset) {
      n_ops++;
      if (core.I<core.D_size) {
        core.D[core.I] = core.getF();
        core.D_saved[core.I] = true;
      }
      else
        n_invops++;
    }
};

class Swap : public Instruction
{
  public:
    Swap() : Instruction() { name="swap"; DIS_flag = true; };
    ~Swap() {};
    inline void code(MemCore& core, InstructionSet& iset) {
      n_ops++;
      if (core.I<core.D_size) {
        if (core.D_saved[core.I]) {
          double aux = core.D[core.I];
          core.D[core.I] = core.getF();
          core.setF(aux);
        }
        else
          n_invops++;
      }
      else
        n_invops++;
    }
};

class Label : public Instruction
{
  public:
    Label() : Instruction() { name="label"; DIS_flag = true; };
    ~Label() {};
    inline void code(MemCore& core, InstructionSet& iset) {
      n_ops++;
      if (core.I<core.L_size) {
        core.L[core.I] = core.c; // saves current position (next instruction will be executed when this label is called)
        core.L_saved[core.I] = true;
      }
      else
        n_invops++;
    }
};

class GotoIfP : public Instruction
{
  public:
    GotoIfP() : Instruction() { name="gotoifp"; DIS_flag = true; };
    ~GotoIfP() {};
    inline void code(MemCore& core, InstructionSet& iset) {
      n_ops++;
      if (core.I < core.L_size) {
        if (core.L_saved[core.I]) {
          if (core.getF()>=0) 
            core.c=core.L[core.I];
        }    
        else
          n_invops++;
      }
      else
        n_invops++;
    }
};

class JumpIfN : public Instruction
{
  private:
    bool never_called;
    std::vector<unsigned> J_table; // jump-table

    /* 
     * The present implementation of JumpIfN uses a "jump table". In Revision 1, upon every call to jumpifn
     * (if indeed F<0) the instruction implementation would search for the corresponding jumphere. This is
     * a very time-consuming task, especially when such jumps appear within long loops.
     *
     * With a jump-table formalism, we perform a one-time search for all the jumpifn's and their corresponding 
     * jumphere's, feeding that information to the jump-table. Later calls to jumpifn simply jump to the
     * corresponding point stored at the table, without any additional searches.
     *
     * Because JumpHere are dummy instructions, all of the implementation of JumpsIfN can be confined to here.
     *
     */
    
    inline void build_J_table(MemCore& core, InstructionSet& iset)
    {
      const unsigned C_size=(*core.C).size();
      unsigned curr_c=0; // starting from c=0 IS important! 
                         // other flow control instructions, including another jumpifn, might cause the first
                         // occurrence of jumpifn in the code to be bypassed
      unsigned searching_c;
      unsigned n_openjumps; // n_openjumps counts the number of jumps without a corresponding jumphere
      
      J_table.clear();
      
      for (unsigned i=0;i<C_size;i++)
        J_table.push_back(0); // zeroes table

      while (curr_c<C_size) // this loop searches for "jumpifn" instructions in the code
      {
        if (iset.getName((*core.C)[curr_c]) == "jumpifn") // if it's a jumpifn, searches for the corresponding jumphere
        {
          n_openjumps=1; // the current jumpifn is open
          searching_c=curr_c+1; // starts at the next instruction
          while ( (n_openjumps>0) && (searching_c<C_size) ) // searches for the corresponding jumphere
          {
            if (iset.getName((*core.C)[searching_c]) == "jumpifn") n_openjumps++;
            if (iset.getName((*core.C)[searching_c]) == "jumphere") n_openjumps--;
            searching_c++;
          }

          if (n_openjumps>0)
            J_table[curr_c]=0; // could not find a corresponding jumphere!
          else
            J_table[curr_c]=searching_c-1; // points to the jumphere instruction (interpreter will execute next one)
        } // if name is jumpifn
        curr_c++;
      }
      never_called=false;
    };

  public:
    void clear() { never_called = true; }
    JumpIfN() : Instruction() { name="jumpifn"; DIS_flag=true; clear(); };
    ~JumpIfN() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    {
      n_ops++;
      if (core.getF()<0) 
      {
        if (never_called) 
          build_J_table(core, iset); // builds the jump-table on first call
        
        if (J_table[core.c]) // only jumps if a corresponding jumphere exists
          core.c = J_table[core.c];
        else
          n_invops++;
      }
    }
};

class JumpHere : public Instruction
{
  public:
    JumpHere() : Instruction() { name="jumphere"; DIS_flag = true; };
    ~JumpHere() {};
    inline void code(MemCore& core, InstructionSet& iset) { n_ops++; }
};

class Loop : public Instruction
{
    inline void build_L_table(MemCore& core, InstructionSet& iset)
    {
      const unsigned C_size=(*core.C).size();
      unsigned curr_c=0;
      unsigned searching_c;
      unsigned n_openloops; // counts the number of repeats without a corresponding endloop
      int depth, max_depth=0;
      
      core.L_table_addr.clear();
      core.L_table_count.clear();
      
      for (unsigned i=0;i<C_size;i++)
      {
        core.L_table_addr.push_back(0); // initializes and zeroes table
        core.L_table_count.push_back(0); // initializes and zeroes table
      }

      while (curr_c<C_size) // this loop searches for "loop" instructions in the code
      {
        if (iset.getName((*core.C)[curr_c]) == "loop") // if it's a "loop", searches for the corresponding endloop
        {
          depth=1;
          n_openloops=1; // the current loop is open
          searching_c=curr_c+1; // starts at the next instruction
          while ( (n_openloops>0) && (searching_c<C_size) ) // searches for the corresponding jumphere
          {
            if (iset.getName((*core.C)[searching_c]) == "loop") { n_openloops++; depth++; }
            if (iset.getName((*core.C)[searching_c]) == "endloop") n_openloops--;
            searching_c++;
          };

          if (n_openloops<=0) // has the corresponding "endloop" been found?
          {
            if (depth>max_depth)
              max_depth=depth;
            core.L_table_addr[curr_c]=searching_c-1;
            core.L_table_addr[searching_c-1]=curr_c; // points to the occurrence of "loop"
          }
        }
        curr_c++;
      } // end while

      if (iset.getMaxLoopDepth()>=0)
        if (max_depth>iset.getMaxLoopDepth())
          throw 0; // program has depth greater than allowed
    };

  public:
    Loop() : Instruction() { name="loop"; DIS_flag=true; };
    ~Loop() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    {
      n_ops++;
      if (core.L_table_addr.size()==0)
        build_L_table(core, iset); // builds the loop-table on first call
      
      if (core.L_table_addr[core.c]) // does this loop instruction have a corresponding endloop?
      {
        if (core.I==0) // were we asked to jump the loop block?
          core.c=core.L_table_addr[core.c]; // points c to the corresponding endloop address (interpreter will execute the following instruction)
        else
          core.L_table_count[core.c]=core.I; // we're in a loop -- set the loop counter to core.I
      }
      else
        n_invops++; // could not find endloop for this loop!
    }
};

class EndLoop : public Instruction
{
  public:
    EndLoop() : Instruction() { name="endloop"; DIS_flag = true; };
    ~EndLoop() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    {
      n_ops++;
      if (core.L_table_addr.size()>0) // does L_table_* exist?
      {
        if (core.L_table_addr[core.c]) // does this EndLoop have a corresponding Loop?
        {
          const unsigned loop_addr = core.L_table_addr[core.c];
          if (core.L_table_count[loop_addr]>1) // do we have any more loops left?
          {
            core.c = loop_addr; // points c to the corresponding "loop"
            core.L_table_count[loop_addr] -= 1; // decreases the loop counter
          }
        }
        else
          n_invops++;
      }
      else
        n_invops++;
    }
};

class Input : public Instruction
{
  public:
    Input() : Instruction() { name="input"; DIS_flag = true; };
    ~Input() {};
    inline void code(MemCore& core, InstructionSet& iset) {
      n_ops++;
      if ( (*core.input).size() == 0 ) 
      {
        double finput;
        std::cout << "Enter input #" << n_inputs+1 << ": ";
        std::cin >> finput;
        core.setF(finput);
      }
      else 
      {
        if ( n_inputs < (*core.input).size() )
          core.setF( (*core.input)[n_inputs] );
      }

      n_inputs++;
      if (!core.output_executed)
        n_inputs_bf_output++;
    }
};

class Output : public Instruction
{
  public:
    Output() : Instruction() { name="output"; DIS_flag = true; };
    ~Output() {};
    inline void code(MemCore& core, InstructionSet& iset) {
      n_ops++;
      if ( (*core.input).size() == 0 )
        std::cout << "Output #" << n_outputs+1 << ": " << core.getF() << std::endl;
      else
        (*core.output).push_back(core.getF());

      n_outputs++;
      core.output_executed=true;
    }
};

class Abs : public Instruction
{
  public:
    Abs() : Instruction() { name="abs"; DIS_flag = true; };
    ~Abs() {};
    inline void code(MemCore& core, InstructionSet& iset) { core.setF( fabs(core.getF()) ); n_ops++; }
};

class Sign : public Instruction
{
  public:
    Sign() : Instruction() { name="sign"; DIS_flag = true; };
    ~Sign() {};
    inline void code(MemCore& core, InstructionSet& iset) { core.setF( -core.getF() );  n_ops++; }
};

class Exp : public Instruction
{
  public:
    Exp() : Instruction() { name="exp"; DIS_flag = true; };
    ~Exp() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    { 
      n_ops++; 
      core.setF( exp(core.getF()) ); 
    }
};

class Log : public Instruction
{
  public:
    Log() : Instruction() { name="log"; DIS_flag = true; };
    ~Log() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    { 
      n_ops++;
      if ( !core.setF( log(core.getF()) ) )
        n_invops++;
    }
};

class Sin : public Instruction
{
  public:
    Sin() : Instruction() { name="sin"; DIS_flag = true; };
    ~Sin() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    { 
      if ( !core.setF(sin(core.getF())) )
        n_ops++; 
    }
};

class Add : public Instruction
{
  public:
    Add() : Instruction() { name="add"; DIS_flag = true; };
    ~Add() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    { 
      n_ops++;
      if (core.I < core.D_size) 
      {
        if ( core.D_saved[core.I] )
        {
          if ( !core.setF( core.getF()+core.D[core.I] ) )
            n_invops++;
        }
        else
          n_invops++;  // variable D[core.I] hasn't been saved
      }
      else
        n_invops++;  // variable D[core.I] is out of range        
    }
};

class Sub : public Instruction
{
  public:
    Sub() : Instruction() { name="sub"; DIS_flag = true; };
    ~Sub() {};
    inline void code(MemCore& core, InstructionSet& iset) {
      n_ops++;
      if (core.I < core.D_size) {
        if ( core.D_saved[core.I] )
        {
          if ( !core.setF( core.getF()-core.D[core.I] ) )
            n_invops++;
        }
        else
          n_invops++;  // variable D[core.I] hasn't been saved
      }
      else
        n_invops++;  // variable D[core.I] is out of range        
    }
};

class Mul : public Instruction
{
  public:
    Mul() : Instruction() { name="mul"; DIS_flag = true; };
    ~Mul() {};
    inline void code(MemCore& core, InstructionSet& iset) {
      n_ops++;
      if (core.I < core.D_size) {
        if ( core.D_saved[core.I] )
        {
          if ( !core.setF( core.getF()*core.D[core.I] ) )
            n_invops++;
        }
        else
          n_invops++;  // variable D[core.I] hasn't been saved
      }
      else
        n_invops++;  // variable D[core.I] is out of range        
    }
};

class Div : public Instruction
{
  public:
    Div() : Instruction() { name="div"; DIS_flag = true; };
    ~Div() {};
    inline void code(MemCore& core, InstructionSet& iset) {
      n_ops++;
      if (core.I < core.D_size) {
        if ( core.D_saved[core.I]  )
        {
          if ( !core.setF( core.getF()/core.D[core.I] ) )
            n_invops++;
        }
        else
          n_invops++;  // variable D[core.I] hasn't been saved
      }
      else
        n_invops++;  // variable D[core.I] is out of range
    }
};

class Pow : public Instruction
{
  public:
    Pow() : Instruction() { name="pow"; DIS_flag = true; };
    ~Pow() {};
    inline void code(MemCore& core, InstructionSet& iset) {
      n_ops++;
      if (core.I < core.D_size) {
        if ( core.D_saved[core.I] ) 
        {
          if ( !core.setF(pow(core.getF(),core.D[core.I])) )
            n_invops++;
        }
        else
          n_invops++;  // variable D[core.I] hasn't been saved
      }
      else
        n_invops++;  // variable D[core.I] is out of range
    }
};

class Ran : public Instruction
{
  public:
    Ran() : Instruction() { name="ran"; DIS_flag = true; };
    ~Ran() {};
    inline void code(MemCore& core, InstructionSet& iset) 
    {
      if ( !core.setF( NumericalRecipes::ran2(core.ran_ptr) ) )
        n_invops++;
      n_ops++;
    }
};

class Nop : public Instruction
{
  public:
    Nop() : Instruction() { name="nop"; DIS_flag = true; };
    ~Nop() {};
    inline void code(MemCore& core, InstructionSet& iset) { n_ops++; }
};

} // namespace DIS

} // namespace SlashA

