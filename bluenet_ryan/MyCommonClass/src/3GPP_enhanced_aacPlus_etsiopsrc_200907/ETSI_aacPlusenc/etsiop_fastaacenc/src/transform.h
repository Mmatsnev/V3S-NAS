/*
   MDCT Transform
 */
#include "ffr.h"
#include "ffr.h"
                  
void mdct(Word32 *dctdata,
          const Word16 *trigData,
          const Word16 *sineWindow,
          Word32 n,
          Word32 ld_n
          );

void Transform_Real(Word16 *mdctDelayBuffer,
                    Word16 *timeSignal,
                    Word16 chIncrement,     /*! channel increment */
                    Word32 *realOut,
                    Word16 *mdctScale,
                    Word16 windowSequence
                    );

