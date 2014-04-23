//
//  polylineFunctions.h
//  googlePolylineTest
//
//  Created by James Snook on 22/04/2014.
//  Copyright (c) 2014 James Snook. All rights reserved.
//

#ifndef googlePolylineTest_polylineFunctions_h
#define googlePolylineTest_polylineFunctions_h

typedef struct Coordinate
{
  double latitude;
  double longitude;
} Coordinate;

/* Encodes a single int32 to its polyline equivelent.
 
 val is the int to encode.
 result is the area in memory to write the result to.
 length is the amount of length that the encoding used.
 
 Discussion: the polylines can use up to 6 bytes of memory so result must 
 have at least this amount of space available. */
void encodedIntValue (int32_t val, char *result, unsigned *length);

/* Encodes a sersion of location Coordinates to a C string and passes the
   string out as the result. */
char *copyEncodedLocationsString (Coordinate *coords, unsigned coordsCount);

/* Decodes the first int32 from the given polyline pointed to by chars. */
int32_t decodeDifferenceVal (char *chars, unsigned *usedChars);

/* Decodes the polyline c string back into its coordinates. */
Coordinate *decodeLocationsString (char *polylineString, unsigned *locsCount);

#endif
