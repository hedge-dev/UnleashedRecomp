# Building

## 1. Clone the Repository

Clone **UnleashedRecomp** with submodules using [Git](https://git-scm.com/).
```
git clone --recurse-submodules https://github.com/hedge-dev/UnleashedRecomp.git
```

### Windows
If you skipped the `--recurse-submodules` argument during cloning, you can run `update_submodules.bat` to ensure the submodules are pulled.

## 2. Add the Required Game Files

Copy the following files from the game and place them inside `./UnleashedRecompLib/private/`:
- `default.xex`
- `default.xexp`
- `shader.ar`

These files are located in the game's root directory, apart from `default.xexp`, which must be obtained via the title update package.

> [!TIP]
> It is recommended that you install the game using [an existing Unleashed Recompiled release](https://github.com/hedge-dev/UnleashedRecomp/releases/latest) to acquire these files, otherwise you'll need to rely on third-party tools to extract them.
>
> Using the Unleashed Recompiled installation wizard will also ensure that these files are compatible with each other so that they can be used with the build environment.
>
> When sourcing these files from an Unleashed Recompiled installation, they will be stored under `game` and `update` subdirectories.

## 3. Install Dependencies

### Windows
You will need to install [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/).

In the installer, you must select the following **Workloads** and **Individual components** for installation:
- Desktop development with C++
- C++ Clang Compiler for Windows
- C++ CMake tools for Windows

### Linux
The following command will install the required dependencies on a distro that uses `apt` (such as Debian-based distros).
```bash
sudo apt install autoconf automake libtool pkg-config curl cmake ninja-build clang clang-tools libgtk-3-dev
```
The following command will install the required dependencies on a distro that uses `pacman` (such as Arch-based distros).
```bash
sudo pacman -S base-devel ninja lld clang gtk3
```
You can also find the equivalent packages for your preferred distro.

> [!NOTE]
> This list may not be comprehensive for your particular distro and you may be required to install additional packages, should an error occur during configuration.

## 4. Build the Project

### Windows
1. Open the repository directory in Visual Studio and wait for CMake generation to complete. If you don't plan to debug, switch to the `Release` configuration.

> [!TIP]
> If you need a Release-performant build and want to iterate on development without debugging, **it is highly recommended** that you use the `RelWithDebInfo` configuration for faster compile times.

2. Under **Solution Explorer**, right-click and choose **Switch to CMake Targets View**.
3. Right-click the **UnleashedRecomp** project and choose **Set as Startup Item**, then choose **Add Debug Configuration**.
4. Add a `currentDir` property to the first element under `configurations` in the generated JSON and set its value to the path to your game directory (where root is the directory containing `dlc`, `game`, `update`, etc).
5. Start **UnleashedRecomp**. The initial compilation may take a while to complete due to code and shader recompilation.

### Linux
1. Configure the project using CMake by navigating to the repository and running the following command.
```bash
cmake . --preset linux-release
```

> [!NOTE]
> The available presets are `linux-debug`, `linux-relwithdebinfo` and `linux-release`.

2. Build the project using the selected configuration.
```bash
cmake --build ./out/build/linux-release --target UnleashedRecomp
```

3. Navigate to the directory that was specified as the output in the previous step and run the game.
```bash
./UnleashedRecomp
```
