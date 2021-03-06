#ifndef BBCONE_H
#define BBCONE_H

#include <Singular/mod2.h>
#ifdef HAVE_FANS

#include <gfanlib/gfanlib.h>
#include <kernel/intvec.h>

extern int coneID;

void bbcone_setup();

/***
 * Conversion functions for data types
 **/
gfan::Integer* numberToInteger(const number &n);
number integerToNumber(const gfan::Integer &I);
bigintmat* zVectorToBigintmat(const gfan::ZVector &zv);
bigintmat* zMatrixToBigintmat(const gfan::ZMatrix &zm);
gfan::ZMatrix* bigintmatToZMatrix(const bigintmat &bim);
gfan::ZVector* bigintmatToZVector(const bigintmat &bim);

gfan::ZVector intStar2ZVector(const int d, const int* i);
char* toString(gfan::ZMatrix const &m);
std::string toString(const gfan::ZCone* const c);

/***
 * Other functions
 **/
int getDimension(gfan::ZCone* zc);
int getCodimension(gfan::ZCone* zc);
int getLinealityDimension(gfan::ZCone* zc);
gfan::ZVector randomPoint(const gfan::ZCone* zc);
gfan::ZCone liftUp(const gfan::ZCone &zc);

#endif
#endif
