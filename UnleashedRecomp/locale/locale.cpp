#include <user/config.h>
#include <locale/locale.h>

/*
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! LOCALISATION NOTES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    - Ensure brand names are always presented on the same line.

      Correct:
      This is a string that contains a brand name like
      Xbox 360, which is one of the few consoles to have a port of
      Sonic Unleashed.

      Incorrect:
      This is a string that contains a brand name like Xbox
      360, which is one of the few consoles to have a port of Sonic
      Unleashed.

    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

std::unordered_map<std::string, std::unordered_map<ELanguage, std::string>> g_locale =
{
    {
        "Options_Header_Name",
        {
            { ELanguage::English, "OPTIONS" },
            { ELanguage::Japanese, "OPTION" },
            { ELanguage::German, "OPTIONEN" },
            { ELanguage::French, "OPTIONS" },
            { ELanguage::Spanish, "OPCIONES" },
            { ELanguage::Italian, "OPZIONI" }
        }
    },
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
        // Notes: integer values in the options menu (e.g. FPS) when at their maximum value.
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
        // Notes: description for options that cannot be accessed anywhere but the title screen or world map (e.g. Language).
        "Options_Desc_NotAvailable",
        {
            { ELanguage::English, "This option is not available at this location." }
        }
    },
    {
        // Notes: currently the description for Window Size when in fullscreen.
        "Options_Desc_NotAvailableFullscreen",
        {
            { ELanguage::English, "This option is not available in fullscreen mode." }
        }
    },
    {
        // Notes: currently the description for Monitor when in fullscreen.
        "Options_Desc_NotAvailableWindowed",
        {
            { ELanguage::English, "This option is not available in windowed mode." }
        }
    },
    {
        // Notes: currently the description for Monitor when the user only has one display connected.
        "Options_Desc_NotAvailableHardware",
        {
            { ELanguage::English, "This option is not available with your current hardware configuration." }
        }
    },
    {
        // Notes: description for Transparency Anti-Aliasing when MSAA is disabled.
        "Options_Desc_NotAvailableMSAA",
        {
            { ELanguage::English, "This option is not available without MSAA." }
        }
    },
    {
        // Notes: description for Music Attenuation when the user is not running a supported OS.
        "Options_Desc_OSNotSupported",
        {
            { ELanguage::English, "This option is not supported by your operating system." }
        }
    },
    {
        // Notes: message appears when changing the Language option and backing out of the options menu.
        "Options_Message_Restart",
        {
            { ELanguage::English, "The game will now restart." }
        }
    },
    {
        // Notes: used for the button guide at the pause menu.
        "Achievements_Name",
        {
            { ELanguage::English, "Achievements" }
        }
    },
    {
        // Notes: used for the header in the achievements menu.
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
            { ELanguage::English, "Add the files for the game and its title update. You can use digital dumps or an extracted folder containing the unmodified files." }
        }
    },
    {
        "Installer_Page_SelectDLC",
        {
            { ELanguage::English, "Add the files for the DLC. You can use digital dumps or an extracted folder containing the unmodified files." }
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
            { ELanguage::English, "Installation complete!\nThis project is brought to you by:" }
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
        // Notes: message appears when using the "Add Files" option and choosing any file that is not an Xbox 360 game dump.
        "Installer_Message_InvalidFilesList",
        {
            { ELanguage::English, "The following selected files are invalid:" },
            { ELanguage::Italian, "I seguenti file non sono validi:" }
        }
    },
    {
        // Notes: message appears in the event there are some invalid files after adding the DLC and moving onto the next step.
        "Installer_Message_InvalidFiles",
        {
            { ELanguage::English, "Some of the files that have\nbeen provided are not valid.\n\nPlease make sure all the\nspecified files are correct\nand try again." },
            { ELanguage::Italian, "Alcuni dei file che sono stati\nselezionati non sono validi.\n\nAssicurati che tutti\ni file sono quelli corretti\ne riprova." }
        }
    },
    {
        // Notes: message appears when clicking the "Add Files" option for the first time.
        "Installer_Message_FilePickerTutorial",
        {
            { ELanguage::English, "Select a digital dump.\n\nFor choosing a folder with\nextracted and unmodified\ngame files, use the\n\"Add Folder\" option instead." },
        }
    },
    {
        // Notes: message appears when clicking the "Add Folder" option for the first time.
        "Installer_Message_FolderPickerTutorial",
        {
            { ELanguage::English, "Select a folder that contains\nthe unmodified files that have\nbeen extracted from the game.\n\nFor choosing a digital dump,\nuse the\"Add Files\" option instead." },
        }
    },
    {
        // Notes: message appears when providing a title update that does not match the region or version of the game dump.
        "Installer_Message_IncompatibleGameData",
        {
            { ELanguage::English, "The specified game and\ntitle update are incompatible.\n\nPlease ensure the files are\nfor the same version and\nregion and try again." },
            { ELanguage::Italian, "I file del gioco\ne dell'aggiornamento sono incompatibili.\n\nAssicurati che i file sono\ndella stessa versione\ne regione e riprova." }
        }
    },
    {
        // Notes: message appears when clicking Skip at the DLC step.
        "Installer_Message_DLCWarning",
        {
            { ELanguage::English, "It is highly recommended\nthat you install all of the\nDLC, as it includes high\nquality lighting textures\nfor the base game.\n\nAre you sure you want to\nskip this step?" },
            { ELanguage::Italian, "Si consiglia di installare\ntutti i DLC, poichè includono\ntexture di illuminazione di qualità migliore.\n\nSei sicuro di voler saltare?" }
        }
    },
    {
        // Notes: message appears when choosing the Install option at the title screen when the user is missing DLC content.
        "Installer_Message_TitleMissingDLC",
        {
            { ELanguage::English, "This will restart the game to\nallow you to install any DLC\nthat you are missing.\n\nInstalling DLC will improve the\nlighting quality across the game.\n\nWould you like to install missing\ncontent?" },
            { ELanguage::Italian, "Questa opzione riavviera il gioco\nper farti installare qualsiasi DLC\nche non hai installato.\n\nL'installazione dei DLC migliorerà la qualità\ndell'illuminazione del gioco.\n\nVuoi installare i DLC mancanti?" }
        }
    },
    {
        // Notes: message appears when choosing the Install option at the title screen when the user is not missing any content.
        "Installer_Message_Title",
        {
            { ELanguage::English, "This restarts the game to\nallow you to install any DLC\nthat you may be missing.\n\nYou are not currently\nmissing any DLC.\n\nWould you like to proceed\nanyway?" },
            { ELanguage::Italian, "Questa opzione riavviera il gioco\nper farti installare qualsiasi DLC\nche non hai installato.\n\nHai già installato tutti i DLC.\n\nVuoi procedere comunque?" }
        }
    },
    {
        // Notes: message appears when pressing B at the title screen.
        "Title_Message_Quit",
        {
            { ELanguage::English, "Are you sure you want to quit?" },
            { ELanguage::Italian, "Sei sicuro di voler uscire?" }
        }
    },
    {
        // Notes: message appears when SYS-DATA is corrupted (mismatching file size) upon pressing start at the title screen.
        // To make this occur, open the file in any editor and just remove a large chunk of data.
        // Do not localise this unless absolutely necessary, these strings are from the XEX.
        "Title_Message_SaveDataCorrupt",
        {
            { ELanguage::English, "The save file appears to be\ncorrupted and cannot be loaded." },
            { ELanguage::Japanese, "ゲームデータの読み込みに失敗しました。\nこのまま続けるとゲームデータをセーブすることはできません" },
            { ELanguage::German, "Diese Speicherdatei ist beschädigt\nund kann nicht geladen werden." },
            { ELanguage::French, "Le fichier de sauvegarde semble être\nendommagé et ne peut être chargé." },
            { ELanguage::Spanish, "El archivo parece estar dañado\ny no se puede cargar." },
            { ELanguage::Italian, "I file di salvataggio sembrano danneggiati\ne non possono essere caricati." }
        }
    },
    {
        // Notes: message appears when ACH-DATA is corrupted (mismatching file size, bad signature, incorrect version or invalid checksum) upon pressing start at the title screen.
        // To make this occur, open the file in any editor and just remove a large chunk of data.
        "Title_Message_AchievementDataCorrupt",
        {
            { ELanguage::English, "The achievement data appears to be\ncorrupted and cannot be loaded.\n\nProceeding from this point will\nclear your achievement data." }
        }
    },
    {
        // Notes: message appears when ACH-DATA cannot be loaded upon pressing start at the title screen.
        // To make this occur, lock the ACH-DATA file using an external program so that it cannot be accessed by the game.
        "Title_Message_AchievementDataIOError",
        {
            { ELanguage::English, "The achievement data could not be loaded.\nYour achievements will not be saved." }
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
