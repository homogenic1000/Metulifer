# Metulifer

Plugin audio JUCE (VST3 / AU / Standalone) créé avec le Projucer pour un cours — fabricant **Saumure**.

> **Metulifer** is a digital sequencer with 6 different programmable sequences. Each sequence has its own synth, based on 2 VCOs, 2 ADSRs, and a filter.

> **Metulifer** is a digital sequencer with 6 different programmable sequences. Each sequence has its own synth, based on 2 VCOs, 2 ADSRs, and a filter.

## Structure du projet

```
Metulifer/
├── Metulifer.jucer           # Fichier projet du Projucer (format XML)
├── Source/                   # Ton code (le seul dossier à modifier)
│   ├── PluginProcessor.h/.cpp  # Le "cerveau" du plugin (AudioProcessor)
│   ├── PluginEditor.h/.cpp     # L'interface graphique (AudioProcessorEditor)
│   └── Main.cpp                # Placeholder (inutilisé pour un plugin)
├── JuceLibraryCode/          # Généré automatiquement — NE PAS MODIFIER
│   ├── JuceHeader.h          #     (écrasé à chaque sauvegarde du Projucer)
│   └── include_juce_*.cpp/mm #     includes des modules JUCE
├── Builds/                   # Projets natifs générés (Xcode, etc.)
│   └── MacOSX/               #     Metulifer.xcodeproj + plists (AU/VST3/Standalone)
├── .gitignore
└── README.md
```

## Rôle des fichiers

### Metulifer.jucer
C'est la source de vérité du projet. Le Projucer l'ouvre (`.jucer`) et y régénère les fichiers de build (`Builds/`) et du code JUCE (`JuceLibraryCode/`). On y configure : type de projet (`audioplug`), nom du plugin (`pluginName="Metulifer"`), modules activés, targets d'export, options (`JUCE_*`), deployment target.

> **Règle d'or** : ne modifie jamais `JuceLibraryCode/` ni les projets dans `Builds/` à la main. Tout changement se passe dans le `.jucer`, puis *File → Save Project*.

### Source/PluginProcessor (le modèle + le controller)
`MetuliferAudioProcessor` hérite de `juce::AudioProcessor`. C'est le cœur du plugin :
- déclare les **bus audio** (stéréo in/out),
- effectue le **traitement audio** dans `processBlock()` (appelé en continu, thread temps réel — jamais de GUI, jamais d'I/O),
- l'initialisation DSP se fait dans `prepareToPlay()` / `releaseResources()`,
- gère la **persistance de l'état** via `getStateInformation()` / `setStateInformation()`,
- instancie son éditeur graphique dans `createEditor()`,
- expose le nom affiché par le DAW via `getName()` (retourne `JucePlugin_Name` = `"Metulifer"`).

### Source/PluginEditor (la vue)
`MetuliferAudioProcessorEditor` hérite de `juce::AudioProcessorEditor`. C'est l'UI du plugin :
- dessine dans `paint()`,
- positionne les composants dans `resized()`,
- garde une **référence vers le processor** (`audioProcessor`) pour lire/écrire ses paramètres.

## Le MVC dans JUCE

JUCE n'impose pas MVC, mais le couple Processor/Editor s'y mappe naturellement :

| Concept MVC | Équivalent JUCE |
|---|---|
| **Modèle** | `AudioProcessor` (DSP, paramètres, état sauvegardé) |
| **Vue** | `AudioProcessorEditor` + ses `Component`s (sliders, knobs, boutons) |
| **Controller** | Les *listeners* / *attachments* : `Slider::Listener`, `Button::Listener`, `AudioProcessorValueTreeState::Listener`, `ValueTree::Listener` |

### Où mettre les paramètres (le pattern recommandé)
Le « vrai » modèle, c'est souvent un `AudioProcessorValueTreeState` (APVTS) **vivant dans le Processor** :
- le Processor expose un `SliderAttachment` / `ComboBoxAttachment` par paramètre dans l'Editor,
- le DSP et l'UI observent le **même** ValueTree → on tourne un slider, le processeur est notifié automatiquement (pattern *Observer*),
- la sauvegarde/chargement de l'état se fait en sérialisant ce ValueTree dans `getStateInformation` / `setStateInformation`.

### Les deux threads à retenir
- **Thread audio** : `processBlock()` — temps réel, pas d'allocations, pas de locking sur le GUI.
- **Thread GUI** : tout le reste (Editor, sliders).
- La communication entre les deux passe par l'APVTS / les paramètres atomiques (`AudioParameterFloat`…), jamais par des appels directs à l'UI depuis le DSP.

## Prérequis

- **JUCE** : doit être installé sur la machine. Le Projucer cherche les modules par défaut dans `~/JUCE/modules` (ici via un lien `~/JUCE → ~/Downloads/JUCE`).
- **Xcode** : l'édition du code se fait dans **Zed**, mais la compilation passe par Xcode (ou `xcodebuild`).
- Le projet est réglé sur `macOSDeploymentTarget = 12.0` (minimum requis par le SDK installé ; le défaut JUCE `10.13` fait échouer le build).

## Construire avec Xcode

1. Ouvrir `Builds/MacOSX/Metulifer.xcodeproj` dans Xcode.
2. Choisir le scheme voulu : **`Metulifer - Standalone Plugin`** (app de test), `Metulifer - VST3`, `Metulifer - AU`, ou `Metulifer - All`.
3. ⌘R pour builder/lancer.

Les binaires sont générés dans `Builds/MacOSX/build/Debug/` :
- `Metulifer.app` — app standalone (à lancer pour tester),
- `Metulifer.vst3` — plugin VST3,
- `Metulifer.component` — composant AU.

> Alternatives depuis un terminal :
> - Régénérer le projet après un changement de `.jucer` : `Projucer --resave Metulifer.jucer`
> - Builder : `xcodebuild -project Builds/MacOSX/Metulifer.xcodeproj -scheme "Metulifer - Standalone Plugin" -configuration Debug build`
> - Si `xcodebuild` n'est pas trouvé : `xcode-select -s /Applications/Xcode.app`

> **Dépannage « erreur de build »** : les causes classiques sont (1) `MACOSX_DEPLOYMENT_TARGET` trop vieux → remonter à `12.0` dans le `.jucer` ; (2) modules manquants dans `<MODULES>` du `.jucer` → les cocher dans le Projucer ; (3) chemin de JUCE introuvable → vérifier `~/JUCE/modules`.

## Déploiement des binaires dans un DAW

Pour utiliser le plugin Release dans un DAW, copier vers les dossiers système :
- `Metulifer.vst3` → `~/Library/Audio/Plug-Ins/VST3/`
- `Metulifer.component` → `~/Library/Audio/Plug-Ins/Components/`

puis redémarrer le DAW (ou vider son cache de plugins).
