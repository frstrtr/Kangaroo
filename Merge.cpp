/*
* This file is part of the BSGS distribution (https://github.com/JeanLucPons/Kangaroo).
* Copyright (c) 2020 Jean Luc PONS.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Kangaroo.h"
#include <fstream>
#include "SECPK1/IntGroup.h"
#include "Timer.h"
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#ifndef WIN64
#include <pthread.h>
#endif

using namespace std;

bool Kangaroo::MergeTable(TH_PARAM* p) {

  uint64_t point = (p->hStop- p->hStart) / 16;
  uint64_t pointPrint = 0;

  for(uint64_t h = p->hStart; h < p->hStop && !endOfSearch; h++) {

    hashTable.ReAllocate(h,p->h2->E[h].maxItem);

    for(uint32_t i = 0; i < p->h2->E[h].nbItem && !endOfSearch; i++) {

      // Add
      ENTRY* e = p->h2->E[h].items[i];
      int addStatus = hashTable.Add(h,e);
      switch(addStatus) {

      case ADD_OK:
        break;

      case ADD_DUPLICATE:
        collisionInSameHerd++;
        break;

      case ADD_COLLISION:
        Int dist;
        dist.SetInt32(0);
        uint32_t kType = (e->d.i64[1] & 0x4000000000000000ULL) != 0;
        int sign = (e->d.i64[1] & 0x8000000000000000ULL) != 0;
        dist.bits64[0] = e->d.i64[0];
        dist.bits64[1] = e->d.i64[1];
        dist.bits64[1] &= 0x3FFFFFFFFFFFFFFFULL;
        if(sign) dist.ModNegK1order();

        CollisionCheck(&dist,kType);
        break;

      }

    }

    if(!endOfSearch) {
      pointPrint++;
      if(pointPrint > point) {
        ::printf(".");
        pointPrint = 0;
      }
    }

  }

  return true;

}


// Threaded proc
#ifdef WIN64
DWORD WINAPI _mergeThread(LPVOID lpParam) {
#else
void* _mergeThread(void* lpParam) {
#endif
  TH_PARAM* p = (TH_PARAM*)lpParam;
  p->obj->MergeTable(p);
  p->isRunning = false;
  return 0;
}

void Kangaroo::MergeWork(std::string& file1,std::string& file2,std::string& dest) {

  double t0;
  double t1;
  uint32_t v1;
  uint32_t v2;

  // ---------------------------------------------------

  ::printf("Loading: %s\n",file1.c_str());

  t0 = Timer::get_tick();

  FILE* f1 = ReadHeader(file1,&v1);
  if(f1 == NULL)
    return;

  uint32_t dp1;
  Point k1;
  uint64_t count1;
  double time1;
  Int RS1;
  Int RE1;

  // Read global param
  ::fread(&dp1,sizeof(uint32_t),1,f1);
  ::fread(&RS1.bits64,32,1,f1); RS1.bits64[4] = 0;
  ::fread(&RE1.bits64,32,1,f1); RE1.bits64[4] = 0;
  ::fread(&k1.x.bits64,32,1,f1); k1.x.bits64[4] = 0;
  ::fread(&k1.y.bits64,32,1,f1); k1.y.bits64[4] = 0;
  ::fread(&count1,sizeof(uint64_t),1,f1);
  ::fread(&time1,sizeof(double),1,f1);

  k1.z.SetInt32(1);
  if(!secp->EC(k1)) {
    ::printf("MergeWork: key1 does not lie on elliptic curve\n");
    fclose(f1);
    return;
  }

  t1 = Timer::get_tick();

  // Read hashTable
  hashTable.LoadTable(f1);
  ::printf("MergeWork: [HashTable1 %s] [%s]\n",hashTable.GetSizeInfo().c_str(),GetTimeStr(t1 - t0).c_str());

  fclose(f1);

  // ---------------------------------------------------

  ::printf("Loading: %s\n",file2.c_str());

  t0 = Timer::get_tick();

  FILE* f2 = ReadHeader(file2,&v2);
  if(f2 == NULL)
    return;

  uint32_t dp2;
  Point k2;
  uint64_t count2;
  double time2;
  Int RS2;
  Int RE2;

  // Read global param
  ::fread(&dp2,sizeof(uint32_t),1,f2);
  ::fread(&RS2.bits64,32,1,f2); RS2.bits64[4] = 0;
  ::fread(&RE2.bits64,32,1,f2); RE2.bits64[4] = 0;
  ::fread(&k2.x.bits64,32,1,f2); k2.x.bits64[4] = 0;
  ::fread(&k2.y.bits64,32,1,f2); k2.y.bits64[4] = 0;
  ::fread(&count2,sizeof(uint64_t),1,f2);
  ::fread(&time2,sizeof(double),1,f2);

  if(v1 != v2) {
    ::printf("MergeWork: cannot merge workfile of different version\n");
    fclose(f2);
    return;
  }

  k2.z.SetInt32(1);
  if(!secp->EC(k2)) {
    ::printf("MergeWork: key2 does not lie on elliptic curve\n");
    fclose(f2);
    return;
  }

  if(!RS1.IsEqual(&RS2) || !RE1.IsEqual(&RE2)) {

    ::printf("MergeWork: File range differs\n");
    ::printf("RS1: %s\n",RS1.GetBase16().c_str());
    ::printf("RE1: %s\n",RE1.GetBase16().c_str());
    ::printf("RS2: %s\n",RS2.GetBase16().c_str());
    ::printf("RE2: %s\n",RE2.GetBase16().c_str());
    fclose(f2);
    return;

  }

  if(!k1.equals(k2)) {

    ::printf("MergeWork: key differs, multiple keys not yet supported\n");
    fclose(f2);
    return;

  }

  t1 = Timer::get_tick();

  // Read hashTable
  HashTable* h2 = new HashTable();
  h2->LoadTable(f2);
  ::printf("MergeWork: [HashTable2 %s] [%s]\n",h2->GetSizeInfo().c_str(),GetTimeStr(t1 - t0).c_str());

  fclose(f2);

  endOfSearch = false;

  // Set starting parameters
  keysToSearch.clear();
  keysToSearch.push_back(k1);
  keyIdx = 0;
  collisionInSameHerd = 0;
  rangeStart.Set(&RS1);
  rangeEnd.Set(&RE1);
  InitRange();
  InitSearchKey();

  t0 = Timer::get_tick();

  int nbCore = Timer::getCoreNumber();
  int l2 = (int)log2(nbCore);
  int nbThread = (int)pow(2.0,l2);
  int stride = HASH_SIZE / nbThread;

  ::printf("Thread: %d\n",nbThread);
  ::printf("Merging");

  TH_PARAM* params = (TH_PARAM*)malloc(nbThread * sizeof(TH_PARAM));
  THREAD_HANDLE* thHandles = (THREAD_HANDLE*)malloc(nbThread * sizeof(THREAD_HANDLE));
  memset(params,0,nbThread * sizeof(TH_PARAM));

  for(int i = 0; i < nbThread; i++) {
    params[i].threadId = i;
    params[i].isRunning = true;
    params[i].h2 = h2;
    params[i].hStart = i * stride;
    params[i].hStop = (i + 1) * stride;
    thHandles[i] = LaunchThread(_mergeThread,params + i);
  }
  JoinThreads(thHandles,nbThread);
  FreeHandles(thHandles,nbThread);

  t1 = Timer::get_tick();

  if(!endOfSearch) {

    ::printf("Done [%.3fs]",(t1 - t0));

    // Write the new work file
    dpSize = (dp1 < dp2) ? dp1 : dp2;
    workFile = dest;
    SaveWork(count1 + count2,time1 + time2,NULL,0);

  }

  ::printf("Dead kangaroo: %d\n",collisionInSameHerd);
  ::printf("Total f1+f2: count 2^%.2f [%s]\n",log2((double)count1 + (double)count2),GetTimeStr(time1 + time2).c_str());

}

