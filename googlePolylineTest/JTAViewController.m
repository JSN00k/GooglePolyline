//
//  JTAViewController.m
//  googlePolylineTest
//
//  Created by James Snook on 21/04/2014.
//  Copyright (c) 2014 James Snook. All rights reserved.
//

#import "JTAViewController.h"
#import "polylineFunctions.h"

#define BUFFER_INCREMENT_SIZE 512

@interface JTAViewController ()

@end

@implementation JTAViewController
{
  BOOL isRecording;
  CLLocationManager *manager;
  int32_t prevIntLat;
  int32_t prevIntLng;
  
  int32_t *savedDiffs;
  NSUInteger savedDiffsLength;
  NSUInteger savedDiffsCount;
  
  NSMutableString *encodedPolyline;
  
  CLLocationCoordinate2D *recordedLocs;
  NSUInteger recordedLocsLength;
  NSUInteger recordedLocsCount;
}

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
    prevIntLat = 0;
    prevIntLng = 0;
    
    if (!manager) {
      manager = [[CLLocationManager alloc] init];
      
      if ([manager respondsToSelector:@selector(requestWhenInUseAuthorization)]) {
        [manager requestWhenInUseAuthorization];
      }
      
      [manager setDelegate:self];
      [manager setDesiredAccuracy:kCLLocationAccuracyBest];
      
      savedDiffsLength = 2 * BUFFER_INCREMENT_SIZE;
      savedDiffs = malloc (savedDiffsLength * sizeof(int32_t));
      savedDiffsCount = 0;
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
  CLLocationCoordinate2D loc = [[locations lastObject] coordinate];
  [_latLng setText:[NSString stringWithFormat:
                    @"%f, %f", loc.latitude, loc.longitude]];
  
  int32_t latDiff = 0;
  int32_t lngDiff = 0;
  
  [_diffs setText:[NSString stringWithFormat:
                   @"%d, %d", latDiff, lngDiff]];
  char encoded[7];
  unsigned length;
  
  uint32_t prevIntLat1 = prevIntLat;
  encodedValue(loc.latitude, &prevIntLat, encoded, &length);
  lngDiff = prevIntLat1 - prevIntLat;
  encoded[length] = '\0';
  
  int32_t decodedLat = decodeDifferenceVal (encoded, &length);
  
  NSString *encodedLatStr = [NSString stringWithUTF8String:encoded];
  
  int32_t prevIntLng1 = prevIntLng;
  
  encodedValue(loc.longitude, &prevIntLng, encoded, &length);
  
  latDiff = prevIntLng1 - prevIntLng;
  encoded[length] = '\0';
  NSString *encodedLngStr = [NSString stringWithUTF8String:encoded];
  
  int32_t decodedLng = decodeDifferenceVal (encoded, &length);
  
  [_encodedDiffs setText:[NSString stringWithFormat:@"%@, %@",
                          encodedLatStr, encodedLngStr]];
  
  [_decodedDiffs setText:[NSString stringWithFormat:@"%d, %d",
                          decodedLat, decodedLng]];
  
  
  [encodedPolyline appendFormat:@"%@%@", encodedLatStr, encodedLngStr];
  [_polyline setText:encodedPolyline];
  
  recordedLocs[recordedLocsCount++] = loc;
  savedDiffs[savedDiffsCount++] = latDiff;
  savedDiffs[savedDiffsCount++] = lngDiff;
  
  if (recordedLocsCount >= recordedLocsLength - 1) {
    recordedLocsLength += BUFFER_INCREMENT_SIZE;
    recordedLocs = realloc(recordedLocs, recordedLocsLength
                           * sizeof(CLLocationCoordinate2D));
  }
  
  if (savedDiffsCount >= savedDiffsLength - 1) {
    savedDiffsLength += 2 * BUFFER_INCREMENT_SIZE;
    savedDiffs = realloc(savedDiffs, savedDiffsLength * sizeof(int32_t));
  }
}

@end
