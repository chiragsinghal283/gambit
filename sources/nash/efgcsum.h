//
// FILE: efgcsum.h -- Interface to Constant Sum Extensive Form Game Solver 
//
// $Id$
//

#ifndef EFGCSUM_H
#define EFGCSUM_H

#include "base/base.h"
#include "algutils.h"
#include "numerical/tableau.h"
#include "numerical/lpsolve.h"
#include "behavsol.h"

class CSSeqFormParams : public AlgParams {
public:
  CSSeqFormParams(void);
};

#include "subsolve.h"

#ifdef UNUSED
class efgLpSolve : public SubgameSolver  {
private:
  long npivots;
  CSSeqFormParams params;

  void SolveSubgame(const FullEfg &, const EFSupport &,
		    gList<BehavSolution> &, gStatus &);
  EfgAlgType AlgorithmID(void) const { return algorithmEfg_LP_EFG; }    

public:
  efgLpSolve(const CSSeqFormParams &, int max = 0);
  virtual ~efgLpSolve();
  
  long NumPivots(void) const  { return npivots; }
};
#endif  // UNUSED

#endif    // EFGCSUM_H


