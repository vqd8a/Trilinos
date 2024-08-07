// @HEADER
// *****************************************************************************
//                   Basker: A Direct Linear Solver package
//
// Copyright 2011 NTESS and the Basker contributors.
// SPDX-License-Identifier: LGPL-2.1-or-later
// *****************************************************************************
// @HEADER

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "basker_decl.hpp"
#include "basker_def.hpp"


using namespace std;

int main(int argc, char* argv[])
{

  typedef int Int;
  typedef double Entry;

  Int lnnz; //left guess
  Int unnz; //right guess


 /*load martrix*/
  Int annz;
  Int anrow, ancol;
  Int *Ap, *Ai;
  Entry *Ax;



  //Read in Matrix file from CSR file
  string temp;
  ifstream fp;
  fp.open(argv[1]);
  fp >> temp;
  anrow = atoi(temp.c_str());
  ancol =anrow;
  fp >> temp;
  annz = atoi(temp.c_str());

  cout << "Size: " << anrow << " nnz: " << annz << endl;
  Ap = new Int[anrow+1];
  Ai = new Int[annz];
  Ax = new Entry[annz];
  for(int i=0; i < anrow+1; i++)
    {
      string t;
      fp >> t;
      Ap[i] = atoi(t.c_str())-1;
    }
  for(int i=0; i < annz; i++)
    {
      string t;
      fp >> t;
      Ai[i] = atoi(t.c_str())-1;
      if(i == 0)
        {
          cout << "First index: " << Ai[i] << endl;
        }
    }
  for(int i=0; i < annz; i++)
    {
      string t;
      fp >> t;
      Ax[i] = atof(t.c_str());
    }


  if(argc > 2)
    {
      lnnz = atoi(argv[2]);
       unnz = atoi(argv[3]);
    }
  else
    {
      lnnz = 2*annz;
      unnz = 2*annz;
    }



  //Allocate some work space.




  cout << "Done allocating space" << endl;
  BaskerClassicNS::BaskerClassic<int, double> mybasker;
  mybasker.factor(anrow, ancol,annz, Ap, Ai, Ax);

  Int *pp;
  mybasker.returnP(&pp);


  free(pp);
  //cout << "pp(0): " << pp[0] << endl;
  //cout << "pp(2): " << pp[2] << endl;

  /*Try to solve a problem*/
  Entry *x = (Entry *)calloc(anrow, sizeof(Entry));
  Entry *b = (Entry *)calloc(anrow, sizeof(Entry));


  /*Make a fake rhs so the solution is all ones*/

  for(int i = 0; i < anrow; i++)
    {
       b[i] = (Entry) 1.0;
    }

 mybasker.solve(b, x);


  cout << "Solution:" << endl;
  for(int i = 0; i < anrow; i++)
    {
      cout << x[i] << endl;
    }



  free(x);
  free(b);
  delete [] Ap;
  delete [] Ai;
  delete [] Ax;

  //Int result = basker::basker<Int, Entry>(Ap, Ai, Ax, anrow, ancol, ws, X, Lp, &Li, &Lx, Up, &Ui, &Ux, &lnnz, &unnz, pinv);

}
