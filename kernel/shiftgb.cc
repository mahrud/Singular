/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: shiftgb.cc,v 1.2 2007-06-24 16:44:41 levandov Exp $ */
/*
* ABSTRACT: kernel: utils for shift GB and free GB
*/

#include "mod2.h"
#include "febase.h"
#include "ring.h"
#include "polys.h"
#include "numbers.h"
#include "ideals.h"
#include "matpol.h"
#include "kbuckets.h"
#include "kstd1.h"
#include "sbuckets.h"
#include "p_Mult_q.h"
#include "kutil.h"
#include "structs.h"
#include "omalloc.h"
#include "khstd.h"
#include "kbuckets.h"
#include "weight.h"
#include "intvec.h"
#include "structs.h"
#include "kInline.cc"
#include "stairc.h"
#include "weight.h"
#include "intvec.h"
#include "timer.h"
#include "shiftgb.h"
#include "sca.h"


#define freeT(A,v) omFreeSize((ADDRESS)A,(v+1)*sizeof(int))

poly pLPshift(poly p, int sh, int uptodeg, int lV)
{
  /* assume shift takes place */
  /* shifts the poly p by sh */

  /* assume sh and uptodeg agree */

  if (sh == 0) return(p); /* the zero shift */

  poly q  = NULL;
  poly pp = pCopy(p);
  while (pp!=NULL)
  {
    q = p_Add_q(q, pmLPshift(pp,sh,uptodeg,lV),currRing);
    pIter(pp);
  }
  /* delete pp? */
  /* int version: returns TRUE if it was successful */
  return(q);
}

poly pmLPshift(poly p, int sh, int uptodeg, int lV)
{
  /* pm is a monomial */

  if (sh == 0) return(p); /* the zero shift */

  if (sh < 0 )
  {
#ifdef PDEBUG
    Print("pmLPshift: negative shift requested");
#endif
    return(NULL); /* violation, 2check */
  }

  int L = pmLastVblock(p,lV);
  if (L+sh-1 > uptodeg)
  {
#ifdef PDEBUG
    Print("pmLPshift: too big shift requested");
#endif
    return(NULL); /* violation, 2check */
  }
  int *e=(int *)omAlloc0((currRing->N+1)*sizeof(int));
  int *s=(int *)omAlloc0((currRing->N+1)*sizeof(int));
  pGetExpV(p,e);
  number c = pGetCoeff(p);
  int j;
  for (j=1; j<=currRing->N; j++)
  {
    if (e[j])
    {
      s[j + (sh*lV)] = e[j]; /* actually 1 */
    }
  }
  poly m = pOne();
  pSetExpV(m,s);
  /*  pSetm(m); */ /* done in the pSetExpV */
  pSetCoeff0(m,c);
  freeT(e, currRing->N);
  freeT(s, currRing->N);
  return(m);
}

int pLastVblock(poly p, int lV)
{
  /* returns the number of maximal block */
  /* appearing among the monomials of p */
  poly q = pCopy(p); /* need it ? */
  int ans = 0; 
  int ansnew = 0;
  while (q!=NULL)
  {
    ansnew = pmLastVblock(q,lV);
    ans    = si_max(ans,ansnew);
    pIter(q);
  }
  /* do not need to delete q */
  return(ans);
}

int pmLastVblock(poly p, int lV)
{
  /* for a monomial p, returns the number of the last block */
  /* where a nonzero exponent is sitting */
  int *e=(int *)omAlloc0((currRing->N+1)*sizeof(int));
  pGetExpV(p,e);
  int j,b;
  j = currRing->N;
  while ( (!e[j]) && (j>=1) ) j--;
  if (j==0) 
  {
#ifdef PDEBUG
    Print("pmLastVblock: unexpected zero exponent");
#endif   
    return(j);
  }
  b = (int)(j/lV) + 1; /* the number of the block, >=1 */
  return (b);
}

int isInV(poly p, int lV)
{
  if (lV <= 0) return(0);
  /* returns 1 iff p is in V */
  /* that is in each block up to a certain one there is only one nonzero exponent */
  /* lV = the length of V = the number of orig vars */
  int *e = (int *)omAlloc0((currRing->N+1)*sizeof(int));
  int  b = (int)(currRing->N)/lV; /* the number of blocks */
  int *B = (int *)omAlloc0((b+1)*sizeof(int)); /* the num of elements in a block */
  pGetExpV(p,e);
  int i,j;
  for (j=1; j<=b; j++)
  {
    /* we go through all the vars */
    /* by blocks in lV vars */
    for (i=(j-1)*lV + 1; i<= j*lV; i++)
    {
      if (!e[i]) B[j] = B[j]+1;
    }
  }
  j = b;
  while ( (!B[j]) && (j>=1)) j--;
  if (j==0)
  {
    /* it is a zero exp vector, which is in V */
    return(1);
  }
  /* now B[j] != 0 */
  for (j; j>=1; j--)
  {
    if (B[j]!=1)
    {
      return(0);
    }
  }
  return(1);
}

/* shiftgb stuff */

void initBbaShift(ideal F,kStrategy strat)
{
  int i;
  idhdl h;
 /* setting global variables ------------------- */
  strat->enterS = enterSBba;

  strat->red = redFirstShift;

  /* perhaps the following?
   *    strat->LazyPass *=4;
   *    strat->red = redHomogShift;
   */

  /*    strat->red = redHoney;
   *  if (strat->honey)
   *    strat->red = redHoney;
   *  else if (pLexOrder && !strat->homog)
   *    strat->red = redLazy;
   *  else
   *  {
   *    strat->LazyPass *=4;
   *    strat->red = redHomog;
   *  }
   *#ifdef HAVE_RINGS  //TODO Oliver
   *  if (rField_is_Ring(currRing)) {
   *    strat->red = redRing2toM;
   *  }
   *#endif
  */

  if (pLexOrder && strat->honey)
    strat->initEcart = initEcartNormal;
  else
    strat->initEcart = initEcartBBA;
  if (strat->honey)
    strat->initEcartPair = initEcartPairMora;
  else
    strat->initEcartPair = initEcartPairBba;
  strat->kIdeal = NULL;
  //if (strat->ak==0) strat->kIdeal->rtyp=IDEAL_CMD;
  //else              strat->kIdeal->rtyp=MODUL_CMD;
  //strat->kIdeal->data=(void *)strat->Shdl;
  if ((TEST_OPT_WEIGHTM)&&(F!=NULL))
  {
    //interred  machen   Aenderung
    pFDegOld=pFDeg;
    pLDegOld=pLDeg;
    //h=ggetid("ecart");
    //if ((h!=NULL) /*&& (IDTYP(h)==INTVEC_CMD)*/)
    //{
    //  ecartWeights=iv2array(IDINTVEC(h));
    //}
    //else
    {
      ecartWeights=(short *)omAlloc((pVariables+1)*sizeof(short));
      /*uses automatic computation of the ecartWeights to set them*/
      kEcartWeights(F->m,IDELEMS(F)-1,ecartWeights);
    }
    pRestoreDegProcs(totaldegreeWecart, maxdegreeWecart);
    if (TEST_OPT_PROT)
    {
      for(i=1; i<=pVariables; i++)
        Print(" %d",ecartWeights[i]);
      PrintLn();
      mflush();
    }
  }
}

