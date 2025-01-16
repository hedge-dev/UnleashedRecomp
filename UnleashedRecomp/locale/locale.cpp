#include <user/config.h>
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
        "Options_Desc_NotAvailableFullscreen",
        {
            { ELanguage::English, "This option is not available in fullscreen mode." }
        }
    },
    {
        "Options_Desc_NotAvailableWindowed",
        {
            { ELanguage::English, "This option is not available in windowed mode." }
        }
    },
    {
        "Options_Desc_NotAvailableHardware",
        {
            { ELanguage::English, "This option is not available with your current hardware configuration." }
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
            { ELanguage::Italian, "INSTALLATORE" },
        },
    },
    {
        "Installer_Header_Installing",
        {
            { ELanguage::English, "INSTALLING" },
            { ELanguage::Spanish, "INSTALANDO" },
            { ELanguage::Italian, "INSTALLANDO" },
        }
    },
    {
        "Installer_Page_SelectLanguage",
        {
            { ELanguage::English, "Please select a language." },
            { ELanguage::Italian, "Seleziona una lingua." }
        }
    },
    {
        "Installer_Page_Introduction",
        {
            { ELanguage::English, "Welcome to Unleashed Recompiled!\n\nYou'll need an Xbox 360 copy\nof Sonic Unleashed in order to proceed with the installation." },
            { ELanguage::Italian, "Benvenuto a Unleashed Recompiled!\n\nDovrai avere una copia di\nSonic Unleashed per la Xbox 360\nper proseguire con l'installazione." }
        }
    },
    {
        "Installer_Page_SelectGameAndUpdate",
        {
            { ELanguage::English, "Add the files for the game and its title update. You can use digital dumps or pre-extracted folders containing the necessary files." },
            { ELanguage::Italian, "Aggiungi i file del gioco e del suo aggiornamento. Puoi utilizzare una copia digitale o delle cartelle contenenti i file necessari." }
        }
    },
    {
        "Installer_Page_SelectDLC",
        {
            { ELanguage::English, "Add the files for the DLC. You can use digital dumps or pre-extracted folders containing the necessary files." },
            { ELanguage::Italian, "Aggiungi i file dei DLC. Puoi utilizzare una copia digitale o cartelle contenenti i file necessari." }
        }
    },
    {
        "Installer_Page_CheckSpace",
        {
            { ELanguage::English, "The content will be installed to the program's folder. Please confirm you have enough free space.\n\n" },

            { ELanguage::Italian, "Il contenuto verrà installato nella cartella di questo programma. Assicurati di avere abbastanza spazio libero sul tuo hard disk.\n\n" }
        }
    },
    {
        "Installer_Page_Installing",
        {
            { ELanguage::English, "Please wait while the content is being installed..." },
            { ELanguage::Italian, "Attendi mentre il contenuto viene installato... " }
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
            { ELanguage::English, "Installation failed.\n\nError:\n\n" },
            { ELanguage::Italian, "Installazione fallita.\n\nErrore:\n\n" }
        }
    },
    {
        "Installer_Step_Game",
        {
            { ELanguage::English, "GAME" },
            { ELanguage::Italian, "GIOCO" }
        }
    },
    {
        "Installer_Step_Update",
        {
            { ELanguage::English, "UPDATE" },
            { ELanguage::Italian, "AGGIORNAMENTO" }
        }
    },
    {
        "Installer_Step_RequiredSpace",
        {
            { ELanguage::English, "Required space: %2.2f GiB" },
            { ELanguage::Italian, "Spazio necessario: %2.2f GiB" }
        }
    },
    {
        "Installer_Step_AvailableSpace",
        {
            { ELanguage::English, "Available space: %2.2f GiB" },
            { ELanguage::Italian, "Spazio disponibile: %2.2f GiB" }
        }
    },
    {
        "Installer_Button_Next",
        {
            { ELanguage::English, "NEXT" },
            { ELanguage::Spanish, "SIGUIENTE" },
            { ELanguage::German, "WEITER" },
            { ELanguage::Italian, "CONTINUA" }
        }
    },
    {
        "Installer_Button_Skip",
        {
            { ELanguage::English, "SKIP" },
            { ELanguage::Spanish, "SALTAR" },
            { ELanguage::German, "ÜBERSPRINGEN" },
            { ELanguage::Italian, "SALTA" }
        }
    },
    {
        "Installer_Button_AddFiles",
        {
            { ELanguage::English, "ADD FILES" },
            { ELanguage::Spanish, "AÑADIR ARCHIVOS" },
            { ELanguage::German, "DATEIEN HINZUFÜGEN" },
            { ELanguage::Italian, "AGGIUNGI FILE" }
        }
    },
    {
        "Installer_Button_AddFolder",
        {
            { ELanguage::English, "ADD FOLDER" },
            { ELanguage::Spanish, "AÑADIR CARPETA" },
            { ELanguage::German, "ORDNER HINZUFÜGEN" },
            { ELanguage::Italian, "AGGIUNGI CARTELLA" }
        }
    },
    {
        "Installer_Message_InvalidFilesList",
        {
            { ELanguage::English, "The following selected files are invalid:" },
            { ELanguage::Italian, "I seguenti file non sono validi:" }
        }
    },
    {
        "Installer_Message_InvalidFiles",
        {
            { ELanguage::English, "Some of the files that have\nbeen provided are not valid.\n\nPlease make sure all the\nspecified files are correct\nand try again." },
            { ELanguage::Italian, "Alcuni dei file che sono stati\nselezionati non sono validi.\n\nAssicurati che tutti\ni file sono quelli corretti\ne riprova." }
        }
    },
    {
        "Installer_Message_FilePickerTutorial",
        {
            { ELanguage::English, "Select a digital dump from\nyour Xbox 360.\n\nFor choosing a folder with\npre-existing game files,\nuse the \"Add Folder\"\noption instead." },
        }
    },
    {
        "Installer_Message_FolderPickerTutorial",
        {
            { ELanguage::English, "Select a folder that contains\nthe files that have been\nextracted from the game.\n\nFor choosing a digital dump\nfrom your Xbox 360, use the\n\"Add Files\" option instead." },
        }
    },
    {
        "Installer_Message_IncompatibleGameData",
        {
            { ELanguage::English, "The specified game and\nupdate file are incompatible.\n\nPlease ensure the files are\nfor the same version and\nregion and try again." },

            { ELanguage::Italian, "I file del gioco\ne dell'aggiornamento sono incompatibili.\n\nAssicurati che i file sono\ndella stessa versione\ne regione e riprova." }
        }
    },
    {
        "Installer_Message_DLCWarning",
        {
            { ELanguage::English, "It is highly recommended\nthat you install all of the\nDLC, as it includes high\nquality lighting textures\nfor the base game.\n\nAre you sure you want to\nskip this step?" },
            { ELanguage::Italian, "Si consiglia di installare\ntutti i DLC, poichè includono\ntexture di illuminazione di qualità migliore.\n\nSei sicuro di voler saltare?" }
        }
    },
    {
        "Installer_Message_TitleMissingDLC",
        {
            { ELanguage::English, "This will restart the game to\nallow you to install any DLC\nthat you are missing.\n\nInstalling DLC will improve the\nlighting quality across the game.\n\nWould you like to install missing\ncontent?" },
            { ELanguage::Italian, "Questa opzione riavviera il gioco\nper farti installare qualsiasi DLC\nche non hai installato.\n\nL'installazione dei DLC migliorerà la qualità\ndell'illuminazione del gioco.\n\nVuoi installare i DLC mancanti?" }
        }
    },
    {
        "Installer_Message_Title",
        {
            { ELanguage::English, "This restarts the game to\nallow you to install any DLC\nthat you may be missing.\n\nYou are not currently\nmissing any DLC.\n\nWould you like to proceed\nanyway?" },

            { ELanguage::Italian, "Questa opzione riavviera il gioco\nper farti installare qualsiasi DLC\nche non hai installato.\n\nHai già installato tutti i DLC.\n\nVuoi procedere comunque?" }
        }
    },
    {
        "Title_Message_Quit",
        {
            { ELanguage::English, "Are you sure you want to quit?" },
            { ELanguage::Italian, "Sei sicuro di voler uscire?" }
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
            { ELanguage::English, "Yes" },

            { ELanguage::Italian, "Sì" }
        }
    },
    {
        "Common_No",
        {
            { ELanguage::English, "No" },
            { ELanguage::Italian, "No" }
        }
    },
    {
        "Common_Next",
        {
            { ELanguage::English, "Next" },
            { ELanguage::Italian, "Continua" }
        }
    },
    {
        "Common_Select",
        {
            { ELanguage::English, "Select" },
            { ELanguage::Italian, "Seleziona" }
        }
    },
    {
        "Common_Back",
        {
            { ELanguage::English, "Back" },
            { ELanguage::Italian, "Indietro" }
        }
    },
    {
        "Common_Reset",
        {
            { ELanguage::English, "Reset" },
            { ELanguage::Italian, "Ripristina" }
        }
    },
    {
        "Common_Switch",
        {
            { ELanguage::English, "Switch" },
            { ELanguage::Italian, "Cambia" }
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
