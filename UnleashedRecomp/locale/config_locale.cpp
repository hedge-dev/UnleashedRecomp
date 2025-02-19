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
    { ELanguage::German,   { "Sprache", "" } },
    { ELanguage::French,   { "Langue", "" } },
    { ELanguage::Spanish,  { "Idioma", "" } },
    { ELanguage::Italian,  { "Lingua", "" } }
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
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Indizi", "Mostra degli indizzi durante il gioco." } }
};

CONFIG_DEFINE_LOCALE(ControlTutorial)
{
    { ELanguage::English,  { "Control Tutorial", "Show controller hints during gameplay.\n\nThe Werehog Critical Attack prompt will be unaffected." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Tutorial dei comandi", "Mostra i tutorial dei comandi durante il gioco.\n\nIl tutorial per l'attacco critico del Werehog non verrà influenzato da questa opzione." } }
};

CONFIG_DEFINE_LOCALE(AchievementNotifications)
{
    { ELanguage::English,  { "Achievement Notifications", "Show notifications for unlocking achievements.\n\nAchievements will still be rewarded with notifications disabled." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Notifiche obiettivi", "Mostra delle notifiche quando sblocchi degli obiettivi.\n\nGli obiettivi verranno comunque assegnati anche con le notifiche disattivate." } }
};

CONFIG_DEFINE_LOCALE(TimeOfDayTransition)
{
    { ELanguage::English,  { "Time of Day Transition", "Change how the loading screen appears when switching time of day in the hub areas." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
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
            { ETimeOfDayTransition::Xbox,        { "", "" } },
            { ETimeOfDayTransition::PlayStation, { "", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { ETimeOfDayTransition::Xbox,        { "", "" } },
            { ETimeOfDayTransition::PlayStation, { "", "" } }
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
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
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
            { EControllerIcons::Auto,        { "", "" } },
            { EControllerIcons::Xbox,        { "", "" } },
            { EControllerIcons::PlayStation, { "", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EControllerIcons::Auto,        { "", "" } },
            { EControllerIcons::Xbox,        { "", "" } },
            { EControllerIcons::PlayStation, { "", "" } }
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
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Telecamera orizzontale", "Modifica come la telecamera si muove da sinistra a destra." } }
};

CONFIG_DEFINE_LOCALE(VerticalCamera)
{
    { ELanguage::English,  { "Vertical Camera", "Change how the camera moves up and down." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
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
            { ECameraRotationMode::Normal,  { "", "" } },
            { ECameraRotationMode::Reverse, { "", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { ECameraRotationMode::Normal,  { "", "" } },
            { ECameraRotationMode::Reverse, { "", "" } }
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
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Vibrazione", "Attiva/disattiva la vibrazione del controller." } }
};

CONFIG_DEFINE_LOCALE(AllowBackgroundInput)
{
    { ELanguage::English,  { "Allow Background Input", "Allow controller input whilst the game window is unfocused." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Input con la finestra inattiva", "Attiva/disattiva i tasti del controller mentre la finestra è inattiva." } }
};

CONFIG_DEFINE_LOCALE(MasterVolume)
{
    { ELanguage::English,  { "Master Volume", "Adjust the overall volume." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Volume principale", "Regola il volume principale" } }
};

CONFIG_DEFINE_LOCALE(MusicVolume)
{
    { ELanguage::English,  { "Music Volume", "Adjust the volume for the music." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Volume musica di sottofondo", "Regola il volume della musica di sottofondo." } }
};

CONFIG_DEFINE_LOCALE(EffectsVolume)
{
    { ELanguage::English,  { "Effects Volume", "Adjust the volume for sound effects." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Volume effetti sonori", "Regola il volume degli effetti sonori." } }
};

CONFIG_DEFINE_LOCALE(MusicAttenuation)
{
    { ELanguage::English,  { "Music Attenuation", "Fade out the game's music when external media is playing." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Attenuazione musica", "Abbassa il volume della musica di sottofondo quando un'altra applicazione riproduce dei suoni." } }
};

CONFIG_DEFINE_LOCALE(ChannelConfiguration)
{
    { ELanguage::English,  { "Channel Configuration", "Change the output mode for your audio device." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
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
            { EChannelConfiguration::Stereo,   { "", "" } },
            { EChannelConfiguration::Surround, { "", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EChannelConfiguration::Stereo,   { "", "" } },
            { EChannelConfiguration::Surround, { "", "" } }
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
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian, { "Lingua delle voci", "Modifica la lingua utilizzata per le voci dei personaggi." } }
};

CONFIG_DEFINE_LOCALE(Subtitles)
{
    { ELanguage::English,  { "Subtitles", "Show subtitles during dialogue." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Sottotitoli", "Mostra i sottotitoli durante i dialoghi." } }
};

CONFIG_DEFINE_LOCALE(BattleTheme)
{
    { ELanguage::English,  { "Battle Theme", "Play the Werehog battle theme during combat.\n\nThis option will apply the next time you're in combat.\n\nExorcism missions and miniboss themes will be unaffected." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Musica di combattimento", "Riproduci la musica di combattimento del Werehog quando inizi una battaglia.\n\nQuesta opzione verrà applicata la prossima volta che sei in battaglia.\n\nLa traccia musicale verrà riprodotta ugualmente nelle missioni di Esorcismo e i miniboss." } }
};

CONFIG_DEFINE_LOCALE(WindowSize)
{
    { ELanguage::English,  { "Window Size", "Adjust the size of the game window in windowed mode." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Dimensioni finestra", "Regola le dimensioni della finestra del gioco in modalità finestra." } }
};

CONFIG_DEFINE_LOCALE(Monitor)
{
    { ELanguage::English,  { "Monitor", "Change which monitor to display the game on." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Schermo", "Cambia lo schermo su cui visualizzare il gioco." } }
};

CONFIG_DEFINE_LOCALE(AspectRatio)
{
    { ELanguage::English,  { "Aspect Ratio", "Change the aspect ratio." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
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
            { EAspectRatio::Auto, { "", "" } },
            { EAspectRatio::Wide, { "", "" } },
            { EAspectRatio::Narrow, { "", "" } },
            { EAspectRatio::OriginalNarrow, { "", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EAspectRatio::Auto, { "", "" } },
            { EAspectRatio::Wide, { "", "" } },
            { EAspectRatio::Narrow, { "", "" } },
            { EAspectRatio::OriginalNarrow, { "", "" } }
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
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian, { "Scala risoluzione", "Regola la risoluzione interna del gioco.\n\n%dx%d" } }
};

CONFIG_DEFINE_LOCALE(Fullscreen)
{
    { ELanguage::English,  { "Fullscreen", "Toggle between borderless fullscreen or windowed mode." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Schermo pieno", "Attiva/disattiva tra modalità finestra senza cornice e modalità finestra." } }
};

CONFIG_DEFINE_LOCALE(VSync)
{
    { ELanguage::English,  { "V-Sync", "Synchronize the game to the refresh rate of the display to prevent screen tearing." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "V-Sync", "Sincronizza il gioco con la frequenza d'aggiornamento del display per evitare lo screen tearing." } }
};

CONFIG_DEFINE_LOCALE(FPS)
{
    { ELanguage::English,  { "FPS", "Set the max frame rate the game can run at.\n\nWARNING: this may introduce glitches at frame rates higher than 60 FPS." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "FPS", "Imposta il frame rate massimo del gioco.\n\nATTENZIONE: questa opzione può causare dei glitch a frame rate più alti di 60 FPS." } }
};

CONFIG_DEFINE_LOCALE(Brightness)
{
    { ELanguage::English,  { "Brightness", "Adjust the brightness level until the symbol on the left is barely visible." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Luminosità", "Regola la luminosità dello schermo fino a quando il simbolo a sinistra diventa leggermente visibile." } }
};

CONFIG_DEFINE_LOCALE(AntiAliasing)
{
    { ELanguage::English,  { "Anti-Aliasing", "Adjust the amount of smoothing applied to jagged edges." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
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
            { EAntiAliasing::None, { "", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EAntiAliasing::None, { "", "" } }
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
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Anti-Aliasing su texture trasparenti", "Applica l'anti-aliasing alle texture trasparenti." } }
};

CONFIG_DEFINE_LOCALE(ShadowResolution)
{
    { ELanguage::English,  { "Shadow Resolution", "Set the resolution of real-time shadows." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
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
            { EShadowResolution::Original, { "", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EShadowResolution::Original, { "", "" } }
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
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
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
            { EGITextureFiltering::Bilinear, { "", "" } },
            { EGITextureFiltering::Bicubic,  { "", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EGITextureFiltering::Bilinear, { "", "" } },
            { EGITextureFiltering::Bicubic,  { "", "" } }
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
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
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
            { EMotionBlur::Off,      { "", "" } },
            { EMotionBlur::Original, { "", "" } },
            { EMotionBlur::Enhanced, { "", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EMotionBlur::Off,      { "", "" } },
            { EMotionBlur::Original, { "", "" } },
            { EMotionBlur::Enhanced, { "", "" } }
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
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "", "" } },
    { ELanguage::Italian,  { "Correzione dei colori Xbox", "Applica il filtro di colori più caldi utilizzato dalla versione Xbox del gioco." } }
};

CONFIG_DEFINE_LOCALE(CutsceneAspectRatio)
{
    { ELanguage::English,  { "Cutscene Aspect Ratio", "Change the aspect ratio of the real-time cutscenes." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
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
            { ECutsceneAspectRatio::Original, { "", "" } },
            { ECutsceneAspectRatio::Unlocked, { "", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { ECutsceneAspectRatio::Original, { "", "" } },
            { ECutsceneAspectRatio::Unlocked, { "", "" } }
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
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
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
            { EUIAlignmentMode::Edge,    { "", "" } },
            { EUIAlignmentMode::Centre,  { "", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { EUIAlignmentMode::Edge,    { "", "" } },
            { EUIAlignmentMode::Centre,  { "", "" } }
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
