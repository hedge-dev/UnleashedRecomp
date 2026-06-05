#include <nfd.h>
#include <stdlib.h>

static const char* g_nfd_ios_error = "Native file dialogs are not implemented for iOS yet.";

nfdresult_t NFD_Init(void) { return NFD_OKAY; }
void NFD_Quit(void) {}
const char* NFD_GetError(void) { return g_nfd_ios_error; }
void NFD_ClearError(void) {}

void NFD_FreePathN(nfdnchar_t* filePath) { free(filePath); }
void NFD_FreePathU8(nfdu8char_t* filePath) { free(filePath); }

nfdresult_t NFD_OpenDialogN(nfdnchar_t** outPath, const nfdnfilteritem_t* filterList, nfdfiltersize_t filterCount, const nfdnchar_t* defaultPath) { (void)outPath; (void)filterList; (void)filterCount; (void)defaultPath; return NFD_ERROR; }
nfdresult_t NFD_OpenDialogU8(nfdu8char_t** outPath, const nfdu8filteritem_t* filterList, nfdfiltersize_t filterCount, const nfdu8char_t* defaultPath) { (void)outPath; (void)filterList; (void)filterCount; (void)defaultPath; return NFD_ERROR; }
nfdresult_t NFD_OpenDialogN_With_Impl(nfdversion_t version, nfdnchar_t** outPath, const nfdopendialognargs_t* args) { (void)version; (void)outPath; (void)args; return NFD_ERROR; }
nfdresult_t NFD_OpenDialogU8_With_Impl(nfdversion_t version, nfdu8char_t** outPath, const nfdopendialogu8args_t* args) { (void)version; (void)outPath; (void)args; return NFD_ERROR; }
nfdresult_t NFD_OpenDialogMultipleN(const nfdpathset_t** outPaths, const nfdnfilteritem_t* filterList, nfdfiltersize_t filterCount, const nfdnchar_t* defaultPath) { (void)outPaths; (void)filterList; (void)filterCount; (void)defaultPath; return NFD_ERROR; }
nfdresult_t NFD_OpenDialogMultipleU8(const nfdpathset_t** outPaths, const nfdu8filteritem_t* filterList, nfdfiltersize_t filterCount, const nfdu8char_t* defaultPath) { (void)outPaths; (void)filterList; (void)filterCount; (void)defaultPath; return NFD_ERROR; }
nfdresult_t NFD_OpenDialogMultipleN_With_Impl(nfdversion_t version, const nfdpathset_t** outPaths, const nfdopendialognargs_t* args) { (void)version; (void)outPaths; (void)args; return NFD_ERROR; }
nfdresult_t NFD_OpenDialogMultipleU8_With_Impl(nfdversion_t version, const nfdpathset_t** outPaths, const nfdopendialogu8args_t* args) { (void)version; (void)outPaths; (void)args; return NFD_ERROR; }
nfdresult_t NFD_SaveDialogN(nfdnchar_t** outPath, const nfdnfilteritem_t* filterList, nfdfiltersize_t filterCount, const nfdnchar_t* defaultPath, const nfdnchar_t* defaultName) { (void)outPath; (void)filterList; (void)filterCount; (void)defaultPath; (void)defaultName; return NFD_ERROR; }
nfdresult_t NFD_SaveDialogU8(nfdu8char_t** outPath, const nfdu8filteritem_t* filterList, nfdfiltersize_t filterCount, const nfdu8char_t* defaultPath, const nfdu8char_t* defaultName) { (void)outPath; (void)filterList; (void)filterCount; (void)defaultPath; (void)defaultName; return NFD_ERROR; }
nfdresult_t NFD_SaveDialogN_With_Impl(nfdversion_t version, nfdnchar_t** outPath, const nfdsavedialognargs_t* args) { (void)version; (void)outPath; (void)args; return NFD_ERROR; }
nfdresult_t NFD_SaveDialogU8_With_Impl(nfdversion_t version, nfdu8char_t** outPath, const nfdsavedialogu8args_t* args) { (void)version; (void)outPath; (void)args; return NFD_ERROR; }
nfdresult_t NFD_PickFolderN(nfdnchar_t** outPath, const nfdnchar_t* defaultPath) { (void)outPath; (void)defaultPath; return NFD_ERROR; }
nfdresult_t NFD_PickFolderU8(nfdu8char_t** outPath, const nfdu8char_t* defaultPath) { (void)outPath; (void)defaultPath; return NFD_ERROR; }
nfdresult_t NFD_PickFolderN_With_Impl(nfdversion_t version, nfdnchar_t** outPath, const nfdpickfoldernargs_t* args) { (void)version; (void)outPath; (void)args; return NFD_ERROR; }
nfdresult_t NFD_PickFolderU8_With_Impl(nfdversion_t version, nfdu8char_t** outPath, const nfdpickfolderu8args_t* args) { (void)version; (void)outPath; (void)args; return NFD_ERROR; }
nfdresult_t NFD_PickFolderMultipleN(const nfdpathset_t** outPaths, const nfdnchar_t* defaultPath) { (void)outPaths; (void)defaultPath; return NFD_ERROR; }
nfdresult_t NFD_PickFolderMultipleU8(const nfdpathset_t** outPaths, const nfdu8char_t* defaultPath) { (void)outPaths; (void)defaultPath; return NFD_ERROR; }
nfdresult_t NFD_PickFolderMultipleN_With_Impl(nfdversion_t version, const nfdpathset_t** outPaths, const nfdpickfoldernargs_t* args) { (void)version; (void)outPaths; (void)args; return NFD_ERROR; }
nfdresult_t NFD_PickFolderMultipleU8_With_Impl(nfdversion_t version, const nfdpathset_t** outPaths, const nfdpickfolderu8args_t* args) { (void)version; (void)outPaths; (void)args; return NFD_ERROR; }
nfdresult_t NFD_PathSet_GetCount(const nfdpathset_t* pathSet, nfdpathsetsize_t* count) { (void)pathSet; if (count) *count = 0; return NFD_OKAY; }
nfdresult_t NFD_PathSet_GetPathN(const nfdpathset_t* pathSet, nfdpathsetsize_t index, nfdnchar_t** outPath) { (void)pathSet; (void)index; (void)outPath; return NFD_ERROR; }
nfdresult_t NFD_PathSet_GetPathU8(const nfdpathset_t* pathSet, nfdpathsetsize_t index, nfdu8char_t** outPath) { (void)pathSet; (void)index; (void)outPath; return NFD_ERROR; }
void NFD_PathSet_FreePathN(const nfdnchar_t* filePath) { free((void*)filePath); }
void NFD_PathSet_FreePathU8(const nfdu8char_t* filePath) { free((void*)filePath); }
void NFD_PathSet_Free(const nfdpathset_t* pathSet) { (void)pathSet; }
void NFD_PathSet_FreeEnum(nfdpathsetenum_t* enumerator) { (void)enumerator; }
nfdresult_t NFD_PathSet_GetEnum(const nfdpathset_t* pathSet, nfdpathsetenum_t* outEnumerator) { (void)pathSet; (void)outEnumerator; return NFD_ERROR; }
nfdresult_t NFD_PathSet_EnumNextN(nfdpathsetenum_t* enumerator, nfdnchar_t** outPath) { (void)enumerator; (void)outPath; return NFD_ERROR; }
nfdresult_t NFD_PathSet_EnumNextU8(nfdpathsetenum_t* enumerator, nfdu8char_t** outPath) { (void)enumerator; (void)outPath; return NFD_ERROR; }
