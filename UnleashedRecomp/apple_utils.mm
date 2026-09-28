#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <TargetConditionals.h>

#if TARGET_OS_OSX
#import <AppKit/AppKit.h>
#else
#import <UIKit/UIKit.h>
#endif

#include "apple_utils.h"

namespace apple {

std::filesystem::path GetUserDataDirectory() {
  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                                       NSUserDomainMask, YES);

  if ([paths count] > 0) {
    NSString *documentsPath = [paths objectAtIndex:0];
    return std::filesystem::path([documentsPath UTF8String]);
  }

  const char *homeDir = getenv("HOME");
  if (homeDir != nullptr) {
    return std::filesystem::path(homeDir) / "Documents";
  }
  return std::filesystem::path();
}

std::filesystem::path GetApplicationSupportDirectory() {
  NSArray *paths = NSSearchPathForDirectoriesInDomains(
      NSApplicationSupportDirectory, NSUserDomainMask, YES);

  if ([paths count] > 0) {
    NSString *appSupportPath = [paths objectAtIndex:0];
    // Ensure application support directory exists as it is not guaranteed to be
    // created by the system
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:appSupportPath]) {
      NSError *error = nil;
      [fileManager createDirectoryAtPath:appSupportPath
             withIntermediateDirectories:YES
                              attributes:nil
                                   error:&error];
      if (error) {
        NSLog(@"Failed to create Application Support directory: %@", error);
        return std::filesystem::path();
      }
    }
    return std::filesystem::path([appSupportPath UTF8String]);
  }

  const char *homeDir = getenv("HOME");
  if (homeDir != nullptr) {
    return std::filesystem::path(homeDir) / "Library" / "Application Support";
  }

  return std::filesystem::path();
}

std::filesystem::path GetCachesDirectory() {
  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory,
                                                       NSUserDomainMask, YES);

  if ([paths count] > 0) {
    NSString *cachesPath = [paths objectAtIndex:0];
    return std::filesystem::path([cachesPath UTF8String]);
  }

  const char *homeDir = getenv("HOME");
  if (homeDir != nullptr) {
    return std::filesystem::path(homeDir) / "Library" / "Caches";
  }

  return std::filesystem::path();
}

bool OpenBrowser(const char *url) {
  @try {
    NSString *nsUrl = [NSString stringWithUTF8String:url];
    NSURL *nsUrlObj = [NSURL URLWithString:nsUrl];

    if (nsUrlObj == nil) {
      return false;
    }

#if TARGET_OS_OSX
    return [[NSWorkspace sharedWorkspace] openURL:nsUrlObj];
#else
    if (@available(iOS 13.0, *)) {
      __block bool success = false;
      [[UIApplication sharedApplication] openURL:nsUrlObj
          options:@{}
          completionHandler:^(BOOL succeeded) {
            success = succeeded;
          }];
      return success;
    } else {
      // Fallback for older iOS versions
      return [[UIApplication sharedApplication] openURL:nsUrlObj];
    }
#endif
  } @catch (NSException *exception) {
    return false;
  }
}

bool SupportsBCTextures() {
  static bool cachedResult = false;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    @try {
      id<MTLDevice> device = MTLCreateSystemDefaultDevice();
      if (device == nil) {
        return;
      }
      if (@available(iOS 16.4, macOS 13.3, *)) {
        cachedResult = [device supportsBCTextureCompression];
      }
    } @catch (NSException *exception) {
    }
  });
  return cachedResult;
}

} // namespace apple
