<p align="center">
    <img src="https://raw.githubusercontent.com/hedge-dev/UnleashedRecompResources/refs/heads/main/images/logo/Logo.png" width="512"/>
</p>

---

Unleashed Recompiled is an unofficial PC port of the Xbox 360 version of Sonic Unleashed created through the process of static recompilation. The port offers Windows and Linux support with numerous built-in enhancements such as high resolutions, ultrawide support, high frame rates, improved performance and modding.

**This project does not include any game assets. You must provide the files from your own legally acquired copy of the game to install or build Unleashed Recompiled.**

[Check out the latest release here](https://github.com/hedge-dev/UnleashedRecomp/releases/latest).

[XenonRecomp](https://github.com/hedge-dev/XenonRecomp) and [XenosRecomp](https://github.com/hedge-dev/XenosRecomp) are the main recompilers used for converting the game's original PowerPC code and Xenos shaders into compatible C++ and HLSL code respectively. The development of these recompilers was directly inspired by [N64: Recompiled](https://github.com/N64Recomp/N64Recomp), which was used to create [Zelda 64: Recompiled](https://github.com/Zelda64Recomp/Zelda64Recomp).

## Table of Contents

- [Minimum System Requirements](#minimum-system-requirements)
- [How to Install](#how-to-install)
- [Features](#features)
- [Update Roadmap](#update-roadmap)
- [Known Issues](#known-issues)
- [FAQ](#faq)
- [Building](#building)
- [Credits](#credits)

## Minimum System Requirements

- CPU with support for the AVX instruction set:
  - Intel: Sandy Bridge (Intel Core 2nd Generation)
  - AMD: Bulldozer (AMD FX series)
- GPU with support for Direct3D 12.0 (Shader Model 6) or Vulkan 1.2:
  - NVIDIA: GeForce GT 630 (Kepler)
  - AMD: Radeon HD 7750 (2012, not the RX 7000)
  - Intel: HD Graphics 510 (Skylake)
- Memory:
  - 8 GB minimum
- Operating System:
  - Windows 10 (version 1909)
  - A modern Linux distro such as Ubuntu 22.04 LTS
- Storage:
  - With DLC: 10 GiB required
  - Without DLC: 6 GiB required

> [!NOTE]
> More storage space may be required if uncompressed game files are provided during installation.

## How to Install

1) You must have access to the following:

    - Xbox 360 (modifications not necessary)
    - Xbox 360 Storage Device (either an Xbox 360 hard drive or an external USB storage device)
    - Xbox 360 Hard Drive Transfer Cable or a compatible SATA to USB adapter (only required for dumping from an Xbox 360 hard drive)
    - Sonic Unleashed for Xbox 360 (US or EU, **JP is not supported**)
        - Retail Disc or Digital Copy (can be purchased and downloaded from the [Xbox Store](https://www.xbox.com/en-US/games/store/sonic-unleashed/c098fgnmts8f)).
        - Title Update required.
        - All available DLC (Adventure Packs) are optional, but **highly recommended**. **The DLC includes high quality lighting for the entire game**.

> [!TIP]
> If you do not have the Xbox 360 Hard Drive Transfer Cable, please ensure that you purchase the correct revision of it for your console.
>
> The latest revision works with both original Xbox 360 and Xbox 360 S|E hard drives, but the first revision only works with original Xbox 360 hard drives.
>
> To know which is which, the first revision cable is gray, whereas the latest revision (which supports any Xbox 360 hard drive) is black.

2) **Before proceeding with the installation**, make sure to follow the guide on how to acquire the game files from your Xbox 360.

    - Xbox 360 Hard Drive Dumping Guide
        - [English](/docs/DUMPING-en.md)
    - Xbox 360 USB Dumping Guide
        - [English](/docs/DUMPING-USB-en.md)

3) Download [the latest release](https://github.com/hedge-dev/UnleashedRecomp/releases/latest) of Unleashed Recompiled and extract it to where you'd like the game to be installed.

4) Run the executable and you will be guided through the installation process. You will be asked to provide the files you acquired in the previous step. When presented with options for how to do this:

    - **Add Files** will only allow you to provide **containers or images dumped from an Xbox 360**. These often come in the form of very large files without associated extensions. Don't worry if you're not aware of what's inside of them, the installer will automatically detect what type of content is inside the container.

    - **Add Folder** will only allow you to provide a **directory with the game's raw files** corresponding to the piece of content that is requested. **It will NOT scan your folder for compatible content!**

> [!NOTE]
> Please note that it is **not possible** to complete the installation if your files have been **modified**. In case of other problems such as black screens or crashes, **do not try to reinstall the game** as it is not possible for the process to result in an invalid installation.

## Features

### Easy to Use Installer

A built-in installation wizard will guide you through the process of installing the game with many integrity checks to ensure the process goes as smoothly as possible. The installer can also be accessed from the title screen, if you wish to add the DLC at a later time.

### Options Menu

A completely new options menu accessible from the title screen or pause menu, with an unprecedented level of fidelity to the game's design language. Get access to many quality of life features and graphics options directly within the game with full controller navigation.

### Achievements

You will be rewarded with achievements as you progress through the game just like on its original platform. Achievements are recreated with integrated notifications and a new menu also very faithful to the game's design language. Get all of them and you will be rewarded with a gold trophy!

> [!NOTE]
> Achievements cannot be used with platforms such as Steam or RetroAchievements.

### Custom Localization

All of the new menus in the port fully support localization for each of the game's originally supported languages; English, Japanese, German, French, Spanish and Italian. As a bonus, switching the game to Japanese also changes the title screen logo to its original Japanese counterpart.

### High Fidelity

Special care and attention was taken to recreate the game's visuals as accurately as possible and was always compared to the game running on the original hardware. The colors are as vibrant as those of the PlayStation 3 version, which did not use the color correction filter present in the Xbox 360 version, as was originally intended. Nevertheless, an option to recreate the original warm filter from the Xbox 360 version has been included as an option.

### High Resolution Enhancements

Many improvements have been provided to accompany support for higher resolutions:

- Multisample Anti-Aliasing (MSAA) levels beyond the 2x used by the original game.
- Higher quality Depth of Field (DoF), an effect which commonly breaks in emulators when increasing the game's resolution. The original effect's formula was reverse-engineered and versions with 5x5, 7x7 and 9x9 taps were added based on the target resolution.
- An "Enhanced" Motion Blur option which uses more samples than the original for a smoother blur.
- Support for "Alpha to Coverage" Anti-Aliasing to enhance the visual result of transparent textures with hard edges.
- A Bicubic Texture Filtering option which greatly enhances the visual result of Global Illumination textures.
- A more precise implementation of the game's reverse Z technique, eliminating most Z-fighting issues and fixing the jittery motion blur in stages like Jungle Joyride.

### High Performance Renderer

A new renderer was written from scratch to translate the game's draw calls to modern APIs in a highly efficient way while also taking advantage of multi-threading.

As emulation of the Xbox 360's GPU is not required in a recompilation, many decisions were made to skip quirks of the original hardware that are not required in a PC port, resulting in great improvements to performance.

Modern rendering techniques such as bindless textures and shader specialization are also used to maximize the performance on systems with modern hardware whilst also supporting a wide range of lower end hardware.

Multiple optimizations were made to the game's shadow map rendering and special techniques were developed to automatically detect and skip unnecessary texture copies that are no longer needed in modern APIs.

Additionally, to support the game's extensive use of asset streaming, parallel transfer queues are leveraged, a feature only available in modern APIs that enable efficient use of available PCI-E bandwidth.

### High Frame Rate Support

The game's frame rate cap has been increased by default to 60 FPS, with support for higher targets and unlocked frame rate being available from the options menu. A vast amount of glitches that usually occur at higher frame rates have been fixed and are included as part of the recompilation.

> [!NOTE]
> While the game is considered to be beatable at frame rates higher than 60 FPS, please note that [some issues](#high-frame-rate-glitches) can still occur. Some of these issues may be addressed in future updates.

### Ultrawide Support

Aspect ratios for ultrawide displays (such as 21:9 or even wider) are supported out of the box, with options to adjust the alignment of the UI to the edges or to the original 16:9 safe area, if desired. No external codes are required!

> [!NOTE]
> By default, cutscenes are locked to their original aspect ratio to prevent [presentation issues](#ultrawide-visual-issues), as the game was not designed to present these scenes with wider aspect ratios. However, you can use the included option to unlock this feature if you don't mind these issues.

### Extended Controller Features

Support for the D-Pad has been added to various parts of the game, allowing the full game to be completed using it over the analog stick, if you so desire.

If you have a DualShock 4 or DualSense controller, the LED will dynamically change color depending on the game context and support for the touchpad has been added to the World Map, allowing you to spin the planet freely!

> [!NOTE]
> LED and touchpad support for the DualShock 4 and DualSense controllers may be limited when using third-party input translation layers (such as DS4Windows or Steam Input).
>
> Support for both features may also be limited on Linux.

### Low Input Latency

Modern input latency reduction techniques are included to improve the game's responsiveness as much as possible, such as support for [Waitable Swap Chains](https://learn.microsoft.com/en-us/windows/uwp/gaming/reduce-latency-with-dxgi-1-3-swap-chains) in both D3D12 and Vulkan. This methodology was applied directly before the game's input polling to minimize latency. Additionally, the D3D12 backend also supports [Flip Models](https://devblogs.microsoft.com/directx/dxgi-flip-model/), allowing the game to directly present to the screen as quickly as possible.

### Asynchronous Shader Compilation

One of the biggest improvements that the recompilation features over emulators is the fact that Pipeline Compilation (commonly known as "Shader Compilation") is directly integrated into the game as part of the asset loading process. This means there's **no stutters during gameplay the first time new objects or effects appear**.

The renderer will traverse the game's rendering structures and automatically determine what pipelines must be compiled before it considers the asset is loaded. As a result, pipeline compilation is performed in parallel as part of the game's background workers that take care of streaming in the assets, either during regular gameplay or as part of the game's loading screens. This is an improvement that'd never be possible with an emulator, as it requires direct modification of the game to implement this feature.

While this system is very extensive, some special shaders such as post-processing effects or 2D elements are not detected ahead of time, so a list of pre-determined pipelines are compiled during the game's boot sequence. However, the amount is so low that this is done completely in the background and it was determined that there was no need to show a shader compilation screen on most systems that were tested.

> [!NOTE]
> While stutters from shader compilation are non-existent, please be aware that [some stutters](#unavoidable-stutters) may be encountered due to the way the game was programmed. If you encounter these, please keep in mind that **these are not related to shader compilation**. Some of these issues may be addressed in future updates.

### Support for Xbox and PlayStation Controller Icons

You can freely choose whether to use Xbox 360 or PlayStation 3 controller icons. By default, the game will automatically detect which to use based on your controller, but you can select a different option based on your personal preference from within the options menu.

Game objects that display controller icons such as Reaction Plates or Jump Selectors will automatically switch their textures to match the option in use. Even small details such as the Tornado Defense missions using different colors for the missiles have been accounted for.

### Quality of Life Options

Many options have been integrated to address some common quality of life improvements that were deemed to be essential to the port:

- Hint rings and other types of hints provided by the game during exploration or boss fights can be disabled.
- Control tutorials (as referred to by current Sonic games) can be disabled to remove button prompts that show up during gameplay to teach the player how to use certain moves.
- The Werehog's Battle Theme, commonly considered to be an annoyance in the original game due to its frequency and interruption of the stage's background music, can now be disabled.
- The day/night transformation cutscene in towns can use either the Xbox 360 or PlayStation 3 version, with the Xbox version artificially extending loading times for the full video play out, whilst the PlayStation version ends as soon as it's done loading.
- Music Attenuation is a feature that was originally present in the Xbox 360 version of the game, where it'd automatically mute the background music if the console's media player was in use. This feature has been implemented using information provided by the [Windows Media Control](https://learn.microsoft.com/en-us/uwp/api/windows.media.control?view=winrt-26100) APIs in [WinRT](https://en.wikipedia.org/wiki/Windows_Runtime). Applications that interface with Windows 10/11 to display media controls are supported.

> [!TIP]
> You may refer to Music Presence's [list of supported media players](https://github.com/ungive/discord-music-presence/blob/master/documentation/supported-media-players.md) for players that work with Music Attenuation out of the box.

> [!NOTE]
> Please note that Music Attenuation is not currently available on Linux. Support for this feature may be added in a future update.

### Steam Deck Support

Native Linux builds that work on the Steam Deck out of the box are available. The Flatpak version can be installed directly and added as a non-Steam game easily via Desktop Mode.

Be aware that installing the game on a Steam Deck can be a slow process and can easily consume the Deck's storage on the 64 GB model, so make sure you provide the game's files via external storage, such as a microSD card or a network share if possible.

Performance has been found to improve significantly when the Deck's GPU clock is manually set to its maximum value via Gamescope, even if the game appears to be primarily CPU bottlenecked. Be aware that maintaining the target frame rate may be difficult during demanding sections, such as Werehog combat or DLC stages. Overall, performance is considered to be noticeably better than on the original hardware while using the default settings.

> [!NOTE]
> It is recommended to perform the installation process while in Desktop Mode, as the [file picker may not be available in Game Mode](#file-picker-unavailable-on-steam-deck-in-game-mode) at all times.

### Mod Support

The game includes a mod loader compatible with [Hedge Mod Manager](https://github.com/thesupersonic16/HedgeModManager). As an option, users looking for Linux support should try the development version of [HMM 8](https://github.com/hedge-dev/HedgeModManager), which should work out of the box on platforms such as the Steam Deck.

The mod format uses the same standard as the one used by Sonic Generations mods. Mods that were originally created for [Unleashed Mod Manager](https://github.com/hyperbx/Unleashed-Mod-Manager) are also supported for the sake of backwards compatibility. However, using this mod format is not recommended in the long term and should now be considered deprecated.

Modded files for the Xbox 360 version of the game are compatible with Unleashed Recompiled. However, some mods may have targeted a version of the game that doesn't contain the title update, so those may have issues and require updates from their authors in order to work with the recompilation. Mods that replace the game's executable file (`default.xex`) or shaders are not supported.

> [!NOTE]
> Code modding is currently not possible and is [planned for a future update](#code-modding). As a workaround for the time being, some codes have been directly embedded into the game and can be accessed through Hedge Mod Manager's Codes tab.
>
> For information on the mod format, check out Hedge Mod Manager's [Basic Mod File Structure](https://github.com/thesupersonic16/HedgeModManager/wiki/Basic-Mod-File-Structure) wiki page.

## Update Roadmap

A number of different features are planned for future updates.

### Action Remapping

Options for changing the bindings of the player actions to the controller buttons, as well as support for keyboard bindings. In the meantime, refer to this [question in the FAQ](#what-are-the-keyboard-bindings) for the default keyboard bindings, as well as where to edit them if necessary.

### More Linux Builds

Linux builds that don't require Flatpak will be added in the future when a way to [create the shortcuts required for HedgeModManager](https://github.com/hedge-dev/UnleashedRecomp/issues/451) is implemented. In the meantime, you can access these builds through the [Actions](https://github.com/hedge-dev/UnleashedRecomp/actions) panel or [build them yourself](#building).

### High Frame Rate Fixes

A number of [gameplay issues](#high-frame-rate-glitches) are currently present that will gradually be fixed as more research into how the game works is done. At the moment, the game is considered to be beatable at high frame rates, but you may be required to limit the game to 60 FPS in order to clear some optional content.

### Code Modding

Modifying the code of a recompilation is a fundamentally different process than doing it for a game that only supports one platform on a single executable version. Everyone can build and fork Unleashed Recompiled on their own, which makes the method of targeting a single executable essentially impossible.

A convenient and maintainable method for code modding is under research and will come in a future update, which will work consistently across all the platforms that Unleashed Recompiled currently supports.

In the meantime, those interested in doing extensive code modding are recommended to fork the repository and introduce their changes in preparation for the code modding update.

### Stance on New Features

The team behind Unleashed Recompiled is committed to preserving the game's original design as best as possible. Additional features that don't fit the vision of the project will be left to mods or forks to cover. As such, requests for such features will not be considered. If you request a new feature, make sure it makes sense within the design of the original game and fits the goal of delivering a PC version of the game.

## Known Issues

### Unavoidable Stutters

While Unleashed Recompiled does its best effort on fixing any hitches and stutters that could originate from the translation process, some problems are not possible to fix due to the way the game was built.

Even if the recompilation process introduces a small overhead, the game does not perform some tasks in an asynchronous way. These stutters exist on original hardware, but are much more noticeable when targeting higher frame rates.

Most of the known stutters are related to the game loading in layers of new objects at certain points in stages, spawning in particle systems or playing multiple sound effects. These operations can produce pauses that can last almost 10 milliseconds, even on powerful hardware. These hitches are much easier to notice when targeting frame rates such as 120 FPS, where one frame only lasts around 8 milliseconds.

These problems are not possible to mitigate without further research into how the game works and will remain a goal for future updates to fix if possible. However, [as mentioned in the FAQ](#how-can-i-improve-performance), it's been found that these stutters can be significantly mitigated by playing on Linux, possibly due to the amount of synchronization primitives used by the game when these operations are performed.

### High Frame Rate Glitches

Sonic Unleashed is not a game that runs at a fixed rate on any of its target platforms. While it does target 30 FPS on Xbox 360 and is uncapped on PlayStation 3, it doesn't exactly hit these targets often depending on the area, and if it does, the time step itself is subject to large deviations. The game's programming simply follows the time step as closely as possible, which means inconsistencies in the time step are impossible to reproduce without targeting one exact piece of hardware.

A lot of the bugs present in the game can largely be explained by sudden spikes in frame times. To make matters worse, some bugs were found to be present only if the frame rate is too stable, such as this [infamous bug](https://github.com/hedge-dev/UnleashedRecomp/issues/100) that shows up in the Wii version of the game because the cutscene was recorded with a fixed time step.

Unleashed Recompiled features a vast amount of fixes for issues that present themselves when targeting high frame rates, but fixing *all of them* is too big of a task without more knowledge of how the game works. It is possible that these issues will be fixed in the future or that alternative solutions such as transform interpolation are implemented instead.

If you encounter game breaking bugs or unplayable sections, try temporarily limiting the frame rate to resolve them. Also, check the [Issues](https://github.com/hedge-dev/UnleashedRecomp/issues) page for any existing reports.

### Ultrawide Visual Issues

Visual oddities may occur when unlocking the aspect ratio during cutscenes, as the game was not designed for wider formats. Adjusting these scenes can be subjective, so it will be up to mods to address these issues, as they require modifying the game's animations and object placement within cutscene files.

### Original Game Bugs

Game bugs present on the original hardware are intentionally preserved and will not be fixed. Please do not report issues for these bugs and verify that the issue does not occur on original hardware before reporting. Bug reports for issues found in the original game will be rejected. Bugs that only happen in Unleashed Recompiled must be accompanied by footage captured on original Xbox 360 hardware showing that the bug does not happen there.

### File Picker Unavailable on Steam Deck in Game Mode

Due to some restrictions of how the desktop environment on the Steam Deck works whilst in Game Mode, please note that you may need to at least first boot into Desktop Mode to be able to use the file picker to provide the game files.

Simply booting at least once in Desktop Mode will enable the Deck to use the file picker when going back to Game Mode. You can complete the entire installation process while in Desktop Mode to save yourself the trouble of browsing through Game Mode if necessary.

## FAQ

### Do you have a website or Discord server?

Unleashed Recompiled does not have an official website, nor is it affiliated with any Discord servers.

**Please link here when directing anyone to the project.**

> [!CAUTION]
> Do not download builds of Unleashed Recompiled from anywhere but our [Releases](https://github.com/hedge-dev/UnleashedRecomp/releases/latest) page.
>
> **We will never distribute builds on other websites, via Discord servers or via third-party update tools.**

### Why does the installer say my files are invalid?

The installer may display this error for several reasons. Please check the following to ensure your files are valid:

- Please read the [How to Install](#how-to-install) section and make sure you've acquired all of the necessary files correctly.

- Verify that you're not trying to add compressed files such as `.zip`, `.7z`, `.rar` or other formats.

- Only use the **Add Folder** option if you're sure you have a directory with the content's files already extracted, which means it'll only contain files like `.xex`, `.ar.00`, `.arl` and others. **This option will not scan your folder for compatible content**.

- Ensure that the files you've acquired correspond to the same region. **Discs and Title Updates from different regions can't be used together** and will fail to generate a patch.

- The installer will only accept **original and unmodified files**. Do not attempt to provide modified files to the installer.

### What are the keyboard bindings?

Pad|Key
-|-
A (Cross)|S
B (Circle)|D
X (Square)|A
Y (Triangle)|W
D-Pad - Up|Unbound
D-Pad - Down|Unbound
D-Pad - Left|Unbound
D-Pad - Right|Unbound
Start|Return
Back (Select)|Backspace
Left Trigger (L2)|1
Right Trigger (R2)|3
Left Bumper (L1)|Q
Right Bumper (R1)|E
Left Stick - Up|Up Arrow
Left Stick - Down|Down Arrow
Left Stick - Left|Left Arrow
Left Stick - Right|Right Arrow
Right Stick - Up|Unbound
Right Stick - Down|Unbound
Right Stick - Left|Unbound
Right Stick - Right|Unbound

---

You can change the keyboard bindings by editing `config.toml` located in the [configuration directory](#where-is-the-save-data-and-configuration-file-stored), although using a controller is highly recommended until [Action Remapping](#action-remapping) is added in a future update.

Refer to the left column of [this enum template](https://github.com/hedge-dev/UnleashedRecomp/blob/main/UnleashedRecomp/user/config.cpp#L40) for a list of valid keys.

*The default keyboard layout is based on Devil's Details' keyboard layout for Sonic Generations (2011)*.

### Where is the save data and configuration file stored?

The save data and configuration files are stored at the following locations:

- Windows: `%APPDATA%\UnleashedRecomp\`
- Linux: `~/.config/UnleashedRecomp/`

You will find the save data under the `save` folder (or `mlsave`, if using Hedge Mod Manager's save file redirection). The configuration file is named `config.toml`.

### I want to update the game. How can I avoid losing my save data? Do I need to reinstall the game?

Updating the game can be done by simply copying and replacing the files from a [release](https://github.com/hedge-dev/UnleashedRecomp/releases) on top of your existing installation. **Your save data and configuration will not be lost.** You won't need to reinstall the game, as the game files will always remain the same across versions of Unleashed Recompiled.

### How can I force the game to store the save data and configuration in the installation folder?

You can make the game ignore the [default configuration paths](#where-is-the-save-data-and-configuration-file-stored) and force it to save everything in the installation directory by creating an empty `portable.txt` file. You are directly responsible for the safekeeping of your save data and configuration if you choose this option.

### How can I install mods?

**Do not install mods by modifying the game data**. Use [Hedge Mod Manager](https://github.com/thesupersonic16/HedgeModManager) instead. You will not get support for modifying your game files directly.

> [!WARNING]
> Unleashed Mod Manager is not recommended for use with Unleashed Recompiled, as it can make permanent changes to your game files.

### How can I force the game to run the installation again?

While it's unlikely you'll need to do this unless you've modified your game files by accident, you can force the installer to run again by using the launch argument: `--install`.

### How can I force the game to run under X11 or Wayland?

Use either of the following arguments to force SDL to run under the video driver you want:

- X11: `--sdl-video-driver x11`
- Wayland: `--sdl-video-driver wayland`

The second argument will be passed directly to SDL as a hint to try to initialize the game with your preferred option.

### Where is the game data for the Flatpak version installed?

Given it is not possible to run the game where the Flatpak is stored, the game data will be installed to `~/.var/app/io.github.hedge_dev.unleashedrecomp/data`. The Flatpak build will only recognize this directory as valid. Feel free to reuse this data directory with a native Linux build if you wish to switch in the future.

If you wish to move this data to another location, you can do so by creating a symlink from this directory to the one where you'll migrate your installation to.

### How can I improve performance?

You can lower the values of some of the following graphics options to improve performance. Other options may help, but these usually have the biggest impact:

- Resolution Scale
- Anti-Aliasing
- Shadow Resolution

If you want a detailed performance report along with relevant system information, press F1 to view multiple performance graphs. This will aid in the process of gathering as much information as possible in order to identify the problem.

When using a system with multiple GPUs (such as a gaming laptop), please make sure that the game has chosen your dedicated graphics adapter and not your integrated one. The F1 menu will display which device has been selected by its name along with other options that might be available. If you're unable to get the game to select the correct device, you can attempt to override this by changing the `GraphicsDevice` property in `config.toml`. The name of the device must be an exact match.

Some of the game's more demanding sections require strong CPU single-thread performance. While the recompilation process adds minimal CPU overhead, modern hardware is typically bottlenecked by this factor before the GPU.

Linux has an unexpected advantage when it comes to CPU performance, showing improvements in CPU-bound scenarios. It's currently speculated that this could be due to the heavy amount of thread synchronization the game performs, an operation that is likely to be more performant on Linux's CPU scheduler than on Windows' scheduler. If you wish to gain some additional performance, playing on Linux instead of Windows could yield better results.

> [!WARNING]
> Using external frame rate limiters or performance overlays may degrade performance or have negative consequences.

### Can I install the game with a PlayStation 3 copy?

**You cannot use the files from the PlayStation 3 version of the game.** Supporting these files would require an entirely new recompilation, as they have proprietary formatting that only works on PS3 and the code for these formats is only present in that version. All significant differences present in the PS3 version of the game have been included in this project as options.

### Can I install the game with a Japanese copy?

The Japanese version of Sonic Unleashed has some minor differences in both file structure and content that make this version of the game incompatible with the international release. Furthermore, the US and EU versions of the game already support Japanese. Supporting this version would only cause mod compatibility issues in the future, so it is unlikely to be added to the update roadmap as it would also require its own recompilation.

### Why is the game detecting my PlayStation controller as an Xbox controller?

If you're using a third-party input translation layer (such as DS4Windows or Steam Input), it is recommended that you disable these for full controller support.

### Will macOS be supported?

While macOS is not currently on the roadmap, this project welcomes any effort to add support for this platform. Unleashed Recompiled relies on [plume](https://github.com/renderbag/plume), a rendering hardware abstraction layer that will be getting support for Metal in the near future. You can join the macOS discussion on [this issue](https://github.com/hedge-dev/UnleashedRecomp/issues/455).

### What other platforms will be supported?

This project does not plan to support any more platforms other than Windows, Linux and potentially macOS at the moment. Any contributors who wish to support more platforms should do so through a fork.

### Do you have plans to recompile other Xbox 360 games or Sonic games?

The team behind Unleashed Recompiled does not currently have any plans to port more Xbox 360 games or Sonic games. Anyone is free to look into [XenonRecomp](https://github.com/hedge-dev/XenonRecomp) if they wish to investigate these opportunities.

## Building

[Check out the building instructions here](/docs/BUILDING.md).

## Credits

### Unleashed Recompiled
- [Skyth](https://github.com/blueskythlikesclouds): Creator and Lead Developer of the recompilation, as well as the developer of technologies created for it such as [XenonRecomp](https://github.com/hedge-dev/XenonRecomp) and [XenosRecomp](https://github.com/hedge-dev/XenosRecomp). Other responsibilities include the creation of the graphics and audio backends for the project, alongside custom menus, dynamic UI aspect ratio and various patches and new features added to the game.

- [Sajid](https://github.com/Sajidur78): Co-creator and Developer of the recompilation, as well as the developer of [XenonAnalyse](https://github.com/hedge-dev/XenonRecomp/?tab=readme-ov-file#XenonAnalyse). Other responsibilities include the implementation of core components for the project, like the Xbox 360 kernel translation layer used to make the game function.

- [Hyper](https://github.com/hyperbx): Developer of system level features, such as achievement support and the custom menus, alongside various other patches and features to make the game feel right at home on modern systems. Aided in the creation of concept art and the final options menu thumbnails.

- [Darío](https://github.com/DarioSamo): Creator of the graphics hardware abstraction layer [plume](https://github.com/renderbag/plume), used by the project's graphics backend. Alongside providing consultation for graphics and aiding with shader research and development, other responsibilities include the installer wizard and Linux support. Provided Spanish localization for the custom menus.

- [ĐeäTh](https://github.com/DeaTh-G): Supervisor of game accurate design philosophy regarding the custom menus. Aided in the implementation of annotation support for Japanese localization, whilst providing minor support for all localization.

- [RadiantDerg](https://github.com/RadiantDerg): Lead Artist behind the thumbnails used in the options menu. Other responsibilities include the creation of several debugging related codes for Hedge Mod Manager and providing aid with the research of the game's internals.

- [PTKay](https://github.com/PTKay): Lead Concept Artist for the custom menus. Aided in the development of the installer wizard's visuals.

- [SuperSonic16](https://github.com/thesupersonic16): Lead Developer of [Hedge Mod Manager](https://github.com/thesupersonic16/HedgeModManager), providing compatibility for modding with the recompilation. Aided in the creation of the deployment system for Linux builds.

- [NextinHKRY](https://github.com/NextinMono): Aided in researching the game's internals and creating concept art for some options menu thumbnails used in the final release. Provided Italian localization for the custom menus.

- [LadyLunanova](https://linktr.ee/ladylunanova): Artist behind the achievement trophy sprite and the keyboard and mouse icons used in the installer wizard. 

- [LJSTAR](https://github.com/LJSTARbird): Artist behind the project logo, along with several thumbnail designs used in the options menu and created new icons for the button guide for opening the achievements menu. Provided French localization for the custom menus.

- [saguinee](https://twitter.com/saguinee): Artist behind thumbnail designs used in the options menu such as Hints and Battle Theme.

- [Goalringmod27](https://linktr.ee/goalringmod27): Concept Artist behind the achievements overlay shown during gameplay. Aided in the creation of the Transparency Anti-Aliasing thumbnail.

- [M&M](https://github.com/ActualMandM): Provisional support for dynamic UI aspect ratio.

- [DaGuAr](https://twitter.com/TheDaguar): Provided Spanish localization for the custom menus alongside Darío.

- [brianuuuSonic](https://github.com/brianuuu): Provided Japanese localization for the custom menus.

- [Kitzuku](https://github.com/Kitzuku): Provided German localization for the custom menus.

### Special Thanks
- [Mr-Wiseguy](https://github.com/Mr-Wiseguy): Creator of [N64: Recompiled](https://github.com/N64Recomp/N64Recomp), which was the inspiration behind the creation of this project. Provided information and assistance at the beginning of development.

- [xenia-project](https://github.com/xenia-project/xenia): Extraordinary amounts of research regarding the inner workings of the Xbox 360, which sped up the development of the recompilation.

- [Katlin Daigler](https://katlindaigler.carbonmade.com): Provided consultation for logo design.

- [ocornut](https://github.com/ocornut): Creator of [Dear ImGui](https://github.com/ocornut/imgui), which is used as the backbone of the custom menus.

- Raymond Chen: Useful resources on Windows application development with his blog ["The Old New Thing"](https://devblogs.microsoft.com/oldnewthing/).
