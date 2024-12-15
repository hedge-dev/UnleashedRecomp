#include <locale/locale.h>

std::unordered_map<std::string, std::unordered_map<ELanguage, std::string>> g_locale =
{
    {
        "Options_Category_System",
        {
            { ELanguage::English, "SYSTEM" }
        }
    },
    {
        "Options_Category_Input",
        {
            { ELanguage::English, "INPUT" }
        }
    },
    {
        "Options_Category_Audio",
        {
            { ELanguage::English, "AUDIO" }
        }
    },
    {
        "Options_Category_Video",
        {
            { ELanguage::English, "VIDEO" }
        }
    },
    {
        "Options_Value_Max",
        {
            { ELanguage::English, "MAX" }
        }
    },
    {
        "Options_Name_WindowSize",
        {
            { ELanguage::English, "Window Size" }
        }
    },
    {
        "Options_Desc_WindowSize",
        {
            { ELanguage::English, "Adjust the size of the game window in windowed mode." }
        }
    },
    {
        "Options_Desc_NotAvailable",
        {
            { ELanguage::English, "This option is not available at this location." }
        }
    },
    {
        "Options_Desc_NotAvailableMSAA",
        {
            { ELanguage::English, "This option is not available without MSAA." }
        }
    },
    {
        "Options_Desc_OSNotSupported",
        {
            { ELanguage::English, "This option is not supported by your operating system." }
        }
    },
    {
        "Options_Message_Restart",
        {
            { ELanguage::English, "The game will now restart." }
        }
    },
    {
        "Achievements_Name",
        {
            { ELanguage::English, "Achievements" }
        }
    },
    {
        "Achievements_Name_Uppercase",
        {
            { ELanguage::English, "ACHIEVEMENTS" }
        }
    },
    {
        "Achievements_Unlock",
        {
            { ELanguage::English, "Achievement Unlocked!" }
        }
    },
    {
        "Installer_Header_Installer",
        {
            { ELanguage::English, "INSTALLER" },
            { ELanguage::Spanish, "INSTALADOR" },
        },
    },
    {
        "Installer_Header_Installing",
        {
            { ELanguage::English, "INSTALLING" },
            { ELanguage::Spanish, "INSTALANDO" },
        }
    },
    {
        "Installer_Page_SelectLanguage",
        {
            { ELanguage::English, "Please select a language." }
        }
    },
    {
        "Installer_Page_Introduction",
        {
            { ELanguage::English, "Welcome to Unleashed Recompiled!\n\nYou'll need an Xbox 360 copy\nof Sonic Unleashed in order to proceed with the installation." }
        }
    },
    {
        "Installer_Page_SelectGameAndUpdate",
        {
            { ELanguage::English, "Add the files for the game and its title update. You can use digital dumps or pre-extracted folders containing the necessary files." }
        }
    },
    {
        "Installer_Page_SelectDLC",
        {
            { ELanguage::English, "Add the files for the DLC. You can use digital dumps or pre-extracted folders containing the necessary files." }
        }
    },
    {
        "Installer_Page_CheckSpace",
        {
            { ELanguage::English, "The content will be installed to the program's folder. Please confirm you have enough free space.\n\n" }
        }
    },
    {
        "Installer_Page_Installing",
        {
            { ELanguage::English, "Please wait while the content is being installed..." }
        }
    },
    {
        "Installer_Page_InstallSucceeded",
        {
            { ELanguage::English, "Installation complete.\n\nThis project is brought to you by:\n\n" }
        }
    },
    {
        "Installer_Page_InstallFailed",
        {
            { ELanguage::English, "Installation failed.\n\nError:\n\n" }
        }
    },
    {
        "Installer_Step_Game",
        {
            { ELanguage::English, "GAME" }
        }
    },
    {
        "Installer_Step_Update",
        {
            { ELanguage::English, "UPDATE" }
        }
    },
    {
        "Installer_Step_RequiredSpace",
        {
            { ELanguage::English, "Required space: %2.2f GiB" }
        }
    },
    {
        "Installer_Step_AvailableSpace",
        {
            { ELanguage::English, "Available space: %2.2f GiB" }
        }
    },
    {
        "Installer_Button_Next",
        {
            { ELanguage::English, "NEXT" },
            { ELanguage::Spanish, "SIGUIENTE" },
            { ELanguage::German, "WEITER" },
        }
    },
    {
        "Installer_Button_Skip",
        {
            { ELanguage::English, "SKIP" },
            { ELanguage::Spanish, "SALTAR" },
            { ELanguage::German, "ÜBERSPRINGEN" },
        }
    },
    {
        "Installer_Button_AddFiles",
        {
            { ELanguage::English, "ADD FILES" },
            { ELanguage::Spanish, "AÑADIR ARCHIVOS" },
            { ELanguage::German, "DATEIEN HINZUFÜGEN" },
        }
    },
    {
        "Installer_Button_AddFolder",
        {
            { ELanguage::English, "ADD FOLDER" },
            { ELanguage::Spanish, "AÑADIR CARPETA" },
            { ELanguage::German, "ORDNER HINZUFÜGEN" },
        }
    },
    {
        "Installer_Message_InvalidFilesList",
        {
            { ELanguage::English, "The following selected files are invalid:" }
        }
    },
    {
        "Installer_Message_InvalidFiles",
        {
            { ELanguage::English, "Some of the files that have\nbeen provided are not valid.\n\nPlease make sure all the\nspecified files are correct\nand try again." }
        }
    },
    {
        "Installer_Message_IncompatibleGameData",
        {
            { ELanguage::English, "The specified game and\nupdate file are incompatible.\n\nPlease ensure the files are\nfor the same version and\nregion and try again." }
        }
    },
    {
        "Installer_Message_DLCWarning",
        {
            { ELanguage::English, "It is highly recommended\nthat you install all of the\nDLC, as it includes high\nquality lighting textures\nfor the base game.\n\nAre you sure you want to\nskip this step?" }
        }
    },
    {
        "Installer_Message_TitleMissingDLC",
        {
            { ELanguage::English, "This will restart the game to\nallow you to install any DLC\nthat you are missing.\n\nInstalling DLC will improve the\nlighting quality across the game.\n\nWould you like to install missing\ncontent?" }
        }
    },
    {
        "Installer_Message_Title",
        {
            { ELanguage::English, "This restarts the game to\nallow you to install any DLC\nthat you may be missing.\n\nYou are not currently\nmissing any DLC.\n\nWould you like to proceed\nanyway?" }
        }
    },
    {
        "Title_Message_Quit",
        {
            { ELanguage::English, "Are you sure you want to quit?" }
        }
    },
    {
        "Common_On",
        {
            { ELanguage::English,  "ON" },
            { ELanguage::Japanese, "オン" },
            { ELanguage::German,   "EIN" },
            { ELanguage::French,   "OUI" },
            { ELanguage::Spanish,  "SÍ" },
            { ELanguage::Italian,  "SÌ" }
        }
    },
    {
        "Common_Off",
        {
            { ELanguage::English,  "OFF" },
            { ELanguage::Japanese, "オフ" },
            { ELanguage::German,   "AUS" },
            { ELanguage::French,   "NON" },
            { ELanguage::Spanish,  "NO" },
            { ELanguage::Italian,  "NO" }
        }
    },
    {
        "Common_Yes",
        {
            { ELanguage::English, "Yes" }
        }
    },
    {
        "Common_No",
        {
            { ELanguage::English, "No" }
        }
    },
    {
        "Common_Next",
        {
            { ELanguage::English, "Next" }
        }
    },
    {
        "Common_Select",
        {
            { ELanguage::English, "Select" }
        }
    },
    {
        "Common_Back",
        {
            { ELanguage::English, "Back" }
        }
    },
    {
        "Common_Reset",
        {
            { ELanguage::English, "Reset" }
        }
    },
    {
        "Common_Switch",
        {
            { ELanguage::English, "Switch" }
        }
    }
};

std::string& Localise(const char* key)
{
    if (!g_locale.count(key))
        return g_localeMissing;

    if (!g_locale[key].count(Config::Language))
    {
        if (g_locale[key].count(ELanguage::English))
        {
            return g_locale[key][ELanguage::English];
        }
        else
        {
            return g_localeMissing;
        }
    }

    return g_locale[key][Config::Language];
}
