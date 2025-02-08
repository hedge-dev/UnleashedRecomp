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
    { ELanguage::Spanish,  { "Idioma", "Cambia el idioma utilizado para el texto y los logos." } },
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
    { ELanguage::Spanish,  { "Pistas", "Muestra pistas durante el juego." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(ControlTutorial)
{
    { ELanguage::English,  { "Control Tutorial", "Show controller hints during gameplay.\n\nThe Werehog Critical Attack prompt will be unaffected." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Tutorial de controles", "Muestra pistas de controles durante el juego.\n\nEl indicador de ataque crítico del Werehog no se verá afectado." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(AchievementNotifications)
{
    { ELanguage::English,  { "Achievement Notifications", "Show notifications for unlocking achievements.\n\nAchievements will still be rewarded with notifications disabled." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Notificaciones de logros", "Muestra notificaciones al desbloquear logros.\n\nLos logros se seguirán obteniendo aunque las notificaciones estén desactivadas." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(TimeOfDayTransition)
{
    { ELanguage::English,  { "Time of Day Transition", "Change how the loading screen appears when switching time of day in the hub areas." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Transición de hora del día", "Cambia cómo aparece la pantalla de carga al cambiar la hora del día en las áreas principales." } },
    { ELanguage::Italian,  { "", "" } }
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
            { ETimeOfDayTransition::Xbox,        { "XBOX", "Xbox: la cinemática de transformación se reproducirá con tiempos de carga artificiales." } },
            { ETimeOfDayTransition::PlayStation, { "PLAYSTATION", "PlayStation: se usará una pantalla de carga con una medalla giratoria." } }
        }
    },
    {
        ELanguage::Italian,
        {
            { ETimeOfDayTransition::Xbox,        { "", "" } },
            { ETimeOfDayTransition::PlayStation, { "", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(ControllerIcons)
{
    { ELanguage::English,  { "Controller Icons", "Change the icons to match your controller." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Iconos del mando", "Cambia los iconos para que coincidan con tu mando." } },
    { ELanguage::Italian,  { "", "" } }
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
            { EControllerIcons::Auto,        { "AUTO", "Auto: el juego determinará de forma automática qué iconos utilizar dependiendo del dispositivo de entrada actual." } },
            { EControllerIcons::Xbox,        { "XBOX", "" } },
            { EControllerIcons::PlayStation, { "PLAYSTATION", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EControllerIcons::Auto,        { "", "" } },
            { EControllerIcons::Xbox,        { "", "" } },
            { EControllerIcons::PlayStation, { "", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(HorizontalCamera)
{
    { ELanguage::English,  { "Horizontal Camera", "Change how the camera moves left and right." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Cámara horizontal", "Cambia cómo se mueve la camara hacia la izquierda y la derecha." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(VerticalCamera)
{
    { ELanguage::English,  { "Vertical Camera", "Change how the camera moves up and down." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish, { "Cámara vertical", "Cambia cómo se mueve la camara hacia arriba y abajo." } },
    { ELanguage::Italian,  { "", "" } }
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
            { ECameraRotationMode::Normal,  { "NORMAL", "" } },
            { ECameraRotationMode::Reverse, { "INVERTIDO", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { ECameraRotationMode::Normal,  { "", "" } },
            { ECameraRotationMode::Reverse, { "", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(Vibration)
{
    { ELanguage::English,  { "Vibration", "Toggle controller vibration." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Vibración", "Activa o desactiva la vibración del mando." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(AllowBackgroundInput)
{
    { ELanguage::English,  { "Allow Background Input", "Allow controller input whilst the game window is unfocused." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Control en segundo plano", "Permite controlar el juego con un mando mientras la ventana esté en segundo plano." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(MasterVolume)
{
    { ELanguage::English,  { "Master Volume", "Adjust the overall volume." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Volumen maestro", "Ajusta el volumen general." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(MusicVolume)
{
    { ELanguage::English,  { "Music Volume", "Adjust the volume for the music." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Volumen de la música", "Ajusta el volumen de la música." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(EffectsVolume)
{
    { ELanguage::English,  { "Effects Volume", "Adjust the volume for sound effects." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Volumen de efectos", "Ajusta el volumen de los efectos de sonido." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(MusicAttenuation)
{
    { ELanguage::English,  { "Music Attenuation", "Fade out the game's music when external media is playing." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Atenuación de música", "Atenúa la música del juego cuando un reproductor multimedia se encuentra activo." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(ChannelConfiguration)
{
    { ELanguage::English,  { "Channel Configuration", "Change the output mode for your audio device." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Configuración de canales", "Cambia el modo de salida para tu dispositivo de audio." } },
    { ELanguage::Italian,  { "", "" } }
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
            { EChannelConfiguration::Stereo,   { "ESTÉREO", "" } },
            { EChannelConfiguration::Surround, { "ENVOLVENTE", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EChannelConfiguration::Stereo,   { "", "" } },
            { EChannelConfiguration::Surround, { "", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(VoiceLanguage)
{
    { ELanguage::English,  { "Voice Language", "Change the language used for character voices." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Idioma de voz", "Cambia el idioma utilizado para las voces de los personajes." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(Subtitles)
{
    { ELanguage::English,  { "Subtitles", "Show subtitles during dialogue." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Subtítulos", "Muestra subtítulos durante los diálogos." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(BattleTheme)
{
    { ELanguage::English,  { "Battle Theme", "Play the Werehog battle theme during combat.\n\nThis option will apply the next time you're in combat.\n\nExorcism missions and miniboss themes will be unaffected." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Tema de batalla", "Reproduce el tema de batalla del Werehog durante el combate.\n\nEsta opción se aplicará la próxima vez que entres en combate.\n\nLas misiones de exorcismo y los temas de los minijefes no se verán afectados." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(WindowSize)
{
    { ELanguage::English,  { "Window Size", "Adjust the size of the game window in windowed mode." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Tamaño de ventana", "Ajusta el tamaño de la ventana de juego." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(Monitor)
{
    { ELanguage::English,  { "Monitor", "Change which monitor to display the game on." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Pantalla", "Cambia la pantalla en la cuál se muestra el juego." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(AspectRatio)
{
    { ELanguage::English,  { "Aspect Ratio", "Change the aspect ratio." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Relación de aspecto", "Cambia la relación de aspecto." } },
    { ELanguage::Italian,  { "", "" } }
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
            { EAspectRatio::Auto, { "AUTO", "Auto: la relación de aspecto se ajusta de forma dinámica al tamaño de la ventana." } },
            { EAspectRatio::Wide, { "16:9", "16:9: muestra el juego en relación de aspecto de pantalla ancha." } },
            { EAspectRatio::Narrow, { "4:3", "4:3: muestra el juego en relación de aspecto de pantalla estrecha." } },
            { EAspectRatio::OriginalNarrow, { "ORIGINAL 4:3", "Original 4:3: muestra el juego en relación de aspecto de pantalla estrecha y mantiene paridad con la implementación original del juego." } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EAspectRatio::Auto, { "", "" } },
            { EAspectRatio::Wide, { "", "" } },
            { EAspectRatio::Narrow, { "", "" } },
            { EAspectRatio::OriginalNarrow, { "", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(ResolutionScale)
{
    { ELanguage::English,  { "Resolution Scale", "Adjust the internal resolution of the game.\n\n%dx%d" } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Escala de resolución", "Ajusta la resolución interna del juego.\n\n%dx%d" } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(Fullscreen)
{
    { ELanguage::English,  { "Fullscreen", "Toggle between borderless fullscreen or windowed mode." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Pantalla completa", "Cambia entre modo de pantalla completa o ventana." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(VSync)
{
    { ELanguage::English,  { "V-Sync", "Synchronize the game to the refresh rate of the display to prevent screen tearing." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "V-Sync", "Sincroniza el juego a la tasa de refresco de la pantalla para prevenir el rasgado de la imagen." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(FPS)
{
    { ELanguage::English,  { "FPS", "Set the max frame rate the game can run at.\n\nWARNING: this may introduce glitches at frame rates higher than 60 FPS." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "FPS", "Establece la tasa de fotogramas máxima a la que puede correr el juego.\n\nADVERTENCIA: esto puede introducir fallos en tasas mayores a 60 FPS." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(Brightness)
{
    { ELanguage::English,  { "Brightness", "Adjust the brightness level until the symbol on the left is barely visible." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Brillo", "Ajusta el nivel de brillo hasta que el símbolo a la izquierda sea apenas visible." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(AntiAliasing)
{
    { ELanguage::English,  { "Anti-Aliasing", "Adjust the amount of smoothing applied to jagged edges." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Anti-Aliasing", "Ajusta el nivel de suavizado aplicado a los dientes de sierra." } },
    { ELanguage::Italian,  { "", "" } }
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
            { EAntiAliasing::None, { "NO", "" } },
        }
    },
    {
        ELanguage::Italian,
        {
            { EAntiAliasing::None, { "", "" } }
        }
    },
};

CONFIG_DEFINE_LOCALE(TransparencyAntiAliasing)
{
    { ELanguage::English,  { "Transparency Anti-Aliasing", "Apply anti-aliasing to alpha transparent textures." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Anti-Aliasing de transparencias", "Aplica antialiasing a las texturas transparentes." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(ShadowResolution)
{
    { ELanguage::English,  { "Shadow Resolution", "Set the resolution of real-time shadows." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Resolución de sombras", "Establece la resolución de las sombras de tiempo real." } },
    { ELanguage::Italian,  { "", "" } }
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
            { EShadowResolution::Original, { "ORIGINAL", "Original: el juego determinará de forma automática la resolución de las sombras." } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EShadowResolution::Original, { "", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(GITextureFiltering)
{
    { ELanguage::English,  { "GI Texture Filtering", "Change the quality of the filtering used for global illumination textures." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Filtrado de texturas de GI", "Cambia la calidad del filtrado utilizado en las texturas de iluminación global." } },
    { ELanguage::Italian,  { "", "" } }
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
            { EGITextureFiltering::Bilinear, { "BILINEAR", "" } },
            { EGITextureFiltering::Bicubic,  { "BICÚBICO", "" } },
        }
    },
    {
        ELanguage::Italian,
        {
            { EGITextureFiltering::Bilinear, { "", "" } },
            { EGITextureFiltering::Bicubic,  { "", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(MotionBlur)
{
    { ELanguage::English,  { "Motion Blur", "Change the quality of the motion blur." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Desenfoque de movimiento", "Cambia la calidad del desenfoque de movimiento." } },
    { ELanguage::Italian,  { "", "" } }
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
            { EMotionBlur::Off,      { "APAGADO", "" } },
            { EMotionBlur::Original, { "ORIGINAL", "" } },
            { EMotionBlur::Enhanced, { "MEJORADO", "Mejorado: utiliza más muestras para suavizar el desenfoque de movimiento a costa del rendimiento." } }
        }
    },
    {
        ELanguage::Italian,
        {
            { EMotionBlur::Off,      { "", "" } },
            { EMotionBlur::Original, { "", "" } },
            { EMotionBlur::Enhanced, { "", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(XboxColorCorrection)
{
    { ELanguage::English,  { "Xbox Color Correction", "Use the warm tint from the Xbox version of the game." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Corrección de color de Xbox", "Utiliza el tono cálido de la versión Xbox del juego." } },
    { ELanguage::Italian,  { "", "" } }
};

CONFIG_DEFINE_LOCALE(CutsceneAspectRatio)
{
    { ELanguage::English,  { "Cutscene Aspect Ratio", "Change the aspect ratio of the real-time cutscenes." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Relación de aspecto de cinemáticas", "Cambia la relación de aspecto de las cinemáticas de tiempo real." } },
    { ELanguage::Italian,  { "", "" } }
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
            { ECutsceneAspectRatio::Original, { "ORIGINAL", "Original: muestra las cinemáticas en su relación de aspecto original de 16:9." } },
            { ECutsceneAspectRatio::Unlocked, { "DESBLOQUEADO", "Desbloqueado: permite que las cinemáticas ajusten su relación de aspecto al tamaño de la ventana.\n\nADVERTENCIA: esto introducirá rarezas visuales más allá de la relación de aspecto original de 16:9." } },
        }
    },
    {
        ELanguage::Italian,
        {
            { ECutsceneAspectRatio::Original, { "", "" } },
            { ECutsceneAspectRatio::Unlocked, { "", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(UIAlignmentMode)
{
    { ELanguage::English,  { "UI Alignment Mode", "Change how the UI aligns with the display." } },
    { ELanguage::Japanese, { "", "" } },
    { ELanguage::German,   { "", "" } },
    { ELanguage::French,   { "", "" } },
    { ELanguage::Spanish,  { "Modo de alineamiento de UI", "Cambia la alineación de la interfaz de usuario con la pantalla." } },
    { ELanguage::Italian,  { "", "" } }
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
            { EUIAlignmentMode::Edge,    { "BORDE", "Borde: la interfaz de usuario se alineará con los bordes de la pantalla." } },
            { EUIAlignmentMode::Centre,  { "CENTRO", "Centro: la interfaz de usuario se alineará con el centro de la pantalla." } },
        }
    },
    {
        ELanguage::Italian,
        {
            { EUIAlignmentMode::Edge,    { "", "" } },
            { EUIAlignmentMode::Centre,  { "", "" } }
        }
    }
};
