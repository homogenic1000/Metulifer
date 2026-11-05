# Metulifer

Plugin audio professionnel JUCE (VST3 / AU / Standalone) — Fabricant **Saumure**.

> **Metulifer** est un séquenceur numérique modulaire à **6 séquences polyrythmiques indépendantes**. Chaque séquence dispose de son propre synthétiseur double oscillateur (2 VCOs polyBLEP, 2 ADSRs exponentielles, filtre VCF TPT d'état variable, processeur Multi-FX 3-en-1 et arpégiateur 11 modes).

---

## Fonctionnalités principales

- **Séquenceur 6 pistes indépendantes** :
  - Polyrythmie intégrale : chaque piste dispose de sa propre longueur (1 à 32 pas) et de son volume.
  - **Mode Pinceau glissé (Click-and-Drag Paintbrush)** : activation/désactivation continue de pas en balayant la rangée à la souris.
  - Presse-papiers de motifs rapide avec copie (`C`) et collage (`P`) inter-séquences et alertes à l'écran.
- **Moteur de Synthèse Double VCO par voix** :
  - Formes d'onde Sine, Saw, Square et Triangle avec anti-repliement **polyBLEP (Polynomial Bandlimited Step)**.
  - Deux générateurs d'enveloppe `juce::ADSR` à décroissance exponentielle naturelle.
  - Filtre `StateVariableTPTFilter` 12 dB/octave commutable Low-Pass / High-Pass avec balayage logarithmique 20 Hz – 20 kHz.
- **Arpégiateur mélodique par séquence** :
  - Bouton **ARP** direct sur chaque rangée et panneau de contrôle dédié.
  - **11 modes algorithmiques** : Up, Down, Up&Down, Down&Up, Pinky Up, Pinky Down, Thumb-Pinky, Converge, Diverge, Random, Order.
  - **10 gammes et accords** : Major, Minor, Pentatonic, Octaves, Maj7, Min7, Dom7, Sus4, Fifths, Chromatic.
  - Extension d'octave sur 1, 2 ou 3 octaves.
- **Processeur Multi-FX 3-en-1** :
  - **Reverb Stéréo** : modes Room et Hall avec mise en cache dirty-check des coefficients.
  - **Delay Stéréo Ping-Pong** : vraie rétroaction croisée (*Cross-Feedback*) avec saturation non-linéaire douce par tangente hyperbolique (`std::tanh`).
  - **Redux** : bitcrusher (2 à 16 bits) et décimateur temporel (taux 1 à 32).
- **Modulation & Câblage Virtuel (LFOs)** :
  - 2 LFOs indépendants à formes d'onde multiples et sortie jack virtuelle.
  - Câblage direct en glisser-déposer vers n'importe quel paramètre modulable.
  - Écrans OLED LFO affichant en direct les destinations cibles connectées (ex. `SEQ 1 CUTOFF`).
  - **Potentiomètres animés en temps réel** : bague de modulation bicolore et aiguille pivotante suivant l'oscillation LFO.
- **Écran OLED & Monitoring Studio 60 Hz** :
  - **Oscilloscope temps réel** avec déclencheur de front montant (*zero-crossing trigger*) et calibrage automatique de période sur 2 cycles stables.
  - **dB-mètre stéréo de studio** calibré en dBFS (-48 à +3 dB) avec attaque instantanée, Peak-Hold 1 s, voyant CLIP et lecture numérique au dixième de dB.
- **Moteur 3D Skinning & Performance** :
  - Substitution automatique sans casse par des rendus 3D Blender (filmstrips de potentiomètres, boutons multi-états, calque de verre d'écran, leviers d'octave).
  - **Voice Sleep & Tail Management** : extinction automatique des voix inactives (< 5% CPU sous charge, 0.0% au repos).
  - Negative caching en mémoire : zéro appel système disque dans les boucles de rendu graphique.
  - Accélération matérielle macOS (`setOpaque(true)`) : déplacement de fenêtre fluide à 120 FPS via composition GPU native.
  - Inspecteur de composants visuel intégré (**Melatonin Inspector**, raccourci `⌘I`).

---

## Documentation Complète

Le projet inclut un site de documentation complet propulsé par **Fumadocs** (bilingue Français / Anglais) dans le dossier `docs/` :

```bash
cd docs
pnpm install
pnpm dev
```
Accessible sur `http://localhost:3000`.

- **Guides Utilisateur** : Interface, Séquenceur, Synthétiseur, Filtre, Multi-FX, Arpégiateur, Modulations LFO.
- **Architecture Développeur** : Traitement du signal DSP, État atomique du séquenceur, Paramètres APVTS, Moteur d'habillage Skinning 3D, Suite de tests automatisés.

---

## Structure du projet

```
Metulifer/
├── Metulifer.jucer           # Fichier projet Projucer (source de vérité)
├── Source/                   # Code source du plugin C++20 / JUCE
│   ├── PluginProcessor.h/.cpp  # AudioProcessor (DSP, APVTS, horloge, LFO)
│   ├── PluginEditor.h/.cpp     # AudioProcessorEditor (UI principale, 1280x720)
│   ├── DSP/                    # Moteur de voix polyphonique (Voice.h/.cpp)
│   ├── Sequencer/              # Moteur d'événements rythmiques (SequencerEngine)
│   ├── UI/                     # Composants graphiques, LookAndFeel, Skin, LFO, CableLayer
│   └── Tests/                  # Tests unitaires et d'intégration
├── docs/                     # Documentation Fumadocs / Next.js (FR & EN)
├── Resources/                # Assets graphiques et rendus 3D
├── AGENTS.md                 # Protocole de collaboration et journal d'ingénierie
└── README.md
```

---

## Compilation & Validation

### Prérequis
- macOS 12.0 ou ultérieur.
- Xcode 14+ (avec SDK macOS).
- JUCE 7+ / 8+ / 9+.

### Compilation (Debug et Release)
```bash
# Compilation de toutes les cibles (VST3, AU, Standalone)
DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer xcodebuild -project Builds/MacOSX/Metulifer.xcodeproj -scheme "Metulifer - All" -configuration Debug build
DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer xcodebuild -project Builds/MacOSX/Metulifer.xcodeproj -scheme "Metulifer - All" -configuration Release build
```

### Validation automatisée pluginval
```bash
/Applications/pluginval.app/Contents/MacOS/pluginval --strictness-level 5 --validate-in-process --validate "~/Library/Audio/Plug-Ins/VST3/Metulifer.vst3"
/Applications/pluginval.app/Contents/MacOS/pluginval --strictness-level 5 --validate-in-process --validate "~/Library/Audio/Plug-Ins/Components/Metulifer.component"
```
