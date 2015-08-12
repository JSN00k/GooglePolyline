//
//  polylineFunctions.c
//  googlePolylineTest
//
//  Created by James Snook on 21/04/2014.
//  Copyright (c) 2014 James Snook. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "polylineFunctions.h"

/* Value: The latitude or longitude to encode.
   previousIntVal: For the first call this should point at 0. It is updated
                   in the function to the value that it needs to be for
                   subsequent calls. This value must be unique to each set of
                   values you're encoding (i.e. you can't point at the same
                   thing for latitude and longitude).
   result: A buffer with at least 5 chars of space available (as this is the
           maximum number of characters that can be added).
   length: A value that returns the number of chars that were added to result.
*/
void encodedValue (double val, int32_t *previousIntVal, char *result, unsigned *length)
{
  /* Convert the current latitude and longitude to their integer
     representation. */
  int32_t intVal = round (val * 1e5);
  int32_t diffVal = intVal - *previousIntVal;
  *previousIntVal = intVal;
  
  bool isNeg = diffVal < 0;
  /* Shift the value right by 1 to make room for the sign bit on the right 
     hand side. */
  diffVal <<= 1;
  
  if (isNeg) {
    /* As the value is stored as a twos compliment value small values have a 
       lot of bits set so not the value. This will also flip the value of the
       sign bit so when we come to decode the value we will know that it is 
       negative. */
    diffVal = ~diffVal;
  }
  
  unsigned count = 0;
  
  do {
    /* get the smallest 5 bits from our value and add them to the charaters. */
    result[count] = diffVal & 0x1f;
    
    /* We've saved the last 5 bits we can remove them from the value. We shift
       the value by 5 meaning that the next 5 bits that we need to save will be
       at the end of the value. */
    diffVal >>= 5;
    
    if (diffVal) {
      /* We have more bits to encode, so we need to set the continuation bit. */
      result[count] |= 0x20;
    }
    
    result[count] += 63;
    
    ++count;
  } while (diffVal);

  if (!length)
    printf ("required value `length` not set in `encodeIntValue`");
  else
    *length = count;
}

char *copyEncodedLocationsString (Coordinate *coords, unsigned coordsCount)
{
  int32_t previousIntLat = 0;
  int32_t previousIntLng = 0;
  unsigned resultCount = 0;
  
  /* To ensure that we can encode one value we need at least 10 characters, 
     which is why we plus 7. */
  unsigned resultLength = 3 * coordsCount + 7;
  /* Hopefully this should be enough that we don't have to allocate more data
     later. */
  char *result = malloc (resultLength);
  
  unsigned encodedLen;
  
  for (int i = 0; i < coordsCount; ++i) {
    /* Encode the difference between the current integer representation, and
       the previous integer representaion. */
    encodedValue (coords[i].latitude,
                     &previousIntLat,
                     result + resultCount,
                     &encodedLen);
    resultCount += encodedLen;
    
    /* Then do the same for the latitudes. */
    encodedValue (coords[i].longitude,
                     &previousIntLng,
                     result + resultCount,
                     &encodedLen);
    resultCount += encodedLen;
    
    if (resultLength - resultCount < 10) {
      /* In this case we may overfun our results buffer, so we need to allocate
       more memory. */
      resultLength += (coordsCount - i) * 3 + 7;
      result = realloc (result, resultLength);
    }
  }
  
  result[resultCount] = '\0';
  result = realloc(result, resultCount + 1);
  
  return result;
}

int32_t decodeDifferenceVal (char *chars, unsigned *usedChars)
{
  unsigned i = 0;
  int32_t result = 0;
  char currentByte;
  
  do {
    currentByte = chars[i] - 63;
    result |= (uint32_t)(currentByte & 0x1f) << (5 * i);
    ++i;
  } while (currentByte & 0x20);
  
  if (result & 1) {
    result = ~result;
  }
  
  result >>= 1;
  
  *usedChars = i;
  return result;
}

Coordinate *decodeLocationsString (char *polylineString, unsigned *locsCount)
{
  unsigned charsCount = (unsigned)strlen (polylineString);
  if (!charsCount)
    return NULL;
  
  unsigned resultLength = charsCount / 3 + 2;
  Coordinate *result = malloc (sizeof(Coordinate) * resultLength);
  
  unsigned totalUsedChars = 0;
  unsigned usedChars = 0;
  unsigned resultCount = 0;
  
  int32_t intLat = 0;
  int32_t intLng = 0;
  
  do {
    intLat += decodeDifferenceVal (polylineString + totalUsedChars, &usedChars);
    totalUsedChars += usedChars;
    
    intLng += decodeDifferenceVal (polylineString + totalUsedChars, &usedChars);
    totalUsedChars += usedChars;
    
    result[resultCount].latitude  = intLat * 1e-5;
    result[resultCount].longitude = intLng * 1e-5;
    
    ++resultCount;
    
    if (resultCount >= resultLength - 1) {
      /* We know how many chars we've used, and we know how many lat/longs we've
         got. Assume that we'll continue to have this many chars/latlong and 
         calculate the number of lat/longs we think we'll get in totall, add a 
         small amount in the hope that we're less likely to need to realloc 
         again. */
      float charsPerLatLong = (float)totalUsedChars / resultCount;
      resultLength += (charsCount - totalUsedChars) / charsPerLatLong + 10;
      result = realloc (result, resultLength * sizeof(Coordinate));
    }
    
  } while (totalUsedChars != charsCount);
  
  result = realloc (result, resultCount * sizeof(Coordinate));
  *locsCount = resultCount;
  return result;
}
