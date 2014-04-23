//
//  JTAViewController.h
//  googlePolylineTest
//
//  Created by James Snook on 21/04/2014.
//  Copyright (c) 2014 James Snook. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreLocation/CoreLocation.h>

@interface JTAViewController : UIViewController <CLLocationManagerDelegate>

@property(strong, nonatomic) IBOutlet UIButton *recordButton;

- (IBAction)buttonPressed:(id)sender;

@property(strong, nonatomic) IBOutlet UITextView *polyline;
@property(strong, nonatomic) IBOutlet UILabel *latLng;
@property(strong, nonatomic) IBOutlet UILabel *diffs;
@property(strong, nonatomic) IBOutlet UILabel *encodedDiffs;
@property(strong, nonatomic) IBOutlet UILabel *decodedDiffs;

@property(strong, nonatomic) IBOutlet UILabel *resultLabel;

@end
