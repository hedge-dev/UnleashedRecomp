# Unleashed Recompiled

Unleashed Recompiled is an unofficial PC port of Sonic Unleashed created through the process of static recompilation, resulting in a native executable for Windows or Linux with numerous built-in enhancements such as high resolutions, ultrawide support, high frame rates, improved performance and modding support.

**This project does not include any game assets. You must provide the files from your own legally acquired copy of the game to install or build Unleashed Recompiled.**

[Check out the latest release here](https://github.com/hedge-dev/UnleashedRecomp/releases/latest).

[XenonRecomp](https://github.com/hedge-dev/XenonRecomp) and [XenosRecomp](https://github.com/hedge-dev/XenosRecomp) are the main recompilers used for converting the game’s original PowerPC code and Xenos shaders into compatible C++ and HLSL code respectively. The development of these recompilers was directly inspired by [N64: Recompiled](https://github.com/N64Recomp/N64Recomp), which was used to create [Zelda 64: Recompiled](https://github.com/Zelda64Recomp/Zelda64Recomp).

## Table of Contents

- [Minimum System Requirements](#minimum-system-requirements)
- [How To Install](#how-to-install)
- [Features](#features)
- [Update Roadmap](#update-roadmap)
- [Known Issues](#known-issues)
- [FAQ](#faq)
- [Building](#building)
- [Credits](#credits)

## Minimum System Requirements

* CPU with support for the AVX instruction set:
  * Intel: Sandy Bridge (Intel Core 2nd Generation)
  * AMD: Bulldozer (AMD FX series)
* GPU with support for Direct3D 12.0 (Shader Model 6\) or Vulkan 1.2:
  * NVIDIA: GeForce GT 630
  * AMD: Radeon HD 7750 (2012, not the RX 7000\)
  * Intel: Intel HD 510 (Skylake)
* Operating system:
  * Windows 10 (version 1909\)
  * A modern Linux distro such as Ubuntu 22.04 LTS
* Storage:
  * No DLC: Approximately 6 GiB of space is required.
  * With DLC: Approximately 10 GiB of space is required.
  * More space might be required if uncompressed game files are provided during installation.

## How To Install

1) You must have access to the following:

- Xbox 360.
- Sonic Unleashed for Xbox 360 (US or EU, **JP is not supported**).
  - Physical Retail Disc or Digital Copy (can be bought and downloaded from the [Xbox Store](https://www.xbox.com/en-US/games/store/sonic-unleashed/c098fgnmts8f)).
  - Title Update.
  - All available DLC (Adventure Packs) are optional but **highly recommended**. **The DLC includes high quality lighting for the entire game**.

2) **Before proceeding with the installation**, make sure to follow the guide on how to acquire the game’s files from your Xbox 360.

[English Dumping Guide](/DUMPING-en.md)

3) Download [the latest release](https://github.com/hedge-dev/UnleashedRecomp/releases/latest) of Unleashed Recompiled and extract it to where you’d like the game to be installed.

4) Run the executable and you will be guided through the game installation process. You will be asked to provide the files you acquired in the previous step. When presented with options for how to do this:

* **Add Files** will only allow you to provide **containers or images dumped from an Xbox 360**. These often come in the form of very large files without associated extensions. Don’t worry if you’re not aware of what’s inside of them, the installer will automatically detect what type of content is inside the container.


* **Add Folder** will only allow you to provide a **directory with the game’s raw files** corresponding to the piece of content that is requested. **It will NOT scan your folder for compatible content!**

> [!NOTE]
> Please note that it is **not possible** to complete the installation if your files have been **modified**. In case of other problems such as black screens or crashes, **do not try to reinstall the game** as it’s not possible for the process to result in an invalid installation.

## Features

### Easy To Use Installer

A built-in installation wizard will guide you through the process of installing the game with many integrity checks to ensure the process goes as smoothly as possible. The installer can also be accessed from the game’s main menu if you wish to add the DLC at a later time.

### Options Menu

A completely new options menu accessible from the title screen or pause menu, with an unprecedented level of fidelity to the game's interface design. Get access to many quality of life features and graphics options directly within the game with full controller navigation.

### Achievements

You will be rewarded with achievements as you progress through the game just like on its original platform. Achievements are recreated with integrated notifications and a new menu very faithful to the game's visual design. Get all of them and you will be rewarded with a gold trophy\!

> [!NOTE]
> Achievements can’t be used with platforms such as Steam or RetroAchievements.

### Custom Localization

All the new menus in the port fully support localization for all the original game’s languages: English, Japanese, Spanish, Italian, French, and German. As a bonus, switching the game to Japanese also changes the Title Screen logo to its original Japanese counterpart.

### High Fidelity

Special care and attention was taken to recreate the game's visuals as accurately as possible and always compared to the game running on the original hardware. The colors are also as vibrant as those of the PS3 version as originally intended, which did not use the color correction filter present in the Xbox 360 version. Nevertheless, an option to recreate the original warm filter from the Xbox 360 version has been included as an option.

### High Resolution Enhancements

Multiple improvements have been provided to accompany support for higher resolutions.

- More “Multisample anti-aliasing” (MSAA) levels beyond the 2x used by the original game.
- Higher quality versions of “Depth of Field” (DoF), an effect which commonly breaks in emulators when increasing the game’s resolution. The original effect’s formula was reverse-engineered and versions with 5x5, 7x7 and 9x9 taps were added based on the target resolution.
- An Enhanced Motion Blur option which uses more samples than the original.
- Support for “Alpha to coverage” anti-aliasing to enhance the visual result of transparent textures with hard edges.
- A Bicubic Texture Filtering option which greatly enhances the visual result of Global Illumination textures.
- A more precise implementation of the game’s reverse Z technique, eliminating most Z-fighting issues and fixing the flickery motion blur in stages like Jungle Joyride.

### High Performance Renderer

A new renderer was written from scratch to translate the game's draw calls to modern APIs in a highly efficient way while also taking advantage of multithreading.

As emulation of the Xbox 360’s GPU is not required in a Recompilation, many decisions were made to skip quirks of the original hardware that are not required on a PC port, resulting in large improvements to performance.

Modern rendering techniques such as bindless textures and shader specialization are also used to maximize the performance on systems with modern hardware while also supporting a wide range of lower end hardware.

Multiple optimizations were made to the game's shadow map rendering and special techniques were developed to automatically detect and skip unnecessary texture copies that are no longer needed in modern APIs.

Additionally, to support the game's extensive use of asset streaming, parallel transfer queues are leveraged, a feature only available in modern APIs that enable efficient use of available PCIE bandwidth.

### High Frame Rate Support

The game’s frame rate cap has been increased by default to 60 FPS, with support for higher targets and unlocked frame rate being available from the options menu. A vast amount of glitches that usually occur on higher frame rates have been fixed and are included as part of the Recompilation.

> [!NOTE]
> While the game is considered to be beatable at frame rates higher than 60, please note that [some issues](#high-frame-rate-glitches) can still occur. Some of these issues might be addressed in future updates.

### Ultrawide Support

Aspect ratios for ultrawide displays, such as 21:9 or even larger, are supported out of the box, with options to adjust the placement of the UI to the edges or in its original 16:9 area if desired. No external codes are required\!

> [!NOTE]
> Cutscenes by default are locked to their original aspect ratio to prevent [presentation issues](#ultrawide-visual-issues), as the game was not designed to present these scenes with wider aspect ratios. However, you can use the included option to unlock this feature if you don’t mind these issues.

### Low Input Latency

Modern input latency reduction techniques are included to improve the game’s responsiveness as much as possible, such as support for [Waitable Swap Chains](https://learn.microsoft.com/en-us/windows/uwp/gaming/reduce-latency-with-dxgi-1-3-swap-chains) in both D3D12 and Vulkan. This methodology was applied directly before the game’s input polling to minimize latency. Additionally, the D3D12 backend also supports [flip models](https://devblogs.microsoft.com/directx/dxgi-flip-model/), allowing the game to directly present to the screen as quickly as possible.

### Asynchronous Shader Compilation

One of the biggest improvements that the Recompilation features over emulators is the fact that Pipeline Compilation, also commonly known as Shader Compilation, is directly integrated into the game as part of the asset loading process. This means there’s **no stutters during gameplay the first time new objects or effects appear**. The renderer will traverse the game’s rendering structures and automatically determine what pipelines must be compiled before it considers the asset is loaded. As a result, pipeline compilation is performed in parallel as part of the game’s background workers that take care of streaming in the assets, either during regular gameplay or as part of the game’s loading screens. This is an improvement that’d never be possible with an emulator as it required direct modification of the game to implement this feature.

While the system is very extensive, some special shaders such as post processing effects or 2D elements are not detected ahead of time, so a list of pre-determined pipelines are compiled during the game’s introduction sequence. However, the amount is so low that this is done completely in the background and it was determined there was no need to show a shader compilation screen in most systems that were tested.

> [!NOTE]
> While stutters from shader compilation are non-existent, please be aware that [some stutters](#unavoidable-stutters) may be encountered due to the way the game was programmed. If you encounter these, keep in mind **these are not related to shader compilation**. Some of these issues might be addressed in future updates.

### Support For Xbox And PlayStation Button Icons

You can freely choose whether to use Xbox or PlayStation button icons. By default, the game will automatically detect which one to use based on your controller, but you can select a different option based on your personal preference from within the options menu.

Game objects that display controller icons such as Reaction Plates or Jump Selectors will automatically switch their textures to match the option in use. Even small details such as the Tornado Defense missions using different colors for the missiles have been accounted for.

### Quality Of Life Options

Multiple options have been integrated to address some common quality of life improvements that were deemed to be essential to the port:

- Hint rings and other types of hints provided by the game during exploration or boss fights can be disabled.
- Control tutorials, as referred to in current Sonic games, can be disabled to deactivate button prompts that show up during gameplay that teach the player how to use certain moves.
- The Werehog’s Battle Theme can be disabled, considered to be a very common annoyance in the original game due to its frequency and interruption of the stage’s background music.
- The Day-Night transformation cutscene in Towns can use either the Xbox 360 or PlayStation 3 version, with the Xbox version extending load time for the full video while the PS3 version ends as soon as possible
- Music Attenuation is a feature originally present in the Xbox 360 version of the game where it’d automatically mute the background music if an external media player was in use. This feature has been integrated by using the information provided by the [Windows Media Control](https://learn.microsoft.com/en-us/uwp/api/windows.media.control?view=winrt-26100) class provided in [WinRT](https://en.wikipedia.org/wiki/Windows_Runtime).

> [!NOTE]
> Please note that Music Attenuation is not currently available on Linux. Support for this feature may be added in a future update.

### Steam Deck Support

Native Linux builds that work on the Steam Deck out of the box are available. The Flatpak version can be installed directly and added as a non-Steam game easily through desktop mode.

Be aware that installing the game on a Steam Deck can be a slow process and can easily consume the Deck’s storage on the 64 GB Model, so make sure you provide the game’s files in external storage such as an SD Card or a Network Drive if possible.

Performance has been found to improve significantly when the Deck’s GPU clock is manually set to its maximum value through Game Scope, even if the game appears to be primarily CPU-bottlenecked. Be aware that maintaining the target frame rate may be difficult during demanding sections, such as Werehog combat or DLC stages. Overall, performance is considered to be noticeably better than on the original hardware while using the default settings.

> [!NOTE]
> It’s recommended to perform the installation process while on desktop mode, as the [File Picker might not be available in Game Mode](#file-picker-unavailable-on-steam-deck-in-game-mode) at all times.

### Mod Support

The game includes a mod loader compatible with [Hedge Mod Manager](https://github.com/thesupersonic16/HedgeModManager). As an option, users looking for Linux support can try the in-development version of [HMM 8](https://github.com/hedge-dev/HedgeModManager), which should work out of the box on platforms like the Steam Deck.

The mod format is the same standard as the one used for Generations mods. Mods that were originally developed for tools such as [Unleashed Mod Manager](https://github.com/hyperbx/Unleashed-Mod-Manager) are also supported for the sake of backwards compatibility. However, using this mod format is not recommended in the long term.

Modded files for the Xbox 360 version of the game are compatible with Unleashed Recompiled. However, please note that some mods may have used as a target the version of the game that doesn’t contain the Title Update, so some mods may have issues and require updates from their authors to work with the Recompilation.

> [!NOTE]
> Code modding is currently not possible and is [planned for a future update](#code-modding). As a workaround for the time being, some codes have been directly embedded into the game and can be accessed through Hedge Mod Manager’s Codes tab.

## Update Roadmap

### A number of different features are planned for future updates.

### Input Remapping

An option for changing the mappings of the actions to the controller buttons as well as support for keyboard bindings. In the meantime, refer to this [question in the FAQ](#what-are-the-keyboard-mappings) for the default keyboard bindings as well as where to edit them if necessary.

### High Frame Rate Fixes

A number of [gameplay issues](#high-frame-rate-glitches) are currently present that will gradually be fixed as more research into how the game works is done. At the moment, the game is considered to be beatable in High Frame Rates, but it may be required to limit it to 60 FPS to clear some optional content.

### Code Modding

Modifying the code of a Recompilation is a fundamentally different process than doing it for a game that only supports one platform on a single executable version. Everyone can build and fork Unleashed Recompiled on their own, which makes the method of targeting a single executable essentially impossible.

A convenient and maintainable method for code modding is under research and will come in a future update, and will work consistently across all the platforms that Unleashed Recompiled currently supports.

In the meantime, people interested in doing extensive code modding are recommended to fork the repository and introduce their changes in preparation for the code modding update.

### Stance On New Features

The team behind Unleashed Recompiled is committed to preserving the game's original design as best as possible. Additional features that don't fit the vision of the project will be left to Mods or Forks to cover. As such, requests for such features will not be considered. If you request a new feature, make sure it makes sense within the design of the original game and fits the goal of delivering a PC version of the game.

## Known Issues

### Unavoidable Stutters

While Unleashed Recompiled does its best effort on fixing any hitches and stutters that could originate from the translation process, some problems are not possible to fix due to the way the game was built.

Even if the Recompilation process introduces a small overhead, the game does not perform some tasks in an asynchronous way. These stutters exist on original hardware, but are much more noticeable when targeting higher frame rates.

Most of the known stutters are related to the game loading in layers of new objects at certain points in stages or simply spawning in particle systems, which can have fixed stutters nearly up to 10 milliseconds every time they’re created even on strong hardware. These hitches are much easier to notice when targeting frame rates such as 120 Hz, where one frame only lasts around 8 milliseconds.

These problems are not possible to mitigate without further research in how the game works and will remain a goal for future updates to fix if possible. However, [as mentioned in the FAQ](#how-can-i-improve-performance), it’s been found that these stutters can be significantly mitigated by playing on Linux, possibly due to the amount of synchronization primitives used by the game when these operations are performed.

### High Frame Rate Glitches

Sonic Unleashed is not a game that runs at a fixed rate in any of its target platforms. While it does target 30 FPS on Xbox 360 and 60 FPS on PlayStation 3, it doesn’t exactly hit this target often depending on the area, and if it does, the timestep itself is subject to large deviations. The game’s programming simply follows the timestep as closely as possible, which means inconsistencies in the timestep are impossible to reproduce without targeting one exact piece of hardware.

A lot of the bugs present in the game can largely be explained by sudden spikes in frametimes. To make matters worse, some bugs were found to be present only if the frame rate is too stable, such as this [infamous bug](https://github.com/hedge-dev/UnleashedRecomp/issues/100) that shows up in the Wii version of the game because the cutscene was recorded with a fixed timestep.

Unleashed Recompiled features a vast amount of fixes for issues that present themselves when targeting high frame rates, and fixing *all of them* is a task that is too big without more knowledge into how the game works. It is possible that these issues will be fixed in the future or that alternative solutions such as transform interpolation are implemented instead.

If you encounter game-breaking issues or unplayable sections, try temporarily limiting the framerate to resolve them. Also, check the [Issues](https://github.com/hedge-dev/UnleashedRecomp/issues) page for any existing reports.

### Ultrawide Visual Issues

Visual oddities may occur when unlocking the aspect ratio during cutscenes, as the game was not designed for wider formats. Adjusting these scenes can be subjective, so it will be up to mods to address these issues, as they require modifying the game's animations and object placements within cutscene files.

### Original Game Bugs

Game bugs present in the original hardware are intentionally preserved and will not be fixed. Please do not report issues for these bugs, and verify that the issue does not occur on original hardware before reporting. Bug reports for issues found in the original game will be rejected. Bugs that only happen in Unleashed Recompiled must be accompanied by footage captured in original hardware showing that the bug does not happen there.

### File Picker Unavailable On Steam Deck In Game Mode

Due to some restrictions of how the desktop environment in the Steam Deck works while in Game Mode, please note that you might need to at least first boot into Desktop mode to be able to use the file picker to provide the game files.

Simply booting at least once in Desktop Mode will enable the Deck to use the file picker when going back to Game Mode. You can complete the entire installation process while in Desktop mode to save yourself the trouble of browsing through game mode if necessary.

## FAQ

### Do you have a Discord server?

Unleashed Recompiled is not associated with any Discord servers. Use the [Issues](https://github.com/hedge-dev/UnleashedRecomp/issues) page if you require support.

### What are the keyboard mappings?

[PLACEHOLDER TABLE]
You can change the keyboard bindings by editing `config.toml` located in the [configuration directory](#where-is-the-save-file-and-the-configuration-stored), although using a gamepad is highly recommended until [Input Remapping](#input-remapping) is added in a future update.

### Where is the save file and the configuration stored?

### The save data and the configuration files are stored in the following paths:

- Windows: `%AppData%/UnleashedRecomp`
- Linux: `~/.config/UnleashedRecomp`

You will find the save data either under the folder `save` or under `mlsave` if using Hedge Mod Manager’s save file redirection. The configuration file is named `config.toml`.

### I want to update the game. How can I avoid losing my save file? Do I need to reinstall the game?

Updating the game can be done by simply copying and replacing the files from a [release](https://github.com/hedge-dev/UnleashedRecomp/releases) on top of your existing installation. **Your save file and configuration won’t be lost.** You won’t need to reinstall the game, as the game’s files will always remain the same across versions of Unleashed Recompiled.

### How can I force the game to store the save data and configuration on the installation folder?

You can make the game ignore the [default configuration paths](#where-is-the-save-file-and-the-configuration-stored) and force it to save everything on the installation directory by creating an empty `portable.txt` file. You’re directly responsible for safekeeping your save data and configuration if you choose this option.

### How can I install mods?

**Do not install mods by modifying the game’s folders.** Use [Hedge Mod Manager](https://github.com/thesupersonic16/HedgeModManager) instead. You will not get support for modifying your game’s files directly.

### How can I force the game to run the installation again?

While it’s unlikely you’ll need to do this unless you’ve modified your game files by accident, you can force the installer to run again by using the launch argument `--installer`.

### How can I force the game to run under X11/Wayland?

Use either of the following arguments to force SDL to run under the video driver you want.

- X11: `--sdl-video-driver x11`
- Wayland: `--sdl-video-driver wayland`

The second argument will be passed directly to SDL as a hint to try to initialize the game with your preferred option.

### Where is the game data for the Flatpak version installed?

Given it is not possible to run the game where the Flatpak is stored, the game data will be installed to `~/.var/app/io.github.hedge_dev.unleashedrecomp/data`. The Flatpak build will only recognize this directory as valid. Feel free to reuse this data directory with a native Linux build if you wish to switch in the future.

### How can I improve performance?

You can decrease the values of some of the following graphics options to improve performance. Other options can help, but these usually have the biggest impact.

- Resolution Scale
- Anti-aliasing
- Shadow Resolution

If you want a detailed performance report along with relevant system information, press F1 to view multiple performance graphs. This will aid the process of gathering as much information as possible to identify the problem.

When using a system with multiple GPUs, such as a gaming laptop, please make sure that the game has chosen your dedicated graphics card and not your integrated one. The F1 menu will display which device has been selected by its name along with other options that might be available. If you’re unable to get the game to select the correct device, you can attempt to override this by editing `config.toml`’s `GraphicsDevice` option. The name of the device must be an exact match.

Some of the game's more demanding sections require strong CPU single-thread performance. While the recompilation process adds minimal CPU overhead, modern hardware is typically bottlenecked by this factor before the GPU.

Linux has an unexpected advantage when it comes to CPU performance, showing improvements in CPU-bound scenarios. It’s currently speculated that this could be due to the heavy amount of thread synchronization the game performs, an operation that is likely to be more performant in Linux’s CPU scheduler than Windows. If you wish to gain some additional performance, playing on Linux instead of Windows could give you good results.

### Can I install the game with a PS3 copy?

You can’t use the files from the PS3 version of the game. Supporting these files would require an entire new recompilation, as the files have proprietary formatting that only works on the target platform and the code for these formats is only present in the PS3 version. All significant differences present in the PS3 version of the game have been included in this project as options.

### Can I install the game with a Japanese copy?

The Japanese version of Sonic Unleashed has some minor differences in both file structure and content that make the version of the game incompatible with the international release. Furthermore, the US and EU versions of the game already support Japanese. Supporting this version would only cause mod compatibility issues in the future, so it is unlikely to be added to the update roadmap as it would also require its own recompilation.

### Will macOS be supported?

While macOS is not currently on the roadmap, the project welcomes any effort to add support for this platform. Unleashed Recompiled relies on [plume](https://github.com/renderbag/plume), a rendering hardware abstraction that will be getting support for Metal in the near future. You can join the macOS discussion on [this issue](https://github.com/hedge-dev/UnleashedRecomp/issues/455).

### What other platforms will be supported?

The project does not plan to support any more platforms other than Windows, Linux and potentially macOS at the moment. Any other contributors wishing to support more platforms should do so through a fork.

## Building

[Check out the building instructions here](/BUILDING.md).

## Credits

### Unleashed Recompiled

* [Skyth](https://github.com/blueskythlikesclouds): TODO

-- TODO --

### Special Thanks

* [Mr-Wiseguy](https://github.com/Mr-Wiseguy): TODO

-- TODO --
