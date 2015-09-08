//
//  JTAViewController.m
//  googlePolylineTest
//
//  Created by James Snook on 21/04/2014.
//  Copyright (c) 2014 James Snook. All rights reserved.
//

#import "JTAViewController.h"
#import "polylineFunctions.h"
#import "AppendableDataStore.h"

#define BUFFER_INCREMENT_SIZE 512

@interface JTAViewController ()

@end

@implementation JTAViewController
{
  BOOL isRecording;
  CLLocationManager *manager;
  
  NSMutableString *encodedPolyline;
  
  CLLocationCoordinate2D *recordedLocs;
  NSUInteger recordedLocsLength;
  NSUInteger recordedLocsCount;
  
  NSMutableString *exampleFileStr;
  PolylineEncoder *encoder;
}

struct PolylineEncoder {
  int32_t intLat;
  int32_t intLng;
  AppendableDataStore *dataStore;
  unsigned nodeCount;
  char *unusedChars;
};

- (void)viewDidLoad
{
  [super viewDidLoad];
}

- (void)didReceiveMemoryWarning
{
  [super didReceiveMemoryWarning];
  // Dispose of any resources that can be recreated.
}

- (void)buttonPressed:(id)sender
{
  if (!isRecording) {
    if (!manager) {
      manager = [[CLLocationManager alloc] init];
      
      if ([manager respondsToSelector:@selector(requestWhenInUseAuthorization)]) {
        [manager requestWhenInUseAuthorization];
      }
      
      [manager setDelegate:self];
      [manager setDesiredAccuracy:kCLLocationAccuracyBest];
      
      encodedPolyline = [NSMutableString string];
      
      recordedLocsLength = BUFFER_INCREMENT_SIZE;
      recordedLocs = malloc (recordedLocsLength * sizeof(CLLocationCoordinate2D));
      recordedLocsCount = 0;
    }
    
    [manager startUpdatingLocation];
    isRecording = YES;
    [_recordButton setTitle:@"Stop Recording" forState:UIControlStateNormal];
     
    [_recordButton setNeedsDisplay];
  } else {
    [manager stopUpdatingLocation];
    NSLog(@"%@", exampleFileStr);
    exampleFileStr = nil;
    
    char *encodeAll = copyEncodedLocationsString((Coordinate *)recordedLocs,
                                                 (int)recordedLocsCount);
    BOOL passedTest = [encodedPolyline isEqualToString:
                       [NSString stringWithUTF8String:encodeAll]];
    
    
    if (passedTest) {
      unsigned locsCount;
      CLLocationCoordinate2D *coords
       = (CLLocationCoordinate2D *)decodeLocationsString(encodeAll, &locsCount);
      if (locsCount == recordedLocsCount) {
        for (int i = 0; i < locsCount; ++i) {
          CLLocationCoordinate2D roundedLoc = {
            round(recordedLocs[i].latitude * 1e5) * 1e-5,
            round(recordedLocs[i].longitude * 1e5) * 1e-5
          };
          
          if (coords[i].latitude != roundedLoc.latitude
              || coords[i].longitude != roundedLoc.longitude) {
            passedTest = NO;
            break;
          }
        }
      } else {
        passedTest = NO;
      }
    }
    
    NSLog(@"%@", encodedPolyline);
    
    if (passedTest)
      [_resultLabel setText:@"Succeeded"];
    else
      [_resultLabel setText:@"FAILED"];
    
    isRecording = NO;
    
    [_recordButton setTitle:@"Start Recording Locations" forState:UIControlStateNormal];
  }
}

- (void)locationManager:(CLLocationManager *)manager
     didUpdateLocations:(NSArray *)locations
{    
  if (!exampleFileStr)
    exampleFileStr = [NSMutableString string];
  
  CLLocationCoordinate2D loc = [[locations lastObject] coordinate];
  [exampleFileStr appendFormat:@"%lf, %lf\n", loc.latitude, loc.longitude];
  [_latLng setText:[NSString stringWithFormat:
                    @"%f, %f", loc.latitude, loc.longitude]];
  
  if (!encoder)
    encoder = PolylineEncoderCreate();
  
  char encoded[11];
  
  Coordinate coord;
  coord.latitude = loc.latitude;
  coord.longitude = loc.longitude;
  
  int32_t previousIntLat = encoder->intLat;
  int32_t previousIntLng = encoder->intLng;
  unsigned len = PolylineEncoderGetEncodedCoordinate(encoder,
                                                     coord,
                                                     encoded);
  
  encoded[len] = '\0';
  
  unsigned decodedCount = 0;
  /* Make a temporary encoder to decode only the string segment that 
     we just encoded without the context of the rest of the string. */
  PolylineEncoder *tmpEncoder = PolylineEncoderCreate();
  PolylineEncoderGetDecodedCoordinates(tmpEncoder, encoded,
                                       &decodedCount);
  

  int32_t latDiff = encoder->intLat - previousIntLat;
  int32_t lngDiff = encoder->intLng - previousIntLng;
  [_diffs setText:[NSString stringWithFormat:
                   @"%d, %d", latDiff, lngDiff]];
  
  [_encodedDiffs setText:[NSString stringWithFormat:@"%s", encoded]];
  
  [_decodedDiffs setText:[NSString stringWithFormat:@"%d, %d",
                          tmpEncoder->intLat,
                          tmpEncoder->intLng]];
  
  NSAssert (latDiff == tmpEncoder->intLat && lngDiff == tmpEncoder->intLng,
            @"Either the encoder failed to encode the value properly or the decoder "
            @"failed to decode the value properly, as they don't match (or possibly both)");
  
  PolylineEncoderFree(tmpEncoder);
  
  [encodedPolyline appendFormat:@"%s", encoded];
  [_polyline setText:encodedPolyline];
  
  recordedLocs[recordedLocsCount++] = loc;
  
  if (recordedLocsCount >= recordedLocsLength - 1) {
    recordedLocsLength += BUFFER_INCREMENT_SIZE;
    recordedLocs = realloc(recordedLocs, recordedLocsLength
                           * sizeof(CLLocationCoordinate2D));
  }
}

@end
