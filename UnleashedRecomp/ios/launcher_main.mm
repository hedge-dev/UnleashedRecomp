#import <Foundation/Foundation.h>
#import <os/log.h>
#import <unistd.h>

#include <SDL.h>
#include <SDL_main.h>

int UnleashedMain(int argc, char *argv[]);

#if _DEBUG
@interface StdRedirector : NSObject
- (void)start;
@end
@interface StdRedirector ()
@property (nonatomic, strong) NSFileHandle *pipeReadHandle;
@property (nonatomic) dispatch_source_t source;
@property (nonatomic) os_log_t log;
@end

@implementation StdRedirector

- (void)start {
    self.log = os_log_create("com.yourapp.stdout", "console");

    int pipefd[2];
    pipe(pipefd);

    int readFD = pipefd[0];
    int writeFD = pipefd[1];

    // Redirect stdout + stderr to pipe
    dup2(writeFD, STDOUT_FILENO);
    dup2(writeFD, STDERR_FILENO);

    close(writeFD);

    self.pipeReadHandle = [[NSFileHandle alloc] initWithFileDescriptor:readFD closeOnDealloc:YES];

    dispatch_queue_t queue = dispatch_get_global_queue(QOS_CLASS_UTILITY, 0);

    self.source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ,
                                         readFD,
                                         0,
                                         queue);

    __weak typeof(self) weakSelf = self;

    dispatch_source_set_event_handler(self.source, ^{
        __strong typeof(weakSelf) self = weakSelf;
        if (!self) return;

        NSData *data = [self.pipeReadHandle availableData];
        if (data.length == 0) return;

        NSString *output = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        if (!output) return;

        NSArray<NSString *> *lines = [output componentsSeparatedByString:@"\n"];

        for (NSString *line in lines) {
            if (line.length == 0) continue;

            os_log_info(self.log, "%{public}@", line);
        }
    });

    dispatch_resume(self.source);
}

@end
#endif

int main(int argc, char *argv[]) {
    @autoreleasepool {
    #if _DEBUG
        StdRedirector *redirector = [[StdRedirector alloc] init];
        [redirector start];
    #endif
        return SDL_UIKitRunApp(argc, argv, UnleashedMain);
    }
}
