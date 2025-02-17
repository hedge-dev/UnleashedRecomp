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

    - Ensure your locale is added in the correct order following the language enum.

      Correct:
      {
          { ELanguage::English,  "Example" },
          { ELanguage::Japanese, "Example" },
          { ELanguage::German,   "Example" },
          { ELanguage::French,   "Example" },
          { ELanguage::Spanish,  "Example" },
          { ELanguage::Italian,  "Example" }
      }

      Incorrect:
      {
          { ELanguage::English,  "Example" },
          { ELanguage::French,   "Example" },
          { ELanguage::Spanish,  "Example" },
          { ELanguage::German,   "Example" },
          { ELanguage::Italian,  "Example" },
          { ELanguage::Japanese, "Example" }
      }

    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

std::unordered_map<std::string, std::unordered_map<ELanguage, std::string>> g_locale =
{
    {
        "Options_Header_Name",
        {
            { ELanguage::English,  "OPTIONS" },
            { ELanguage::Japanese, "OPTION" },
            { ELanguage::German,   "OPTIONEN" },
            { ELanguage::French,   "OPTIONS" },
            { ELanguage::Spanish,  "OPCIONES" },
            { ELanguage::Italian,  "OPZIONI" }
        }
    },
    {
        "Options_Category_System",
        {
            { ELanguage::English,  "SYSTEM" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "SYSTEM" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Options_Category_Input",
        {
            { ELanguage::English,  "INPUT" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "EINGABE" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Options_Category_Audio",
        {
            { ELanguage::English,  "AUDIO" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "AUDIO" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Options_Category_Video",
        {
            { ELanguage::English,  "VIDEO" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "VIDEO" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: integer values in the options menu (e.g. FPS) when at their maximum value.
        "Options_Value_Max",
        {
            { ELanguage::English,  "MAX" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "MAX" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Options_Name_WindowSize",
        {
            { ELanguage::English,  "Window Size" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Fenstergröße" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Options_Desc_WindowSize",
        {
            { ELanguage::English,  "Adjust the size of the game window in windowed mode." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Passe die Fenstergröße des Spiels im Fenstermodus an." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: description for options that cannot be accessed anywhere but the title screen or world map (e.g. Language).
        "Options_Desc_NotAvailable",
        {
            { ELanguage::English,  "This option is not available at this location." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Diese Option ist an dieser Stelle nicht verfügbar." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: currently the description for Window Size when in fullscreen.
        "Options_Desc_NotAvailableFullscreen",
        {
            { ELanguage::English,  "This option is not available in fullscreen mode." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Diese Option ist im Vollbildmodus nicht verfügbar." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: currently the description for Monitor when in fullscreen.
        "Options_Desc_NotAvailableWindowed",
        {
            { ELanguage::English,  "This option is not available in windowed mode." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Diese Option ist im Fenstermodus nicht verfügbar." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: currently the description for Monitor when the user only has one display connected.
        "Options_Desc_NotAvailableHardware",
        {
            { ELanguage::English,  "This option is not available with your current hardware configuration." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Diese Option ist mit der momentanen Hardwarekonfiguration nicht verfügbar." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: description for Transparency Anti-Aliasing when MSAA is disabled.
        "Options_Desc_NotAvailableMSAA",
        {
            { ELanguage::English,  "This option is not available without MSAA." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Diese Option ist ohne MSAA nicht verfügbar." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: description for Music Attenuation when the user is not running a supported OS.
        "Options_Desc_OSNotSupported",
        {
            { ELanguage::English,  "This option is not supported by your operating system." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Diese Option wird von diesem Betriebssystem nicht unterstützt." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: message appears when changing the Language option and backing out of the options menu.
        "Options_Message_Restart",
        {
            { ELanguage::English,  "The game will now restart." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Das Spiel wird jetzt neu starten." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: used for the button guide at the pause menu.
        "Achievements_Name",
        {
            { ELanguage::English,  "Achievements" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Erfolge" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: used for the header in the achievements menu.
        "Achievements_Name_Uppercase",
        {
            { ELanguage::English,  "ACHIEVEMENTS" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "ERFOLGE" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Achievements_Unlock",
        {
            { ELanguage::English,  "Achievement Unlocked!" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Erfolg Freigeschaltet!" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Installer_Header_Installer",
        {
            { ELanguage::English,  "INSTALLER" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "INSTALLATION" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "INSTALADOR" },
            { ELanguage::Italian,  "INSTALLATORE" },
        },
    },
    {
        "Installer_Header_Installing",
        {
            { ELanguage::English,  "INSTALLING" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "INSTALLATION" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "INSTALANDO" },
            { ELanguage::Italian,  "INSTALLANDO" },
        }
    },
    {
        "Installer_Page_SelectLanguage",
        {
            { ELanguage::English,  "Please select a language." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Bitte eine Sprache auswählen." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Seleziona una lingua." }
        }
    },
    {
        "Installer_Page_Introduction",
        {
            { ELanguage::English,  "Welcome to\nUnleashed Recompiled!\n\nYou'll need an Xbox 360 copy\nof Sonic Unleashed in order to proceed with the installation." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Willkommen zu \nUnleashed Recompiled!\n\nEs wird eine Xbox 360 Kopie\nvon Sonic Unleashed benötigt\num mit der Installation fortfahren zu können." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Benvenuto a\nUnleashed Recompiled!\n\nDovrai avere una copia di\nSonic Unleashed per la Xbox 360\nper proseguire con l'installazione." }
        }
    },
    {
        "Installer_Page_SelectGameAndUpdate",
        {
            { ELanguage::English,  "Add the sources for the game and its title update." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Füge die Quellen für das Spiel und dessen Update hinzu." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Installer_Page_SelectDLC",
        {
            { ELanguage::English,  "Add the sources for the DLC." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Füge die Quellen für die Erweiterungen des Spiels hinzu." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Installer_Page_CheckSpace",
        {
            { ELanguage::English,  "The content will be installed to the program's folder.\n\n" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Der Inhalt wird in dem Ordner des Programms installiert.\n" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Il contenuto verrà installato nella cartella di questo programma.\n\n" }
        }
    },
    {
        "Installer_Page_Installing",
        {
            { ELanguage::English,  "Please wait while the content is being installed..." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Bitte warten. Der Inhalt wird installiert..." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Attendi mentre il contenuto viene installato... " }
        }
    },
    {
        "Installer_Page_InstallSucceeded",
        {
            { ELanguage::English,  "Installation complete!\nThis project is brought to you by:" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Installation abgeschlossen!\nDieses Projekt wird präsentiert von:" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Installer_Page_InstallFailed",
        {
            { ELanguage::English,  "Installation failed.\n\nError:\n\n" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Installation fehlgeschlagen\n\nFehler:\n\n" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Installazione fallita.\n\nErrore:\n\n" }
        }
    },
    {
        "Installer_Step_Game",
        {
            { ELanguage::English,  "GAME" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "SPIEL" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "GIOCO" }
        }
    },
    {
        "Installer_Step_Update",
        {
            { ELanguage::English,  "UPDATE" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "UPDATE" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "AGGIORNAMENTO" }
        }
    },
    {
        "Installer_Step_RequiredSpace",
        {
            { ELanguage::English,  "Required space: %2.2f GiB" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Benötigter Speicherplatz:\n%2.2f GiB" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Spazio necessario: %2.2f GiB" }
        }
    },
    {
        "Installer_Step_AvailableSpace",
        {
            { ELanguage::English,  "Available space: %2.2f GiB" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Verfügbarer Speicherplatz:\n%2.2f GiB" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Spazio disponibile: %2.2f GiB" }
        }
    },
    {
        "Installer_Button_Next",
        {
            { ELanguage::English,  "NEXT" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "WEITER" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "SIGUIENTE" },
            { ELanguage::Italian,  "CONTINUA" }
        }
    },
    {
        "Installer_Button_Skip",
        {
            { ELanguage::English,  "SKIP" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "ÜBERSPRINGEN" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "SALTAR" },
            { ELanguage::Italian,  "SALTA" }
        }
    },
    {
        "Installer_Button_Retry",
        {
            { ELanguage::English,  "RETRY" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "ERNEUT VERSUCHEN" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Installer_Button_AddFiles",
        {
            { ELanguage::English,  "ADD FILES" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "DATEIEN HINZUFÜGEN" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "AÑADIR ARCHIVOS" },
            { ELanguage::Italian,  "AGGIUNGI FILE" }
        }
    },
    {
        "Installer_Button_AddFolder",
        {
            { ELanguage::English,  "ADD FOLDER" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "ORDNER HINZUFÜGEN" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "AÑADIR CARPETA" },
            { ELanguage::Italian,  "AGGIUNGI CARTELLA" }
        }
    },
    {
        // Notes: message appears when using the "Add Files" option and choosing any file that is not an Xbox 360 game dump.
        "Installer_Message_InvalidFilesList",
        {
            { ELanguage::English,  "The following selected files are invalid:" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Die folgenden Dateien sind ungültig:" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "I seguenti file non sono validi:" }
        }
    },
    {
        // Notes: message appears in the event there are some invalid files after adding the DLC and moving onto the next step.
        "Installer_Message_InvalidFiles",
        {
            { ELanguage::English,  "Some of the files that have\nbeen provided are not valid.\n\nPlease make sure all the\nspecified files are correct\nand try again." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Einige Dateien, die bereitgestellt\nwurden sind ungültig.\n\nBitte stelle sicher, dass\ndie angegebenen Dateien korrekt\nsind und versuche es erneut." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Alcuni dei file che sono stati\nselezionati non sono validi.\n\nAssicurati che tutti\ni file sono quelli corretti\ne riprova." }
        }
    },
    {
        // Notes: message appears when clicking the "Add Files" option for the first time.
        "Installer_Message_FilePickerTutorial",
        {
            { ELanguage::English,  "Select a digital dump with\ncontent from the game.\n\nThese files can be obtained from\nyour Xbox 360 hard drive by\nfollowing the instructions on\nthe GitHub page.\n\nFor choosing a folder with extracted\nand unmodified game files, use\nthe \"Add Folder\" option instead." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Wähle einen digitalen Dump\nmit Inhalten von dem Spiel.\n\nDie entsprechenden Dateien\nkönnen über die Festplatte deiner\nXbox 360 erlangt werden.\nFolge hierfür den Anweisungen\nauf der GitHub Seite.\n\nUm einen Ordner mit\nden unmodifizierten extrahierten\nSpieldateien auszuwählen, benutze\ndie \"Ordner Hinzufügen\" Option\nstattdessen." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: message appears when clicking the "Add Folder" option for the first time.
        "Installer_Message_FolderPickerTutorial",
        {
            { ELanguage::English,  "Select a folder that contains the\nunmodified files that have been\nextracted from the game.\n\nThese files can be obtained from\nyour Xbox 360 hard drive by\nfollowing the instructions on\nthe GitHub page.\n\nFor choosing a digital dump,\nuse the\"Add Files\" option instead." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Wähle einen Ordner, der unmodifizierte\nDateien, die vom Spiel extrahiert wurden\nenthält.\n\nDie entsprechenden Dateien\nkönnen über die Festplatte deiner\nXbox 360 erlangt werden.\nFolge hierfür den Anweisungen\nauf der GitHub Seite.\n\nUm einen digitalen Dump\nauszuwählen, benutze\ndie \"Datei Hinzufügen\" Option\nstattdessen." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: message appears when providing a title update that does not match the region or version of the game dump.
        "Installer_Message_IncompatibleGameData",
        {
            { ELanguage::English,  "The specified game and\ntitle update are incompatible.\n\nPlease ensure the files are\nfor the same version and\nregion and try again." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Die ausgewählten Spiel- und\nUpdatedateien sind inkompatibel.\n\nBitte stelle sicher, dass\ndie Dateien für die selbe\nVersion und Region vorgesehen sind\nund versuche es erneut." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "I file del gioco\ne dell'aggiornamento sono incompatibili.\n\nAssicurati che i file sono\ndella stessa versione\ne regione e riprova." }
        }
    },
    {
        // Notes: message appears when clicking Skip at the DLC step.
        "Installer_Message_DLCWarning",
        {
            { ELanguage::English,  "It is highly recommended\nthat you install all of the\nDLC, as it includes high\nquality lighting textures\nfor the base game.\n\nAre you sure you want to\nskip this step?" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Es wird empgohlen alle\nErweiterungen zu installieren\n, da sie Beleuchtungs-Texturen\nin einer höheren Qualität\nfür das Basisspiel beinhalten.\n\nBist du dir sicher, dass\ndu diesen Schritt überspringen\nmöchtest?" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Si consiglia di installare\ntutti i DLC, poichè includono\ntexture di illuminazione di qualità migliore.\n\nSei sicuro di voler saltare?" }
        }
    },
    {
        // Notes: message appears when choosing the Install option at the title screen when the user is missing DLC content.
        "Installer_Message_TitleMissingDLC",
        {
            { ELanguage::English,  "This will restart the game to\nallow you to install any DLC\nthat you are missing.\n\nInstalling DLC will improve the\nlighting quality across the game.\n\nWould you like to install missing\ncontent?" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Das Spiel wird neu gestartet\num die Installation einer fehlenden\nErweiterung zu ermöglichen.\n\nDie Installation einer\nErweiterung erhöht die Qualität\nder Beleuchtung im gesamten Spiel.\n\nMöchtest du den fehlenden\nInhalt installieren?" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Questa opzione riavviera il gioco\nper farti installare qualsiasi DLC\nche non hai installato.\n\nL'installazione dei DLC migliorerà la qualità\ndell'illuminazione del gioco.\n\nVuoi installare i DLC mancanti?" }
        }
    },
    {
        // Notes: message appears when choosing the Install option at the title screen when the user is not missing any content.
        "Installer_Message_Title",
        {
            { ELanguage::English,  "This restarts the game to\nallow you to install any DLC\nthat you may be missing.\n\nYou are not currently\nmissing any DLC.\n\nWould you like to proceed\nanyway?" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Das Spiel wird neu gestartet\num die Installation einer fehlenden\nErweiterung zu ermöglichen.\n\nEs kann keine weitere Erweiterung\ninstalliert werden.\n\nMöchtest du trotzdem fortfahren?" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Questa opzione riavviera il gioco\nper farti installare qualsiasi DLC\nche non hai installato.\n\nHai già installato tutti i DLC.\n\nVuoi procedere comunque?" }
        }
    },
    {
        // Notes: message appears when user chooses "Quit" on the first available installation screen.
        "Installer_Message_Quit",
        {
            { ELanguage::English,  "Are you sure you want to quit?" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Bist du dir sicher, dass du die Installation verlassen möchtest?" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: message appears when user chooses "Cancel" during installation.
        "Installer_Message_Cancel",
        {
            { ELanguage::English,  "Are you sure you want to cancel the installation?" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Bist du dir sicher, dass du die Installation abbrechen möchtest?" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: message appears when pressing B at the title screen.
        "Title_Message_Quit",
        {
            { ELanguage::English,  "Are you sure you want to quit?" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Bist du dir sicher, dass du das Spiel verlassen möchtest?" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Sei sicuro di voler uscire?" }
        }
    },
    {
        // Notes: message appears when SYS-DATA is corrupted (mismatching file size) upon pressing start at the title screen.
        // To make this occur, open the file in any editor and just remove a large chunk of data.
        // Do not localise this unless absolutely necessary, these strings are from the XEX.
        "Title_Message_SaveDataCorrupt",
        {
            { ELanguage::English,  "The save file appears to be\ncorrupted and cannot be loaded." },
            { ELanguage::Japanese, "ゲームデータの読み込みに失敗しました。\nこのまま続けるとゲームデータをセーブすることはできません" },
            { ELanguage::German,   "Diese Speicherdatei ist beschädigt\nund kann nicht geladen werden." },
            { ELanguage::French,   "Le fichier de sauvegarde semble être\nendommagé et ne peut être chargé." },
            { ELanguage::Spanish,  "El archivo parece estar dañado\ny no se puede cargar." },
            { ELanguage::Italian,  "I file di salvataggio sembrano danneggiati\ne non possono essere caricati." }
        }
    },
    {
        // Notes: message appears when ACH-DATA is corrupted (mismatching file size, bad signature, incorrect version or invalid checksum) upon pressing start at the title screen.
        // To make this occur, open the file in any editor and just remove a large chunk of data.
        "Title_Message_AchievementDataCorrupt",
        {
            { ELanguage::English,  "The achievement data appears to be\ncorrupted and cannot be loaded.\n\nProceeding from this point will\nclear your achievement data." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Die Erfolgsdaten sind möglicherweise\nfehlerhaft und können nicht\ngeladen werden.\n\nDurch das Fortfahren werden\ndeine bisherigen Erfolgsdaten gelöscht." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        // Notes: message appears when ACH-DATA cannot be loaded upon pressing start at the title screen.
        // To make this occur, lock the ACH-DATA file using an external program so that it cannot be accessed by the game.
        "Title_Message_AchievementDataIOError",
        {
            { ELanguage::English,  "The achievement data could not be loaded.\nYour achievements will not be saved." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Die Erfolgsdaten konnten nicht geladen werden.\nDeine Erfolge werden nicht gespeichert." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Title_Message_UpdateAvailable",
        {
            { ELanguage::English,  "An update is available!\n\nWould you like to visit the\nreleases page to download it?" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Ein Update ist verfügbar!\n\nMöchtest du die Release-Seite\nbesuchen um es herunterzuladen?" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Video_BackendError",
        {
            { ELanguage::English,  "Unable to create a D3D12 (Windows) or Vulkan backend.\n\nPlease make sure that:\n\n- Your system meets the minimum requirements.\n- Your GPU drivers are up to date.\n- Your operating system is on the latest version available." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Es ist nicht möglich, ein D3D12 (Windows) oder Vulkan-Backend zu erstellen.\n\nBitte stelle sicher, dass:\n\n- Dein System die Mindestanforderungen erfüllt.\n- Deine GPU-Treiber auf dem neuesten Stand sind.\n- Dein Betriebssystem auf der neuesten verfügbaren Version ist." },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
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
            { ELanguage::English,  "Yes" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Ja" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Sì" }
        }
    },
    {
        "Common_No",
        {
            { ELanguage::English,  "No" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Nein" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "No" }
        }
    },
    {
        "Common_Next",
        {
            { ELanguage::English,  "Next" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Weiter" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Continua" }
        }
    },
    {
        "Common_Select",
        {
            { ELanguage::English,  "Select" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Auswahl" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Seleziona" }
        }
    },
    {
        "Common_Back",
        {
            { ELanguage::English,  "Back" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Zurück" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Indietro" }
        }
    },
    {
        "Common_Quit",
        {
            { ELanguage::English,  "Quit" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Verlassen" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Common_Cancel",
        {
            { ELanguage::English,  "Cancel" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Abbrechen" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "" }
        }
    },
    {
        "Common_Reset",
        {
            { ELanguage::English,  "Reset" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Zurücksetzen" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Ripristina" }
        }
    },
    {
        "Common_Switch",
        {
            { ELanguage::English,  "Switch" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Wechsel" },
            { ELanguage::French,   "" },
            { ELanguage::Spanish,  "" },
            { ELanguage::Italian,  "Cambia" }
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
