/* This #define makes it so the 'fileno' function is declared in stdio.h 
   It's not part of the C standard so making it work on Windows may
   require some extra work. */
#define _POSIX_C_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "polylineFunctions.h"

/* result is passed in as a pointer as it makes it easy to set it to NULL
   at the end of the file. */
bool nextLocation (FILE *in, Coordinate *result) {
  double lat, lng;
  int count = fscanf (in, "%lf, %lf", &lat, &lng);
  if (count == EOF) {
    /* We've got to the end of the file, so we're done! */
    result = NULL;
    return true;
  } else if (count < 2) {
    fprintf (stderr, "count = %d\n", count);
    fprintf (stderr, "first = %f", lat);
    fprintf (stderr, "second = %f", lng);
    /* For no apparent reason we've not read enough enough values assume 
       malformed input. */
    fprintf (stderr, "Malformed input, so stopped.\n");
    exit (1);
  } else {
    result->latitude = lat;
    result->longitude = lng;
  }

  return false;
}

void encodeLocations (FILE *instream, FILE *outstream)
{
  PolylineEncoder *encoder = PolylineEncoderCreate ();
  char charBuffer[11];
  while (true) {
    Coordinate nextCoord;
    bool finished = nextLocation (instream, &nextCoord);
    if (finished) {
      /* Hey we're all done! let's return. */
      return;
    } else {
      /* We've got a new coordinate to encode let's do that. */
      unsigned charCount = PolylineEncoderGetEncodedCoordinate (encoder,
                                                                 nextCoord,
                                                                 charBuffer);
      
      charBuffer[charCount] = '\0';
      fprintf (outstream, "%s", charBuffer);
    }
  }
}

void decodeLocations (FILE *instream, FILE *outstream) {
  PolylineEncoder *encoder = PolylineEncoderCreate ();
  char polylineChars[128];
  unsigned charsCount = fread (polylineChars, sizeof (char), 127, instream);
  polylineChars[127] = '\0';
  unsigned decodedCount;
  
  do {
    Coordinate *decoded = PolylineEncoderGetDecodedCoordinates (encoder,
                                                                polylineChars,
                                                                &decodedCount);
    if (decodedCount) {
      for (int i = 0; i < decodedCount; ++i) {
        fprintf (outstream, "%lf, %lf\n",
                 decoded[i].latitude, decoded[i].longitude);
      }
    }
    
    if (charsCount < 127) {
      /* We've either ended or something has gone wrong!*/
      if (feof (instream)) {
        /* Great we got to the end of the file without any issues let's return. */
        return;
      } else {
        fprintf (stderr, "Failed to read characters from the input stream.");
        exit (1);
      }
    }
  } while (true);
}

int main (int argc, char **argv) {
  if (isatty (fileno(stdin))) {
    printf ("I've currently only written an implementation for "
            "recieving a piped input so whatever you just tried won't work!\n");
    return 0;
  }
  encodeLocations (stdin, stdout);
  return 0;
}
