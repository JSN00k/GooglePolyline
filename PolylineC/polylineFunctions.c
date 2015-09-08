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
#include "AppendableDataStore.h"

static unsigned charsPerNode = 1024;
static unsigned coordsPerNode = 1024;

struct PolylineEncoder {
  int32_t intLat;
  int32_t intLng;
  AppendableDataStore *dataStore;
  unsigned nodeCount;
  char *unusedChars;
};

PolylineEncoder *PolylineEncoderCreate () {
  PolylineEncoder *result = calloc (1, sizeof (PolylineEncoder));
  result->dataStore = NULL;
  return result;
}

void PolylineEncoderFree (PolylineEncoder *encoder) {
  if (encoder->dataStore)
    free (encoder->dataStore);

  if (encoder->unusedChars)
    free (encoder->unusedChars);

  free (encoder);
}

/* Value: The latitude or longitude to encode.
   previousIntVal: For the first call this should point at 0. It is updated
                   in the function to the value that it needs to be for
                   subsequent calls. This value must be unique to each set of
                   values you're encoding (i.e. you can't point at the same
                   thing for latitude and longitude).
   result: A buffer with at least 5 chars of space available (as this is the
           maximum number of characters that can be added).
   charCount: Incremented by the number of characters added to result.
*/
static void encodeValue (double val, int32_t *previousIntVal,
                         char *result, unsigned *charCount);

/* string: The string to get the next difference value from, if there aren't
           enough characters to get a valid value the function will return 
           false.
   usedChars: This value is incremented by the number of characters used to
              decode the next value.
   previousIntValue: For the first call this should point at 0. It is updated
                     in the function to the value that it needs to be for
                     subsequent calls. This value must be unique to each set of
                     values you're encoding (i.e. you can't point at the same
                     thing for latitude and longitude).
   result: The decoded double value.
   n: Will stop trying to decode the value when that many chars have been read.
   return: Returns true if we successfully decoded a value false otherwise.
           We may fail to decode a value if we are streaming and we don't 
           have all the charaters needed.
*/
static bool decodenValue (char *string, unsigned *usedChars,
                         int32_t *previousIntValue, double *result,
                         size_t n);

/* Decodes a single Coordinate from the encodedString. */
bool PolylineEncoderDecodeNextCoord (PolylineEncoder *encoder,
                                     char *encodedString,
                                     size_t n,
                                     Coordinate *returnVal,
                                     unsigned *usedCharsCount);

unsigned PolylineEncoderGetEncodedCoordinate (PolylineEncoder *encoder,
                                              Coordinate coord,
                                              char *result) {
  if (!result) {
    printf ("result variable was not set. Line %d, file %s",
            __LINE__, __FILE__);
    exit (1);
  }
  
  unsigned usedChars = 0;
  encodeValue (coord.latitude, &encoder->intLat, result, &usedChars);
  encodeValue (coord.longitude, &encoder->intLng, result + usedChars, &usedChars);
  
#if DEBUG
  if (usedChars > 10) {
    printf ("Problem encoding coordinate it should never take "
            "more than 10 chars to encode two coordinates. There is a "
            "programmer error!"
            "line %d in file %s", __LINE__, __FILE__);
    exit (1);
  }
#endif
  
  return usedChars;
}

static inline void PolylineEncoderEncodeCoordinateInternal (PolylineEncoder *encoder,
                                              Coordinate coord) {
  char result[10];
  unsigned usedChars = PolylineEncoderGetEncodedCoordinate (encoder,
                                                            coord,
                                                            result);
  
  if (!encoder->dataStore)
    encoder->dataStore = AppendableDataStoreCreate (charsPerNode, sizeof (char));

  AppendableDataStoreAddData (encoder->dataStore, result, usedChars);
}

void PolylineEncoderEncodeCoordinate (PolylineEncoder *encoder, Coordinate coord) {
  PolylineEncoderEncodeCoordinateInternal (encoder, coord);
}

void PolylineEncoderEncodeCoordintates (PolylineEncoder *encoder,
                                        Coordinate *coords,
                                        unsigned coordCount) {
  for (unsigned i = 0; i < coordCount; ++i) {
    PolylineEncoderEncodeCoordinateInternal (encoder, coords[i]);
  }
}

char *copyEncodedLocationsString (Coordinate *coords, unsigned coordsCount)
{
  unsigned resultCount = 0;
  /* Take a guess that it takes about 3 characters to encode each double
     there are two doubles per coord. Then add a bit in the hope that we 
     don't need to realloc. */
  unsigned resultLength = 6 * coordsCount + 20;
  char *result = malloc (resultLength);

  PolylineEncoder *encoder = PolylineEncoderCreate ();
  
  for (int i = 0; i < coordsCount; ++i) {
    resultCount += PolylineEncoderGetEncodedCoordinate (encoder,
                                                        coords[i],
                                                        result + resultCount);
    if (resultLength - resultCount < 11) {
      /* In this case we may overrun our results buffer, so we need to allocate
         more memory. We know how many characters on average it has taken to encode
         a coordinate assuming this continues we can work out approximately how 
         many more charaters we need. */
      float charsPerCoord = (float)resultCount / i;
      /* Add a bit extra for luck (this reduces the chance that we'll have to 
         reallocate memory again. */
      resultLength = charsPerCoord * coordsCount + 20;
      result = realloc (result, resultLength);
    }
  }
  
  result[resultCount] = '\0';
  result = realloc(result, resultCount + 1);
  
  return result;
}

static inline void PolylineEncoderAppendCoordinate (PolylineEncoder *encoder,
                                                    Coordinate coord) {
  if (!encoder->dataStore) {
    encoder->dataStore = AppendableDataStoreCreate (coordsPerNode,
                                                    sizeof (Coordinate)); 
  }

  AppendableDataStoreAddData (encoder->dataStore, &coord, 1);
}

/* Part of the PolylineEncoderDecodeCoordinates function, this function
   decodes any unused chars from the previous decoding using some of
   the new characters. If there aren't enough new characters it returns
   -1. Otherwise it returns the number of new characters used. */
static inline int PolylineEncoderDecodeUnusedChars (PolylineEncoder *encoder,
                                                    char *encodedString)
{
  if (!encoder->unusedChars)
    return -1;
  
  size_t unusedLen = strlen (encoder->unusedChars);
  unsigned minStringLength = 0;
  unsigned usedChars;
  Coordinate coord;
  for (; minStringLength < 11 - unusedLen; ++minStringLength) {
    if (encodedString[minStringLength] == '\0')
      break;
  }

  if (!minStringLength) {
    /* There were no new characters in encodedString. */
    return -1;
  }

  /* The largest the resulting string can be is 10 chars. */
  strncat (encoder->unusedChars, encodedString, minStringLength);

  /* If there are less than 10 charaters in the string now it's possible
     that we still won't be able to decode the next value. */
  bool gotNextCoord = PolylineEncoderDecodeNextCoord (encoder, encodedString,
                                                      10, &coord, &usedChars);
  /* It's possoble that we still don't have enough charaters to decode
     a value. */
  if (!gotNextCoord)
    return -1;

  PolylineEncoderAppendCoordinate (encoder, coord);
  free (encoder->unusedChars);
  encoder->unusedChars = NULL;
  return usedChars - (unsigned)unusedLen;
}

void PolylineEncoderDecodeCoordinates (PolylineEncoder *encoder,
                                       char *encodedString,
                                       unsigned *decodedCoordCount) {
  Coordinate coord;
  unsigned usedChars = 0;

  *decodedCoordCount = 0;

  if (encoder->unusedChars) {
    usedChars = PolylineEncoderDecodeUnusedChars (encoder,
                                                  encodedString);
    if (usedChars <= 0) {
      *decodedCoordCount = 0;
      return;
    } else {
      *decodedCoordCount += 1;
    }

    encodedString += usedChars;
  }

  while (PolylineEncoderDecodeNextCoord (encoder, encodedString,
                                         10, &coord, &usedChars)) {
    encodedString += usedChars;
    PolylineEncoderAppendCoordinate (encoder, coord);
    *decodedCoordCount += 1;
  }

  size_t remainingLen = strlen (encodedString);
  if (!remainingLen)
    return;

  if (remainingLen > 10) {
    printf ("We exited decode a polyline for an unknow reason. "
            "line %d, file %s", __LINE__, __FILE__);
    exit (1);
  }

  encoder->unusedChars = malloc (11 * sizeof (char));
  strcpy (encoder->unusedChars, encodedString);
}

Coordinate *PolylineEncoderGetDecodedCoordinates (PolylineEncoder *encoder,
                                                  char *encodedString,
                                                  unsigned *decodedCount)
{
  PolylineEncoderDecodeCoordinates (encoder,
                                    encodedString,
                                    decodedCount);
  Coordinate *result = AppendableDataStoreGetData (encoder->dataStore);
  AppendableDataStoreFree (encoder->dataStore);
  encoder->dataStore = NULL;
  return result;
}

Coordinate *decodeLocationsString (char *polylineString, unsigned *locsCount)
{
  PolylineEncoder *encoder = PolylineEncoderCreate ();
  PolylineEncoderDecodeCoordinates (encoder,
                                    polylineString,
                                    locsCount);
  return AppendableDataStoreGetData (encoder->dataStore);
}

char *PolylineEncoderCopyEncodedString (PolylineEncoder *encoder) {
  size_t size = AppendableDataStoreDataSize (encoder->dataStore);
  char *result = malloc (size + 1);
  AppendableDataStoreCollapseDataIntoResult (encoder->dataStore, result);
  result[size] = '\0';
  return result;
}

static void encodeValue (double val, int32_t *previousIntVal,
                   char *result, unsigned *charCount)
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

  if (!charCount)
    printf ("required value `length` not set in `encodeIntValue`");
  else
    *charCount += count;
}

bool decodenValue (char *string, unsigned *usedChars,
                   int32_t *previousIntValue, double *result,
                   size_t n) {
  unsigned i = 0;
  int32_t diff = 0;
  char currentByte;

#if DEBUG
  if (!usedChars) {
    printf ("error: usedChars is cannot be NULL. "
            "line %d file %s", __LINE__, __FILE__);
    exit (1);
  }
#endif
  
  do {
    if (i >= n || string[i] == '\0')
      return false;
    
    currentByte = string[i] - 63;
    diff |= (uint32_t)(currentByte & 0x1f) << (5 * i);
    ++i;
  } while (currentByte & 0x20);

  if (diff & 1) {
    diff = ~diff;
  }

  diff >>= 1;
  
  *previousIntValue += diff;
  *result = *previousIntValue * 1e-5;

  *usedChars += i;
  return true;
}

bool PolylineEncoderDecodeNextCoord (PolylineEncoder *encoder,
                                     char *encodedString,
                                     size_t n,
                                     Coordinate *returnVal,
                                     unsigned *usedCharsCount)
{
  /* We need to be a little careful here as if we can't decode both
     locations we want to leave the intLat and intLng values as 
     they were before we tried to decode them both. */
  Coordinate result;
  unsigned usedChars = 0;
  int32_t tmpLat = encoder->intLat;

  /* Any value above 5 for the last argument will result in us getting
     a decoded value, provided the characters are in range. */
  bool success = decodenValue (encodedString, &usedChars, &encoder->intLat,
                               &result.latitude, n);
  if (!success)
    return false;

  success = decodenValue (encodedString + usedChars, &usedChars,
                          &encoder->intLng, &result.longitude,
                          n - usedChars);
  if (!success) {
    /* We need to reset the encoders intLat value as we've failed
       to decode an entire coordinate. */
    encoder->intLat = tmpLat;
    return false;
  }

  *returnVal = result;
  *usedCharsCount = usedChars;

  return true;
}
