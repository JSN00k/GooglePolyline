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
#include <getopt.h>

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
      fprintf (outstream, "\n");
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
  polylineChars[charsCount] = '\0';
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
        fprintf (outstream, "\n");
        return;
      } else {
        fprintf (stderr, "Failed to read characters from the input stream.");
        exit (1);
      }
    }

    charsCount = fread (polylineChars, sizeof (char), 127, instream);
    polylineChars[charsCount] = '\0';
  } while (true);
}

bool strcicmp (char *a, char *b) {
  unsigned place = 0;
  while (true) {
    
    int testVal = abs (a[place] - b[place]);
    if (testVal && testVal != 'a' - 'A') {
      return false;
    }

    if (a[place] == '\0') {
      if (testVal)
        return false;
      else
        return true;
    }

    ++place;
  }

  return true;
}

void usage () {
  printf ("PolylineTool: a tool for encoding and decoding Google Polylines.\n\n"
          "PolylineTool [-ioadef?]\n"
          "-i <FileName> Reads input from a file with FileName instead of stdin\n"
          "-o <FileName> Writes output to file instead of standard out. This "
          "won't automatically overwirte files if they already exist.\n"
          "-f Forces -o to overwrite the existing file.\n"
          "-a <FileName> Appends Polyline/Coordinates to the end of file "
          "FileName. If the file doesn't exist it will create it.\n"
          "-d Decode, used when you want to decode a polyline rather than encode "
          "coordinates.\n"
          "-e Encode, used to encode coordinates, this is the default so doesn't "
          "need to be used.");
          
  exit(1);
}

int main (int argc, char **argv) {
  char ch;
  FILE *input = stdin;
  bool noInput = true;
  FILE *output = stdout;
  bool fileAlreadyExists = false;
  bool dontCareIfFileAlreadyExists = false;
  bool hadOpenFileArg = false;
  char *outputFileStr = NULL;
  bool decode = false;
  
  while ((ch = getopt (argc, argv, "i:o:a:def")) != -1) {
    switch (ch) {
    case 'i':
      if (access (optarg, R_OK) == -1) {
        fprintf (stderr, "Unable to open input file for reading. "
                 "Check file exists.");
        exit (1);
      }

      input = fopen (optarg, "r");
      noInput = false;
      if (!input) {
        fprintf (stderr, "Couldn't open input file");
        exit (1);
      }
      break;
    case 'o':
      if (hadOpenFileArg) {
        /* We've already either had a 'o' or 'a' arg so can't have another one. */
        fprintf (stderr, "Can't run with arguments -a and -o\n");
        usage ();
      }

      hadOpenFileArg = true;
      if (!access (optarg, W_OK)) {
        /* This isn't a big deal if the -f option is set as this 
           forcefully overwrites the output file that's given.
           if the -f option isn't set then we need to check that
           we should overwrite the file before doing anything (if we're
           on the terminal, otherwise give an error message). */
        fileAlreadyExists = true;
        outputFileStr = malloc (strlen (optarg) * sizeof (char));
        strcpy (outputFileStr, optarg);
      } else {
        output = fopen (optarg, "w");
        if (!output) {
          fprintf (stderr, "Couldn't open output file.\n");
          usage ();
          exit (1);
        }
      }
      break;
    case 'a':
      output = fopen (optarg, "a");
      if (!output) {
        fprintf (stderr, "Couldn't open output file.\n");
        usage ();
        exit (1);
      }
    case 'd':
      decode = true;
      break;
    case 'e':
      /* The case for encoding which is th default anyway. */
      break;
    case 'f':
      /* Forcefull overwrite output. */
      dontCareIfFileAlreadyExists = true;
      break;
    case '?':
      usage ();
    }
  }

  if (outputFileStr) {
    if (!dontCareIfFileAlreadyExists) {
      /* If the tool is being used from the command line then we need
         to check with the user that they want to destroy the original
         file. Otherwise we'll exit and leave a message to use -f. */
      if (isatty (fileno (stdin))) {
        printf ("This will overwrite file %s Are you sure you want to "
                "continue? [Y/N]:", outputFileStr);
        char result[5];
        do {
          fgets (result, 5, stdin);
          unsigned len = strlen (result);
          if ((len == 4 && strcicmp ("yes\n", result))
              || (len == 2 && strcicmp ("y\n", result))) {
              break;
          } else if ((len == 2 && strcicmp ("no\n", result))
                     || (strcicmp ("n\n", result))) {
            usage ();
            exit (1);
          }

          printf ("You need to type 'y' or 'n':");
        } while (true);
      }
    }

    output = fopen (outputFileStr, "w+");
    if (!output) {
      fprintf (stderr, "Couldn't open output file.");
      exit (1);
    }
  }
    
  if (noInput && isatty (fileno(stdin))) {
    printf ("No input given so nothing to decode. \n");
    exit (1);
  }

  if (decode) {
    decodeLocations (input, output);
  } else {
    encodeLocations (input, output);
  }

  return 0;
}
