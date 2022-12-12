#ifndef SINGLEPOOL__H
#define SINGLEPOOL__H
typedef
pair<float, float> SPEnt;
class SinglePool {
 public:
   SinglePool():fNMax(10),fFilled(0) {};
   ~SinglePool() {};
   SinglePool(int nMax):fFilled(0) { fVec = vector<SPEnt>(nMax); for(auto &i:fVec) i=lNULL; fItt = fVec.begin(); fNMax = nMax; };
   void SetMaxNumber(int nMax) { fVec.clear(); fVec = vector<SPEnt>(nMax); for(auto &i:fVec) i=lNULL; fItt = fVec.begin(); fNMax = nMax; };
   vector<SPEnt> fVec;
   std::vector<SPEnt, std::allocator<SPEnt> >::iterator fItt;
   int fNMax;
   int fFilled;
   void AddValue(SPEnt newval) { *fItt=newval; fItt++; if(fItt==fVec.end()) fItt=fVec.begin(); if(fFilled<fNMax) fFilled++; };
   bool IsReady() { if(fFilled<fNMax) return false; return true;  };
   SPEnt lNULL = make_pair(-999,-999);
};
#endif
