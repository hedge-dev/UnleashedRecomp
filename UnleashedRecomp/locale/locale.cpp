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

std::unordered_map<std::string_view, std::unordered_map<ELanguage, std::string>> g_locale =
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
            { ELanguage::French,   "SYSTÈME" },
            { ELanguage::Spanish,  "SISTEMA" },
            { ELanguage::Italian,  "SISTEMA" }
        }
    },
    {
        "Options_Category_Input",
        {
            { ELanguage::English,  "INPUT" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "EINGABE" },
            { ELanguage::French,   "COMMANDES" },
            { ELanguage::Spanish,  "CONTROLES" },
            { ELanguage::Italian,  "COMANDI" }
        }
    },
    {
        "Options_Category_Audio",
        {
            { ELanguage::English,  "AUDIO" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "AUDIO" },
            { ELanguage::French,   "SON" },
            { ELanguage::Spanish,  "AUDIO" },
            { ELanguage::Italian,  "AUDIO" }
        }
    },
    {
        "Options_Category_Video",
        {
            { ELanguage::English,  "VIDEO" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "VIDEO" },
            { ELanguage::French,   "AFFICHAGE" },
            { ELanguage::Spanish,  "VÍDEO" },
            { ELanguage::Italian,  "VIDEO" }
        }
    },
    {
        // Notes: integer values in the options menu (e.g. FPS) when at their maximum value.
        "Options_Value_Max",
        {
            { ELanguage::English,  "MAX" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "MAX" },
            { ELanguage::French,   "MAX" },
            { ELanguage::Spanish,  "MÁX" },
            { ELanguage::Italian,  "MAX" }
        }
    },
    {
        "Options_Name_WindowSize",
        {
            { ELanguage::English,  "Window Size" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Fenstergröße" },
            { ELanguage::French,   "Taille de la fenêtre" },
            { ELanguage::Spanish,  "Tamaño de ventana" },
            { ELanguage::Italian,  "Dimensioni della finestra" }
        }
    },
    {
        "Options_Desc_WindowSize",
        {
            { ELanguage::English,  "Adjust the size of the game window in windowed mode." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Passe die Fenstergröße des Spiels im Fenstermodus an." },
            { ELanguage::French,   "Définir la résolution de jeu en mode fenêtré." },
            { ELanguage::Spanish,  "Ajusta el tamaño de la ventana de juego en modo ventana." },
            { ELanguage::Italian,  "Regola la dimensione della finestra del gioco in modalità finestra." }
        }
    },
    {
        // Notes: description for options that cannot be accessed anywhere but the title screen or world map (e.g. Language).
        "Options_Desc_NotAvailable",
        {
            { ELanguage::English,  "This option is not available at this location." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Diese Option ist an dieser Stelle nicht verfügbar." },
            { ELanguage::French,   "Cette option n'est pas disponible pour l'instant." },
            { ELanguage::Spanish,  "Esta opción no está disponible en este momento." },
            { ELanguage::Italian,  "Questa opzione non può essere modificata in questo momento." }
        }
    },
    {
        // Notes: currently the description for Window Size when in fullscreen.
        "Options_Desc_NotAvailableFullscreen",
        {
            { ELanguage::English,  "This option is not available in fullscreen mode." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Diese Option ist im Vollbildmodus nicht verfügbar." },
            { ELanguage::French,   "Cette option n'est pas disponible en mode plein écran." },
            { ELanguage::Spanish,  "Esta opción no está disponible en modo pantalla completa." },
            { ELanguage::Italian,  "Questa opzione non è disponibile in modalità schermo pieno." }
        }
    },
    {
        // Notes: currently the description for Monitor when in fullscreen.
        "Options_Desc_NotAvailableWindowed",
        {
            { ELanguage::English,  "This option is not available in windowed mode." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Diese Option ist im Fenstermodus nicht verfügbar." },
            { ELanguage::French,   "Cette option n'est pas disponible en mode fenêtré." },
            { ELanguage::Spanish,  "Esta opción no está disponible en modo ventana." },
            { ELanguage::Italian,  "Questa opzione non è disponibile in modalità finestra." }
        }
    },
    {
        // Notes: currently the description for Monitor when the user only has one display connected.
        "Options_Desc_NotAvailableHardware",
        {
            { ELanguage::English,  "This option is not available with your current hardware configuration." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Diese Option ist mit der momentanen Hardwarekonfiguration nicht verfügbar." },
            { ELanguage::French,   "Cette option n'est pas disponible avec votre configuration matérielle actuelle." },
            { ELanguage::Spanish,  "Esta opción no está disponible con tu configuración actual de hardware." },
            { ELanguage::Italian,  "Questa opzione non è disponibile con l'hardware in tuo possesso." }
        }
    },
    {
        // Notes: description for Transparency Anti-Aliasing when MSAA is disabled.
        "Options_Desc_NotAvailableMSAA",
        {
            { ELanguage::English,  "This option is not available without MSAA." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Diese Option ist ohne MSAA nicht verfügbar." },
            { ELanguage::French,   "Cette option n'est pas disponible sans MSAA." },
            { ELanguage::Spanish,  "Esta opción no está disponible sin MSAA." },
            { ELanguage::Italian,  "Questa opzione non è disponibile senza MSAA." }
        }
    },
    {
        // Notes: description for Music Attenuation when the user is not running a supported OS.
        "Options_Desc_OSNotSupported",
        {
            { ELanguage::English,  "This option is not supported by your operating system." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Diese Option wird von diesem Betriebssystem nicht unterstützt." },
            { ELanguage::French,   "Cette option n'est pas prise en charge par votre système d'exploitation." },
            { ELanguage::Spanish,  "Está opción no está soportada por tu sistema operativo." },
            { ELanguage::Italian,  "Questa opzione non è disponibile con il tuo sistema operativo." }
        }
    },
    {
        // Notes: message appears when changing the Language option and backing out of the options menu.
        "Options_Message_Restart",
        {
            { ELanguage::English,  "The game will now restart." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Das Spiel wird jetzt neu starten." },
            { ELanguage::French,   "Le jeu va maintenant redémarrer." },
            { ELanguage::Spanish,  "El juego se va a reiniciar." },
            { ELanguage::Italian,  "Il gioco verrà riavviato." }
        }
    },
    {
        // Notes: used for the button guide at the pause menu.
        "Achievements_Name",
        {
            { ELanguage::English,  "Achievements" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Erfolge" },
            { ELanguage::French,   "Succès" },
            { ELanguage::Spanish,  "Logros" },
            { ELanguage::Italian,  "Obiettivi" }
        }
    },
    {
        // Notes: used for the header in the achievements menu.
        "Achievements_Name_Uppercase",
        {
            { ELanguage::English,  "ACHIEVEMENTS" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "ERFOLGE" },
            { ELanguage::French,   "SUCCÈS" },
            { ELanguage::Spanish,  "LOGROS" },
            { ELanguage::Italian,  "OBIETTIVI" }
        }
    },
    {
        "Achievements_Unlock",
        {
            { ELanguage::English,  "Achievement Unlocked!" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Erfolg Freigeschaltet!" },
            { ELanguage::French,   "Succès déverrouillé !" },
            { ELanguage::Spanish,  "¡Logro desbloqueado!" },
            { ELanguage::Italian,  "Obiettivo sbloccato!" }
        }
    },
    {
        "Installer_Header_Installer",
        {
            { ELanguage::English,  "INSTALLER" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "INSTALLATION" },
            { ELanguage::French,   "INSTALLATEUR" },
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
            { ELanguage::French,   "INSTALLATION" },
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
            { ELanguage::French,   "Choisissez une langue." },
            { ELanguage::Spanish,  "Selecciona un idioma." },
            { ELanguage::Italian,  "Seleziona una lingua." }
        }
    },
    {
        "Installer_Page_Introduction",
        {
            { ELanguage::English,  "Welcome to\nUnleashed Recompiled!\n\nYou'll need an Xbox 360 copy\nof Sonic Unleashed in order to proceed with the installation." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Willkommen zu\nUnleashed Recompiled!\nEs wird eine Xbox 360 Kopie von Sonic Unleashed benötigt um mit der Installation fortfahren zu können." },
            { ELanguage::French,   "Bienvenue sur\nUnleashed Recompiled !\n\nVous aurez besoin d'une copie de Sonic Unleashed pour Xbox\n360 pour procéder à l'installation." },
            { ELanguage::Spanish,  "¡Bienvenido a\nUnleashed Recompiled!\n\nNecesitas una copia de\nSonic Unleashed de Xbox 360\npara continuar con la instalación." },
            { ELanguage::Italian,  "Benvenuto a\nUnleashed Recompiled!\n\nDovrai avere una copia di\nSonic Unleashed per la Xbox 360\nper proseguire con l'installazione." }
        }
    },
    {
        "Installer_Page_SelectGameAndUpdate",
        {
            { ELanguage::English,  "Add the sources for the game and its title update." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Füge die Quellen für das Spiel und dessen Update hinzu." },
            { ELanguage::French,   "Ajouter les fichiers du jeu ainsi que ses mises à jour." },
            { ELanguage::Spanish,  "Añade las fuentes para el juego y su actualización." },
            { ELanguage::Italian,  "Aggiungi le fonti per il gioco e per il suo file d'aggiornamento." }
        }
    },
    {
        "Installer_Page_SelectDLC",
        {
            { ELanguage::English,  "Add the sources for the DLC." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Füge die Quellen für die Erweiterungen des Spiels hinzu." },
            { ELanguage::French,   "Ajouter les fichiers pour les DLCs." },
            { ELanguage::Spanish,  "Añade las fuentes para el DLC." },
            { ELanguage::Italian,  "Aggiungi le fonti per i DLC." }
        }
    },
    {
        "Installer_Page_CheckSpace",
        {
            { ELanguage::English,  "The content will be installed to the program's folder.\n\n" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Der Inhalt wird in dem Ordner des Programms installiert.\n" },
            { ELanguage::French,   "Le contenu sera installé dans le même dossier que le programme.\n" },
            { ELanguage::Spanish,  "El contenido será instalado a la carpeta del programa.\n\n" },
            { ELanguage::Italian,  "Il contenuto verrà installato nella cartella di questo programma.\n\n" }
        }
    },
    {
        "Installer_Page_Installing",
        {
            { ELanguage::English,  "Please wait while the content is being installed..." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Bitte warten. Der Inhalt wird installiert..." },
            { ELanguage::French,   "Veuillez patienter pendant l'installation du contenu..." },
            { ELanguage::Spanish,  "Por favor, espera mientras el contenido se instala... " },
            { ELanguage::Italian,  "Attendi mentre il contenuto viene installato... " }
        }
    },
    {
        "Installer_Page_InstallSucceeded",
        {
            { ELanguage::English,  "Installation complete!\nThis project is brought to you by:" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Installation abgeschlossen!\nDieses Projekt wird präsentiert von:" },
            { ELanguage::French,   "Installation terminée !\nCe projet vous est présenté par:" },
            { ELanguage::Spanish,  "¡Instalación completada!\nEste proyecto ha sido posible gracias a:" },
            { ELanguage::Italian,  "Installazione completata!\nQuesto progetto è stato creato da:" }
        }
    },
    {
        "Installer_Page_InstallFailed",
        {
            { ELanguage::English,  "Installation failed.\n\nError: " },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Installation fehlgeschlagen.\n\nFehler: " },
            { ELanguage::French,   "L'installation a échouée.\n\nErreur : " },
            { ELanguage::Spanish,  "La instalación falló.\n\nError: " },
            { ELanguage::Italian,  "Installazione fallita.\n\nErrore: " }
        }
    },
    {
        "Installer_Step_Game",
        {
            { ELanguage::English,  "GAME" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "SPIEL" },
            { ELanguage::French,   "JEU" },
            { ELanguage::Spanish,  "JUEGO" },
            { ELanguage::Italian,  "GIOCO" }
        }
    },
    {
        "Installer_Step_Update",
        {
            { ELanguage::English,  "UPDATE" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "UPDATE" },
            { ELanguage::French,   "MISE À JOUR" },
            { ELanguage::Spanish,  "ACTUALIZACIÓN" },
            { ELanguage::Italian,  "AGGIORNAMENTO" }
        }
    },
    {
        "Installer_Step_RequiredSpace",
        {
            { ELanguage::English,  "Required space: %2.2f GiB" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Benötigter Speicherplatz:\n%2.2f GiB" },
            { ELanguage::French,   "Espace nécessaire :\n%2.2f Gio" },
            { ELanguage::Spanish,  "Espacio necesario: %2.2f GiB" },
            { ELanguage::Italian,  "Spazio necessario: %2.2f GiB" }
        }
    },
    {
        "Installer_Step_AvailableSpace",
        {
            { ELanguage::English,  "Available space: %2.2f GiB" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Verfügbarer Speicherplatz:\n%2.2f GiB" },
            { ELanguage::French,   "Espace disponible :\n%2.2f Gio" },
            { ELanguage::Spanish,  "Espacio disponible: %2.2f GiB" },
            { ELanguage::Italian,  "Spazio disponibile: %2.2f GiB" }
        }
    },
    {
        "Installer_Button_Next",
        {
            { ELanguage::English,  "NEXT" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "WEITER" },
            { ELanguage::French,   "SUIVANT" },
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
            { ELanguage::French,   "IGNORER" },
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
            { ELanguage::French,   "RÉESSAYER" },
            { ELanguage::Spanish,  "REINTENTAR" },
            { ELanguage::Italian,  "RIPROVA" }
        }
    },
    {
        "Installer_Button_AddFiles",
        {
            { ELanguage::English,  "ADD FILES" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "DATEIEN HINZUFÜGEN" },
            { ELanguage::French,   "AJOUTER DES FICHIERS" },
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
            { ELanguage::French,   "AJOUTER UN DOSSIER" },
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
            { ELanguage::French,   "Les fichiers suivants ne sont pas valides :" },
            { ELanguage::Spanish,  "Los siguientes archivos no son válidos:" },
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
            { ELanguage::French,   "Certains fichiers fournis ne\nsont pas valides.\n\nVeuillez vous assurer que tous\nles fichiers spécifiés sont\ncorrects et réessayez." },
            { ELanguage::Spanish,  "Algunos de los archivos\nseleccionados no son válidos.\n\nPor favor, asegúrate de que\ntodos los archivos son correctos\ne inténtalo de nuevo.\n" },
            { ELanguage::Italian,  "Alcuni dei file che sono stati\nselezionati non sono validi.\n\nAssicurati che tutti\ni file sono quelli corretti\ne riprova." }
        }
    },
    {
        // Notes: message appears when clicking the "Add Files" option for the first time.
        "Installer_Message_FilePickerTutorial",
        {
            { ELanguage::English,  "Select a digital dump with\ncontent from the game.\n\nThese files can be obtained from\nyour Xbox 360 hard drive by\nfollowing the instructions on\nthe GitHub page.\n\nFor choosing a folder with extracted\nand unmodified game files, use\nthe \"Add Folder\" option instead." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Wähle einen digitalen Dump von dem Spiel.\n\nDie Dateien können über die Festplatte deiner\nXbox 360 erlangt werden.\nFolge hierfür den Anweisungen auf der GitHub Seite.\n\nUm einen Ordner mit unmodifizierten Spieldateien auszuwählen, benutze die \"Ordner Hinzufügen\" Option stattdessen." },
            { ELanguage::French,   "Sélectionnez une copie\ndématérialisée avec le contenu du\njeu de base.\n\nCes fichiers peuvent être obtenus\nà partir du disque dur de votre\nXbox 360 en suivant les\ninstructions de la page GitHub.\n\nPour choisir un dossier contenant\nles fichiers de jeu extraits et\nnon modifiés, utilisez plutôt\nl'option \"Ajouter un dossier\"." },
            { ELanguage::Spanish,  "Selecciona una copia digital\ncon contenido del juego.\n\nPuedes obtener los archivos\nde tu disco duro de Xbox 360\nsiguiendo las instrucciones de\nla página de GitHub.\n\nPara elegir una carpeta con\narchivos extraídos sin modificar,\nutiliza la opción \"Añadir Carpeta\"." },
            { ELanguage::Italian,  "Seleziona una copia digitale\ncon i contenuti del gioco.\n\nQuesti file possono essere ottenuti\ndall'hard drive della tua Xbox 360\nseguendo le istruzioni\nsulla pagina GitHub.\n\nPer selezionare una cartella\ncon file estratti e non modificati\nusa l'opzione \"Aggiungi cartella\"." }
        }
    },
    {
        // Notes: message appears when clicking the "Add Folder" option for the first time.
        "Installer_Message_FolderPickerTutorial",
        {
            { ELanguage::English,  "Select a folder that contains the\nunmodified files that have been\nextracted from the game.\n\nThese files can be obtained from\nyour Xbox 360 hard drive by\nfollowing the instructions on\nthe GitHub page.\n\nFor choosing a digital dump,\nuse the\"Add Files\" option instead." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Wähle einen Ordner, der unmodifizierte Dateien, die vom Spiel extrahiert wurden enthält.\n\nDie Dateien können über die Festplatte deiner\nXbox 360 erlangt werden.\nFolge hierfür den Anweisungen auf der GitHub Seite.\n\nUm einen digitalen Dump auszuwählen, benutze die \"Datei Hinzufügen\" Option stattdessen." },
            { ELanguage::French,   "Sélectionnez un dossier contenant\nles fichiers extraits du jeu de\nbase.\n\nCes fichiers peuvent être obtenus\nà partir du disque dur de votre\nXbox 360 en suivant les\ninstructions de la page GitHub.\n\nPour choisir une copie\ndématérialisée, utilisez plutôt\nl'option \"Ajouter des fichiers\"." },
            { ELanguage::Spanish,  "Selecciona una carpeta que\ncontenga los archivos sin\nmodificar extraídos del juego.\n\nPuedes obtener los archivos\nde tu disco duro de Xbox 360\nsiguiendo las instrucciones de\nla página de GitHub.\n\nPara elegir una copia digital,\nutiliza la opción \"Añadir Archivos\"." },
            { ELanguage::Italian,  "Seleziona una cartella che contiene\ni file non modificati che sono stati\nestratti dal gioco.\n\nQuesti file possono essere ottenuti\ndall'hard drive della tua Xbox 360\nseguendo le istruzioni\nsulla pagina GitHub.\n\nPer selezionare una copia digitale\nusa l'opzione \"Aggiungi file\"." }
        }
    },
    {
        // Notes: message appears when providing a title update that does not match the region or version of the game dump.
        "Installer_Message_IncompatibleGameData",
        {
            { ELanguage::English,  "The specified game and\ntitle update are incompatible.\n\nPlease ensure the files are\nfor the same version and\nregion and try again." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Die ausgewählten Spiel- und\nUpdatedateien sind inkompatibel.\n\nBitte stelle sicher, dass\ndie Dateien für die selbe\nVersion und Region vorgesehen sind\nund versuche es erneut." },
            { ELanguage::French,   "Les fichiers du jeu et la mise à\njour sont incompatibles.\n\nVeuillez vous assurer que les\nfichiers sont pour la même\nversion/région puis réessayez." },
            { ELanguage::Spanish,  "El juego seleccionado\ny su actualización son incompatibles.\n\nPor favor, asegúrate de que que los archivos\nson de la misma versión y\nregión e inténtalo de nuevo." },
            { ELanguage::Italian,  "I file del gioco\ne dell'aggiornamento sono incompatibili.\n\nAssicurati che i file sono\ndella stessa versione\ne regione e riprova." }
        }
    },
    {
        // Notes: message appears when clicking Skip at the DLC step.
        "Installer_Message_DLCWarning",
        {
            { ELanguage::English,  "It is highly recommended\nthat you install all of the\nDLC, as it includes high\nquality lighting textures\nfor the base game.\n\nAre you sure you want to\nskip this step?" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Es wird empgohlen alle Erweiterungen zu installieren, da sie Beleuchtungs-Texturen in einer höheren Qualität für das Basisspiel beinhalten.\n\nBist du dir sicher, dass du diesen Schritt überspringen möchtest?" },
            { ELanguage::French,   "Il est fortement recommandé\nd'installer l'ensemble du\nDLC car elle inclut des\ntextures de lumière de\nhaute qualité pour le jeu\nde base.\n\nÊtes-vous sûr de vouloir\nignorer cette étape ?" },
            { ELanguage::Spanish,  "Se recomienda encarecidamente\ninstalar todo el DLC, ya que\ncontiene texturas de iluminación\nde alta calidad para el juego base.\n\n¿Seguro que quieres saltar este paso?" },
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
            { ELanguage::French,   "Cela redémarrera le jeu pour vous\npermettre d'installer les DLC\nmanquants.\n\nL'installation du DLC améliorera\nla qualité de l'éclairage dans le\njeu.\n\nSouhaitez-vous installer le\ncontenu manquant ?" },
            { ELanguage::Spanish,  "Esta opción reiniciará el juego\npara permitirte instalar los DLC\nque falten.\n\nInstalar el DLC mejorará la calidad\nde iluminación en todo el juego.\n\n¿Quieres instalar el contenido\nque falta?" },
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
            { ELanguage::French,   "Cela redémarrera le jeu pour vous\npermettre d'installer les DLC\nmanquants.\n\nIl ne vous manque aucun DLC.\n\nVoulez-vous quand même continuer ?" },
            { ELanguage::Spanish,  "Esto reiniciará el juego\npara permitirte instalar\nlos DLC que falten.\n\nActualmente, no falta ningún\nDLC por instalarse.\n\n¿Quieres continuar de todos\nmodos?" },
            { ELanguage::Italian,  "Questa opzione riavviera il gioco\nper farti installare qualsiasi DLC\nche non hai installato.\n\nHai già installato tutti i DLC.\n\nVuoi procedere comunque?" }
        }
    },
    {
        // Notes: message appears when user chooses "Quit" on the first available installation screen.
        "Installer_Message_Quit",
        {
            { ELanguage::English,  "Are you sure you want to quit?" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Installation verlassen?" },
            { ELanguage::French,   "Êtes-vous sûr de vouloir quitter ?" },
            { ELanguage::Spanish,  "¿Estás seguro de que quieres\nsalir?" },
            { ELanguage::Italian,  "Sicuro di voler uscire?" }
        }
    },
    {
        // Notes: message appears when user chooses "Cancel" during installation.
        "Installer_Message_Cancel",
        {
            { ELanguage::English,  "Are you sure you want to cancel the installation?" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Bist du dir sicher, dass du die Installation abbrechen möchtest?" },
            { ELanguage::French,   "Êtes-vous sûr de vouloir annuler l'installation ?" },
            { ELanguage::Spanish,  "¿Estás seguro de que quieres cancelar la instalación?" },
            { ELanguage::Italian,  "Sicuro di voler annullare l'installazione?" }
        }
    },
    {
        // Notes: message appears when pressing B at the title screen.
        "Title_Message_Quit",
        {
            { ELanguage::English,  "Are you sure you want to quit?" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Spiel verlassen?" },
            { ELanguage::French,   "Êtes-vous sûr de vouloir quitter ?" },
            { ELanguage::Spanish,  "¿Estás seguro de que quieres\nsalir?" },
            { ELanguage::Italian,  "Sicuro di voler uscire ?" }
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
            { ELanguage::French,   "Les données des succès semblent être\nendommagées et ne peuvent être\nchargées.\n\nSi vous continuez, vos données\nseront écrasées." },
            { ELanguage::Spanish,  "Los datos de logros parecen estar\ncorruptos y no pueden cargarse.\n\nContinuar a partir de este punto\neliminará los datos de logros." },
            { ELanguage::Italian,  "I file degli obiettivi sembrano danneggiati\ne non possono essere caricati.\n\nSe prosegui da questo punto\ni tuoi obiettivi verranno cancellati." }
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
            { ELanguage::French,   "Les données des succès ne\npeuvent être chargées.\nVos succès ne seront pas\nsauvegardés." },
            { ELanguage::Spanish,  "Los datos de logros no pueden cargarse.\nTus logros no serán guardados." },
            { ELanguage::Italian,  "I file degli obiettivi non possono essere caricati.\nI tuoi obiettivi non verranno salvati." }
        }
    },
    {
        "Title_Message_UpdateAvailable",
        {
            { ELanguage::English,  "An update is available!\n\nWould you like to visit the\nreleases page to download it?" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Ein Update ist verfügbar!\n\nMöchtest du die Release-Seite\nbesuchen um es herunterzuladen?" },
            { ELanguage::French,   "Une mise à jour est disponible !\n\nVoulez-vous visiter la page\ndes mises à jour pour la\ntélécharger ?" },
            { ELanguage::Spanish,  "¡Hay una actualización disponible!\n\n¿Quieres ir a la página\npara descargarla?" },
            { ELanguage::Italian,  "È disponibile un aggiornamento!\n\nVuoi visitare la pagina releases\nper scaricarlo?" }
        }
    },
    {
        "Video_BackendError",
        {
            { ELanguage::English,  "Unable to create a D3D12 (Windows) or Vulkan backend.\n\nPlease make sure that:\n\n- Your system meets the minimum requirements.\n- Your GPU drivers are up to date.\n- Your operating system is on the latest version available." },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Es ist nicht möglich, ein D3D12 (Windows) oder Vulkan-Backend zu erstellen.\n\nBitte stelle sicher, dass:\n\n- Dein System die Mindestanforderungen erfüllt.\n- Deine GPU-Treiber auf dem neuesten Stand sind.\n- Dein Betriebssystem auf der neuesten verfügbaren Version ist." },
            { ELanguage::French,   "Impossible de créer un backend D3D12 (Windows) ou Vulkan.\n\nVeuillez vous assurer que :\n\n- Votre système répond aux critères minimums requis.\n- Les pilotes de votre processeur graphique sont à jour.\n- Votre système d'exploitation est à jour." },
            { ELanguage::Spanish,  "No se puede crear un entorno de D3D12 (Windows) o de Vulkan.\n\nPor favor, asegúrate de que:\n\n- Tu equipo cumple con los requisitos mínimos.\n- Los drivers de tu tarjeta gráfica están actualizados.\n- Tu sistema operativo está actualizado a la última versión.\n" },
            { ELanguage::Italian,  "Impossibile creare un backend D3D12 (Windows) o Vulkan.\n\nAssicurati che:\n\n- Il tuo sistema soddisfi i requisiti minimi.\n- I driver della scheda grafica siano aggiornati.\n- Il tuo sistema operativo sia aggiornato." }
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
            { ELanguage::French,   "Oui" },
            { ELanguage::Spanish, "Sí" },
            { ELanguage::Italian,  "Sì" }
        }
    },
    {
        "Common_No",
        {
            { ELanguage::English,  "No" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Nein" },
            { ELanguage::French,   "Non" },
            { ELanguage::Spanish,  "No" },
            { ELanguage::Italian,  "No" }
        }
    },
    {
        "Common_Next",
        {
            { ELanguage::English,  "Next" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Weiter" },
            { ELanguage::French,   "Suivant" },
            { ELanguage::Spanish, "Siguiente" },
            { ELanguage::Italian,  "Avanti" }
        }
    },
    {
        "Common_Select",
        {
            { ELanguage::English,  "Select" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Auswählen" },
            { ELanguage::French,   "Choisir" },
            { ELanguage::Spanish,  "Seleccionar" },
            { ELanguage::Italian,  "Seleziona" }
        }
    },
    {
        "Common_Back",
        {
            { ELanguage::English,  "Back" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Zurück" },
            { ELanguage::French,   "Retour" },
            { ELanguage::Spanish,  "Atrás" },
            { ELanguage::Italian,  "Indietro" }
        }
    },
    {
        "Common_Quit",
        {
            { ELanguage::English,  "Quit" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Verlassen" },
            { ELanguage::French,   "Quitter" },
            { ELanguage::Spanish,  "Salir" },
            { ELanguage::Italian,  "Esci" }
        }
    },
    {
        "Common_Cancel",
        {
            { ELanguage::English,  "Cancel" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Abbrechen" },
            { ELanguage::French,   "Annuler" },
            { ELanguage::Spanish,  "Cancelar" },
            { ELanguage::Italian,  "Annulla" }
        }
    },
    {
        "Common_Reset",
        {
            { ELanguage::English,  "Reset" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Zurücksetzen" },
            { ELanguage::French,   "Par défaut" },
            { ELanguage::Spanish,  "Reiniciar" },
            { ELanguage::Italian,  "Ripristina" }
        }
    },
    {
        "Common_Switch",
        {
            { ELanguage::English,  "Switch" },
            { ELanguage::Japanese, "" },
            { ELanguage::German,   "Wechseln" },
            { ELanguage::French,   "Changer" },
            { ELanguage::Spanish,  "Cambiar" },
            { ELanguage::Italian,  "Cambia" }
        }
    }
};

std::string& Localise(const std::string_view& key)
{
    auto localeFindResult = g_locale.find(key);
    if (localeFindResult != g_locale.end())
    {
        auto languageFindResult = localeFindResult->second.find(Config::Language);
        if (languageFindResult == localeFindResult->second.end())
            languageFindResult = localeFindResult->second.find(ELanguage::English);

        if (languageFindResult != localeFindResult->second.end())
            return languageFindResult->second;
    }

    return g_localeMissing;
}
