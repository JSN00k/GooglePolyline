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

struct PolylineEncoder;
typedef struct PolylineEncoder PolylineEncoder;

/* Creates a polyline info structure so that you can stream locations for
   encoding, or chars for decoding into it. */
PolylineEncoder *PolylineEncoderCreate ();

void PolylineEncoderFree (PolylineEncoder *encoder);

/* Encodes a coordinate to a polyline string. If you have previously
   encoded a coordinate using this method it will encode the new
   coordinate as if you're continuing the polyline from the last coordinate
   encoded. result must have enough space to contain the characters, this is
   at most 10 characters. 
   returns the number characters that have been written to result. 
   Use this function if you want to manage the storage of the chars 
   yourself. If you use this function encoder WON'T store the encoded
   string, you shouldn't mix calls to this method with calls to 
   PolylineEncoderEncodeCoordinate  with the same encoder. */
unsigned PolylineEncoderGetEncodedCoordinate (PolylineEncoder *encoder,
                                              Coordinate coord,
                                              char *result);

/* Encodes a coordinate and appends the encoded character to the internal
   string respresentation, the entire encoded string can be got using
   PolylineEncoderCopyEncodedString() */
void PolylineEncoderEncodeCoordinate (PolylineEncoder *encoder, Coordinate coord);

/* Encoded a goup of coordinates. Use PolylineEncoderCopyEncodedString() to 
   get the encoded string. */
void PolylineEncoderEncodeCoordintates (PolylineEncoder *encoder,
                                        Coordinate *coords,
                                        unsigned coordCount);

/* Returns the encoded polyline from the encoder. Your code has ownership
   of this string. */
char *PolylineEncoderCopyEncodedString (PolylineEncoder *encoder);

/* Encodes all the coordinates passed to the function. 
   Returns the encoded C string */
char *copyEncodedLocationsString (Coordinate *coords, unsigned coordsCount);

void PolylineEncoderDecoderCoordinates (PolylineEncoder *encoder,
                                        char *encodedString,
                                        unsigned *decodedCoordCount);

/* Decodes as many Coordinates as possible from the passed in string.
   PolylineEncoder: The encoder being used to decode the string.
   encodedString: The string section to decode, this doesn't have to
                  be the whole of the string.
   decodedCoordCount: The number of coordinates that have been decoded.
   return: returns a pointer to the decoded coordinates.
   discussion: In the case that you're sending the string in chunks
               the encoder man not be able to decode the last few charaters,
               the encoder will keep a copy of these chars and prepend them
               to the next chunk, so you don't need to worry about resending
               them yourself. */
Coordinate *PolylineEncoderGetDecodedCoordinates (PolylineEncoder *encoder,
                                                  char *encodedString,
                                                  unsigned *decodedCount);

/* Decodes the polyline c string back into its coordinates. */
Coordinate *decodeLocationsString (char *polylineString, unsigned *locsCount);

#endif
