#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "PolylineFunctions.h"

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
  int32_t prevLatVal = 0;
  int32_t prevLngVal = 0;
  char charBuff[10];
  unsigned charCount = 0;
  bool finished = false;
 
  while (true) {
    Coordinate nextCoord;
    finished  = nextLocation (instream, &nextCoord);
    if (finished) {
      /* Hey we're all done! let's return. */
      return;
    } else {
      /* We've got a new coordinate to encode let's do that. */
      encodedValue (nextCoord.latitude, &prevLatVal, charBuff, &charCount);
      charBuff[charCount] = '\0';
      fprintf (outstream, "%s", charBuff);

      encodedValue (nextCoord.longitude, &prevLngVal, charBuff, &charCount);
      charBuff[charCount] = '\0';
      fprintf (outstream, "%s", charBuff);
    }
  }
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
