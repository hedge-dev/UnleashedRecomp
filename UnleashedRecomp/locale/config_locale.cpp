#include <user/config.h>

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

#define CONFIG_DEFINE_LOCALE(name) \
    CONFIG_LOCALE g_##name##_locale =

#define CONFIG_DEFINE_ENUM_LOCALE(type) \
    CONFIG_ENUM_LOCALE(type) g_##type##_locale =

CONFIG_DEFINE_LOCALE(Language)
{
    { ELanguage::English,  { "Language", "Change the language used for text and logos." } },
    { ELanguage::Japanese, { "言語", "" } },
    { ELanguage::German,   { "Sprache", "Ändere die Sprache für Text und Logos." } },
    { ELanguage::French,   { "Langue", "Modifie la langue utilisée pour le texte et logos." } },
    { ELanguage::Spanish,  { "Idioma", "" } },
    { ELanguage::Italian,  { "Lingua", "Cambia la lingua utilizzata per i testi e i logo." } }
};

// Notes: do not localise this.
CONFIG_DEFINE_ENUM_LOCALE(ELanguage)
{
    {
        ELanguage::English,
        {
            { ELanguage::English,  { "ENGLISH", "" } },
            { ELanguage::Japanese, { "日本語", "" } },
            { ELanguage::German,   { "DEUTSCH", "" } },
            { ELanguage::French,   { "FRANÇAIS", "" } },
            { ELanguage::Spanish,  { "ESPAÑOL", "" } },
            { ELanguage::Italian,  { "ITALIANO", "" } }
        }
    }
};

CONFIG_DEFINE_ENUM_LOCALE(EVoiceLanguage)
{
    {
        ELanguage::English,
        {
            { EVoiceLanguage::English,  { "ENGLISH", "" } },
            { EVoiceLanguage::Japanese, { "JAPANESE", "" } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EVoiceLanguage::English,  { "英語", "" } },
            { EVoiceLanguage::Japanese, { "日本語", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { EVoiceLanguage::English,  { "ENGLISCH", "" } },
            { EVoiceLanguage::Japanese, { "JAPANISCH", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EVoiceLanguage::English,  { "ANGLAIS", "" } },
            { EVoiceLanguage::Japanese, { "JAPONAIS", "" } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { EVoiceLanguage::English,  { "INGLÉS", "" } },
            { EVoiceLanguage::Japanese, { "JAPONÉS", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EVoiceLanguage::English,  { "INGLESE", "" } },
            { EVoiceLanguage::Japanese, { "GIAPPONESE", "" } }
        }
    },
};

CONFIG_DEFINE_LOCALE(Hints)
{
    { ELanguage::English,  { "Hints", "Show hints during gameplay." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Hinweise", "Zeige Hinweise während des Spiels." } },
    { ELanguage::French,   { "Indices", "Affiche les indices pendant le jeu." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Indizi", "Mostra degli indizzi durante il gioco." } }
};

CONFIG_DEFINE_LOCALE(ControlTutorial)
{
    { ELanguage::English,  { "Control Tutorial", "Show controller hints during gameplay.\n\nThe Werehog Critical Attack prompt will be unaffected." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Steuerungsanleitung", "Zeige Steuerungshinweise während des Spiels.\n\nKritische Angriffe des Werehogs werden hiervon nicht beeinflusst." } },
    { ELanguage::French,   { "Indication des commandes", "Affiche les indications des commandes pendant le jeu.\n\nCeci n'affecte pas les Coup critiques du Werehog." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Tutorial dei comandi", "Mostra i tutorial dei comandi durante il gioco.\n\nIl tutorial per l'attacco critico del Werehog non verrà influenzato da questa opzione." } }
};

CONFIG_DEFINE_LOCALE(AchievementNotifications)
{
    { ELanguage::English,  { "Achievement Notifications", "Show notifications for unlocking achievements.\n\nAchievements will still be rewarded with notifications disabled." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Erfolgsbenachrichtigungen", "Zeige Benachrichtigungen für das Freischalten von Erfolgen.\n\nErfolge werden weiterhin freigeschaltet, auch wenn die Benachrichtigungen ausgeschaltet sind." } },
    { ELanguage::French,   { "Notification des succès", "Affiche les notifications pour le déverrouillage des succès.\n\nLes succès seront toujours obtenus même si les notifications sont désactivées." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Notifiche obiettivi", "Mostra delle notifiche quando sblocchi degli obiettivi.\n\nGli obiettivi verranno comunque assegnati anche con le notifiche disattivate." } }
};

CONFIG_DEFINE_LOCALE(TimeOfDayTransition)
{
    { ELanguage::English,  { "Time of Day Transition", "Change how the loading screen appears when switching time of day in the hub areas." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Tageszeit Übergang", "Verändere den Ladebildschirm beim Verändern der Tageszeit in Städten und Eingangslevel." } },
    { ELanguage::French,   { "Transition jour/nuit", "Change l'écran de chargement lors de la transition dans le hub." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Transizione giorno/notte", "Modifica l'aspetto della schermata di caricamento quando fai scorrere il tempo." } }
};

CONFIG_DEFINE_ENUM_LOCALE(ETimeOfDayTransition)
{
    {
        ELanguage::English,
        {
            { ETimeOfDayTransition::Xbox,        { "XBOX", "Xbox: the transformation cutscene will play with artificial loading times." } },
            { ETimeOfDayTransition::PlayStation, { "PLAYSTATION", "PlayStation: a spinning medal loading screen will be used instead." } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { ETimeOfDayTransition::Xbox,        { "", "" } },
            { ETimeOfDayTransition::PlayStation, { "", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { ETimeOfDayTransition::Xbox,        { "XBOX", "Xbox: Eine Transformationszwischensequenz mit künstlicher Ladezeit wird abgespielt." } },
            { ETimeOfDayTransition::PlayStation, { "PLAYSTATION", "PlayStation: Eine sich drehende Medallie wird stattdessen als Ladebildschirm verwendet." } }
        }
    },
    {
        ELanguage::French,
        {
            { ETimeOfDayTransition::Xbox,        { "XBOX", "Xbox: la scène de transformation sera jouée avec des temps de chargement artificiel." } },
            { ETimeOfDayTransition::PlayStation, { "PLAYSTATION", "PlayStation: un écran de chargement avec une médaille tournoyante sera utilisé à la place." } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { ETimeOfDayTransition::Xbox,        { "", "" } },
            { ETimeOfDayTransition::PlayStation, { "", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { ETimeOfDayTransition::Xbox,        { "XBOX", "Xbox: la scena di trasformazione verrà riprodotta con tempi di caricamento artificiali." } },
            { ETimeOfDayTransition::PlayStation, { "PLAYSTATION", "PlayStation: verrà utilizzata una schermata di caricamento con una medaglia che gira." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(ControllerIcons)
{
    { ELanguage::English,  { "Controller Icons", "Change the icons to match your controller." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Controllersymbole", "Ändere die Controllersymbole, um sie auf dein Modell anzupassen." } },
    { ELanguage::French,   { "Icône des boutons", "Modifie les icônes pour les faire correspondre à votre manette." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Icone dei tasti", "Modifica le icone per farle corrispondere con il tuo controller." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EControllerIcons)
{
    {
        ELanguage::English,
        {
            { EControllerIcons::Auto,        { "AUTO", "Auto: the game will determine which icons to use based on the current input device." } },
            { EControllerIcons::Xbox,        { "XBOX", "" } },
            { EControllerIcons::PlayStation, { "PLAYSTATION", "" } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EControllerIcons::Auto,        { "", "" } },
            { EControllerIcons::Xbox,        { "", "" } },
            { EControllerIcons::PlayStation, { "", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { EControllerIcons::Auto,        { "AUTO", "Auto: Das Spiel erkennt automatisch deinen Controller um die Symbole dementsprechend anzupassen." } },
            { EControllerIcons::Xbox,        { "XBOX", "" } },
            { EControllerIcons::PlayStation, { "PLAYSTATION", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EControllerIcons::Auto,        { "AUTO", "Auto: le jeu déterminera automatiquement quelles icônes utiliser en fonction du périphérique d'entrée." } },
            { EControllerIcons::Xbox,        { "XBOX", "" } },
            { EControllerIcons::PlayStation, { "PLAYSTATION", "" } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { EControllerIcons::Auto,        { "", "" } },
            { EControllerIcons::Xbox,        { "", "" } },
            { EControllerIcons::PlayStation, { "", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EControllerIcons::Auto,        { "AUTO", "Auto: il gioco determinerà quali icone da utilizzare in base al dispositivo di input attuale." } },
            { EControllerIcons::Xbox,        { "XBOX", "" } },
            { EControllerIcons::PlayStation, { "PLAYSTATION", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(HorizontalCamera)
{
    { ELanguage::English,  { "Horizontal Camera", "Change how the camera moves left and right." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Horizontale Kamera", "Ändere wie sich die Kamera nach links und rechts bewegt." } },
    { ELanguage::French,   { "Caméra horizontale", "Modifie la rotation horizontale de la caméra." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Telecamera orizzontale", "Modifica come la telecamera si muove da sinistra a destra." } }
};

CONFIG_DEFINE_LOCALE(VerticalCamera)
{
    { ELanguage::English,  { "Vertical Camera", "Change how the camera moves up and down." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Vertikale Kamera", "Ändere wie sich die Kamera nach oben und unten bewegt." } },
    { ELanguage::French,   { "Caméra verticale", "Modifie la rotation verticale de la caméra." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Telecamera verticale", "Modifica come la telecamera si muove su e giù." } }
};

CONFIG_DEFINE_ENUM_LOCALE(ECameraRotationMode)
{
    {
        ELanguage::English,
        {
            { ECameraRotationMode::Normal,  { "NORMAL", "" } },
            { ECameraRotationMode::Reverse, { "REVERSE", "" } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { ECameraRotationMode::Normal,  { "", "" } },
            { ECameraRotationMode::Reverse, { "", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { ECameraRotationMode::Normal,  { "NORMAL", "" } },
            { ECameraRotationMode::Reverse, { "INVERTIERT", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { ECameraRotationMode::Normal,  { "NORMALE", "" } },
            { ECameraRotationMode::Reverse, { "INVERSÉE", "" } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { ECameraRotationMode::Normal,  { "", "" } },
            { ECameraRotationMode::Reverse, { "", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { ECameraRotationMode::Normal,  { "NORMALE", "" } },
            { ECameraRotationMode::Reverse, { "INVERTITA", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(Vibration)
{
    { ELanguage::English,  { "Vibration", "Toggle controller vibration." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Vibration", "Schalte die Controllervibration an oder aus." } },
    { ELanguage::French,   { "Vibration", "Active les vibrations manette." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Vibrazione", "Attiva/disattiva la vibrazione del controller." } }
};

CONFIG_DEFINE_LOCALE(AllowBackgroundInput)
{
    { ELanguage::English,  { "Allow Background Input", "Allow controller input whilst the game window is unfocused." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Erlaube Hintergrundeingaben", "Erlaube Eingaben deines Controllers auch wenn das Spielfenster nicht fokussiert ist." } },
    { ELanguage::French,   { "Manette en arrière plan", "Permet d'utiliser la manette dans le jeu lorsque qu'il n'est pas au premier plan." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Input con la finestra inattiva", "Attiva/disattiva i tasti del controller mentre la finestra è inattiva." } }
};

CONFIG_DEFINE_LOCALE(MasterVolume)
{
    { ELanguage::English,  { "Master Volume", "Adjust the overall volume." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Gesamtlautstärke", "Passe die Gesamtlautstärke an." } },
    { ELanguage::French,   { "Volume général", "Réglage du volume général." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Volume principale", "Regola il volume principale" } }
};

CONFIG_DEFINE_LOCALE(MusicVolume)
{
    { ELanguage::English,  { "Music Volume", "Adjust the volume for the music." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Musiklautstärke", "Passe die Lautstärke der Musik an." } },
    { ELanguage::French,   { "Volume de la musique", "Réglage du volume de la musique." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Volume musica di sottofondo", "Regola il volume della musica di sottofondo." } }
};

CONFIG_DEFINE_LOCALE(EffectsVolume)
{
    { ELanguage::English,  { "Effects Volume", "Adjust the volume for sound effects." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Soundeffektlautstärke", "Passe die Lautstärke der Soundeffekte an." } },
    { ELanguage::French,   { "Volume des effets sonores", "Réglage du volume des effets sonores." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Volume effetti sonori", "Regola il volume degli effetti sonori." } }
};

CONFIG_DEFINE_LOCALE(MusicAttenuation)
{
    { ELanguage::English,  { "Music Attenuation", "Fade out the game's music when external media is playing." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Musikdämpfung", "Stelle die Musik des Spiels stumm während externe Medien abgespielt werden." } },
    { ELanguage::French,   { "Atténuation audio", "Abaisse le volume des musiques du jeu lorsqu'un média externe est en cours de lecture." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Attenuazione musica", "Abbassa il volume della musica di sottofondo quando un'altra applicazione riproduce dei suoni." } }
};

CONFIG_DEFINE_LOCALE(ChannelConfiguration)
{
    { ELanguage::English,  { "Channel Configuration", "Change the output mode for your audio device." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Kanalkonfiguration", "Ändere den Ausgabemodus für dein Audioausgabegerät." } },
    { ELanguage::French,   { "Configuration sortie audio", "Modifie le mode de sortie pour votre périphérique audio." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Configurazione canali audio", "Modifica la modalità di output per il tuo dispositivo audio." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EChannelConfiguration)
{
    {
        ELanguage::English,
        {
            { EChannelConfiguration::Stereo,   { "STEREO", "" } },
            { EChannelConfiguration::Surround, { "SURROUND", "" } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EChannelConfiguration::Stereo,   { "", "" } },
            { EChannelConfiguration::Surround, { "", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { EChannelConfiguration::Stereo,   { "STEREO", "" } },
            { EChannelConfiguration::Surround, { "SURROUND", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EChannelConfiguration::Stereo,   { "STÉRÉO", "" } },
            { EChannelConfiguration::Surround, { "SURROUND", "" } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { EChannelConfiguration::Stereo,   { "", "" } },
            { EChannelConfiguration::Surround, { "", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EChannelConfiguration::Stereo,   { "STEREO", "" } },
            { EChannelConfiguration::Surround, { "SURROUND", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(VoiceLanguage)
{
    { ELanguage::English,  { "Voice Language", "Change the language used for character voices." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Stimmeinstellung", "Ändere die Sprache, die für Charakterstimmen benutzt wird." } },
    { ELanguage::French,   { "Langue de doublage", "Modifie la langue utilisée pour la voix des personnages." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian, { "Lingua delle voci", "Modifica la lingua utilizzata per le voci dei personaggi." } }
};

CONFIG_DEFINE_LOCALE(Subtitles)
{
    { ELanguage::English,  { "Subtitles", "Show subtitles during dialogue." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Untertitel", "Zeige Untertitel bei Dialogen." } },
    { ELanguage::French,   { "Sous-titres", "Affiche les sous-titres pendant les dialogues." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Sottotitoli", "Mostra i sottotitoli durante i dialoghi." } }
};

CONFIG_DEFINE_LOCALE(BattleTheme)
{
    { ELanguage::English,  { "Battle Theme", "Play the Werehog battle theme during combat.\n\nThis option will apply the next time you're in combat.\n\nExorcism missions and miniboss themes will be unaffected." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Kampfmusik", "Spiele die Kampfmusik des Werehogs während dem Kämpfen ab.\n\nDiese Option tritt das nächste Mal, wenn du in einen Kampf gerätst, in Kraft.\n\nExorzismen und Mini-Bosse werden hiervon nicht beeinflusst." } },
    { ELanguage::French,   { "Thème de combat", "Joue le thème de combat du Werehog pendant les combat.\n\nCette option s'appliquera la prochaine fois que vous serez en combat.\n\nLes missions d'exorcisme et les thèmes des miniboss ne seront pas affectés." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Musica di combattimento", "Riproduci la musica di combattimento del Werehog quando inizi una battaglia.\n\nQuesta opzione verrà applicata la prossima volta che sei in battaglia.\n\nLa traccia musicale verrà riprodotta ugualmente nelle missioni di Esorcismo e i miniboss." } }
};

CONFIG_DEFINE_LOCALE(WindowSize)
{
    { ELanguage::English,  { "Window Size", "Adjust the size of the game window in windowed mode." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Fenstergröße", "Ändere die Größe des Spielfensters im Fenstermodus." } },
    { ELanguage::French,   { "Taille de la fenêtre", "Modifie la taille de la fenêtre de jeu en mode fenêtré." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Dimensioni finestra", "Regola le dimensioni della finestra del gioco in modalità finestra." } }
};

CONFIG_DEFINE_LOCALE(Monitor)
{
    { ELanguage::English,  { "Monitor", "Change which monitor to display the game on." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Monitor", "Ändere auf welchem Monitor das Spiel angezeigt wird." } },
    { ELanguage::French,   { "Moniteur", "Change le moniteur sur lequel le jeu sera affiché." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Schermo", "Cambia lo schermo su cui visualizzare il gioco." } }
};

CONFIG_DEFINE_LOCALE(AspectRatio)
{
    { ELanguage::English,  { "Aspect Ratio", "Change the aspect ratio." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Seitenverhältnis", "Verändere das Seitenverhältnis." } },
    { ELanguage::French,   { "Format d'image", "Modifie le format d'image." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Rapporto d'aspetto", "Modifica il rapporto d'aspetto." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EAspectRatio)
{
    {
        ELanguage::English,
        {
            { EAspectRatio::Auto, { "AUTO", "Auto: the aspect ratio will dynamically adjust to the window size." } },
            { EAspectRatio::Wide, { "16:9", "16:9: locks the game to a widescreen aspect ratio." } },
            { EAspectRatio::Narrow, { "4:3", "4:3: locks the game to a narrow aspect ratio." } },
            { EAspectRatio::OriginalNarrow, { "ORIGINAL 4:3", "Original 4:3: locks the game to a narrow aspect ratio and retains parity with the game's original implementation." } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EAspectRatio::Auto, { "", "" } },
            { EAspectRatio::Wide, { "", "" } },
            { EAspectRatio::Narrow, { "", "" } },
            { EAspectRatio::OriginalNarrow, { "", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { EAspectRatio::Auto, { "AUTO", "Auto: Das Seitenverhältnis passt sich automatisch der Fenstergröße an." } },
            { EAspectRatio::Wide, { "16:9", "16:9: Stellt das Spiel in einem Breitbildschirm-Vormat dar." } },
            { EAspectRatio::Narrow, { "4:3", "4:3: Stellt das Spiel in einem Mittel-Vormat dar." } },
            { EAspectRatio::OriginalNarrow, { "ORIGINAL 4:3", "Original 4:3: Stellt das Spiel in einem Mittel-Vormat dar, was der ursprünglichen Implementation originalgetreut bleibt." } }
        }
    },
    {
        ELanguage::French,
        {
            { EAspectRatio::Auto, { "AUTO", "Auto: le format d'image s'adapte automatiquement à la taille de la fenêtre." } },
            { EAspectRatio::Wide, { "16:9", "16:9: force le jeu sur un format d'image large." } },
            { EAspectRatio::Narrow, { "4:3", "4:3: force le jeu sur un format d'image carré." } },
            { EAspectRatio::OriginalNarrow, { "4:3 ORIGINAL", "4:3 Original: force le jeu à un format d'image carré et conserve la parité avec l'implémentation d'origine du jeu." } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { EAspectRatio::Auto, { "", "" } },
            { EAspectRatio::Wide, { "", "" } },
            { EAspectRatio::Narrow, { "", "" } },
            { EAspectRatio::OriginalNarrow, { "", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EAspectRatio::Auto, { "AUTO", "Auto: il rapporto d'aspetto verra cambiato automaticamente in base alle dimensioni della finestra." } },
            { EAspectRatio::Wide, { "16:9", "16:9: blocca il gioco a un rapporto d'aspetto widescreen." } },
            { EAspectRatio::Narrow, { "4:3", "4:3: blocca il gioco a un rapporto d'aspetto stretto." } },
            { EAspectRatio::OriginalNarrow, { "4:3 ORIGINALE", "4:3 Originale: blocca il gioco a un rapporto d'aspetto stretto e mantiene la parità con l'implementazione originale del gioco." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(ResolutionScale)
{
    { ELanguage::English,  { "Resolution Scale", "Adjust the internal resolution of the game.\n\n%dx%d" } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Rendering-Auflösung", "Passe die Auflösung der internen Darstellung an.\n\n%dx%d" } },
    { ELanguage::French,   { "Échelle de rendu", "Modifie la résolution interne du jeu.\n\n%dx%d" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian, { "Scala risoluzione", "Regola la risoluzione interna del gioco.\n\n%dx%d" } }
};

CONFIG_DEFINE_LOCALE(Fullscreen)
{
    { ELanguage::English,  { "Fullscreen", "Toggle between borderless fullscreen or windowed mode." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Vollbild", "Wechsle zwischen dem randlosen Vollbildmodus und dem Fenstermodus." } },
    { ELanguage::French,   { "Plein écran", "Alterne entre le mode plein écran sans bordure et le mode fenêtré." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Schermo pieno", "Attiva/disattiva tra modalità finestra senza cornice e modalità finestra." } }
};

CONFIG_DEFINE_LOCALE(VSync)
{
    { ELanguage::English,  { "V-Sync", "Synchronize the game to the refresh rate of the display to prevent screen tearing." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "V-Sync", "Synchronisiere das Spiel mit der Bildwiederholrate deines Bildschirms um Bildverzerrungen zu vermeiden." } },
    { ELanguage::French,   { "V-Sync", "Synchronise le jeu avec la fréquence de rafraîchissement de l'écran pour éviter le screen tearing." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "V-Sync", "Sincronizza il gioco con la frequenza d'aggiornamento del display per evitare lo screen tearing." } }
};

CONFIG_DEFINE_LOCALE(FPS)
{
    { ELanguage::English,  { "FPS", "Set the max frame rate the game can run at.\n\nWARNING: this may introduce glitches at frame rates higher than 60 FPS." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "FPS", "Setze die maximale Anzahl der Bilder pro Sekunde, die das Spiel darstellen kann.\n\nWARNUNG: Das Spiel kann bei höheren FPS als 60 ungewolltes Verhalten aufweisen." } },
    { ELanguage::French,   { "FPS", "Détermine la fréquence d'images maximale du jeu.\n\nATTENTION: cela peut entraîner des problèmes à des taux de rafraîchissement supérieurs à 60 FPS." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "FPS", "Imposta il frame rate massimo del gioco.\n\nATTENZIONE: questa opzione può causare dei glitch a frame rate più alti di 60 FPS." } }
};

CONFIG_DEFINE_LOCALE(Brightness)
{
    { ELanguage::English,  { "Brightness", "Adjust the brightness level until the symbol on the left is barely visible." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Helligkeit", "Passe die Helligkeit des Spiels an bis das linke Symbol noch gerade so sichtbar ist." } },
    { ELanguage::French,   { "Luminosité", "Règle le niveau de luminosité jusqu'à ce que le symbole à gauche soit à peine visible." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Luminosità", "Regola la luminosità dello schermo fino a quando il simbolo a sinistra diventa leggermente visibile." } }
};

CONFIG_DEFINE_LOCALE(AntiAliasing)
{
    { ELanguage::English,  { "Anti-Aliasing", "Adjust the amount of smoothing applied to jagged edges." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Kantenglättung", "Passe die Menge an Kantenglättung an." } },
    { ELanguage::French,   { "Anticrénelage", "Ajuste le niveau d'anticrénelage appliqué aux bords des objets." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Anti-Aliasing", "Regola la quantità di smussamento applicata ai bordi." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EAntiAliasing)
{
    {
        ELanguage::English,
        {
            { EAntiAliasing::None, { "NONE", "" } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EAntiAliasing::None, { "", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { EAntiAliasing::None, { "KEINE", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EAntiAliasing::None, { "SANS", "" } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { EAntiAliasing::None, { "", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EAntiAliasing::None, { "NULLO", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(TransparencyAntiAliasing)
{
    { ELanguage::English,  { "Transparency Anti-Aliasing", "Apply anti-aliasing to alpha transparent textures." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Transparenz-Kantenglättung", "Wende Kantenglättung auf Alpha-Transparenz-Texturen an." } },
    { ELanguage::French,   { "Anticrénelage de transparence", "Applique l'anticrénelage sur les textures transparentes." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Anti-Aliasing su texture trasparenti", "Applica l'anti-aliasing alle texture trasparenti." } }
};

CONFIG_DEFINE_LOCALE(ShadowResolution)
{
    { ELanguage::English,  { "Shadow Resolution", "Set the resolution of real-time shadows." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Schattenauflösung", "Stelle die Auflösung der Echtzeit-Schatten ein." } },
    { ELanguage::French,   { "Résolution des ombres", "Défini la résolution des ombres en temps réel." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Risoluzione ombre", "Imposta la risoluzioni delle ombre in tempo reale." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EShadowResolution)
{
    {
        ELanguage::English,
        {
            { EShadowResolution::Original, { "ORIGINAL", "Original: the game will automatically determine the resolution of the shadows." } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EShadowResolution::Original, { "", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { EShadowResolution::Original, { "ORIGINAL", "Original: Das Spiel bestimmt die Auflösung der Schatten automatisch." } }
        }
    },
    {
        ELanguage::French,
        {
            { EShadowResolution::Original, { "ORIGINALE", "Originale: le jeu déterminera automatiquement la résolution des ombres." } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { EShadowResolution::Original, { "", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EShadowResolution::Original, { "ORIGINALE", "Originale: il gioco determinerà automaticamente la risoluzione delle ombre." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(GITextureFiltering)
{
    { ELanguage::English,  { "GI Texture Filtering", "Change the quality of the filtering used for global illumination textures." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "GI Texturfilterung", "Ändere die Qualität der Filterung für Global-Illumination-Texturen." } },
    { ELanguage::French,   { "Filtre des textures IG", "Modifie la qualité du filtre utilisé pour les textures d'illumination globale." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Filtraggio delle texture GI", "Regola la qualità del sistema di filtraggio utilizzato per le texture dell'illuminazione." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EGITextureFiltering)
{
    {
        ELanguage::English,
        {
            { EGITextureFiltering::Bilinear, { "BILINEAR", "" } },
            { EGITextureFiltering::Bicubic,  { "BICUBIC", "" } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EGITextureFiltering::Bilinear, { "", "" } },
            { EGITextureFiltering::Bicubic,  { "", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { EGITextureFiltering::Bilinear, { "BILINEAR", "" } },
            { EGITextureFiltering::Bicubic,  { "BICUBIC", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EGITextureFiltering::Bilinear, { "BILINÉAIRE", "" } },
            { EGITextureFiltering::Bicubic,  { "BICUBIQUE", "" } },
        }
    },
    {
        ELanguage::Spanish,
        {
            { EGITextureFiltering::Bilinear, { "", "" } },
            { EGITextureFiltering::Bicubic,  { "", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EGITextureFiltering::Bilinear, { "BILINEARE", "" } },
            { EGITextureFiltering::Bicubic,  { "BICUBICO", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(MotionBlur)
{
    { ELanguage::English,  { "Motion Blur", "Change the quality of the motion blur." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Bewegungsunschärfe", "Verändere die Qualität der Bewegungsunschärfe." } },
    { ELanguage::French,   { "Flou de mouvement", "Modifie la qualité du flou de mouvement." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Sfocatura di movimento", "Regola la qualità della sfocatura di movimento." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EMotionBlur)
{
    {
        ELanguage::English,
        {
            { EMotionBlur::Off,      { "OFF", "" } },
            { EMotionBlur::Original, { "ORIGINAL", "" } },
            { EMotionBlur::Enhanced, { "ENHANCED", "Enhanced: uses more samples for smoother motion blur at the cost of performance." } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EMotionBlur::Off,      { "", "" } },
            { EMotionBlur::Original, { "", "" } },
            { EMotionBlur::Enhanced, { "", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { EMotionBlur::Off,      { "AUS", "" } },
            { EMotionBlur::Original, { "ORIGINAL", "" } },
            { EMotionBlur::Enhanced, { "VERBESSERT", "Verbessert: Benutzt mehr Samples um eine weichere Bewegungsunschärfe auf Kosten von Performanz zu erzeugen." } }
        }
    },
    {
        ELanguage::French,
        {
            { EMotionBlur::Off,      { "AUCUN", "" } },
            { EMotionBlur::Original, { "ORIGINAL", "" } },
            { EMotionBlur::Enhanced, { "AMÉLIORÉ", "Amélioré: utilise plus d'échantillons pour un flou de mouvement plus lisse au détriment des performances." } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { EMotionBlur::Off,      { "", "" } },
            { EMotionBlur::Original, { "", "" } },
            { EMotionBlur::Enhanced, { "", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EMotionBlur::Off,      { "DISATTIVATO", "" } },
            { EMotionBlur::Original, { "ORIGINALE", "" } },
            { EMotionBlur::Enhanced, { "AUMENTATO", "Aumentato: usa più passaggi per una sfocatura più fluida, con un impatto sulle prestazioni." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(XboxColorCorrection)
{
    { ELanguage::English,  { "Xbox Color Correction", "Use the warm tint from the Xbox version of the game." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Xbox Farbkorrektur", "Benutzt den warmen Farbstich aus der Xbox-Version des Spiels." } },
    { ELanguage::French,   { "Correction couleurs Xbox", "Utilise le filtre de couleur provenant de la version Xbox du jeu." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Correzione dei colori Xbox", "Applica il filtro di colori più caldi utilizzato dalla versione Xbox del gioco." } }
};

CONFIG_DEFINE_LOCALE(CutsceneAspectRatio)
{
    { ELanguage::English,  { "Cutscene Aspect Ratio", "Change the aspect ratio of the real-time cutscenes." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Zwischensequenz-Seitenverhältnis", "Verändere das Seitenverhältnis der Echtzeit-Zwischensequenzen." } },
    { ELanguage::French,   { "Format des cinématiques", "Modifie le format d'image des cinématiques en temps réel." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Rapporto d'aspetto dei filmati", "Cambia il rapporto d'aspetto dei filmati in tempo reale." } }
};

CONFIG_DEFINE_ENUM_LOCALE(ECutsceneAspectRatio)
{
    {
        ELanguage::English,
        {
            { ECutsceneAspectRatio::Original, { "ORIGINAL", "Original: locks cutscenes to their original 16:9 aspect ratio." } },
            { ECutsceneAspectRatio::Unlocked, { "UNLOCKED", "Unlocked: allows cutscenes to adjust their aspect ratio to the window size.\n\nWARNING: this will introduce visual oddities past the original 16:9 aspect ratio." } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { ECutsceneAspectRatio::Original, { "", "" } },
            { ECutsceneAspectRatio::Unlocked, { "", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { ECutsceneAspectRatio::Original, { "ORIGINAL", "Original: Behält die Zwischensequenzen in ihrem originalen 16:9 Seitenverhältnis." } },
            { ECutsceneAspectRatio::Unlocked, { "ENTSPERRT", "Entsperrt: Erlaubt Zwischensequenzen ihr Seitenverhältnis der Fenstergröße anzupassen.\n\nWARNUNG: Diese Option kann zu visuellen Fehlern außerhalb der Grenzen des ursprünglichen Seitenverhältnisses führen." } }
        }
    },
    {
        ELanguage::French,
        {
            { ECutsceneAspectRatio::Original, { "ORIGINAL", "Original: force les cinématiques dans leur format 16:9 d'origine." } },
            { ECutsceneAspectRatio::Unlocked, { "LIBRE", "Libre: permet aux cinématiques d'adapter leur format d'image à la taille de la fenêtre.\n\nAttention: au dela du format 16:9 d'origine, des bugs visuels apparaitront." } },
        }
    },
    {
        ELanguage::Spanish,
        {
            { ECutsceneAspectRatio::Original, { "", "" } },
            { ECutsceneAspectRatio::Unlocked, { "", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { ECutsceneAspectRatio::Original, { "ORIGINALE", "Originale: blocca il rapporto d'aspetto dei filmati a 16:9." } },
            { ECutsceneAspectRatio::Unlocked, { "SBLOCCATO", "Sbloccato: il rapporto d'aspetto verrà regolato in base alle dimensioni della finestra.\n\nATTENZIONE: questa opzione potrebbe causare dei problemi visivi se il rapporto d'aspetto è oltre 16:9." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(UIAlignmentMode)
{
    { ELanguage::English,  { "UI Alignment Mode", "Change how the UI aligns with the display." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "Benutzeroberflächenausrichtung", "Verändere wie die Benutzeroberfläche sich mit dem Bildschirm ausrichtet." } },
    { ELanguage::French,   { "Alignement de l'IU", "Modifie l'alignement de l'interface utilisateur sur l'écran." } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Modalità allineamento interfaccia", "Modifica come l'interfaccia si allinea con lo schermo." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EUIAlignmentMode)
{
    {
        ELanguage::English,
        {
            { EUIAlignmentMode::Edge,    { "EDGE", "Edge: the UI will align with the edges of the display." } },
            { EUIAlignmentMode::Centre,  { "CENTER", "Center: the UI will align with the center of the display." } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { EUIAlignmentMode::Edge,    { "", "" } },
            { EUIAlignmentMode::Centre,  { "", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { EUIAlignmentMode::Edge,    { "RAND", "Rand: Die Benutzeroberfläche richtet sich zum Rand des Bildschirms aus." } },
            { EUIAlignmentMode::Centre,  { "MITTE", "Mitte: Die Benutzeroberfläche richtet sich zur Mitte des Bildschirms aus." } }
        }
    },
    {
        ELanguage::French,
        {
            { EUIAlignmentMode::Edge,    { "BORD", "Bord: l'interface utilisateur sera alignée sur les bords de l'écran." } },
            { EUIAlignmentMode::Centre,  { "CENTRÉE", "Centrée: l'interface utilisateur sera alignée sur le centre de l'écran." } },
        }
    },
    {
        ELanguage::Spanish,
        {
            { EUIAlignmentMode::Edge,    { "", "" } },
            { EUIAlignmentMode::Centre,  { "", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EUIAlignmentMode::Edge,    { "BORDI", "Bordi: l'interfaccia si allineerà con i bordi dello schermo." } },
            { EUIAlignmentMode::Centre,  { "CENTRO", "Centro: l'interfaccia si allineerà con il centro dello schermo." } }
        }
    }
};
