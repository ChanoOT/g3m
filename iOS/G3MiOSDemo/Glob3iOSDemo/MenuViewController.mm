//
//  MenuViewController.m
//  G3MiOSDemo
//
//  Created by Jose Miguel SN on 30/10/15.
//
//

#import "MenuViewController.h"

#import "ViewController.h"

#import <G3MiOSSDK/MarksRenderer.hpp>

#import "AppDelegate.h"



@interface MenuViewController ()

@end



@implementation MenuViewController

@synthesize textField;
@synthesize _theVC;

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
  
  
  AppDelegate* delegate = [UIApplication sharedApplication].delegate;
  
  galaxiesSwitch.on = delegate.showingGalaxies;

}

- (void)viewDidAppear:(BOOL)animated
{
  [super viewDidAppear:animated];
  //galaxiesSwitch.on = _theVC._galaxies->isEnable();
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


-(IBAction)backToMenu:(id)sender{
  [self performSegueWithIdentifier:@"back" sender:nil];
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

-(IBAction)showText:(id)sender{
  
  switch ([sender tag]) {
    case 0:
      textField.text = @"OK 1";
      break;
    case 1:
      textField.text = @"OK 2";
      break;
    case 2:
      textField.text = @"OK 3";
      break;
    case 3:
      textField.text = @"OK 4";
      break;
    case 4:
      textField.text = @"OK 5";
      break;
    default:
      break;
  }
  
  
}

-(IBAction)showGalaxies:(id)sender{
  bool v = ((UISwitch*) sender).on ;
  ((AppDelegate*)[UIApplication sharedApplication].delegate).showingGalaxies = v;
}

@end
