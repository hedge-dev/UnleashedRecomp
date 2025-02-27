# Building

## 1. Clone the Repository

Clone **UnleashedRecomp** with submodules using [git](https://git-scm.com/).
```
git clone --recurse-submodules https://github.com/hedge-dev/UnleashedRecomp.git
```
### Windows

If you skipped the `--recurse-submodules` argument during cloning, you can run `update_submodules.bat` to ensure submodules are pulled.

## 2. Add the Game's Files

Locate the following files from the game and place them inside `./UnleashedRecompLib/private/`:
   *  `default.xex` and `shader.ar`: You can find them on the `game` directory of an Unleashed Recompiled installation or extract them from the game's base contents.
   *  `default.xexp`: You can find it on the `update` directory of an Unleashed Recompiled installation or extract them from the game's title update.
  
> [!NOTE]
> It's recommended to just install the game using an existing [Unleashed Recompiled release](https://github.com/hedge-dev/UnleashedRecomp/releases/latest) to acquire these files if possible, as you'll need to rely on third-party tools to extract them otherwise.

## 3. Install Dependencies

### Windows
You will need to install [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/).
In the setup process you'll need to select the following options and tools for installation:
- Desktop development with C++
- C++ Clang Compiler for Windows
- C++ CMake tools for Windows

## Linux
The following command will install the required dependencies on a distro that uses `apt` such as Ubuntu, but you can find the equivalent packages for your preferred distro.
```bash
sudo apt install autoconf automake libtool pkg-config curl cmake ninja-build clang clang-tools libgtk-3-dev
```
> [!NOTE]
>  This list may not be comprehensive to your particular distro and you may be required to install additional packages if an error happens during configuration.
## 4. Building the Project

### Windows
1. Open the repository directory in Visual Studio 2022 and wait for CMake generation to complete. If you don't plan to debug, switch to the `Release` configuration.
> [!NOTE]
> If you need a Release-performant build and want to iterate on development without debugging, it is highly recommended that you use the `RelWithDebInfo` configuration for faster compile times.
2. Under Solution Explorer, right-click and choose **Switch to CMake Targets View**.
3. Right-click the **UnleashedRecomp** project and choose **Set as Startup Item**, then choose **Add Debug Configuration**.
4. Add a `currentDir` property to the first element under `configurations` in the generated JSON and set its value to the path to your game directory (where root is the directory containing `dlc`, `game`, `update`, etc).
5. Start **UnleashedRecomp**. The initial compilation may take a while to complete due to code and shader recompilation.

### Linux
1. Configure the project using CMake by navigating to the repository and running the following command. The presets available are `linux-debug`, `linux-relwithdebinfo` and `linux-release`.
```bash
cmake . --preset linux-release
```
2. Build the project using the selected configuration.
```bash
cmake --build ./out/build/linux-release --target UnleashedRecomp
```
3. Navigate to the directory that was specified as the output in the previous step and run the game.
```bash
./UnleashedRecomp
```
