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

/* 
 Value: The latitude or longitude to encode.
 previousIntVal: For the first call this should point at 0. It is updated
                 in the function to the value that it needs to be for
                 subsequent calls. This value must be unique to each set of
                 values you're encoding (i.e. you can't point at the same
                 thing for latitude and longitude).
 result: A buffer with at least 5 chars of space available (as this is the
         maximum number of characters that can be added).
 length: A value that returns the number of chars that were added to result.
*/
void encodedValue (double val, int32_t *previousIntVal, char *result, unsigned *length);

/* Encodes a sersion of location Coordinates to a C string and passes the
   string out as the result. */
char *copyEncodedLocationsString (Coordinate *coords, unsigned coordsCount);

/* Decodes the first int32 from the given polyline pointed to by chars. */
int32_t decodeDifferenceVal (char *chars, unsigned *usedChars);

/* Decodes the polyline c string back into its coordinates. */
Coordinate *decodeLocationsString (char *polylineString, unsigned *locsCount);

#endif
