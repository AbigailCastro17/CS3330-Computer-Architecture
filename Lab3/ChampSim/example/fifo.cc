////////////////////////////////////////////
//                                        //
//        FIFO replacement policy          //
//     								      //
//                                        //
////////////////////////////////////////////

#include "../inc/champsim_crc2.h"

#define NUM_CORE 1
#define LLC_SETS NUM_CORE*2048
#define LLC_WAYS 16

uint32_t fifo[LLC_SETS][LLC_WAYS];

// initialize replacement state
void InitReplacementState()
{
    cout << "Initialize FIFO replacement state" << endl;

    for (int i=0; i<LLC_SETS; i++) {
        for (int j=0; j<LLC_WAYS; j++) {
            fifo[i][j] = 0;
        }
    }
}

// find replacement victim
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSet (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
  uint32_t max = fifo[set][0];
  uint32_t index = 0;
  for (int i=0; i<LLC_WAYS; i++){
    if (fifo[set][i] > max){
      max = fifo[set][i];
      index = i;
    }
  }
  return index;
}

// called on every cache hit and cache fill
void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
  // update lru replacement state
  if (hit) {
    for (uint32_t i=0; i<LLC_WAYS; i++) {
      fifo[set][i]++;
    }
     
  }else{
    for (uint32_t i=0; i<LLC_WAYS; i++) {
      fifo[set][i]++;
    }
    fifo[set][way] = 0; // promote to the MRU position  
  }
}

// use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat()
{

}

// use this function to print out your own stats at the end of simulation
void PrintStats()
{

}
