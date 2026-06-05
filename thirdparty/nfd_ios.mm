#include <nfd.h>

#import <UIKit/UIKit.h>

#include <stdlib.h>
#include <string.h>

#include <string>

struct NFDIOSPathSet
{
    nfdpathsetsize_t count;
    char** paths;
};

struct NFDIOSPathSetEnumerator
{
    const NFDIOSPathSet* pathSet;
    nfdpathsetsize_t index;
};

static std::string g_error;
static NSMutableArray<NSURL*>* g_securityScopedURLs;

static void NFDi_SetError(const char* msg)
{
    g_error = msg ? msg : "";
}

static void NFDi_SetErrorNSString(NSString* msg)
{
    NFDi_SetError(msg ? [msg UTF8String] : "Unknown iOS document picker error.");
}

static nfdresult_t NFDi_CopyUtf8String(const char* utf8Str, char** out)
{
    if (!out)
    {
        NFDi_SetError("NFD output path was null.");
        return NFD_ERROR;
    }

    *out = nullptr;

    if (!utf8Str)
    {
        NFDi_SetError("iOS document picker returned a null path.");
        return NFD_ERROR;
    }

    size_t len = strlen(utf8Str);
    char* result = static_cast<char*>(malloc(len + 1));
    if (!result)
    {
        NFDi_SetError("Failed to allocate NFD path.");
        return NFD_ERROR;
    }

    memcpy(result, utf8Str, len + 1);
    *out = result;
    return NFD_OKAY;
}

static UIViewController* NFDi_GetTopViewController()
{
    UIApplication* application = [UIApplication sharedApplication];
    UIWindow* keyWindow = nil;

    if (@available(iOS 13.0, *))
    {
        for (UIScene* scene in [application connectedScenes])
        {
            if (scene.activationState != UISceneActivationStateForegroundActive ||
                ![scene isKindOfClass:[UIWindowScene class]])
            {
                continue;
            }

            UIWindowScene* windowScene = static_cast<UIWindowScene*>(scene);
            for (UIWindow* window in windowScene.windows)
            {
                if (window.isKeyWindow)
                {
                    keyWindow = window;
                    break;
                }
            }

            if (keyWindow)
                break;
        }
    }

    if (!keyWindow)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        keyWindow = application.keyWindow;
#pragma clang diagnostic pop
    }

    UIViewController* controller = keyWindow.rootViewController;
    while (controller.presentedViewController)
        controller = controller.presentedViewController;

    return controller;
}

static nfdresult_t NFDi_CreatePathSet(NSArray<NSString*>* paths, const nfdpathset_t** outPaths)
{
    if (!outPaths)
    {
        NFDi_SetError("NFD output path set was null.");
        return NFD_ERROR;
    }

    *outPaths = nullptr;

    if (!paths || paths.count == 0)
        return NFD_CANCEL;

    NFDIOSPathSet* pathSet = static_cast<NFDIOSPathSet*>(calloc(1, sizeof(NFDIOSPathSet)));
    if (!pathSet)
    {
        NFDi_SetError("Failed to allocate NFD path set.");
        return NFD_ERROR;
    }

    pathSet->count = static_cast<nfdpathsetsize_t>(paths.count);
    pathSet->paths = static_cast<char**>(calloc(pathSet->count, sizeof(char*)));
    if (!pathSet->paths)
    {
        free(pathSet);
        NFDi_SetError("Failed to allocate NFD path set entries.");
        return NFD_ERROR;
    }

    for (nfdpathsetsize_t i = 0; i < pathSet->count; i++)
    {
        NSString* path = [paths objectAtIndex:i];
        if (NFDi_CopyUtf8String([path UTF8String], &pathSet->paths[i]) != NFD_OKAY)
        {
            for (nfdpathsetsize_t j = 0; j < i; j++)
                free(pathSet->paths[j]);

            free(pathSet->paths);
            free(pathSet);
            return NFD_ERROR;
        }
    }

    *outPaths = reinterpret_cast<const nfdpathset_t*>(pathSet);
    return NFD_OKAY;
}

@interface NFDIOSPickerDelegate : NSObject <UIDocumentPickerDelegate>
{
@public
    BOOL completed;
    nfdresult_t result;
    NSArray<NSString*>* paths;
}
@end

@implementation NFDIOSPickerDelegate

- (id)init
{
    self = [super init];
    if (self)
    {
        completed = NO;
        result = NFD_CANCEL;
        paths = nil;
    }

    return self;
}

- (void)dealloc
{
    [paths release];
    [super dealloc];
}

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController*)controller
{
    (void)controller;
    result = NFD_CANCEL;
    completed = YES;
}

- (void)documentPicker:(UIDocumentPickerViewController*)controller didPickDocumentsAtURLs:(NSArray<NSURL*>*)urls
{
    (void)controller;

    NSMutableArray<NSString*>* pickedPaths = [NSMutableArray arrayWithCapacity:urls.count];

    for (NSURL* url in urls)
    {
        BOOL scoped = [url startAccessingSecurityScopedResource];
        if (scoped)
        {
            if (!g_securityScopedURLs)
                g_securityScopedURLs = [[NSMutableArray alloc] init];

            [g_securityScopedURLs addObject:url];
        }

        NSString* path = url.path;
        if (path.length > 0)
            [pickedPaths addObject:path];
    }

    [paths release];
    paths = [[NSArray alloc] initWithArray:pickedPaths];
    result = paths.count > 0 ? NFD_OKAY : NFD_CANCEL;
    completed = YES;
}

@end

static void NFDi_PresentPicker(NFDIOSPickerDelegate* delegate, bool folderMode, bool multiple)
{
    UIViewController* presenter = NFDi_GetTopViewController();
    if (!presenter)
    {
        NFDi_SetError("Unable to find an iOS view controller for the document picker.");
        delegate->result = NFD_ERROR;
        delegate->completed = YES;
        return;
    }

    NSArray<NSString*>* documentTypes = folderMode ? @[ @"public.folder" ] : @[ @"public.item" ];
    UIDocumentPickerViewController* picker =
        [[UIDocumentPickerViewController alloc] initWithDocumentTypes:documentTypes
                                                               inMode:UIDocumentPickerModeOpen];

    picker.delegate = delegate;
    picker.allowsMultipleSelection = multiple ? YES : NO;

    UIPopoverPresentationController* popover = picker.popoverPresentationController;
    if (popover)
    {
        popover.sourceView = presenter.view;
        popover.sourceRect = CGRectMake(CGRectGetMidX(presenter.view.bounds), CGRectGetMidY(presenter.view.bounds), 1.0, 1.0);
        popover.permittedArrowDirections = 0;
    }

    [presenter presentViewController:picker animated:YES completion:nil];
    [picker release];
}

static nfdresult_t NFDi_RunPicker(const nfdpathset_t** outPaths, bool folderMode, bool multiple)
{
    @autoreleasepool
    {
        NFDIOSPickerDelegate* delegate = [[NFDIOSPickerDelegate alloc] init];

        if ([NSThread isMainThread])
        {
            NFDi_PresentPicker(delegate, folderMode, multiple);
            while (!delegate->completed)
            {
                @autoreleasepool
                {
                    [[NSRunLoop mainRunLoop] runMode:NSDefaultRunLoopMode
                                          beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
                }
            }
        }
        else
        {
            dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
            dispatch_async(dispatch_get_main_queue(), ^{
                NFDi_PresentPicker(delegate, folderMode, multiple);
                if (delegate->completed)
                    dispatch_semaphore_signal(semaphore);
            });

            while (!delegate->completed)
            {
                dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, 50 * NSEC_PER_MSEC));
            }
        }

        nfdresult_t result = delegate->result;
        if (result == NFD_OKAY)
            result = NFDi_CreatePathSet(delegate->paths, outPaths);

        [delegate release];
        return result;
    }
}

extern "C" {

nfdresult_t NFD_Init(void)
{
    NFDi_SetError(nullptr);
    return NFD_OKAY;
}

void NFD_Quit(void)
{
    for (NSURL* url in g_securityScopedURLs)
        [url stopAccessingSecurityScopedResource];

    [g_securityScopedURLs release];
    g_securityScopedURLs = nil;
}

const char* NFD_GetError(void)
{
    return g_error.empty() ? nullptr : g_error.c_str();
}

void NFD_ClearError(void)
{
    NFDi_SetError(nullptr);
}

void NFD_FreePathN(nfdnchar_t* filePath)
{
    free(filePath);
}

void NFD_FreePathU8(nfdu8char_t* filePath)
{
    free(filePath);
}

nfdresult_t NFD_OpenDialogN(nfdnchar_t** outPath,
                            const nfdnfilteritem_t* filterList,
                            nfdfiltersize_t filterCount,
                            const nfdnchar_t* defaultPath)
{
    (void)filterList;
    (void)filterCount;
    (void)defaultPath;

    const nfdpathset_t* pathSet = nullptr;
    nfdresult_t result = NFDi_RunPicker(&pathSet, false, false);
    if (result != NFD_OKAY)
        return result;

    result = NFD_PathSet_GetPathN(pathSet, 0, outPath);
    NFD_PathSet_Free(pathSet);
    return result;
}

nfdresult_t NFD_OpenDialogU8(nfdu8char_t** outPath,
                             const nfdu8filteritem_t* filterList,
                             nfdfiltersize_t filterCount,
                             const nfdu8char_t* defaultPath)
{
    return NFD_OpenDialogN(outPath, filterList, filterCount, defaultPath);
}

nfdresult_t NFD_OpenDialogN_With_Impl(nfdversion_t version,
                                      nfdnchar_t** outPath,
                                      const nfdopendialognargs_t* args)
{
    (void)version;
    return NFD_OpenDialogN(outPath,
                           args ? args->filterList : nullptr,
                           args ? args->filterCount : 0,
                           args ? args->defaultPath : nullptr);
}

nfdresult_t NFD_OpenDialogU8_With_Impl(nfdversion_t version,
                                       nfdu8char_t** outPath,
                                       const nfdopendialogu8args_t* args)
{
    return NFD_OpenDialogN_With_Impl(version, outPath, args);
}

nfdresult_t NFD_OpenDialogMultipleN(const nfdpathset_t** outPaths,
                                    const nfdnfilteritem_t* filterList,
                                    nfdfiltersize_t filterCount,
                                    const nfdnchar_t* defaultPath)
{
    (void)filterList;
    (void)filterCount;
    (void)defaultPath;
    return NFDi_RunPicker(outPaths, false, true);
}

nfdresult_t NFD_OpenDialogMultipleU8(const nfdpathset_t** outPaths,
                                     const nfdu8filteritem_t* filterList,
                                     nfdfiltersize_t filterCount,
                                     const nfdu8char_t* defaultPath)
{
    return NFD_OpenDialogMultipleN(outPaths, filterList, filterCount, defaultPath);
}

nfdresult_t NFD_OpenDialogMultipleN_With_Impl(nfdversion_t version,
                                              const nfdpathset_t** outPaths,
                                              const nfdopendialognargs_t* args)
{
    (void)version;
    return NFD_OpenDialogMultipleN(outPaths,
                                   args ? args->filterList : nullptr,
                                   args ? args->filterCount : 0,
                                   args ? args->defaultPath : nullptr);
}

nfdresult_t NFD_OpenDialogMultipleU8_With_Impl(nfdversion_t version,
                                               const nfdpathset_t** outPaths,
                                               const nfdopendialogu8args_t* args)
{
    return NFD_OpenDialogMultipleN_With_Impl(version, outPaths, args);
}

nfdresult_t NFD_SaveDialogN(nfdnchar_t** outPath,
                            const nfdnfilteritem_t* filterList,
                            nfdfiltersize_t filterCount,
                            const nfdnchar_t* defaultPath,
                            const nfdnchar_t* defaultName)
{
    (void)outPath;
    (void)filterList;
    (void)filterCount;
    (void)defaultPath;
    (void)defaultName;
    NFDi_SetError("Save dialogs are not supported by the iOS document picker backend.");
    return NFD_ERROR;
}

nfdresult_t NFD_SaveDialogU8(nfdu8char_t** outPath,
                             const nfdu8filteritem_t* filterList,
                             nfdfiltersize_t filterCount,
                             const nfdu8char_t* defaultPath,
                             const nfdu8char_t* defaultName)
{
    return NFD_SaveDialogN(outPath, filterList, filterCount, defaultPath, defaultName);
}

nfdresult_t NFD_SaveDialogN_With_Impl(nfdversion_t version,
                                      nfdnchar_t** outPath,
                                      const nfdsavedialognargs_t* args)
{
    (void)version;
    return NFD_SaveDialogN(outPath,
                           args ? args->filterList : nullptr,
                           args ? args->filterCount : 0,
                           args ? args->defaultPath : nullptr,
                           args ? args->defaultName : nullptr);
}

nfdresult_t NFD_SaveDialogU8_With_Impl(nfdversion_t version,
                                       nfdu8char_t** outPath,
                                       const nfdsavedialogu8args_t* args)
{
    return NFD_SaveDialogN_With_Impl(version, outPath, args);
}

nfdresult_t NFD_PickFolderN(nfdnchar_t** outPath, const nfdnchar_t* defaultPath)
{
    (void)defaultPath;

    const nfdpathset_t* pathSet = nullptr;
    nfdresult_t result = NFDi_RunPicker(&pathSet, true, false);
    if (result != NFD_OKAY)
        return result;

    result = NFD_PathSet_GetPathN(pathSet, 0, outPath);
    NFD_PathSet_Free(pathSet);
    return result;
}

nfdresult_t NFD_PickFolderU8(nfdu8char_t** outPath, const nfdu8char_t* defaultPath)
{
    return NFD_PickFolderN(outPath, defaultPath);
}

nfdresult_t NFD_PickFolderN_With_Impl(nfdversion_t version,
                                      nfdnchar_t** outPath,
                                      const nfdpickfoldernargs_t* args)
{
    (void)version;
    return NFD_PickFolderN(outPath, args ? args->defaultPath : nullptr);
}

nfdresult_t NFD_PickFolderU8_With_Impl(nfdversion_t version,
                                       nfdu8char_t** outPath,
                                       const nfdpickfolderu8args_t* args)
{
    return NFD_PickFolderN_With_Impl(version, outPath, args);
}

nfdresult_t NFD_PickFolderMultipleN(const nfdpathset_t** outPaths, const nfdnchar_t* defaultPath)
{
    (void)defaultPath;
    return NFDi_RunPicker(outPaths, true, true);
}

nfdresult_t NFD_PickFolderMultipleU8(const nfdpathset_t** outPaths, const nfdu8char_t* defaultPath)
{
    return NFD_PickFolderMultipleN(outPaths, defaultPath);
}

nfdresult_t NFD_PickFolderMultipleN_With_Impl(nfdversion_t version,
                                              const nfdpathset_t** outPaths,
                                              const nfdpickfoldernargs_t* args)
{
    (void)version;
    return NFD_PickFolderMultipleN(outPaths, args ? args->defaultPath : nullptr);
}

nfdresult_t NFD_PickFolderMultipleU8_With_Impl(nfdversion_t version,
                                               const nfdpathset_t** outPaths,
                                               const nfdpickfolderu8args_t* args)
{
    return NFD_PickFolderMultipleN_With_Impl(version, outPaths, args);
}

nfdresult_t NFD_PathSet_GetCount(const nfdpathset_t* pathSet, nfdpathsetsize_t* count)
{
    if (!pathSet || !count)
    {
        NFDi_SetError("NFD path set or count was null.");
        return NFD_ERROR;
    }

    const NFDIOSPathSet* iosPathSet = reinterpret_cast<const NFDIOSPathSet*>(pathSet);
    *count = iosPathSet->count;
    return NFD_OKAY;
}

nfdresult_t NFD_PathSet_GetPathN(const nfdpathset_t* pathSet, nfdpathsetsize_t index, nfdnchar_t** outPath)
{
    if (!pathSet)
    {
        NFDi_SetError("NFD path set was null.");
        return NFD_ERROR;
    }

    const NFDIOSPathSet* iosPathSet = reinterpret_cast<const NFDIOSPathSet*>(pathSet);
    if (index >= iosPathSet->count)
    {
        NFDi_SetError("NFD path set index was out of range.");
        return NFD_ERROR;
    }

    return NFDi_CopyUtf8String(iosPathSet->paths[index], outPath);
}

nfdresult_t NFD_PathSet_GetPathU8(const nfdpathset_t* pathSet, nfdpathsetsize_t index, nfdu8char_t** outPath)
{
    return NFD_PathSet_GetPathN(pathSet, index, outPath);
}

void NFD_PathSet_FreePathN(const nfdnchar_t* filePath)
{
    free(const_cast<nfdnchar_t*>(filePath));
}

void NFD_PathSet_FreePathU8(const nfdu8char_t* filePath)
{
    free(const_cast<nfdu8char_t*>(filePath));
}

void NFD_PathSet_Free(const nfdpathset_t* pathSet)
{
    if (!pathSet)
        return;

    const NFDIOSPathSet* iosPathSet = reinterpret_cast<const NFDIOSPathSet*>(pathSet);
    for (nfdpathsetsize_t i = 0; i < iosPathSet->count; i++)
        free(iosPathSet->paths[i]);

    free(iosPathSet->paths);
    free(const_cast<NFDIOSPathSet*>(iosPathSet));
}

nfdresult_t NFD_PathSet_GetEnum(const nfdpathset_t* pathSet, nfdpathsetenum_t* outEnumerator)
{
    if (!pathSet || !outEnumerator)
    {
        NFDi_SetError("NFD path set or enumerator was null.");
        return NFD_ERROR;
    }

    NFDIOSPathSetEnumerator* enumerator =
        static_cast<NFDIOSPathSetEnumerator*>(calloc(1, sizeof(NFDIOSPathSetEnumerator)));
    if (!enumerator)
    {
        NFDi_SetError("Failed to allocate NFD path set enumerator.");
        return NFD_ERROR;
    }

    enumerator->pathSet = reinterpret_cast<const NFDIOSPathSet*>(pathSet);
    enumerator->index = 0;
    outEnumerator->ptr = enumerator;
    return NFD_OKAY;
}

void NFD_PathSet_FreeEnum(nfdpathsetenum_t* enumerator)
{
    if (!enumerator)
        return;

    free(enumerator->ptr);
    enumerator->ptr = nullptr;
}

nfdresult_t NFD_PathSet_EnumNextN(nfdpathsetenum_t* enumerator, nfdnchar_t** outPath)
{
    if (!enumerator || !outPath)
    {
        NFDi_SetError("NFD enumerator or output path was null.");
        return NFD_ERROR;
    }

    NFDIOSPathSetEnumerator* iosEnumerator = static_cast<NFDIOSPathSetEnumerator*>(enumerator->ptr);
    if (!iosEnumerator || !iosEnumerator->pathSet)
    {
        NFDi_SetError("NFD enumerator was invalid.");
        return NFD_ERROR;
    }

    if (iosEnumerator->index >= iosEnumerator->pathSet->count)
    {
        *outPath = nullptr;
        return NFD_OKAY;
    }

    return NFDi_CopyUtf8String(iosEnumerator->pathSet->paths[iosEnumerator->index++], outPath);
}

nfdresult_t NFD_PathSet_EnumNextU8(nfdpathsetenum_t* enumerator, nfdu8char_t** outPath)
{
    return NFD_PathSet_EnumNextN(enumerator, outPath);
}

}
