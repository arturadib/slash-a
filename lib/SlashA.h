/*
 *
 *  SlashA.h
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

#ifndef SLASHA_INCLUDED // duplicate protection
#define SLASHA_INCLUDED

#include <string>
#include <vector>
#include <cmath>

namespace SlashA
{

  const std::string revNumber = "10";

  const unsigned maxWordLen = 32; // Maximum length of each instruction word

  typedef unsigned ByteCode_Type;
  typedef std::vector<ByteCode_Type> ByteCode;

  /* Classes */
  
  class MemCore
  {
    private:
      double F; // F-register
    public:
      unsigned I; // I-register
      ByteCode* C; // program tape
      unsigned c; // program tape position
      double* D; // data tape
      unsigned D_size;
      bool* D_saved; // saved/unsaved flag for each Data element

      unsigned* L; // label tape
      unsigned L_size;
      bool* L_saved; // analogous to D_saved

      std::vector<unsigned> L_table_addr; // Loop-table containing the addresses of the corresponding EndLoop instructions
      std::vector<unsigned> L_table_count; // Loop-table containing the loop counters

      std::vector<double>* input; // input buffer
      std::vector<double>* output; // output buffer
      bool output_executed; // a flag that tells if any output instruction has been executed so far
      
      long* ran_ptr;
      
  // Methods:
      MemCore(const unsigned _Dsize, 
              const unsigned _Lsize,
              std::vector<double>& _input,
              std::vector<double>& _output);
      ~MemCore();
      
      inline double getF() { return F; }
      inline bool setF(double f) // protects F against assignment of invalid values
      {
        if (std::isnan(f) || std::isinf(f))
          return false;
        else
          F = f;
        return true;
      }
  };
  
  class InstructionSet; // prototype

  class Instruction
  {
    protected:
      std::string name; // to be defined in the derived classes
      bool DIS_flag; // is this a DIS instruction? (Default Instruction Set)
      unsigned n_ops; // number of operations executed with this instruction
      unsigned n_invops; // number of invalid operations executed with the given instruction
      unsigned n_inputs; // number of executed input instructions
      unsigned n_outputs; // number of executed output instructions
      unsigned n_inputs_bf_output; // number of executed input instructions before the first output instruction
    public:
      Instruction() { clearCounters(); DIS_flag = false; }
      virtual ~Instruction() {}

      virtual inline void code(MemCore& core, InstructionSet& iset) { throw (std::string)"Instruction not properly initialized! (method code() undefined)"; } // to be defined in the derived class (i.e. specific instruction)

      bool isDIS() { return DIS_flag; } 
      std::string getName() { return name; }
      unsigned getOps() { return n_ops; }
      unsigned getInvops() { return n_invops; }
      unsigned getInputs() { return n_inputs; }
      unsigned getOutputs() { return n_outputs; }
      unsigned getInputsBeforeOutput() { return n_inputs_bf_output; }
      void clearCounters() { n_ops=0; n_invops=0; n_inputs=0; n_outputs=0; n_inputs_bf_output=0; }
      virtual void clear() {};
      void clearAll() { clearCounters(); clear(); }
  };

  class InstructionSet
  {
    private:
      std::vector<Instruction*> set;
      void insert_DIS_numeric(ByteCode_Type n_num);
      void remove_DIS();
      unsigned n_numericinst;
      int maxloopdepth;
    public:
      InstructionSet(ByteCode_Type n_num) { maxloopdepth=-1; n_numericinst=n_num; insert_DIS_numeric(n_num); }
      ~InstructionSet() { remove_DIS(); }

      void insert_DIS_IO(); // input/output commands
      void insert_DIS_memreg(); // memory-register commands
      void insert_DIS_regreg(); // register-register commands
      void insert_DIS_gotos(); // flow-control: goto commands
      void insert_DIS_jumps(); // flow-control: jump commands
      void insert_DIS_loops(); // flow-control: loop commands
      void insert_DIS_basicmath(); // basic math operations
      void insert_DIS_advmath(); // advanced math functions
      void insert_DIS_misc(); // everything else
      void insert_DIS_full(); // inserts all of the above (with the exception of _DIS_numeric)
      void insert_DIS_full_minus_Gotos(); // avoids infinite loops

      void insert(Instruction* inst) { set.push_back(inst); } // inserts a user-defined instruction
      void exec(unsigned inst_num, MemCore& core) { set[inst_num]->code(core, (*this)); }

      std::string listAll() { std::string s = ""; for (unsigned i=0;i<set.size();i++) s+=set[i]->getName()+'/'; return s + '.'; }
      std::string getName(int inst_num) { return set[inst_num]->getName(); }
      unsigned getOps(int inst_num) { return set[inst_num]->getOps(); }
      unsigned getInvops(int inst_num) { return set[inst_num]->getInvops(); }
      unsigned getTotalOps()
        { unsigned n=0; for (unsigned i=0;i<set.size();i++) n+=set[i]->getOps(); return n; };
      unsigned getTotalInvops() 
        { unsigned n=0; for (unsigned i=0;i<set.size();i++) n+=set[i]->getInvops(); return n; };
      unsigned getTotalInputs() 
        { unsigned n=0; for (unsigned i=0;i<set.size();i++) n+=set[i]->getInputs(); return n; };
      unsigned getTotalOutputs() 
        { unsigned n=0; for (unsigned i=0;i<set.size();i++) n+=set[i]->getOutputs(); return n; };
      unsigned getTotalInputsBFOutput() 
        { unsigned n=0; for (unsigned i=0;i<set.size();i++) n+=set[i]->getInputsBeforeOutput(); return n; };
      void clear() { for (unsigned i=0;i<set.size();i++) set[i]->clearAll(); }
      unsigned size() { return (ByteCode_Type)set.size(); }
      unsigned numericInstructions() { return n_numericinst; }
      int getMaxLoopDepth() { return maxloopdepth; }
      void setMaxLoopDepth(unsigned ldepth) { maxloopdepth=ldepth; }
  };

  /* Functions */

  std::string getHeader();

  bool runByteCode(InstructionSet& iset,
                   MemCore& core,
                   ByteCode& bc,
                   long randseed,
                   long max_rtime,
                   int max_loop_depth);

  ByteCode_Type instruction2ByteCode( std::string inst, 
                                      InstructionSet& iset );
                                      
  void source2ByteCode( std::string src,
                        ByteCode& bc,
                        InstructionSet& iset );

  void bytecode2Source( ByteCode& bc,
                        std::string& src,
                        InstructionSet& iset );
  
}; // namespace SlashA

#endif // SLASHA_INCLUDED
