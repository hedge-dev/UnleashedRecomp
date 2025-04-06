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

    - Japanese localization should use furigana for message windows, options items,
    options descriptions and installer text.

      Furigana should be included in the text as such:
      "[私:わたし]はジョージです。"

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
            { ELanguage::Japanese, "SYSTEM" },
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
            { ELanguage::Japanese, "INPUT" },
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
            { ELanguage::Japanese, "AUDIO" },
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
            { ELanguage::Japanese, "VIDEO" },
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
            { ELanguage::Japanese, "MAX" },
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
            { ELanguage::Japanese, "ウィンドウサイズ" },
            { ELanguage::German,   "Fenstergröße" },
            { ELanguage::French,   "Taille de la fenêtre" },
            { ELanguage::Spanish,  "Tamaño de ventana" },
            { ELanguage::Italian,  "Dimensioni della finestra" }
        }
    },
    {
        // Japanese Notes: This localization should include furigana.
        "Options_Desc_WindowSize",
        {
            { ELanguage::English,  "Adjust the size of the game window in windowed mode." },
            { ELanguage::Japanese, "ゲームの\u200Bウィンドウサイズを\u200B[設定:せってい]できます" },
            { ELanguage::German,   "Passe die Fenstergröße des Spiels im Fenstermodus an." },
            { ELanguage::French,   "Définir la résolution de jeu en mode fenêtré." },
            { ELanguage::Spanish,  "Ajusta el tamaño de la ventana de juego en modo ventana." },
            { ELanguage::Italian,  "Regola la dimensione della finestra del gioco in modalità finestra." }
        }
    },
    {
        // Notes: description for options that cannot be accessed anywhere but the title screen or world map (e.g. Language).
        // Japanese Notes: This localization should include furigana.
        "Options_Desc_NotAvailable",
        {
            { ELanguage::English,  "This option is not available at this location." },
            { ELanguage::Japanese, "この\u200Bオプションは\u200B[現在:げんざい]の\u200B[画面:がめん]では\u200B[変更:へんこう]\u200Bできません" },
            { ELanguage::German,   "Diese Option ist an dieser Stelle nicht verfügbar." },
            { ELanguage::French,   "Cette option n'est pas disponible pour l'instant." },
            { ELanguage::Spanish,  "Esta opción no está disponible en este momento." },
            { ELanguage::Italian,  "Questa opzione non può essere modificata in questo momento." }
        }
    },
    {
        // Notes: currently the description for Window Size when in fullscreen.
        // Japanese Notes: This localization should include furigana.
        "Options_Desc_NotAvailableFullscreen",
        {
            { ELanguage::English,  "This option is not available in fullscreen mode." },
            { ELanguage::Japanese, "この\u200Bオプションは\u200Bフルスクリーンモードでは\u200B[変更:へんこう]\u200Bできません" },
            { ELanguage::German,   "Diese Option ist im Vollbildmodus nicht verfügbar." },
            { ELanguage::French,   "Cette option n'est pas disponible en mode plein écran." },
            { ELanguage::Spanish,  "Esta opción no está disponible en modo pantalla completa." },
            { ELanguage::Italian,  "Questa opzione non è disponibile in modalità schermo pieno." }
        }
    },
    {
        // Notes: currently the description for Monitor when in fullscreen.
        // Japanese Notes: This localization should include furigana.
        "Options_Desc_NotAvailableWindowed",
        {
            { ELanguage::English,  "This option is not available in windowed mode." },
            { ELanguage::Japanese, "この\u200Bオプションは\u200Bウィンドウモードでは\u200B[変更:へんこう]\u200Bできません" },
            { ELanguage::German,   "Diese Option ist im Fenstermodus nicht verfügbar." },
            { ELanguage::French,   "Cette option n'est pas disponible en mode fenêtré." },
            { ELanguage::Spanish,  "Esta opción no está disponible en modo ventana." },
            { ELanguage::Italian,  "Questa opzione non è disponibile in modalità finestra." }
        }
    },
    {
        // Notes: currently the description for Monitor when the user only has one display connected.
        // Japanese Notes: This localization should include furigana.
        "Options_Desc_NotAvailableHardware",
        {
            { ELanguage::English,  "This option is not available with your current hardware configuration." },
            { ELanguage::Japanese, "この\u200Bオプションは\u200B[現在:げんざい]の\u200Bハードウェア[構成:こうせい]で\u200B[変更:へんこう]\u200Bできません" },
            { ELanguage::German,   "Diese Option ist mit der momentanen Hardwarekonfiguration nicht verfügbar." },
            { ELanguage::French,   "Cette option n'est pas disponible avec votre configuration matérielle actuelle." },
            { ELanguage::Spanish,  "Esta opción no está disponible con tu configuración actual de hardware." },
            { ELanguage::Italian,  "Questa opzione non è disponibile con l'hardware in tuo possesso." }
        }
    },
    {
        // Notes: description for Transparency Anti-Aliasing when MSAA is disabled.
        // Japanese Notes: This localization should include furigana.
        "Options_Desc_NotAvailableMSAA",
        {
            { ELanguage::English,  "This option is not available without MSAA." },
            { ELanguage::Japanese, "この\u200Bオプションは\u200BMSAAなしで\u200B[変更:へんこう]\u200Bできません" },
            { ELanguage::German,   "Diese Option ist ohne MSAA nicht verfügbar." },
            { ELanguage::French,   "Cette option n'est pas disponible sans MSAA." },
            { ELanguage::Spanish,  "Esta opción no está disponible sin MSAA." },
            { ELanguage::Italian,  "Questa opzione non è disponibile senza MSAA." }
        }
    },
    {
        // Notes: description for Music Attenuation when the user is not running a supported OS.
        // Japanese Notes: This localization should include furigana.
        "Options_Desc_OSNotSupported",
        {
            { ELanguage::English,  "This option is not supported by your operating system." },
            { ELanguage::Japanese, "この\u200Bオプションは\u200B[現在:げんざい]の\u200BOSで\u200B[変更:へんこう]\u200Bできません" },
            { ELanguage::German,   "Diese Option wird von diesem Betriebssystem nicht unterstützt." },
            { ELanguage::French,   "Cette option n'est pas prise en charge par votre système d'exploitation." },
            { ELanguage::Spanish,  "Esta opción no está soportada por tu sistema operativo." },
            { ELanguage::Italian,  "Questa opzione non è disponibile con il tuo sistema operativo." }
        }
    },
    {
        // Notes: message appears when changing the Language option and backing out of the options menu.
        // Japanese Notes: This localization should include furigana.
        "Options_Message_Restart",
        {
            { ELanguage::English,  "The game will now restart." },
            { ELanguage::Japanese, "ゲームが[再起動:さいきどう]されます" },
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
            { ELanguage::Japanese, "実績" },
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
            { ELanguage::Japanese, "実績" },
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
            { ELanguage::Japanese, "実績のロックが解除されました" },
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
            { ELanguage::Japanese, "INSTALL" },
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
            { ELanguage::Japanese, "INSTALL" },
            { ELanguage::German,   "INSTALLATION" },
            { ELanguage::French,   "INSTALLATION" },
            { ELanguage::Spanish,  "INSTALANDO" },
            { ELanguage::Italian,  "INSTALLANDO" },
        }
    },
    {
        // Japanese Notes: This localization should include furigana.
        "Installer_Page_SelectLanguage",
        {
            { ELanguage::English,  "Please select a language." },
            { ELanguage::Japanese, "[言語:げんご]を[選択:せんたく]してください" },
            { ELanguage::German,   "Bitte eine Sprache auswählen." },
            { ELanguage::French,   "Choisissez une langue." },
            { ELanguage::Spanish,  "Selecciona un idioma." },
            { ELanguage::Italian,  "Seleziona una lingua." }
        }
    },
    {
        // Japanese Notes: This localization should include furigana.
        "Installer_Page_Introduction",
        {
            { ELanguage::English,  "Welcome to\nUnleashed Recompiled!\n\nYou'll need an Xbox 360 copy\nof Sonic Unleashed in order to proceed with the installation." },
            { ELanguage::Japanese, "Unleashed Recompiledへようこそ！\nインストールには\n[北米版:ほくべいばん]またはEU[版:ばん]のXbox 360[用:よう]\n「SONIC UNLEASHED」が[必要:ひつよう]です" },
            { ELanguage::German,   "Willkommen zu\nUnleashed Recompiled!\nEs wird eine Xbox 360 Kopie von Sonic Unleashed benötigt um mit der Installation fortfahren zu können." },
            { ELanguage::French,   "Bienvenue sur\nUnleashed Recompiled !\n\nVous aurez besoin d'une copie de Sonic Unleashed pour Xbox\n360 pour procéder à l'installation." },
            { ELanguage::Spanish,  "¡Bienvenido a\nUnleashed Recompiled!\n\nNecesitas una copia de\nSonic Unleashed de Xbox 360\npara continuar con la instalación." },
            { ELanguage::Italian,  "Benvenuto a\nUnleashed Recompiled!\n\nDovrai avere una copia di\nSonic Unleashed per la Xbox 360\nper proseguire con l'installazione." }
        }
    },
    {
        // Japanese Notes: This localization should include furigana.
        "Installer_Page_SelectGameAndUpdate",
        {
            { ELanguage::English,  "Add the sources for the game and its title update." },
            { ELanguage::Japanese, "ゲームのソースとタイトルアップデート\nを[追加:ついか]" },
            { ELanguage::German,   "Füge die Quellen für das Spiel und dessen Update hinzu." },
            { ELanguage::French,   "Ajouter les fichiers du jeu ainsi que ses mises à jour." },
            { ELanguage::Spanish,  "Añade las fuentes para el juego y su actualización." },
            { ELanguage::Italian,  "Aggiungi le fonti per il gioco e per il suo file d'aggiornamento." }
        }
    },
    {
        // Japanese Notes: This localization should include furigana.
        "Installer_Page_SelectDLC",
        {
            { ELanguage::English,  "Add the sources for the DLC." },
            { ELanguage::Japanese, "DLCのソースを[追加:ついか]" },
            { ELanguage::German,   "Füge die Quellen für die Erweiterungen des Spiels hinzu." },
            { ELanguage::French,   "Ajouter les fichiers pour les DLCs." },
            { ELanguage::Spanish,  "Añade las fuentes para el DLC." },
            { ELanguage::Italian,  "Aggiungi le fonti per i DLC." }
        }
    },
    {
        // Japanese Notes: This localization should include furigana.
        "Installer_Page_CheckSpace",
        {
            { ELanguage::English,  "The content will be installed to the program's folder.\n\n" },
            { ELanguage::Japanese, "コンテンツはプログラムのフォルダに\nインストールされます\n" },
            { ELanguage::German,   "Der Inhalt wird in dem Ordner des Programms installiert.\n" },
            { ELanguage::French,   "Le contenu sera installé dans le même dossier que le programme.\n" },
            { ELanguage::Spanish,  "El contenido será instalado a la carpeta del programa.\n\n" },
            { ELanguage::Italian,  "Il contenuto verrà installato nella cartella di questo programma.\n\n" }
        }
    },
    {
        // Japanese Notes: This localization should include furigana.
        "Installer_Page_Installing",
        {
            { ELanguage::English,  "Please wait while the content is being installed..." },
            { ELanguage::Japanese, "コンテンツのインストール[中:ちゅう]はお[待:ま]ち\nください" },
            { ELanguage::German,   "Bitte warten. Der Inhalt wird installiert..." },
            { ELanguage::French,   "Veuillez patienter pendant l'installation du contenu..." },
            { ELanguage::Spanish,  "Por favor, espera mientras el contenido se instala... " },
            { ELanguage::Italian,  "Attendi mentre il contenuto viene installato... " }
        }
    },
    {
        // Japanese Notes: This localization should include furigana.
        "Installer_Page_InstallSucceeded",
        {
            { ELanguage::English,  "Installation complete!\nThis project is brought to you by:" },
            { ELanguage::Japanese, "インストール[完了:かんりょう]！\nプロジェクト[制作:せいさく]：" },
            { ELanguage::German,   "Installation abgeschlossen!\nDieses Projekt wird präsentiert von:" },
            { ELanguage::French,   "Installation terminée !\nCe projet vous est présenté\npar :" },
            { ELanguage::Spanish,  "¡Instalación completada!\nEste proyecto ha sido posible gracias a:" },
            { ELanguage::Italian,  "Installazione completata!\nQuesto progetto è stato creato da:" }
        }
    },
    {
        // Japanese Notes: This localization should include furigana.
        "Installer_Page_InstallFailed",
        {
            { ELanguage::English,  "Installation failed.\n\nError: " },
            { ELanguage::Japanese, "インストールに[失敗:しっぱい]しました\n\nエラー： " },
            { ELanguage::German,   "Installation fehlgeschlagen.\n\nFehler: " },
            { ELanguage::French,   "L'installation a échoué.\n\nErreur : " },
            { ELanguage::Spanish,  "La instalación falló.\n\nError: " },
            { ELanguage::Italian,  "Installazione fallita.\n\nErrore: " }
        }
    },
    {
        "Installer_Step_Game",
        {
            { ELanguage::English,  "GAME" },
            { ELanguage::Japanese, "ゲーム" },
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
            { ELanguage::Japanese, "アップデート" },
            { ELanguage::German,   "UPDATE" },
            { ELanguage::French,   "MISE À JOUR" },
            { ELanguage::Spanish,  "ACTUALIZACIÓN" },
            { ELanguage::Italian,  "AGGIORNAMENTO" }
        }
    },
    {
        // Japanese Notes: This localization should include furigana.
        "Installer_Step_RequiredSpace",
        {
            { ELanguage::English,  "Required space: %2.2f GiB" },
            { ELanguage::Japanese, "[必要:ひつよう]な[容量:ようりょう]: %2.2f GiB" },
            { ELanguage::German,   "Benötigter Speicherplatz:\n%2.2f GiB" },
            { ELanguage::French,   "Espace nécessaire :\n%2.2f Gio" },
            { ELanguage::Spanish,  "Espacio necesario: %2.2f GiB" },
            { ELanguage::Italian,  "Spazio necessario: %2.2f GiB" }
        }
    },
    {
        // Japanese Notes: This localization should include furigana.
        "Installer_Step_AvailableSpace",
        {
            { ELanguage::English,  "Available space: %2.2f GiB" },
            { ELanguage::Japanese, "[使用:しよう][可能:かのう]な[容量:ようりょう]: %2.2f GiB" },
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
            { ELanguage::Japanese, "次へ" },
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
            { ELanguage::Japanese, "スキップ" },
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
            { ELanguage::Japanese, "リトライ" },
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
            { ELanguage::Japanese, "ファイルを追加" },
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
            { ELanguage::Japanese, "フォルダを追加" },
            { ELanguage::German,   "ORDNER HINZUFÜGEN" },
            { ELanguage::French,   "AJOUTER UN DOSSIER" },
            { ELanguage::Spanish,  "AÑADIR CARPETA" },
            { ELanguage::Italian,  "AGGIUNGI CARTELLA" }
        }
    },
    {
        // Notes: message appears when using the "Add Files" option and choosing any file that is not an Xbox 360 game dump.
        // Japanese Notes: This localization should include furigana.
        "Installer_Message_InvalidFilesList",
        {
            { ELanguage::English,  "The following selected files are invalid:" },
            { ELanguage::Japanese, "[選択:せんたく]した[次:つぎ]のファイルは[無効:むこう]です：" },
            { ELanguage::German,   "Die folgenden Dateien sind ungültig:" },
            { ELanguage::French,   "Les fichiers suivants ne sont pas valides :" },
            { ELanguage::Spanish,  "Los siguientes archivos no son válidos:" },
            { ELanguage::Italian,  "I seguenti file non sono validi:" }
        }
    },
    {
        // Notes: message appears in the event there are some invalid files after adding the DLC and moving onto the next step.
        // Japanese Notes: This localization should include furigana.
        "Installer_Message_InvalidFiles",
        {
            { ELanguage::English,  "Some of the files that have\nbeen provided are not valid.\n\nPlease make sure all the\nspecified files are correct\nand try again." },
            { ELanguage::Japanese, "[提供:ていきょう]されたファイルの[一部:いちぶ]が[有効:ゆうこう]ではありません\n[指定:してい]されたファイルがすべて[正:ただ]しいことを[確認:かくにん]して\nもう[一度:いちど]お[試:ため]しください" },
            { ELanguage::German,   "Einige Dateien, die bereitgestellt\nwurden sind ungültig.\n\nBitte stelle sicher, dass\ndie angegebenen Dateien korrekt\nsind und versuche es erneut." },
            { ELanguage::French,   "Certains fichiers fournis ne\nsont pas valides.\n\nVeuillez vous assurer que tous\nles fichiers spécifiés sont\ncorrects et réessayez." },
            { ELanguage::Spanish,  "Algunos de los archivos\nseleccionados no son válidos.\n\nPor favor, asegúrate de que\ntodos los archivos son correctos\ne inténtalo de nuevo.\n" },
            { ELanguage::Italian,  "Alcuni dei file che sono stati\nselezionati non sono validi.\n\nAssicurati che tutti\ni file sono quelli corretti\ne riprova." }
        }
    },
    {
        // Notes: message appears when clicking the "Add Files" option for the first time.
        // Japanese Notes: This localization should include furigana.
        "Installer_Message_FilePickerTutorial",
        {
            { ELanguage::English,  "Select a digital dump with\ncontent from the game.\n\nThese files can be obtained from\nyour Xbox 360 hard drive by\nfollowing the instructions on\nthe GitHub page.\n\nFor choosing a folder with extracted\nand unmodified game files, use\nthe \"Add Folder\" option instead." },
            { ELanguage::Japanese, "ゲームのコンテンツを[含:ふく]む デジタルダンプを[選択:せんたく]してください\n\nこれらのファイルは GitHubページの[指示:しじ]に[従:したが]って\nXbox 360ハードドライブから[取得:しゅとく]できます\n\n[抽出:ちゅうしゅつ]された[変更:へんこう]されていない\nゲームファイルを[含:ふく]むフォルダーを[選択:せんたく]するには\n[代:か]わりに「フォルダの[追加:ついか]」オプションを[使用:しよう]してください" },
            { ELanguage::German,   "Wähle einen digitalen Dump von dem Spiel.\n\nDie Dateien können über die Festplatte deiner\nXbox 360 erlangt werden.\nFolge hierfür den Anweisungen auf der GitHub Seite.\n\nUm einen Ordner mit unmodifizierten Spieldateien auszuwählen, benutze die \"Ordner Hinzufügen\" Option stattdessen." },
            { ELanguage::French,   "Sélectionnez une copie\ndématérialisée avec le contenu du\njeu de base.\n\nCes fichiers peuvent être obtenus\nà partir du disque dur de votre\nXbox 360 en suivant les\ninstructions de la page GitHub.\n\nPour choisir un dossier contenant\nles fichiers de jeu extraits et\nnon modifiés, utilisez plutôt\nl'option \"Ajouter un dossier\"." },
            { ELanguage::Spanish,  "Selecciona una copia digital\ncon contenido del juego.\n\nPuedes obtener los archivos\nde tu disco duro de Xbox 360\nsiguiendo las instrucciones de\nla página de GitHub.\n\nPara elegir una carpeta con\narchivos extraídos sin modificar,\nutiliza la opción \"Añadir Carpeta\"." },
            { ELanguage::Italian,  "Seleziona una copia digitale\ncon i contenuti del gioco.\n\nQuesti file possono essere ottenuti\ndall'hard drive della tua Xbox 360\nseguendo le istruzioni\nsulla pagina GitHub.\n\nPer selezionare una cartella\ncon file estratti e non modificati\nusa l'opzione \"Aggiungi cartella\"." }
        }
    },
    {
        // Notes: message appears when clicking the "Add Folder" option for the first time.
        // Japanese Notes: This localization should include furigana.
        "Installer_Message_FolderPickerTutorial",
        {
            { ELanguage::English,  "Select a folder that contains the\nunmodified files that have been\nextracted from the game.\n\nThese files can be obtained from\nyour Xbox 360 hard drive by\nfollowing the instructions on\nthe GitHub page.\n\nFor choosing a digital dump,\nuse the \"Add Files\" option instead." },
            { ELanguage::Japanese, "ゲームから[抽出:ちゅうしゅつ]された[変更:へんこう]されていない\nファイルを含むフォルダを[選択:せんたく]してください\n\nこれらのファイルは GitHubページの[指示:しじ]に[従:したが]って\nXbox 360ハードドライブから[取得:しゅとく]できます\n\nデジタルダンプを[選択:せんたく]するには\n[代:か]わりに「ファイルの[追加:ついか]」オプションを[使用:しよう]してください" },
            { ELanguage::German,   "Wähle einen Ordner, der unmodifizierte Dateien, die vom Spiel extrahiert wurden enthält.\n\nDie Dateien können über die Festplatte deiner\nXbox 360 erlangt werden.\nFolge hierfür den Anweisungen auf der GitHub Seite.\n\nUm einen digitalen Dump auszuwählen, benutze die \"Datei Hinzufügen\" Option stattdessen." },
            { ELanguage::French,   "Sélectionnez un dossier contenant\nles fichiers extraits du jeu de\nbase.\n\nCes fichiers peuvent être obtenus\nà partir du disque dur de votre\nXbox 360 en suivant les\ninstructions de la page GitHub.\n\nPour choisir une copie\ndématérialisée, utilisez plutôt\nl'option \"Ajouter des fichiers\"." },
            { ELanguage::Spanish,  "Selecciona una carpeta que\ncontenga los archivos sin\nmodificar extraídos del juego.\n\nPuedes obtener los archivos\nde tu disco duro de Xbox 360\nsiguiendo las instrucciones de\nla página de GitHub.\n\nPara elegir una copia digital,\nutiliza la opción \"Añadir Archivos\"." },
            { ELanguage::Italian,  "Seleziona una cartella che contiene\ni file non modificati che sono stati\nestratti dal gioco.\n\nQuesti file possono essere ottenuti\ndall'hard drive della tua Xbox 360\nseguendo le istruzioni\nsulla pagina GitHub.\n\nPer selezionare una copia digitale\nusa l'opzione \"Aggiungi file\"." }
        }
    },
    {
        // Notes: message appears when providing a title update that does not match the region or version of the game dump.
        // Japanese Notes: This localization should include furigana.
        "Installer_Message_IncompatibleGameData",
        {
            { ELanguage::English,  "The specified game and\ntitle update are incompatible.\n\nPlease ensure the files are\nfor the same version and\nregion and try again." },
            { ELanguage::Japanese, "[指定:してい]されたゲームとタイトルアップデートは[互換性:ごかんせい]がありません\n\nファイルのバージョンと[地域:ちいき]が\n[同:おな]じであることを[確認:かくにん]して\nもう[一度:いちど]お[試:ため]しください" },
            { ELanguage::German,   "Die ausgewählten Spiel- und\nUpdatedateien sind inkompatibel.\n\nBitte stelle sicher, dass\ndie Dateien für die selbe\nVersion und Region vorgesehen sind\nund versuche es erneut." },
            { ELanguage::French,   "Les fichiers du jeu et la mise à\njour sont incompatibles.\n\nVeuillez vous assurer que les\nfichiers sont pour la même\nversion/région puis réessayez." },
            { ELanguage::Spanish,  "El juego seleccionado\ny su actualización son incompatibles.\n\nPor favor, asegúrate de que que los archivos\nson de la misma versión y\nregión e inténtalo de nuevo." },
            { ELanguage::Italian,  "I file del gioco\ne dell'aggiornamento sono incompatibili.\n\nAssicurati che i file sono\ndella stessa versione\ne regione e riprova." }
        }
    },
    {
        // Notes: message appears when clicking Skip at the DLC step.
        // Japanese Notes: This localization should include furigana.
        "Installer_Message_DLCWarning",
        {
            { ELanguage::English,  "It is highly recommended\nthat you install all of the\nDLC, as it includes high\nquality lighting textures\nfor the base game.\n\nAre you sure you want to\nskip this step?" },
            { ELanguage::Japanese, "ベースゲーム[用:よう]の[高品質:こうひんしつ]の\n[照明:しょうめい]テクスチャが[含:ふく]まれているため\nすべてのDLCをインストールすることを\n[強:つよ]くお[勧:すす]めします\n\nこの[手順:てじゅん]をスキップしてもよろしいですか？" },
            { ELanguage::German,   "Es wird empgohlen alle Erweiterungen zu installieren, da sie Beleuchtungs-Texturen in einer höheren Qualität für das Basisspiel beinhalten.\n\nBist du dir sicher, dass du diesen Schritt überspringen möchtest?" },
            { ELanguage::French,   "Il est fortement recommandé\nd'installer l'ensemble du\nDLC car elle inclut des\ntextures de lumière de\nhaute qualité pour le jeu\nde base.\n\nÊtes-vous sûr de vouloir\nignorer cette étape ?" },
            { ELanguage::Spanish,  "Se recomienda encarecidamente\ninstalar todo el DLC, ya que\ncontiene texturas de iluminación\nde alta calidad para el juego base.\n\n¿Seguro que quieres saltar este paso?" },
            { ELanguage::Italian,  "Si consiglia di installare\ntutti i DLC, poichè includono\ntexture di illuminazione di qualità migliore.\n\nSei sicuro di voler saltare?" }
        }
    },
    {
        // Notes: message appears when choosing the Install option at the title screen when the user is missing DLC content.
        // Japanese Notes: This localization should include furigana.
        "Installer_Message_TitleMissingDLC",
        {
            { ELanguage::English,  "This will restart the game to\nallow you to install any DLC\nthat you are missing.\n\nInstalling DLC will improve the\nlighting quality across the game.\n\nWould you like to install missing\ncontent?" },
            { ELanguage::Japanese, "これによりゲームが[再起動:さいきどう]し[不足:ふそく]しているDLCを\nインストールできるようになります\n\nDLCをインストールすると\nゲーム[全体:ぜんたい]の [照明品質:しょうめいひんしつ] が[向上:こうじょう]します\n\n[不足:ふそく]しているコンテンツを\nインストールしますか？" },
            { ELanguage::German,   "Das Spiel wird neu gestartet\num die Installation einer fehlenden\nErweiterung zu ermöglichen.\n\nDie Installation einer\nErweiterung erhöht die Qualität\nder Beleuchtung im gesamten Spiel.\n\nMöchtest du den fehlenden\nInhalt installieren?" },
            { ELanguage::French,   "Cela redémarrera le jeu pour vous\npermettre d'installer les DLC\nmanquants.\n\nL'installation du DLC améliorera\nla qualité de l'éclairage dans le\njeu.\n\nSouhaitez-vous installer le\ncontenu manquant ?" },
            { ELanguage::Spanish,  "Esta opción reiniciará el juego\npara permitirte instalar los DLC\nque falten.\n\nInstalar el DLC mejorará la calidad\nde iluminación en todo el juego.\n\n¿Quieres instalar el contenido\nque falta?" },
            { ELanguage::Italian,  "Questa opzione riavviera il gioco\nper farti installare qualsiasi DLC\nche non hai installato.\n\nL'installazione dei DLC migliorerà la qualità\ndell'illuminazione del gioco.\n\nVuoi installare i DLC mancanti?" }
        }
    },
    {
        // Notes: message appears when choosing the Install option at the title screen when the user is not missing any content.
        // Japanese Notes: This localization should include furigana.
        "Installer_Message_Title",
        {
            { ELanguage::English,  "This restarts the game to\nallow you to install any DLC\nthat you may be missing.\n\nYou are not currently\nmissing any DLC.\n\nWould you like to proceed\nanyway?" },
            { ELanguage::Japanese, "これによりゲームが[再起動:さいきどう]され\n[不足:ふそく]しているDLCを\nインストールできるようになります\n\n[現在:げんざい] [不足:ふそく]しているDLCはありません\n\nそれでも[続行:ぞっこう]しますか？" },
            { ELanguage::German,   "Das Spiel wird neu gestartet\num die Installation einer fehlenden\nErweiterung zu ermöglichen.\n\nEs kann keine weitere Erweiterung\ninstalliert werden.\n\nMöchtest du trotzdem fortfahren?" },
            { ELanguage::French,   "Cela redémarrera le jeu pour vous\npermettre d'installer les DLC\nmanquants.\n\nIl ne vous manque aucun DLC.\n\nVoulez-vous quand même continuer ?" },
            { ELanguage::Spanish,  "Esto reiniciará el juego\npara permitirte instalar\nlos DLC que falten.\n\nActualmente, no falta ningún\nDLC por instalarse.\n\n¿Quieres continuar de todos\nmodos?" },
            { ELanguage::Italian,  "Questa opzione riavviera il gioco\nper farti installare qualsiasi DLC\nche non hai installato.\n\nHai già installato tutti i DLC.\n\nVuoi procedere comunque?" }
        }
    },
    {
        // Notes: message appears when user chooses "Quit" on the first available installation screen.
        // Japanese Notes: This localization should include furigana.
        "Installer_Message_Quit",
        {
            { ELanguage::English,  "Are you sure you want to quit?" },
            { ELanguage::Japanese, "[終了:しゅうりょう]してもよろしいですか？" },
            { ELanguage::German,   "Installation verlassen?" },
            { ELanguage::French,   "Êtes-vous sûr de vouloir quitter ?" },
            { ELanguage::Spanish,  "¿Estás seguro de que quieres\nsalir?" },
            { ELanguage::Italian,  "Sicuro di voler uscire?" }
        }
    },
    {
        // Notes: message appears when user chooses "Cancel" during installation.
        // Japanese Notes: This localization should include furigana.
        "Installer_Message_Cancel",
        {
            { ELanguage::English,  "Are you sure you want to cancel the installation?" },
            { ELanguage::Japanese, "インストールをキャンセルしてもよろしいですか？" },
            { ELanguage::German,   "Bist du dir sicher, dass du die Installation abbrechen möchtest?" },
            { ELanguage::French,   "Êtes-vous sûr de vouloir annuler l'installation ?" },
            { ELanguage::Spanish,  "¿Estás seguro de que quieres cancelar la instalación?" },
            { ELanguage::Italian,  "Sicuro di voler annullare l'installazione?" }
        }
    },
    {
        // Notes: message appears when pressing B at the title screen.
        // Japanese Notes: This localization should include furigana.
        "Title_Message_Quit",
        {
            { ELanguage::English,  "Are you sure you want to quit?" },
            { ELanguage::Japanese, "[終了:しゅうりょう]してもよろしいですか？" },
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
        // Japanese Notes: This localization should include furigana.
        "Title_Message_AchievementDataCorrupt",
        {
            { ELanguage::English,  "The achievement data appears to be\ncorrupted and cannot be loaded.\n\nProceeding from this point will\nclear your achievement data." },
            { ELanguage::Japanese, "[実績:じっせき]データが[破損:はそん]しているため\n[読:よ]み[込:こ]むことができません\n\nこの[先:さき]に[進:すす]むと[実績:じっせき]データが\n[消去:しょうきょ]されます" },
            { ELanguage::German,   "Die Erfolgsdaten sind möglicherweise\nfehlerhaft und können nicht\ngeladen werden.\n\nDurch das Fortfahren werden\ndeine bisherigen Erfolgsdaten gelöscht." },
            { ELanguage::French,   "Les données des succès semblent être\nendommagées et ne peuvent être\nchargées.\n\nSi vous continuez, vos données\nseront écrasées." },
            { ELanguage::Spanish,  "Los datos de logros parecen estar\ncorruptos y no pueden cargarse.\n\nContinuar a partir de este punto\neliminará los datos de logros." },
            { ELanguage::Italian,  "I file degli obiettivi sembrano danneggiati\ne non possono essere caricati.\n\nSe prosegui da questo punto\ni tuoi obiettivi verranno cancellati." }
        }
    },
    {
        // Notes: message appears when ACH-DATA cannot be loaded upon pressing start at the title screen.
        // To make this occur, lock the ACH-DATA file using an external program so that it cannot be accessed by the game.
        // Japanese Notes: This localization should include furigana.
        "Title_Message_AchievementDataIOError",
        {
            { ELanguage::English,  "The achievement data could not be loaded.\nYour achievements will not be saved." },
            { ELanguage::Japanese, "[実績:じっせき]データを[読:よ]み[込:こ]めませんでした\n[実績:じっせき]は[保存:ほぞん]されません。" },
            { ELanguage::German,   "Die Erfolgsdaten konnten nicht geladen werden.\nDeine Erfolge werden nicht gespeichert." },
            { ELanguage::French,   "Les données des succès ne\npeuvent être chargées.\nVos succès ne seront pas\nsauvegardés." },
            { ELanguage::Spanish,  "Los datos de logros no pueden cargarse.\nTus logros no serán guardados." },
            { ELanguage::Italian,  "I file degli obiettivi non possono essere caricati.\nI tuoi obiettivi non verranno salvati." }
        }
    },
    {
        // Japanese Notes: This localization should include furigana.
        "Title_Message_UpdateAvailable",
        {
            { ELanguage::English,  "An update is available!\n\nWould you like to visit the\nreleases page to download it?" },
            { ELanguage::Japanese, "アップデートが[利用:りよう][可能:かのう]です\n\nリリースページにアクセスして\nダウンロードしますか？" },
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
            { ELanguage::Japanese, "D3D12 (Windows)または\nVulkanバックエンドを作成できません\n\n次の点を確認してください：\n\n※システムが最小要件を満たしている\n※GPUドライバーが最新である\n※オペレーティングシステムが最新バージョンである" },
            { ELanguage::German,   "Es ist nicht möglich, ein D3D12 (Windows) oder Vulkan-Backend zu erstellen.\n\nBitte stelle sicher, dass:\n\n- Dein System die Mindestanforderungen erfüllt.\n- Deine GPU-Treiber auf dem neuesten Stand sind.\n- Dein Betriebssystem auf der neuesten verfügbaren Version ist." },
            { ELanguage::French,   "Impossible de créer un backend D3D12 (Windows) ou Vulkan.\n\nVeuillez vous assurer que :\n\n- Votre système répond aux critères minimums requis.\n- Les pilotes de votre processeur graphique sont à jour.\n- Votre système d'exploitation est à jour." },
            { ELanguage::Spanish,  "No se puede crear un entorno de D3D12 (Windows) o de Vulkan.\n\nPor favor, asegúrate de que:\n\n- Tu equipo cumple con los requisitos mínimos.\n- Los drivers de tu tarjeta gráfica están actualizados.\n- Tu sistema operativo está actualizado a la última versión.\n" },
            { ELanguage::Italian,  "Impossibile creare un backend D3D12 (Windows) o Vulkan.\n\nAssicurati che:\n\n- Il tuo sistema soddisfi i requisiti minimi.\n- I driver della scheda grafica siano aggiornati.\n- Il tuo sistema operativo sia aggiornato." }
        }
    },
    {
        "System_Win32_MissingDLLs",
        {
            { ELanguage::English,  "The module \"%s\" could not be found.\n\nPlease make sure that:\n\n- You extracted this copy of Unleashed Recompiled fully and not just the *.exe file.\n- You are not running Unleashed Recompiled from a *.zip file." },
            { ELanguage::Japanese, "モジュール\"%s\"が見つかりませんでした\n\n次の点を確認してください：\n\n※Unleashed Recompiledの*.exeファイルだけを抽出していなく、 コピーを完全に抽出してること\n※Unleashed Recompiledを*.zipファイルから実行していないこと" },
            { ELanguage::German,   "Das Modul \"%s\" konnte nicht gefunden werden.\n\nBitte stelle sicher, dass:\n\n- Diese Kopie von Unleashed Recompiled vollständig entpackt wurde und nicht nur die *.exe-Datei.\n- Unleashed Recompiled nicht direkt aus einer *.zip-Datei ausgeführt wird." },
            { ELanguage::French,   "Le module \"%s\" n'a pas pu être trouvé.\n\nVeuillez vous assurer que :\n\n- Vous avez extrait Unleashed Recompiled dans son entièreté et pas seulement le fichier *.exe.\n- Vous n'exécutez pas Unleashed Recompiled à partir d'un fichier *.zip." },
            { ELanguage::Spanish,  "No se pudo encontrar el módulo \"%s\".\n\nAsegúrese de que:\n\n- Ha extraido esta copia de Unleashed Recompiled por completo y no solo el archivo *.exe.\n- No está ejecutando Unleashed Recompiled desde un archivo *.zip." },
            { ELanguage::Italian,  "Impossibile trovare il modulo \"%s\".\n\nAssicurati che:\n\n- Hai estratto questa copia di Unleashed Recompiled correttamente e non solo il file *.exe.\n- Non stai eseguendo Unleashed Recompiled da un file *.zip." }
        }
    },
    {
        "System_MemoryAllocationFailed",
        {
            { ELanguage::English,  "Failed to allocate game memory.\n\nPlease make sure that:\n\n- You meet the minimum system requirements (8 GB).\n- Your page file is configured with at least 4-8 GB of virtual memory." },
            { ELanguage::Japanese, "ゲームメモリの割り当てに失敗しました\n\n次の点を確認してください：\n\n※最小システム要件（8 GB）を満たしていること。\n※ページファイルに少なくとも4～8 GBの仮想メモリが設定されていること" },
            { ELanguage::German,   "Fehler beim Zuweisen des Spielspeichers.\n\nBitte stelle sicher, dass:\n\n- Die Mindestanforderungen für das System erfüllt sind (8 GB).\n- Die Auslagerungsdatei mit mindestens 4-8 GB virtuellem Speicher konfiguriert ist." },
            { ELanguage::French,   "Échec d'allocation de la mémoire du jeu.\n\nVeuillez vous assurer que :\n\n- Vous disposez de la configuration minimale requise (8 Go).\n- Votre fichier d’échange est configuré avec au moins 4 à 8 Go de mémoire virtuelle." },
            { ELanguage::Spanish,  "Fallo al asignar memoria del juego.\n\nPor favor, asegúrate de que:\n\n- Cumples los requisitos mínimos del sistema (8 GB).\n- Tu archivo de páginación está configurado con al menos 4 u 8 GB de memoria virtual." },
            { ELanguage::Italian,  "Impossibile allocare la memoria per il gioco.\n\nAssicurati che:\n\n- Soddisfi i requisiti minimi di sistema (8 GB).\n- Il tuo file di paging sia configurato con almeno 4 o 8 GB di memoria virtuale." }
        }
    },
    {
        "IntegrityCheck_Success",
        {
            { ELanguage::English,  "Installation check has finished.\n\nAll files seem to be correct.\n\nThe game will now close. Remove the launch argument to play the game." },
            { ELanguage::Japanese, "インストールチェックが終了しました\n\nすべてのファイルは正しいようです\n\nゲームは終了します、ゲームをプレイするには起動引数を削除してください" },
            { ELanguage::German,  "Die Installation wurde überprüft.\n\nAlle Dateien scheinen korrekt zu sein.\n\nDas Spiel wird nun geschlossen. Entferne die Startoption, um das Spiel zu spielen." },
            { ELanguage::French,  "La vérification de l'installation est terminée.\n\nTous les fichiers semblent corrects.\n\nL'application va maintenant se fermer. Retirez l'argument de lancement pour pouvoir lancer le jeu." },
            { ELanguage::Spanish,  "La verificación de la instalación ha terminado.\n\nTodos los archivos parecen correctos.\n\nEl juego se cerrará ahora. Elimina el argumento de lanzamiento para jugar al juego." },
            { ELanguage::Italian,  "La verifica dei file d'installazione è terminata.\n\nTutti i file sembrano corretti.\n\nIl gioco si chiuderà. Rimuovi l'argomento di avvio per poter giocare." }
        }
    },
    {
        "IntegrityCheck_Failed",
        {
            { ELanguage::English,  "Installation check has failed.\n\nError: %s\n\nThe game will now close. Try reinstalling the game by using the --install launch argument." },
            { ELanguage::Japanese, "インストールチェックに失敗しました\n\nエラー：%s\n\nゲームは終了します、--install 起動引数を使用してゲームを再インストールしてください" },
            { ELanguage::German,  "Die Installationsprüfung ist fehlgeschlagen.\n\nFehler: %s\n\nDas Spiel wird nun geschlossen. Versuche das Spiel durch Verwendung der Startoption --install neu zu installieren." },
            { ELanguage::French,  "La vérification de l'installation a échoué.\n\nErreur : %s\n\nL'application va maintenant se fermer. Essayez de réinstaller le jeu en utilisant l'argument de lancement --install." },
            { ELanguage::Spanish,  "La verificación de la instalación ha fallado.\n\nError: %s\n\nEl juego se cerrará ahora. Intenta reinstalar el juego utilizando el argumento de lanzamiento --install." },
            { ELanguage::Italian,  "La verifica dei file d'installazione non è andata a buon fine.\n\nErrore: %s\n\nIl gioco si chiuderà. Prova a reinstallare il gioco utilizzando l'argomento di avvio --install." }
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
            { ELanguage::Japanese, "はい" },
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
            { ELanguage::Japanese, "いいえ" },
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
            { ELanguage::Japanese, "次へ" },
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
            { ELanguage::Japanese, "決定" },
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
            { ELanguage::Japanese, "もどる" },
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
            { ELanguage::Japanese, "終了" },
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
            { ELanguage::Japanese, "キャンセル" },
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
            { ELanguage::Japanese, "リセット" },
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
            { ELanguage::Japanese, "きりかえ" },
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
