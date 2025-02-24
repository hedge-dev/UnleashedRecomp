# Building

## Windows
### Pre-requisites
- Git
- Visual Studio 2022
- CMake
- clang

> [!NOTE]
> The required versions of CMake and clang can be obtained via Visual Studio Installer as part of the **Desktop development with C++** workload.

### Instructions
1. Clone **UnleashedRecomp** with submodules (or run `update_submodules.bat` to ensure submodules are pulled).
```
git clone --recurse-submodules https://github.com/hedge-dev/UnleashedRecomp.git
```
2. Place `default.xex` and `default.xexp` in `./UnleashedRecompLib/private/`.
3. Decompress `shader.ar` and place the resulting file in `./UnleashedRecompLib/private/`.
4. Open the repository directory in Visual Studio 2022 and wait for CMake generation to complete. If you don't plan to debug, switch to the `Release` configuration.
> [!NOTE]
> If you need a Release-performant build and want to iterate on development without debugging, it is highly recommended that you use the `RelWithDebInfo` configuration for faster compile times.
5. Under Solution Explorer, right-click and choose **Switch to CMake Targets View**.
6. Right-click the **UnleashedRecomp** project and choose **Set as Startup Item**, then choose **Add Debug Configuration**.
7. Add a `currentDir` property to the first element under `configurations` in the generated JSON and set its value to the path to your game directory (where root is the directory containing `dlc`, `game`, `update`, etc).
8. Start **UnleashedRecomp**. The initial compilation may take a while to complete due to code and shader recompilation.

## Linux
TODO
