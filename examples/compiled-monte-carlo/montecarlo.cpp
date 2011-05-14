//
// This C++ program implements the Slash/A language as inlines and performs the
// Monte Carlo calculation (montecarlo.sla) in compiled form. This is intended to
// compare interpreted vs. compiled runtimes.
//  

#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include "NR-ran2.h"

using namespace std;

const unsigned nvar = 32768;

double D[nvar];
bool D_saved[nvar];
double F = 0;
unsigned I = 0;
long rseed = -2237;

// Necessary instructions from DIS
inline void seti(unsigned arg) { I = arg; };
inline void itof() { F = (double)I; };
inline void mul() { if (D_saved[I]) F = F*D[I]; };
inline void add() { if (D_saved[I]) F = F+D[I]; };
inline void sub() { if (D_saved[I]) F = F-D[I]; };
inline void ran() { F = NumericalRecipes::ran2(&rseed); }
inline void inc() { F = F+1; }
inline void dec() { F = F-1; }
inline void pow() { F = pow(F,D[I]); }
inline void div()
{
  if (I < nvar) {
    if ( D_saved[I] ) {
      if ( D[I] == 0. ) {
        F = 0;
      }
      else
        F = F/D[I];
    }
  }
}
inline void save() 
{ 
  if (I<nvar) { 
    D[I] = F; 
    D_saved[I]=true; 
  } 
};
inline void load() 
{ 
if (I<nvar) 
  if (D_saved[I]) 
    F = D[I];
};
inline void output() { cout << "Output: " << F << endl; };
// End of DIS


int
main()
{
  for (unsigned i=0;i<nvar;i++) {
    D[i]=0;
    D_saved[i]=false;
  }
  
  seti(7); itof(); seti(0); save(); seti(10); itof(); seti(0); pow(); save();
  seti(0); itof(); seti(1); save(); 
  seti(0); itof(); seti(2); save(); 
  seti(0); 
label0:
    ran(); seti(3); save(); mul(); save();
    ran(); seti(4); save(); mul();
    seti(3); add(); save();
    seti(1); itof(); seti(3); sub();
    if (F<0) goto jumphere0;
      seti(2); load(); inc(); save();
    jumphere0:
    seti(1); load(); inc(); save();
    seti(0); load(); dec(); save();
  if (F>=0) goto label0;
  seti(2); load(); seti(1); div(); save();
  seti(4); itof(); seti(1); mul();
  output();
}
