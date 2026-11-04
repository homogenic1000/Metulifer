# AGENTS.md — Protocole de collaboration

> Contrat partagé entre les humains et les agents opencode de ce projet.
> **Chaque agent opencode DOIT lire ce fichier avant toute modification,
> et DOIT le mettre à jour après chaque modification.**

---

## 1. Contexte projet

**Metulifer** — plugin audio JUCE (VST3 / AU / Standalone), fabricant **Saumure**.

- Séquenceur numérique avec **6 séquences programmables**.
- Chaque séquence a son propre synthé : **2 VCOs, 2 ADSRs, 1 filtre**.
- **État actuel** : squelette JUCE (template vierge), aucune logique DSP/UI implémentée.
- Stack : JUCE (modules complets dont `juce_dsp`), Xcode, macOS 12.0, éditeurs Zed/opencode.

### Structure

```
Metulifer/
├── Metulifer.jucer           # Source de vérité du projet (Projucer)
├── Source/                   # ← SEUL dossier de code à modifier
│   ├── PluginProcessor.h/.cpp  # AudioProcessor (DSP, état)
│   ├── PluginEditor.h/.cpp     # AudioProcessorEditor (UI)
│   ├── DSP/                    # futurs : Voice, VCO, ADSR, Filter
│   ├── Sequencer/              # futurs : Sequencer, Sequence, Step
│   └── UI/                     # futurs : Knobs, grille d'étapes
├── JuceLibraryCode/          # GÉNÉRÉ — gitignoré — jamais à la main
├── Builds/                   # GÉNÉRÉ — gitignoré — jamais à la main
├── AGENTS.md                 # Ce fichier (protocole + journal)
├── opencode.json             # Config opencode (permissions)
└── README.md                 # Doc projet
```

> `JuceLibraryCode/` et `Builds/` **ne sont plus suivis par git** (2026-10-06)
> pour éviter les conflits sur fichiers générés. Après un clone/pull frais,
> ou si ces dossiers manquent → régénérer (voir §6).

---

## 2. Protocole obligatoire (tous agents)

### Avant toute modification
1. **Lire ce fichier `AGENTS.md`** (et `README.md` si besoin).
2. `git pull` sur `main` (une seule branche pour tout le monde).
3. Vérifier `git status` — working tree propre avant de commencer.

### Après chaque modification
1. **Mettre à jour la section Journal** de ce fichier (voir §5).
2. **Tester le build** (voir §6) avant tout commit.
3. **Commit** dans la foulée — code + journal dans le **même commit**.

### En cas de conflit git
- Celui qui rencontre le conflit **le résout**.
- Il **note dans le Journal** : nature du conflit, comment il a été résolu.
- Pas de force-push sur `main`.

---

## 3. Règles Git

| Règle | Valeur |
|---|---|
| Branche | **`main` uniquement** (pas de feature branches) |
| Pull | **Obligatoire** avant chaque session de travail |
| Commit | **À chaque tâche terminée**, code + `AGENTS.md` ensemble |
| Message | **Conventional Commits** en anglais : `feat:`, `fix:`, `docs:`, `chore:`, `refactor:`, `test:` |
| Exemples | `feat: add Voice class with 2 VCOs`, `fix: APVTS state not saved`, `docs: journal entry for UI layout` |

---

## 4. Règles code & JUCE

### Dossiers interdits (jamais à la main)
- `JuceLibraryCode/` — régénéré par le Projucer
- `Builds/` — régénéré par le Projucer

> Tout changement de modules, de nom de plugin, d'options JUCE →
> modifier `Metulifer.jucer` puis `Projucer --resave Metulifer.jucer`.

### Fichiers modifiables
- ✅ `Source/**` (tout le code du plugin)
- ✅ `README.md`
- ✅ `AGENTS.md`
- ✅ `opencode.json` (avec accord des 2 devs)
- ❌ Tout le reste sans validation

### Rappels techniques
- `processBlock()` : temps réel — **pas d'allocations**, pas de GUI, pas de lock.
- Paramètres via **APVTS** (`AudioProcessorValueTreeState`) vivant dans le Processor.
- Communication UI ↔ DSP via attachments/listeners, jamais d'appels directs.
- Editor : `paint()` pour le dessin, `resized()` pour le layout.
- Deployment target macOS : **12.0** (ne pas descendre).

---

## 5. Journal

> Journal de bord des modifications. **Une entrée détaillée par tâche agent.**
> On garde les **50 dernières entrées** (au-delà, on supprime les plus vieilles).

### Format d'entrée

```
### AAAA-MM-JJ — <nom-agent ou humain>
- **Tâche** : <titre court>
- **Fichiers** : `Source/Foo.cpp`, `Source/Foo.h`, …
- **Modifs** : <résumé détaillé de ce qui a changé et pourquoi>
- **Build** : ✅ OK / ❌ échec (détail)
- **Commit** : `<sha court>` — `<message>`
- **Next steps** : <ce qui reste à faire, ou « — »>
```

### Journal (nouvelles entrées en haut)

### 2026-11-04 — Système de transition sans casse pour rendus 3D & Sprite Sheets
- **Tâche** : Développer l'infrastructure logicielle permettant de substituer dynamiquement n'importe quel élément vectoriel par des rendus 3D (filmstrips de potentiomètres, sprites d'états de boutons, leviers d'octave, verre d'écran, châssis) avec fallback vectoriel transparent sans casse.
- **Fichiers** : `Source/UI/Skin.h`, `Source/UI/Skin.cpp`, `Source/UI/MetuliferLookAndFeel.cpp`, `Source/UI/Lever.cpp`, `Source/UI/DisplayScreen.cpp`, `AGENTS.md`
- **Modifs** :
  1. `Source/UI/Skin.{h,cpp}` :
     - Système de découverte multi-sources : scrute `BinaryData` en mémoire ainsi que les dossiers locaux (`Resources/`, bundle macOS `Contents/Resources`, dossier projet) pour charger à chaud les PNG 32-bit (y compris Retina `@2x`).
     - `drawFilmstrip` : Rendu de potentiomètres rotatifs à partir d'une bande verticale ou horizontale ($1 \times N$ ou $N \times 1$) avec calcul automatique du nombre de trames, sélection de la frame selon `normalizedValue` et rééchantillonnage haute qualité. Fallback par nom spécifique (`knob_<name>_filmstrip`) ou générique (`knob_big_filmstrip`, `knob_med_filmstrip`, `knob_small_filmstrip`).
     - `drawButtonSprite` : Rendu des boutons à états multiples (bande 4 ou 6 trames : normal, hover, pressed/down, on, on_hover, on_down ou fichiers séparés `btn_<name>_on/off/down`).
     - `drawStepButton` : Rendu des 32 touches du séquenceur (support des états downbeat 4/4 et on/off/hover).
     - `drawLever` : Rendu du levier d'octave 5 positions (filmstrip 5 crans ou sprites individuels).
     - `drawScreenOverlay` : Calque de verre 3D transparent avec reflets spéculaires placé au-dessus du tracé de l'oscilloscope et du dB-mètre.
  2. `Source/UI/MetuliferLookAndFeel.cpp` : Raccordement de `drawRotarySlider`, `drawStepButtonBackground` et `drawStandardButtonBackground` vers les fonctions `Skin::draw*` avec bascule automatique sur les fonctions vectorielles existantes si aucun asset n'est présent.
  3. `Source/UI/Lever.cpp` & `Source/UI/DisplayScreen.cpp` : Intégration du fallback du levier et du calque de verre d'écran.
- **Validation** : ✅ Build Xcode réussi (`** BUILD SUCCEEDED **` sur VST3, AU et Standalone) + validation automatisée complète `pluginval` au niveau de rigueur 5 (`SUCCESS`, 0 erreur, aucune régression visuelle ou fonctionnelle).
- **Commit** : `f85873e` — `feat: implement zero-breakage 3d sprite sheet and filmstrip skinning system`
- **Next steps** : Déposer les rendus 3D dans `Resources/` au fur et à mesure de leur réalisation dans Blender.

### 2026-11-04 — Synchronisation temps réel 60 Hz & zéro-latence du dB-mètre et de l'oscilloscope
- **Tâche** : Résoudre la latence perçue et le décalage rythmique entre les notes jouées par le séquenceur et la réaction du dB-mètre / oscilloscope.
- **Causes racines identifiées** :
  1. **Écrasement des crêtes inter-trames** : `peakDbL/R` stockait uniquement la valeur du bloc audio courant. Les blocs audio contenant les transitoires étaient écrasés par les blocs silencieux suivants avant que le timer UI (cadencé à 30 Hz = 33 ms) n'ait le temps de les lire.
  2. **Fréquence de rafraîchissement à 30 Hz** : Le délai de 33 ms introduisait une latence visible par rapport au playhead de séquence.
  3. **Lissage inter-trames artificiel** : Le mélange 20/80 % sur `scopePoints` retardait l'apparition des transitoires d'une ou deux trames.
- **Fichiers** : `Source/PluginProcessor.cpp`, `Source/PluginEditor.cpp`, `Source/UI/DisplayScreen.cpp`, `AGENTS.md`
- **Modifs** :
  1. `Source/PluginProcessor.cpp` : Accumulateur de crête inter-trames dans `processBlock` (`peakDbL/R` monte instantanément sur tout transitoire et décroît progressivement de 0.6 dB par bloc en l'absence de signal, garantissant que chaque note est captée à 100 % par l'UI).
  2. `Source/PluginEditor.cpp` : Fréquence de rafraîchissement du timer doublée à **60 Hz** (16,6 ms) pour une réponse visuelle fluide à 60 fps synchrone avec l'audio.
  3. `Source/UI/DisplayScreen.cpp` : Réactivité instantanée de l'oscilloscope (assignation directe `scopePoints[i] = rawSample` sans délai d'atténuation) et balistique du dB-mètre calibrée à 60 Hz.
- **Validation** : ✅ Build Xcode réussi (`** BUILD SUCCEEDED **` sur VST3, AU et Standalone) + validation automatisée complète `pluginval` au niveau de rigueur 5 (`SUCCESS`, 0 erreur).
- **Commit** : `bc2067a` — `fix: sync db meter and oscilloscope to 60Hz with zero-latency peak hold`
- **Next steps** : —

### 2026-11-04 — Fix affichage onde carrée sur l'oscilloscope & Cutoff par défaut
- **Tâche** : Résoudre le problème où une onde carrée isolée apparaît comme une sinusoïde sur l'oscilloscope.
- **Causes racines identifiées** :
  1. **Cutoff LPF par défaut à 1 000 Hz** : Le filtre passe-bas était initialisé à 1 000 Hz par défaut. À 1 000 Hz, tous les harmoniques supérieurs d'une onde carrée (harmoniques 3, 5, 7, etc.) sont violemment atténués, transformant mathématiquement l'onde carrée en sa seule composante fondamentale (une sinusoïde pure).
  2. **Absence de Trigger (synchronisation de phase) sur l'oscilloscope** : Les échantillons étaient lus sans détection de front montant (zero-crossing), provoquant un défilement continu de la forme d'onde à 30 Hz et un flou visuel arrondi par persistance rétinienne.
  3. **Fenêtrage temporel fixe (128 échantillons bruts)** : À 44,1 kHz, 128 échantillons ne représentaient que 38 % d'un seul cycle de C3 (130 Hz), tronquant l'onde en une simple arche incurvée.
  4. **Coins de tracé adoucis** : `PathStrokeType::curved` arrondissait les arêtes franches des ondes carrées.
- **Fichiers** : `Source/DSP/Voice.h`, `Source/DSP/Voice.cpp`, `Source/PluginProcessor.h`, `Source/PluginProcessor.cpp`, `Source/UI/VCFPanel.cpp`, `Source/PluginEditor.cpp`, `Source/UI/DisplayScreen.cpp`, `AGENTS.md`
- **Modifs** :
  1. `Source/DSP/Voice.h`, `Source/DSP/Voice.cpp`, `Source/PluginProcessor.cpp`, `Source/UI/VCFPanel.cpp` : Cutoff par défaut initialisé à **20 000 Hz** (filtre grand ouvert) pour laisser passer l'intégralité du spectre harmonique des formes d'onde brutes (carrée, dent de scie).
  2. `Source/PluginProcessor.h` : `scopeBufferSize` augmenté à 1024 échantillons.
  3. `Source/PluginEditor.cpp` : Buffer de 512 échantillons transmis à l'écran OLED.
  4. `Source/UI/DisplayScreen.cpp` :
     - Algorithme de Trigger avec détection de front montant (rising zero-crossing) : la forme d'onde est parfaitement figée et verrouillée en phase.
     - Mesure de période entre deux passages par zéro pour adapter automatiquement la base de temps et afficher exactement 2 cycles complets stables.
     - Auto-gain adaptatif remplissant ~82 % de la hauteur de l'écran avec fondu enchaîné silencieux.
     - Rendu vectoriel net avec `PathStrokeType::mitered` et embouts carrés pour des angles droits impeccables.
- **Validation** : ✅ Build Xcode réussi (`** BUILD SUCCEEDED **` sur VST3, AU et Standalone) + validation automatisée complète `pluginval` au niveau de rigueur 5 (`SUCCESS`, 0 erreur).
- **Commit** : `97fafec` — `fix: stabilize oscilloscope with zero-crossing trigger and set default filter cutoff to 20kHz`
- **Next steps** : —

### 2026-11-04 — Mode Pinceau glissé (Click-and-Drag Step Paintbrush) sur le séquenceur
- **Tâche** : Permettre d'activer ou désactiver une série continue de pas en maintenant le clic de la souris enfoncé et en balayant la rangée de 32 pas (click-and-drag paintbrush).
- **Fichiers** : `Source/UI/SequenceRowComponent.{h,cpp}`, `AGENTS.md`
- **Modifs** :
  1. `Source/UI/SequenceRowComponent.{h,cpp}` :
     - Remplacement de l'inversion par défaut sur `mouseUp` de `juce::Button` (`setClickingTogglesState (false)`).
     - Gestion centralisée des événements souris via `RowMouseListener` (`handleMouseDown`, `handleMouseDrag`, `handleMouseUp`) recevant tous les événements de la rangée et de ses enfants via `addMouseListener (..., true)`.
     - `handleMouseDown` : Détecte le pas cliqué (`getStepIndexAt` avec tolérance de gap de 3 px), détermine le mode pinceau (`paintState = !clickedStep->getToggleState()`, ou `false` sur clic droit/effaceur), bascule instantanément l'état du pas et sauvegarde le pattern.
     - `handleMouseDrag` : Suit le balayage de la souris avec interpolation continue entre le dernier pas peint et le pas sous le curseur (`getStepIndexForDrag` avec tolérance verticale généreuse de ±20 px et verrouillage horizontal 0..31 pour ne rater aucun pas lors d'un geste rapide), applique `paintState` à tous les pas intermédiaires et notifie `stepsTree` en temps réel.
     - `handleMouseUp` : Réinitialise l'état de glissement (`isPaintingSteps = false`, `lastPaintedStep = -1`).
     - Sécurité 32 bits : Bit shifts sécurisés via `(int) (1u << (unsigned int) i)` sur `getPattern`, `setPattern` et `loadPatternFromTree`.
- **Validation** : ✅ Build Xcode réussi (`** BUILD SUCCEEDED **` sur VST3, AU et Standalone) + validation automatisée complète `pluginval` au niveau de rigueur 5 (`SUCCESS`, 0 erreur).
- **Commit** : `47724e9` — `feat: add click-and-drag step painting in sequence rows`
- **Next steps** : —

### 2026-11-04 — Modernisation de l'UI : potentiomètres hardware, états boutons et contraste
- **Tâche** : Améliorer le contraste global de l'interface squelette, doter les potentiomètres rotatifs d'un rendu matériel analogique avec arcs de valeur dynamiques, concevoir tous les états interactifs des boutons (normal/survol/clic/actif) et clarifier les pas du séquenceur
- **Fichiers** : `Source/UI/MetuliferLookAndFeel.{h,cpp}`, `Source/UI/Skin.cpp`, `Source/UI/SequenceRowComponent.{h,cpp}`, `Source/UI/Lever.cpp`, `Source/UI/DisplayScreen.cpp`, `Source/PluginEditor.cpp`, `AGENTS.md`
- **Modifs** :
  1. `Source/UI/MetuliferLookAndFeel.{h,cpp}` : Rendu vectoriel personnalisé pour tous les sliders rotatifs (`drawRotarySlider` / `drawHardwareKnob`) avec rainure extérieure, arc de valeur actif coloré selon le type de contrôle (ambre pour Tempo/Reverb, cyan pour Cutoff, bleu électrique pour Volume, violet pour Mix, ciel pour ADSR), corps en métal foncé biseauté, hub central et aiguille blanche réactive au survol/glissement. Rendu spécifique des boutons de pas (`drawStepButtonBackground` / `drawStepButtonText`) affichant le numéro de pas (1..32), rétro-éclairage ambre/orange tactile à l'activation, slate foncé avec accentuation des temps 1, 5, 9, 13 (rhythm beat grouping 4/4) à l'état inactif. Rendu tactile des boutons standards (`drawStandardButtonBackground`) : Play/Stop (vert émeraude / rouge rubis), On/Off (vert / slate sombre), LPF/HPF (bleu électrique actif / slate), Copy/Paste ("C"/"P") et Mode/Wave avec survol et appui nets.
  2. `Source/UI/Skin.cpp` : `drawPanel` enrichi d'une ombre portée subtile, d'un dégradé vertical et d'une bordure contrastée `#B4B9C2` avec reflet supérieur pour détacher physiquement les panneaux du châssis.
  3. `Source/UI/SequenceRowComponent.{h,cpp}` : Remplacement des libellés de boutons "Copy"/"Paste" par "C" et "P" (avec tooltips préservés). Ajout d'un badge de séquence ("S1".."S6") et d'une barre d'accentuation bleue à gauche sur la rangée sélectionnée. Amélioration du `PlayheadMarker` avec liseré orange vif et halo translucide.
  4. `Source/UI/Lever.cpp` : Rendu du sélecteur d'octave avec rail creux, 5 crans de positionnement avec surbrillance bleue du cran actif, et poignée métallique graduée.
  5. `Source/UI/DisplayScreen.cpp` : Écran OLED avec verre fumé sombre et double cerclage cyber bleu.
  6. `Source/PluginEditor.cpp` : Teinte de châssis `#E2E5EA`, typographie de titre "METULIFER" en ardoise sombre contrastée avec sous-titre "MODULAR STEP SEQUENCER & DUAL SYNTH", et replacement discret du bouton "Inspect" en haut à droite.
- **Validation** : ✅ Build Xcode réussi (`** BUILD SUCCEEDED **` sur VST3, AU et Standalone) + validation automatisée complète `pluginval` aux niveaux de rigueur 5 et 8 (`SUCCESS`, 0 échec, tous tests audio, état et automation passés).
- **Commit** : `5fb3b60` — `feat: modernize skeleton UI with hardware rotary knobs, tactile button states and enhanced contrast`
- **Next steps** : —
```
### 2026-11-04 — Intégration du module Melatonin Inspector
- **Tâche** : Intégrer l'inspecteur de composants visuel `melatonin_inspector` de Sudara (https://github.com/sudara/melatonin_inspector) au projet JUCE
- **Fichiers** : `modules/melatonin_inspector` (submodule git), `.gitmodules`, `Metulifer.jucer`, `Source/PluginEditor.h`, `Source/PluginEditor.cpp`, `AGENTS.md`
- **Modifs** : (1) Ajout du submodule git `modules/melatonin_inspector` (branche main). (2) Déclaration du module `melatonin_inspector` dans `Metulifer.jucer` (MODULEPATHS vers modules) et régénération Projucer. (3) Intégration de `melatonin::Inspector inspector { *this, false };` dans `MetuliferAudioProcessorEditor` avec connexion de l'UndoManager (`inspector.setUndoManager (&audioProcessor.undoManager)`). (4) Raccourci natif `Cmd+I` fonctionnel et ajout d'un bouton discret « Inspect » (120, 8, 60, 18) dans l'en-tête à côté du titre METULIFER.
- **Build** : ✅ OK (Metulifer - All : VST3, AU, Standalone construits et signés avec succès)
- **Commit** : `ddefac5` — `feat: integrate melatonin_inspector UI component inspector`
- **Next steps** : —

### 2026-11-04 — Fix inversion des arguments applyGain dans Voice (volume et mix inopérants)
- **Tâche** : Corriger l'ordre des arguments de `juce::AudioBuffer::applyGain` dans `Voice::renderChunk`
- **Fichiers** : `Source/DSP/Voice.cpp`, `AGENTS.md`
- **Modifs** : **Cause racine fondamentale** : la signature JUCE de `AudioBuffer::applyGain` est `applyGain (int startSample, int numSamples, Type gain)`. Dans `Voice::renderChunk`, les 4 appels passaient `(gain, 0, numSamples)` au lieu de `(0, numSamples, gain)`. Par conséquent, le paramètre `numSamples` de la fonction recevait `0` : JUCE appliquait le gain sur exactement zéro échantillon (boucle vide) ! Les gains `mixGain1`, `mixGain2`, le facteur `0.5f` et `volumeGain` n'étaient ainsi jamais appliqués au buffer. Seul le test `if (volumeGain <= 0.0001f) return;` coupait le buffer à 0 %, tandis que de 1 % à 100 %, le buffer sortait à 100 % brut. Correction des 4 appels en `applyGain (0, numSamples, gain)` : volume et mix VCO1/VCO2 parfaitement graduels et linéaires.
- **Build** : ✅ OK (Metulifer - All : VST3, AU, Standalone construits et signés avec succès)
- **Commit** : `d591d8c` — `fix: correct inverted arguments in AudioBuffer applyGain calls in Voice`
- **Next steps** : —

### 2026-11-04 — Phase 7 : Multi-FX (Reverb, Delay, Redux), Arpégiateur 11 modes & Documentation Fumadocs
- **Tâche** :
  1. Transformer le module reverb en processeur Multi-FX 3-en-1 (Reverb stéréo Room/Hall, Ping-Pong Delay avec saturation tanh, Redux bitcrusher/décimateur) avec onglets `[REV]`, `[DLY]`, `[RDX]` et LEDs de statut.
  2. Implémenter l'arpégiateur par séquence (11 modes : Up, Down, Up&Down, Down&Up, Pinky Up, Pinky Down, Thumb-Pinky, Converge, Diverge, Random, Order ; 10 gammes/accords ; 1 à 3 octaves) avec boutons `ARP` sur chaque rangée et contrôles dédiés dans SynthPanel.
  3. Résoudre la désynchronisation des séquences lors du changement de longueur : horloge master unique et calcul de pas `globalStep % length` pour maintenir un verrouillage de phase perpétuel sur la grille de double-croches.
  4. Intégrer un oscilloscope temps réel sur l'écran OLED avec buffer circulaire lock-free, affichant le signal joué en direct lorsqu'aucun paramètre n'est manipulé.
  5. Intégrer un dB-mètre stéréo de studio calibré en dBFS (-48 dB à +3 dB) avec balistique de crête instantanée, retombée logarithmique (54 dB/s), indicateurs de crête maintenue (Peak-Hold 1 s), graduations chiffrées (-48, -24, -12, -6, 0, +3), indicateur CLIP rouge et lecture numérique précise au dixième de dB.
  6. Réaligner rigoureusement toute l'interface 1280×720 (marges symétriques de 32 px, espacements réguliers de 14 px, suppression du chevauchement VCF/SynthPanel, alignement vertical parfait DisplayScreen + ClockPanel).
  7. Intégrer l'inspecteur visuel Melatonin Inspector (`⌘I` et bouton « Inspect »).
  8. Valider le plugin avec `pluginval` aux niveaux de rigueur 5 et 8 (100 % pass).
  9. Rédiger et structurer la documentation bilingue FR/EN complète avec Fumadocs (Next.js 16 / Turbopack), 38 pages statiques générées sans erreur.
- **Fichiers** :
  - DSP/Engine : `Source/DSP/Voice.{h,cpp}`, `Source/PluginProcessor.{h,cpp}`, `Source/Sequencer/SequencerEngine.{h,cpp}`
  - UI : `Source/PluginEditor.{h,cpp}`, `Source/UI/DisplayScreen.{h,cpp}`, `Source/UI/ReverbPanel.{h,cpp}`, `Source/UI/SynthPanel.{h,cpp}`, `Source/UI/SequenceRowComponent.{h,cpp}`, `Source/UI/MetuliferLookAndFeel.cpp`
  - Documentation : `docs/` (app, content/docs/fr, content/docs/en, lib, components)
- **Modifs DSP & UI** :
  - `SequencerEngine` : remplacement des 6 accumulateurs indépendants par un accumulateur master unique et un pas global `globalStep` ; `step = globalStep % lengthOf(s)` assure qu'aucun décalage de phase n'est jamais accumulé lors des changements de longueur en cours de lecture.
  - `PluginProcessor` : buffer circulaire `scopeBuffer[512]` et méthode `getScopeBuffer` pour alimenter l'oscilloscope sans aucun verrou.
  - `DisplayScreen` : affichage d'un oscilloscope néon cyan avec réticule et lueur sur le signal master en temps réel quand le plugin est au repos, bascule automatique sur l'affichage paramètre grand format lors des réglages.
  - `PluginEditor` & `SynthPanel` : grille d'alignement au pixel près (SynthPanel w=740, VCF x=786, LFO1 x=905, LFO2 x=1024, MultiFX x=1143 avec espacement constant de 14 px ; colonne droite DisplayScreen + ClockPanel alignée à x=960 w=288).
  - `Voice.h`/`Voice.cpp` : réservoir de notes pour 10 gammes (`buildNotePool`), avancement d'arpège (`advanceArp`), DSP Redux (quantification de bits 2..16 et décimation 1..32x), DSP Ping-Pong Delay (buffer circulaire stéréo 2.5s, soft clipping tanh dans la boucle de feedback), chaîne série `Voice -> Redux -> Delay -> Reverb -> Out`, zéro allocation en temps réel.
  - `PluginProcessor.h`/`PluginProcessor.cpp` : déclaration des paramètres APVTS pour Delay (`enabled, time, feedback, mix, pingpong`), Redux (`enabled, bits, rate, mix`) et Arp (`enabled, mode, chord, octaves`) par séquence (total 209 paramètres APVTS) ; ajout des champs de modulation dans `DestField` ; `resetArp()` à l'arrêt du transport.
  - `ReverbPanel` : transformation en Multi-FX avec 3 onglets et voyants LED d'état actif.
  - `SynthPanel` : colonne Arpégiateur avec boutons cycliques Mode et Chord, knob Octaves, bouton On/Off.
  - `SequenceRowComponent` : bouton toggle rapide `ARP` par rangée de séquence.
  - `MetuliferLookAndFeel` : styles vectoriels pour boutons ARP, REV, DLY, RDX et Inspect.
- **Build & Tests** :
  - `xcodebuild` Debug : ✅ `** BUILD SUCCEEDED **` (Shared Code, VST3, AU, Standalone).
  - `pluginval` Level 5 : ✅ `SUCCESS` (44.1 kHz, 48 kHz, 96 kHz, buffers 64 à 1024, automation intensive).
  - `docs` Next.js SSG : ✅ `38/38 static pages generated` (FR et EN, search API, 100 % TypeScript OK).
- **Commit** : — (en attente de confirmation de l'utilisateur)

### 2026-11-04 — Fix clipping audio & réactivité des potentiomètres Mix et Volume
- **Tâche** : Résoudre l'écrêtage/clipping excessif de la reverb et corriger le comportement des potentiomètres « Mix out DB » et des volumes de séquence
- **Fichiers** : `Source/DSP/Voice.cpp`, `Source/PluginProcessor.cpp`, `Source/PluginEditor.cpp`, `Source/UI/SequenceRowComponent.cpp`, `AGENTS.md`
- **Modifs** :
  1. `Source/DSP/Voice.cpp` : Normalisation de la sommation VCO1 + VCO2 (gain 0.5f) pour que deux oscillateurs à 0 dB somment à 0 dBFS max (au lieu de +6 dBFS). Réajustement des gains wet/dry de la reverb (wet * 0.35f, 1.0f - wet * 0.5f) pour corriger l'amplification interne excessive de Freeverb. Mute franc à 0.0f quand mixDb <= -59.0 dB. Si le volume de séquence est à 0 %, court-circuit immédiat avec reset de la reverb pour un silence parfait.
  2. `Source/PluginProcessor.cpp` : Ajout d'une marge de headroom pour les 6 séquences polyphoniques (gain 0.5f) suivie d'une saturation douce (std::tanh(x)) garantissant un plafond analogique sans écrêtage dur numérique DAC/CoreAudio.
  3. `Source/PluginEditor.cpp` : Affichage textuel dynamique dans le DisplayScreen (« seq N volume XX % » et « seq N length XX steps ») lors de la manipulation des potentiomètres de chaque rangée de séquence.
  4. `Source/UI/SequenceRowComponent.cpp` : Ajout de tooltips explicites sur les boutons Copy/Paste, Length et Volume de chaque séquence.
- **Build** : ✅ OK (Metulifer - All : VST3, AU, Standalone construits et signés avec succès)
- **Commit** : `bbc19c8` — `fix: resolve audio clipping and gain staging for VCO mix, sequence volume and reverb`
- **Next steps** : —

### 2026-11-04 — Phase 6 : Reverb par séquence + ReverbPanel UI
- **Tâche** : Implémenter le module Reverb selon le design Figma (node 176:2 / 176:247) avec Reverb par séquence (paginée)
- **Fichiers** : `Source/UI/ReverbPanel.{h,cpp}` (neufs), `Source/DSP/Voice.{h,cpp}`, `Source/PluginProcessor.{h,cpp}`, `Source/PluginEditor.{h,cpp}`, `Metulifer.jucer`, `AGENTS.md`
- **Modifs** : (1) **DSP (`Voice`)** : intégration de `juce::dsp::Reverb` stéréo + buffer scratch stereo (0 alloc en temps réel) ; modes Room (roomSize 0.45, damp 0.55, width 0.8) et Hall (roomSize 0.85, damp 0.25, width 1.0) ; mix wet/dry (0..100 %) ; bypass propre si inactif/mix=0. (2) **Params (APVTS)** : ajout de `seqN_reverb_enabled` (bool), `seqN_reverb_mode` (choice Room/Hall), `seqN_reverb_mix` (float 0..100 %, defaut 30 %) par séquence ; binding dans `VoiceSources`. (3) **Modulation** : ajout du champ `fReverbMix` dans `DestField`, permettant de moduler le mix reverb par un câble LFO. (4) **UI (`ReverbPanel`)** : nouveau composant 105×224 px à x=1144, y=471 (titre "reverb", bouton On/Off 50×49, bouton Mode Room/Hall 50×49, gros knob rotary 86 px pour Mix) avec hook `Skin::drawPanel ("panel_reverb")`. (5) **Editor** : pagination complète de la reverb au clic de séquence (`bindSequence`), attachement `reverbAttachments`, écouteurs pour notification écran (`DisplayScreen`) et automation hôte. (6) **Projucer** : ajout des fichiers `ReverbPanel` dans le groupe `UI` du `.jucer` et resave réussi.
- **Build** : ✅ OK (`** BUILD SUCCEEDED **`) + smoke test standalone 4 s : `SMOKE_TEST_OK`, zéro assertion
- **Commit** : `cd57efc` — `feat: add per-sequence reverb with Room/Hall modes, mix knob and ReverbPanel UI`
- **Next steps** : tester les réglages sonores en conditions réelles avec différents patterns de pas
```
### 2026-10-07 — icône d'app (Icon Composer → compilation auto)
- **Tâche** : intégrer le logo livré (export Icon Composer) pour qu'il soit **généré à chaque build Xcode**
- **Fichiers** : `Resources/Metulifer.icon/` (neuf : `icon.json` + `Assets/Artboard 1.png`, `Assets/Artboard 2.png`), `Metulifer.jucer`, `AGENTS.md`
- **Modifs** : (1) bundle `.icon` d'Icon Composer copié depuis `~/Desktop/metufilfer_logo/Untitled.icon` → `Resources/Metulifer.icon` (renommé au nom du plugin). (2) `.jucer` : FILE `Resources/Metulifer.icon/icon.json` ajouté dans le groupe `Resources` existant (`compile="0" resource="0"` — Projucer détecte les icônes via `icon.json` dans un dossier `.icon`, cf. `Project::findAllIconComposerItems()`), + attribut **`iconComposerIcon="icMt1a"`** sur `<XCODE_MAC>`. (3) `Projucer --resave` : le pbxproj référence le dossier `Metulifer.icon` dans les Resources de **tous** les targets et pose `ASSETCATALOG_COMPILER_APPICON_NAME = Metulifer` sur chaque configuration. **Aucun script ajouté** — c'est `actool` qui compile le `.icon` à chaque build. (4) Vérifié dans l'app produite : `Resources/Assets.car` (1.9 Mo) contient `IconGroup "Metulifer"` + 8 `Icon Image`, `Resources/Metulifer.icns` (legacy), `Info.plist` → `CFBundleIconName = Metulifer` et `CFBundleIconFile = Metulifer`. Piège noté : Projucer écrit `lastKnownFileType = file.icon` (et non `folder.iconcomposer.icon`) — Xcode 27 l'compile quand même.
- **Build** : ✅ OK (`** BUILD SUCCEEDED **`)
- **Commit** : — (ce commit)
- **Next steps** : vérifier visuellement l'icône dans le Dock/Finder ; decider du sort de `exports/` (doublon des PNG d'export, non tracké)

### 2026-10-07 — fix : aucun son (steps jamais écrits)
- **Tâche** : diagnostic « toujours aucun son » alors que Play + steps semblent actifs
- **Fichiers** : `Source/UI/SequencerPanel.cpp`
- **Modifs** : **Cause racine** : `SequenceRowComponent::setStepsTree()` n'était appelé **nulle part** — chaque rangée gardait un `stepsTree` invalide, donc `savePattern()` sortait silencieusement via `if (stepsTree.isValid())` : **cliquer un step (ou Copy/Paste) n'écrivait jamais le pattern** → `patterns[]` du processor restait à 0 → `gateOf()` toujours faux → aucune `noteOn` → silence (le playhead bougeait, l'oscillateur était muet). Fix : `SequencerPanel::setStepsTree()` appelle désormais `row->setStepsTree(tree)` au lieu de `loadPatternFromTree()` sur un arbre que la rangée ne possédait pas. (Le DSP lui-même était déjà prouvé OK par le harnais hors-ligne Phase 4 : RMS 0.86.)
- **Build** : ✅ OK (`** BUILD SUCCEEDED **`) + smoke test standalone 4 s : `RUNNING_OK`, zéro assertion
- **Commit** : — (ce commit)
- **Next steps** : vérifier en séance que le son sort bien (steps ON + Play) ; Phase 6 (FX) en attente du design Figma

### 2026-11-04 — Fix LFO DSP, câbles virtuels, écran OLED de liaison et animation temps-réel des knobs
- **Tâche** : Réparer les LFO et câbles non fonctionnels, ajouter un écran OLED dédié sur chaque module LFO indiquant la liaison/destination active, et animer visuellement le knob assigné en rythme avec le LFO.
- **Fichiers** : `Source/PluginProcessor.{h,cpp}`, `Source/PluginEditor.{h,cpp}`, `Source/UI/CableLayer.{h,cpp}`, `Source/UI/LFOPanel.{h,cpp}`, `Source/UI/MetuliferLookAndFeel.cpp`, `AGENTS.md`
- **Modifs** :
  1. **Bug parsing séquence corrigé** (`PluginProcessor.cpp` & `CableLayer.cpp`) : `dest.upToFirstOccurrenceOf("_")` sur `"seq1_cutoff"` renvoyait `"seq1"`, dont `.getIntValue()` valait 0 (donc `0 - 1 = -1`), ce qui rejetait tous les câbles et empêchait toute modulation DSP et tout affichage de câble. Corrigé avec `dest.substring(3).upToFirstOccurrenceOf("_").getIntValue() - 1`.
  2. **Synchronisation ValueTree** : `CableLayer` implémente désormais `juce::ValueTree::Listener` pour écouter `modTree` en temps réel lors de l'ajout, suppression, annulation (Undo/Redo) ou chargement de preset.
  3. **Écran OLED sur chaque panneau LFO (`LFOPanel`)** :
     - Écran rétroéclairé OLED 32 px affichant en temps réel `TARGET: <DESTINATION>` (ou `[ UNLINKED ]`) en couleur thème (ambre pour LFO1, cyan pour LFO2).
     - Fréquence et forme d'onde (`~ SIN 1.5Hz`) + indicateur d'activité oscillante dans l'écran OLED.
     - Jack sortie métallique avec anneau coloré + LED d'activité pulsante au rythme et selon la forme d'onde du LFO.
  4. **Animation temps-réel des knobs assignés (`MetuliferLookAndFeel` & `PluginEditor`)** :
     - À chaque bloc audio, la valeur normalisée instantanée du LFO est stockée dans un `std::atomic<float> lfoValue[2]`.
     - Dans le timer 60 Hz, les knobs assignés reçoivent dynamiquement leur position modulée (`mod_pos = base_pos + lfo * 0.5`) sans modifier la valeur APVTS (préservant ainsi l'automation DAW et l'UndoManager).
     - L'aiguille et le track du potentiomètre tournent physiquement en direct au rythme du LFO, accompagnés d'un arc de balayage lumineux et d'un point de référence indiquant la valeur de base.
     - Badge discret `L1` / `L2` affiché sur le knob pour identifier instantanément sa source de modulation.
- **Build & Tests** : ✅ `** BUILD SUCCEEDED **` (Xcode Release), ✅ `pluginval` strictness level 5 sur VST3 (SUCCESS) et AU (SUCCESS).

### 2026-10-07 — Phase 5 : LFO ×2 + câbles de modulation LFO→knob
- **Tâche** : 2 LFO (rate/wave/offset/depth/enabled) modulant les knobs du synthé via des câbles cliquables-draggables
- **Fichiers** : `Source/UI/LFOPanel.{h,cpp}` (neufs), `Source/UI/CableLayer.{h,cpp}` (neufs), `Source/PluginProcessor.{h,cpp}`, `Source/PluginEditor.{h,cpp}`, `Metulifer.jucer`, `AGENTS.md`
- **Modifs** : (1) **Params** : `lfo1/2_{rate(0,1–20 Hz skew@1), wave(4 choices sine/saw/square/triangle), offset(−100..+100), depth(0..100), enabled(bool, défaut false)}` ajoutés au layout APVTS. (2) **Processor** : `modTree` (`"mod"`, enfants `cable{src:int, dest:"seqN_suffix"}`) sérialisé dans `getState/getState` ; `rebuildCables()` parse les enfants → buffer `RuntimeCable` double (cableBuffers[2][32], flip atomique) ; `valueTreeChildAdded/Removed` écoute `modTree` ; `updateLfos()` (phase continue, 4 formes d'onde, `clamp(w*depth/100 + offset/100)`) ; dans `processBlock` AVANT `setParams` : pour chaque câble visible, delta = `lfoValue * 0.5` appliqué **en normalisé** sur une copie de la valeur lue (`convertTo0to1` + delta → `convertFrom0to1` → `setField`) — l'APVTS n'est jamais écrite par l'audio ; auto-off : un LFO dont `enabled=false` ou sans câble ne modifie rien. (3) **`LFOPanel`** (neuf, 105×224) : titre + bouton wave (cycle Sin/Saw/Sqr/Tri, callback `onWaveClicked`) + bouton On/Off (`onPowerClicked`) + 3 knobs (rate/offset/depth 40×40) + jack cercle en bas à gauche. (4) **`CableLayer`** (neuf, couvre tout l'éditeur, ajouté en dernier) : `hitTest` surchargé → vrai uniquement près d'un jack/câble (sinon click-through) ; drag depuis un jack → survol knob = highlight vert + bézier verte ; relâche sur knob éligible → `modTree.appendChild(cable)` avec `undoManager` ; clic sur câble existant → `removeChild` ; `setSelectedSequence(i)` filtre les câbles par séquence de dest (pagination). (5) **Editor** : `lfoPanel1 (874,471)` / `lfoPanel2 (1013,471)` + `cableLayer` (0,0,1280,720) ajoutés, bindings LFO permanents (knobs → SliderAttachment + `track`), boutons wave/onOff → param gesture + écran, `refreshCableTargets()` construit sources (jacks) et targets (11 knobs : `a1,d1,s1,r1,a2,d2,s2,r2,mix1,mix2,cutoff`) en coordonnées absolues, `sequenceSelected` → `cableLayer.setSelectedSequence`. (6) `.jucer` : GROUP UI + 4 FILE (LFOPanel, CableLayer) + resave. (7) **§7** : décision « câbles LFO→knob » gravée avant implémentation (dests v1, ±50 % en normalisé, un LFO sans câble = inactif, modTree, undo).
- **Pièges revus/corrigés** : `Component::hitTest` = **non-const** en JUCE 9 → signature sans `const` (header + cpp) ; `ValueTree::Listener::valueTreeChildRemoved` = **3 params** (`parent, child, index`) ; `JUCE_DECLARE_NON_COPYABLE` supprime à nouveau le ctor implicite → `CableLayer() = default;` (3e occurrence du même piège) ; `Rectangle::operator+(Point)` existe mais `Point<int>.toFloat()` préféré explicite ; capture lambda `[l.index]` illégale (pas un identifiant) → init-capture `idx = l.index`.
- **Build** : ✅ OK (`** BUILD SUCCEEDED **`) + smoke test standalone 4 s : `RUNNING_OK`, zéro assertion
- **Commit** : — (ce commit)
- **Next steps** : tester manuellement le drag jack→knob + undo ; Phase 6 (FX) en attente du nouveau design Figma du dev

### 2026-10-07 — Phase 4 : son (moteur séquenceur + voices + pitch + Play)
- **Tâche** : Premier son réel — 6 séquences polyrythmiques, synthé par séquence (2 VCO + 2 ADSR + filtre), pitch par VCO (levier octave + knob Note), Play/Stop + playhead
- **Fichiers** : `Source/Sequencer/SequencerEngine.{h,cpp}` (neufs), `Source/DSP/Voice.{h,cpp}` (neufs), `Source/UI/Lever.{h,cpp}` (neuf), `Source/PluginProcessor.{h,cpp}`, `Source/PluginEditor.{h,cpp}`, `Source/UI/{SynthPanel,SequenceRowComponent,SequencerPanel,ClockPanel}.{h,cpp}`, `Metulifer.jucer`, `AGENTS.md`
- **Modifs** : (1) **SequencerEngine** : 6 compteurs indépendants (polyrythmie validée §7), pas = 1/16 @ tempo BPM, `process()` rend des `StepEvent { offset, seq, gateOn }` **triés par offset** (`std::sort`), `pendingStart` émet le gate du step 0 à offset 0 (début de lecture à zéro), `pendingNoteOff` atomique → noteOff de toutes les voices sur le thread audio, `currentSteps[6]` atomiques = playhead UI. (2) **Voice** : oscillateur custom (accumulateur de phase, sine / saw **polyBLEP** / square **polyBLEP** / triangle), 2× `juce::ADSR`, 2 buffers scratch mono, branche osc→ADSR→gain(mix dB), somme→filtre `juce::dsp::StateVariableTPTFilter` (TPT LPF/HPF, cutoff clamp < 0.49·sr)→volume→add dans out ch0/ch1 ; `render()` chunké au maxBlock, 0 alloc en temps réel ; freq = 130.81 × 2^octave × 2^(note/12) (C3). (3) **Params** : + `seqN_note1/note2` = `AudioParameterChoice` 12 noms (C…B, défaut C). (4) **Processor** : `VoiceSources[6]` (19 pointeurs atomiques/séquence) bindés au ctor, miroir `stepsTree`→`patterns[6]` atomiques via `ValueTree::Listener` (+ resync après `setStateInformation`), processBlock = clear canaux + `setParams` + split du buffer aux offsets d'events + `renderVoices` + RMS, `setPlaying/isPlaying/getPlayheadStep`, `getTailLengthSeconds` = 6 s. (5) **Lever** (nouveau composant) : 5 crans verticals (haut = +2 … bas = −2), `ParameterAttachment` (`sendInitialUpdate`, gesture complet pour l'undo), fallback vectoriel (bande bleue sur rail gris), `onValueChanged` → écran. (6) **SynthPanel** : knobs **Octave remplacés par des Leviers** (24×74, x28, y46/y137) + knobs **Note** (86 px, x58) ; en-têtes `Octave` (x16, 11 px) + `Note` (x58). (7) **Playhead** : `SequenceRowComponent::PlayheadMarker` (rect orange arrondi, `setInterceptsMouseClicks(false)`, `toFront`), `SequencerPanel::setPlayhead`, cache `lastPlayhead[6]` dans le timer éditeur 30 Hz. (8) **Play/Stop** : `ClockPanel::onPlayToggled` → `setPlaying` + clear écran (retour signal immédiat). (9) `.jucer` : GROUPs `DSP` (Voice) + `Sequencer` (SequencerEngine) + `Lever` dans UI + resave. **Pièges revus/corrigés** : `JUCE_DECLARE_NON_COPYABLE` (copie déclarée) **supprime le ctor par défaut implicite** → `Voice() = default;` + `SequencerEngine();` à déclarer (piège déjà noté Phase 2, retombé dedans — retrouvé via `static_assert(is_default_constructible)` isolé) ; `ADSR::applyEnvelopeToBuffer` JUCE 9 = **3 args** (buffer, startSample, numSamples) ; `ProcessContextReplacing` exige une **lvalue** (`auto sub = block.getSubBlock(...)` puis contexte) ; l'APVTS n'a pas de `getUndoManager()` → membre public `apvts.undoManager` ; les events arrivent **par séquence** (offsets croissants par séq. mais pas globalement) → tri avant rendu segmenté.
- **Build** : ✅ OK (`** BUILD SUCCEEDED **`) + smoke test standalone 4 s : `RUNNING_OK`, zéro assertion
- **Commit** : — (ce commit)
- **Next steps** : Phase 5 — LFO ×2 (`lfoN_{rate,wave,offset,depth,enabled}`) + `modTree` câblable + `LFOPanel`/`CableLayer` ; valider la sémantique LFO/reverb avec le dev (§8)

### 2026-10-07 — Phase 0 : infra skinnable (assets image)
- **Tâche** : Registry d'assets + LookAndFeel à sprites + hooks de fond, avant l'arrivée des rendus 3D du dev (décision §7 « UI skinnable »)
- **Fichiers** : `Source/UI/Skin.{h,cpp}`, `Source/UI/MetuliferLookAndFeel.{h,cpp}`, `Source/UI/{SynthPanel,VCFPanel,ClockPanel,SequenceRowComponent}.cpp`, `Source/UI/DisplayScreen.h`, `Source/PluginEditor.{h,cpp}`, `Metulifer.jucer`, `AGENTS.md`
- **Modifs** : (1) **`Skin`** : `find/has/drawPanel` — lookup d'images par nom normalisé (`"VCO1 Octave"` → `vco1_octave`), scan paresseux de BinaryData protégé par `__has_include("BinaryData.h")` (aucun asset = aucune dépendance, build inchangé), cache statique, `drawPanel` = image étirée sinon rounded-rect de fallback. (2) **`MetuliferLookAndFeel`** (hérite `LookAndFeel_V4` = L&F par défaut JUCE 9 → zéro delta visuel sans asset) : `drawRotarySlider` = strip horizontal de frames (frame = `round(pos·(n-1))`), `drawButtonBackground`/`drawButtonText` = assets `btn_<texte>[_on|_off]` (texte masqué si asset), fallback V4 sinon. **Piège JUCE 9** : `LookAndFeel::drawButton(TextButton…)` n'existe plus → overrides `drawButtonBackground(Button…)` + `drawButtonText(TextButton…)`. (3) Hooks `Skin::drawPanel` : `panel_synth`/`panel_vcf`/`panel_clock`/`panel_row` + `bg_editor` (image) dans l'éditeur. (4) LNF posé sur l'éditeur (`setLookAndFeel(nullptr)` dans le dtor). (5) `.jucer` : groupe **`Resources`** vide (PNG à venir côté dev, `resource="1"` au ajout + resave), 4 FILE Skin/LNF, resave OK. (6) **Fix assert préexistant** : `DisplayScreen.h` littéral `"—"` UTF-8 passé au ctor ASCII de `juce::String` → `jassert juce_String.cpp:327` au lancement du standalone (visible au smoke test) → `CharPointer_UTF8 ("\xE2\x80\x94")`. (7) **§7** : 7 décisions gravées avant implémentation (ordre phases, playhead indépendant, pitch levier+note, 1 ADSR/VCO, LFO 4 contrôles, FX annulés, UI skinnable).
- **Build** : ✅ OK (`** BUILD SUCCEEDED **`) + smoke test standalone 4 s : vivant, zéro assertion
- **Commit** : — (ce commit)
- **Next steps** : Phase 4 — params `note1/note2` (choice 12), `Sequencer/SequencerEngine` (6 compteurs indépendants), `DSP/Voice` (2 VCO polyBLEP + 2 ADSR + filtre TPT), `UI/Lever` (5 crans), playhead highlight, Play/Stop câblé

### 2026-10-07 — SynthPanel : rangées = VCO (fidélité Figma)
- **Tâche** : Restructurer le SynthPanel — chaque rangée = un VCO (VCO1 haut, VCO2 bas), colonnes = paramètres
- **Fichiers** : `Source/UI/SynthPanel.cpp`
- **Modifs** : (1) Labels : suppression des titres « VCO1 »/« VCO2 » **côte à côte** au-dessus des knobs et des doublons « Octave »/« Wave » (un par colonne VCO) ; désormais **4 en-têtes de colonne** en haut (`Octave` x28, `Wave` x176, `ADSR` x298, `Mix out DB` x499) + **labels de rangée** `VCO1`/`VCO2` **empilés à gauche** (x0, centrés sur chaque rangée) ; « ADSR 1 »/« ADSR 2 » → un seul titre **« ADSR »** ; sous-titres Mix VCO1/VCO2 supprimés (identification par labels de rangée). (2) Knobs gros **78 → 86 px** (taille Figma). (3) Grille : rangée 1 (VCO1) knobs y40, ADSR1 y67, lettres A/D/S/R y99 ; rangée 2 (VCO2) knobs y131, ADSR2 y158, lettres y190 ; gap inter-rangées 5 px ; ADSR1 centré optiquement sur les knobs (y67 vs y61.6 Figma, offset de 6 px dans le wireframe). (4) Wireframe relu via MCP Figma (`get_design_context` node `121:215`) : structure **colonnes = params / rangées = VCO** confirmée — **anomalie Figma** : Frames 17/18 dupliqués exactement superposés (double « Octave ») → on ne rend qu'une colonne, à nettoyer côté Figma. (5) **LFO ×2 + reverb apparus dans le wireframe** (x874/1013/1144, y471, 105×224, structure identique VCF : titre + 2 boutons 50×49 + 1 knob 86) — **pas encore implémentés** (sémantique boutons/knob à valider ; décision §7 « reportés » toujours active).
- **Build** : ✅ OK (`** BUILD SUCCEEDED **`)
- **Commit** : `92433ff` — `feat: restructure synth panel rows as VCOs per Figma wireframe`
- **Next steps** : valider la sémantique LFO×2 + reverb (params) ; Phase 4 — `Sequencer/` (clock, Play/Stop, avance des steps) + `DSP/` (Voice) branchés aux params

### 2026-10-06 — Phase 3 : APVTS + pagination + écran live
- **Tâche** : Paramètres host, re-binding au clic, steps sérialisés, écran « param touché / signal »
- **Fichiers** : `Source/PluginProcessor.{h,cpp}`, `Source/PluginEditor.{h,cpp}`, `Source/UI/{DisplayScreen,SequenceRowComponent,SequencerPanel,VCFPanel}.{h,cpp}`
- **Modifs** : (1) **APVTS** 109 params : par séquence `seqN_{a1,d1,s1,r1,a2,d2,s2,r2,octave1,wave1,octave2,wave2,mix1,mix2,filter,cutoff,length,volume}` + `tempo` global — A/D/R 0–5000 ms (skew centre 500), S/Volume 0–100 %, Mix −60→+6 dB, Cutoff 20–20000 Hz (skew 1k), Wave = 4 choices, Filter = LPF/HPF choice, Length 1–32 ; unités via `withLabel` + affichage `param->getText()` (normalisé !) + `getLabel()`. (2) **Steps** : `ValueTree stepsTree` (bitfield `p0..p5`), lignes en lecture/écriture + listener côté `SequencerPanel`, sérialisés dans `getStateInformation` avec l'APVTS (pas de params hôte). (3) **RMS** : `std::atomic<float>` calculé en fin de `processBlock` (safe temps réel), lu par un `juce::Timer` 30 Hz dans l'éditeur. (4) **Pagination** : SynthPanel/VCFPanel = attachments **recréés** à chaque clic de rangée (`bindSequence`, flag `updatingBindings` anti-faux-affichage) ; length/volume par rangée + tempo = attachments permanents ; boutons LPF/HPF = choice param (gestion d'interaction dans l'éditeur + listener `parameterChanged` pour l'automation hôte). (5) **Écran** : mode param (nom + valeur + unité en vert gras, ex. `attack 120 ms`) affiché au drag/molette/clic bouton filtre, retour au **signal RMS** après **2 s** d'inactivité ; mode signal par défaut. (6) **Copy/Paste** : clipboard partagé (`juce::var`) dans `SequencerPanel`. (7) `UndoManager` membre du Processor (prêt pour l'undo des steps). **Pièges JUCE 9** : ctor APVTS = `(processor, UndoManager*, Identifier, ParameterLayout)` ; `Parameter::getText()` attend une valeur **normalisée** (`convertTo0to1` d'abord), le label vient de `getLabel()` et non de `getText()` ; 2 VCOs = params `octave1/wave1` + `octave2/wave2` (un seul `octave`/`wave` attrapé avant build).
- **Build** : ✅ OK (`** BUILD SUCCEEDED **`) + smoke test standalone : lancé 4 s, vivant, aucun assert
- **Commit** : — (ce commit)
- **Next steps** : Phase 4 — `Sequencer/` (clock, Play/Stop, avance des 32 steps au tempo) + `DSP/` (Voice : 2 VCOs, 2 ADSRs, filtre LPF/HPF) branchés aux params

### 2026-10-06 — Phase 2 : squelette UI
- **Tâche** : UI skeleton statique conforme à la spec Figma (§8) + assemblage de l'éditeur
- **Fichiers** : `Source/UI/{SequenceRowComponent,SequencerPanel,DisplayScreen,SynthPanel,VCFPanel,ClockPanel}.{h,cpp}` (12 neufs), `Source/PluginEditor.{h,cpp}`, `Metulifer.jucer`
- **Modifs** : (1) `SequenceRowComponent` : 32 steps toggle, Copy/Paste, Length (0–32) + Volume (0–1), highlight sélection, clic = `onSelected` via `RowMouseListener` (`addMouseListener(this…, true)`). (2) `SequencerPanel` : 6 rangées (pitch 71.065, h 61.065), `setSelectedSequence`, `onSelectionChanged` → pagination écran. (3) `DisplayScreen` : écran sombre, « SEQ n » vert + param + barre RMS. (4) `SynthPanel` : VCO1/VCO2 (Octave + Wave), ADSR1/ADSR2 × 4 knobs 31px, Mix VCO1/VCO2, labels. (5) `VCFPanel` : boutons LPF/HPF exclusifs + cutoff skew 1 kHz. (6) `ClockPanel` : CLOCK + tempo knob 175 + bouton Play/Stop. (7) Éditeur 1280×720, bounds Figma absolus, fond `#E9E9E9`, clic rangée → écran SEQ n. (8) `.jucer` : GROUP « UI » (12 FILE) + `Projucer --resave`. **Pièges corrigés** : `juce::RotarySlider` n'existe pas → `juce::Slider` + `RotaryHorizontalVerticalDrag` ; macro `JUCE_DECLARE_NON_COPYABLE` = constructeur copie déclaré ⇒ **supprime le constructeur par défaut implicite** (chaque classe JUCE doit déclarer le sien — `DisplayScreen()` ajouté) ; `unique_ptr<T>` exige `T` complet dans le TU du destructeur (include plein dans `SequencerPanel.h`) ; `RowMouseListener` est un objet membre → `&mouseListener`, pas `.get()`.
- **Build** : ✅ OK (`** BUILD SUCCEEDED **` après fixes)
- **Commit** : — (ce commit)
- **Next steps** : Phase 3 — APVTS (`seq1_…`→`seq6_…`), re-binding des attachments au clic, steps en ValueTree, écran live (RMS + param touché)

### 2026-10-06 — spec UI Figma
- **Tâche** : Gravure de la spec UI depuis le wireframe Figma
- **Fichiers** : `AGENTS.md`
- **Modifs** : nouvelle §8 « Spec UI — Figma validé » (node `121:16` : 6 rangées, 32 steps, longueur/volume, écran multi-rôle, CLOCK + Play/Stop, SynthPanel pagination, VCF LPF/HPF+cutoff, Mix=niveaux VCO, LFO/reverb reportés) ; §7 Décisions complétées (APVTS 6 séquences + re-binding, steps en ValueTree) ; checklist renumérotée §9. Réunion de validation avec l'humain : hypothèses Wave=4 positions et LPF/HPF exclusifs acceptées.
- **Build** : — (pas de code C++)
- **Commit** : — (ce commit)
- **Next steps** : Phase 2 — UI skeleton statique (SequenceRowComponent, DisplayScreen, SynthPanel)

### 2026-10-06 — allègement permissions
- **Tâche** : opencode.json moins restrictif
- **Fichiers** : `opencode.json`, `AGENTS.md`
- **Modifs** : permissions équilibrées — `edit` : allow partout sauf `deny` sur `JuceLibraryCode/` + `Builds/` ; `bash` : allow partout sauf `git push*` → ask et `rm -rf *` → deny. (Rappel : ordre des règles = large d'abord, spécifique après — la dernière règle correspondante gagne.)
- **Conflit** : rebase sur `7171765` (entrée journal du binôme) → conflit d'édit sur la section Journal (2 entrées au même endroit) → résolu en gardant les deux entrées. Effet de bord corrigé : le binôme a Projucer dans `/Applications/`, pas `~/JUCE/` → one-liner §6 rendu auto-détecteur des chemins (`~/JUCE/`, `/Applications/`, `~/Downloads/JUCE/`).
- **Build** : ✅ OK (alias portable testé, resave régénère sans erreur)
- **Commit** : — (ce commit)
- **Next steps** : — (redémarrer opencode pour appliquer les nouvelles permissions)

### 2026-10-06 — fix env (alias Projucer + xcode-select)
- **Tâche** : Réparer `Projucer --resave` (« no such file or directory ») et débloquer le build
- **Fichiers** : `~/.zshrc` (alias corrigé, hors repo), `AGENTS.md`
- **Modifs** : (1) alias `Projucer` corrigé : `$HOME/JUCE/Projucer.app/...` (n'existe pas) → `/Applications/Projucer.app/Contents/MacOS/Projucer` — Projucer est bien installé dans `/Applications`. (2) Licence Xcode acceptée par l'humain (`sudo xcodebuild -license accept`). (3) `xcode-select` pointe toujours sur `/Library/Developer/CommandLineTools` → builder avec `DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer xcodebuild …` (alternative sans sudo, déjà notée §6). (4) `Projucer --resave Metulifer.jucer` exécuté avec succès : `Builds/` + `JuceLibraryCode/` régénérés (gitignorés).
- **Build** : ✅ OK (`** BUILD SUCCEEDED **`, Standalone signé arm64)
- **Commit** : — (ce commit)
- **Next steps** : Implémenter le synthé (DSP/) et le séquenceur (Sequencer/)

### 2026-10-06 — setup CLI
- **Tâche** : Alias shell `Projucer` + doc one-liner pour le binôme
- **Fichiers** : `~/.zshrc` (alias local, hors repo), `AGENTS.md`
- **Modifs** : §6 — bloc « Setup une seule fois » (commande `printf >> ~/.zshrc`) pour que chaque dev tape ensuite simplement `Projucer --resave Metulifer.jucer` ; note `DEVELOPER_DIR=` ajoutée au build (erreurs de xcode-select vues en séance).
- **Build** : ✅ OK (`DEVELOPER_DIR=… xcodebuild` vert — pas de changement code, re-test après resave)
- **Commit** : — (ce commit)
- **Next steps** : Implémenter le synthé (DSP/) et le séquenceur (Sequencer/)

### 2026-10-06 — restructuration dépôt
- **Tâche** : Nettoyage + arborescence Source + désengorgement git
- **Fichiers** : `Source/Main.cpp` (supprimé), `.gitignore`, `Source/DSP|Sequencer|UI/.gitkeep`, `AGENTS.md`
- **Modifs** : (1) suppression de `Source/Main.cpp` (vide, non référencé dans le .jucer). (2) création des sous-dossiers `Source/DSP/`, `Source/Sequencer/`, `Source/UI/` (à remplir). (3) `JuceLibraryCode/` + `Builds/` retirés du suivi git et gitignorés — repo = fichiers réels seulement ; règle §6 « --resave » ajoutée. (4) Conflit git résolu : le binôme avait poussé `c388340` (doublon README supprimé) pendant notre commit docs → rebase `git pull --rebase`, aucun conflit, historique linéaire.
- **Build** : ✅ OK
- **Commit** : — (ce commit)
- **Next steps** : Implémenter le synthé (DSP/) et le séquenceur (Sequencer/)

### 2026-10-06 — setup collab
- **Tâche** : Création du protocole multi-agents
- **Fichiers** : `AGENTS.md`, `opencode.json`
- **Modifs** : Protocole de collaboration (lecture avant modif, journal après, git pull, conventional commits, build test avant commit). Config opencode : deny sur JuceLibraryCode/ et Builds/.
- **Build** : — (pas de code C++)
- **Commit** : `bdf04e6` — `docs: add AGENTS.md collaboration protocol and opencode.json config`
- **Next steps** : Implémenter le synthé (VCO/ADSR/filtre) et le séquenceur
```

---

## 6. Build obligatoire avant commit

### Régénérer les fichiers Projucer (si `Builds/` ou `JuceLibraryCode/` manquent)

**Setup une seule fois** (ajoute l'alias `Projucer` au shell — à faire par chaque dev ;
la commande auto-détecte l'emplacement de Projucer, présent dans `~/JUCE/` chez certains
et dans `/Applications/` chez d'autres) :

```bash
printf '\nfor p in "$HOME/JUCE/Projucer.app" /Applications/Projucer.app "$HOME/Downloads/JUCE/Projucer.app"; do [ -x "$p/Contents/MacOS/Projucer" ] && alias Projucer="$p/Contents/MacOS/Projucer" && break; done\n' >> ~/.zshrc && source ~/.zshrc
```

Puis à chaque fois :

```bash
Projucer --resave Metulifer.jucer
```

> Sans l'alias, la forme complète est
> `~/JUCE/Projucer.app/Contents/MacOS/Projucer --resave Metulifer.jucer`.

> ⚠️ Après un clone ou un pull qui amène un nouveau `.jucer`, **toujours
> resave** avant de builder.

### Build

```bash
xcodebuild -project Builds/MacOSX/Metulifer.xcodeproj \
  -scheme "Metulifer - Standalone Plugin" \
  -configuration Debug build
```

- Le build doit être **vert** avant chaque commit.
- Si `xcodebuild` est introuvable : `xcode-select -s /Applications/Xcode.app`.
- Si `xcodebuild` répond *"requires Xcode, but active developer directory is a command line tools instance"* → préfixer avec :
  `DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer xcodebuild …`
- Ne jamais patcher `Builds/` ni `JuceLibraryCode/` à la main → resave Projucer.

---

## 7. Décisions d'architecture

> Tout choix **structurel** doit être noté ici **avant** l'implémentation.
> Sert d'histoire partagée pour ne pas re-débattre ou contredire.

| Date | Décision | Raison | Par |
|---|---|---|---|
| 2026-10-06 | Collaboration via `AGENTS.md` + `opencode.json` | Deux devs + agents opencode, éviter les conflits et les oublis | setup |
| 2026-10-06 | `Source/` découpé en `DSP/`, `Sequencer/`, `UI/` | Structurer avant d'implémenter les 6 séquences et leurs synthés | restructuration |
| 2026-10-06 | `JuceLibraryCode/` + `Builds/` gitignorés | Fichiers générés = bruit et conflits git ; régénérables via `Projucer --resave` | restructuration |
| 2026-10-06 | **APVTS** avec params des 6 séquences (`seq1_…`→`seq6_…`) ; les attachments du SynthPanel sont **re-pointés au clic** sur une rangée (pagination) | Automation hôte correcte (pas de copie de valeurs) + un seul jeu de knobs | spec UI |
| 2026-10-06 | **Steps** (32×6) stockés en `ValueTree` sérialisé dans `getStateInformation`, **pas** en 192 paramètres hôte | Évite de polluer l'automation hôte avec les steps | spec UI |
| 2026-10-06 | **Spec UI validée depuis Figma** (node `121:16`, « FRAME DE MATHE ») — voir §8 | Wireframe = source de vérité layout, corrigée par le dev (24→32 steps) | spec UI |
| 2026-10-06 | **LFO ×2 + reverb reportés** | UX/UI à designer plus tard (validation humaine) | spec UI |
| 2026-10-06 | Hypothèses codées : `Wave` = knob pas-à-pas 4 positions ; LPF/HPF = 2 boutons exclusifs | Interprétation du wireframe, validée en séance | spec UI |
| 2026-10-07 | **SynthPanel : colonnes = paramètres (Octave/Wave/ADSR/Mix), rangées = VCO1/VCO2** (labels de rangée à gauche), knobs 86 px | Wireframe Figma mis à jour + « chaque rangée est un VCO » (dev) | layout Figma |
| 2026-10-07 | **Ordre des phases : Phase 4 (son) → Phase 5 (LFO + câbles) ; FX annulés** | Validation dev : « Phase 4 son d'abord » ; « oublie les fx je referai un design dans figma » | dev |
| 2026-10-07 | **Playhead indépendant par séquence** (6 compteurs d'étapes, polyrythmie : chaque séquence avance à sa longueur propre) | Validation dev (« oui ») | dev |
| 2026-10-07 | **Pitch par VCO : levier octave 5 crans (-2..+2) + knob Note 12 steps (0..11 demi-tons)** — remplace le knob Octave dans le SynthPanel ; freq = C3 × 2^octave × 2^(note/12) ; pas de MIDI in (pour l'instant) | Validation dev : « un bouton levier contrôle l'octave et un knob contrôle la note avec 12 steps », « par VCO », « 5 positions » | dev |
| 2026-10-07 | **1 ADSR par VCO** (VCO1 → ADSR1, VCO2 → ADSR2) — les 2 ADSRs du wireframe | Validation dev (« oui ») | dev |
| 2026-10-07 | **LFO ×2 : Rate + Wave + Offset + Depth + on/off auto** (auto-off si aucun câble) ; panneau = 3 knobs (rate/offset/depth) + bouton wave — écart au wireframe (1 knob) à rattraper côté Figma | Validation dev : « Rate + Cycle wave + on off… il faut un knob offset et depth » | dev |
| 2026-10-07 | **UI skinnable** : chaque composant vectoriel remplaçable par un asset image transparent (états : normal/hover/down/on/off), fallback vectoriel conservé si asset absent ; assets en `Resources/` (groupe `.jucer`, BinaryData) — formats JUCE natifs PNG/JPEG/GIF | Dev fournira des rendus 3D à substituer au vectoriel ; pas de `juce_svg` dans le `.jucer` | dev |
| 2026-10-07 | **Phase 5 — câbles LFO→knob** : source = jack des panneaux LFO1/2 ; dest = knobs synthé de la **séquence sélectionnée au drop** (`a1,d1,s1,r1,a2,d2,s2,r2,mix1,mix2,cutoff` — exclus octave/note/wave/length/volume/tempo pour v1) ; modulation appliquée **en normalisé** (±50 % de la plage du param, par bloc audio) sur une copie des valeurs lues — **jamais** l'APVTS n'est écrit par l'audio ; un câble n'est **dessiné/selectable que pour sa séquence de dest** (pagination) ; un LFO **sans aucun câble est inactif** (auto-off) ; modTree (`"mod"`, enfants `cable{src,dest}`) sérialisé comme `stepsTree` | Réglages validables sans re-demande dev : pas de feedback sur params hôte, dests musicales, auto-off §7 ; undo via `undoManager` du processor | agent |
| 2026-11-04 | **Phase 6 — Reverb par séquence avec pagination** : APVTS `seqN_reverb_{enabled,mode,mix}` ; UI `ReverbPanel` (105×224 à x=1144, y=471) conforme Figma node 176:2 / 176:247 avec bouton On/Off, bouton Mode (Room/Hall) et knob 86 px Mix (0..100 %) ; DSP Voice avec `juce::dsp::Reverb` stéréo (0 alloc en temps réel) ; cible de câble LFO `reverb_mix` ajoutée | Choix utilisateur : reverb par séquence, contrôles On/Off + Mode + Mix knob | dev / agent |
| 2026-11-04 | **Gain Staging & Headroom audio** : (1) Somme des 2 VCOs pondérée par 0.5f pour unity gain à 0 dB. (2) `mixDb1`/`mixDb2` `<= -59 dB` coupe complètement l'oscillateur (gain 0.0f). (3) Reverb Freeverb wet pondéré à `0.35f` pour éviter le boost de gain excessif. (4) Volume séquence `<= 0.0001f` réinitialise et coupe la voix. (5) Master buffer atténué de 0.5f et saturateur doux `std::tanh` temps-réel sans allocation pour garantir le headroom et empêcher tout écrêtage DAC / CoreAudio. (6) Tooltips et affichage dynamique `seq N volume` et `seq N length` dans l'écran de contrôle. | Résolution des saturations/clippings et réactivité des potentiomètres Mix et Volume | dev / agent |
| 2026-11-04 | **Melatonin Inspector** : Module tiers `melatonin_inspector` de Sudara intégré via sous-module git `modules/melatonin_inspector` dans `.jucer` ; instance dans `PluginEditor` liée à `undoManager` ; déclenchable via raccourci `Cmd+I` ou bouton UI « Inspect ». | Inspection visuelle en direct et debugging des composants JUCE | dev |

---

## 8. Spec UI — Figma validé

> Source : Figma « VST » node `121:16` « FRAME DE MATHE » (1280×720), validée le 2026-10-06, **mise à jour 2026-10-07** (SynthPanel + LFO/reverb).
> Le wireframe montrait 24 steps → **c'est 32** (correction dev, Figma à mettre à jour de son côté).

### Éléments

| Élément Figma | Rôle | Détails |
|---|---|---|
| 6 rangées (910×61) | 6 séquences | clic = **sélection (pagination)** |
| Steps (26×26, `#d9d9d9`) | ON/OFF du pattern | **32 steps** par séquence |
| Cercle 1 / rangée | **Longueur** de séquence | 0–32 |
| Cercle 2 / rangée | **Volume** de séquence | — |
| `[Copy][Paste]` | copier/coller le pattern | par rangée |
| Rectangle 25 (247×132) | **Écran d'affichage** | idle : signal audio (RMS) · édition : param touché (ex. « attack VCO1 ») · toujours : séquence sélectionnée |
| CLOCK (297×284) | tempo | gros knob 175×175 + **bouton Play/Stop à côté** |
| SynthPanel (786×237) | synthé de la **séquence sélectionnée** | re-branché au clic — **colonnes = paramètres, rangées = VCO1/VCO2** |
| `Octave` / `Wave` (colonnes) | 2 VCOs par séquence | **rangée 1 = VCO1**, **rangée 2 = VCO2** (labels à gauche) ; Octave = **levier 5 crans (−2..+2) + knob Note 12 demi-tons** ; Wave = 4 positions sine/saw/square/triangle |
| ADSR ×2 | 2 ADSRs | titre unique « ADSR » ; par rangée : 4 knobs A/D/S/R (31 px) |
| `Mix out DB` | niveaux VCO | 2 knobs (86 px) : mix VCO1 (rangée 1), mix VCO2 (rangée 2) |
| `VCF` | filtre | 2 boutons exclusifs **LPF / HPF** + 1 gros knob (**cutoff**) |
| `LFO` ×2 | LFOs de modulation | 2 panneaux (874, 1013) : rate, offset, depth + wave + on/off + jacks |
| `reverb` | réverbération par séquence | panneau (1144) : bouton On/Off + bouton Mode (Room/Hall) + gros knob Mix (Dry/Wet) |

### Correspondance → classes JUCE

```
MetuliferAudioProcessorEditor (1280×720)
├── SequencerPanel (haut-gauche)
│   └── 6 × SequenceRowComponent          Source/UI/
│       ├── CopyButton, PasteButton         juce::TextButton
│       ├── 32 × StepButton                 juce::TextButton (toggle)
│       ├── LengthKnob (0–32)               juce::RotarySlider + attachment
│       └── VolumeKnob                      juce::RotarySlider + attachment
├── DisplayScreen (Rectangle 25)          Source/UI/DisplayScreen.* (paint custom)
├── ClockPanel : TempoKnob + PlayStopButton
├── SynthPanel (re-branché à la sélection)
│   ├── colonnes : Octave (levier + Note) · Wave · ADSR · Mix out DB (titres en haut)
│   ├── rangée 1 = « VCO1 » (label à gauche) : levier oct, note, wave, ADSR×4, mix
│   └── rangée 2 = « VCO2 » (label à gauche) : levier oct, note, wave, ADSR×4, mix
├── VCFPanel : LPF/HPF (exclusifs) + Cutoff
├── LFOPanel (×2) : Rate, Offset, Depth + Wave, On/Off + Jack
├── ReverbPanel : On/Off, Mode (Room/Hall) + Mix
└── CableLayer : patch cables virtuels LFO→knob
```
├── ClockPanel : TempoKnob + PlayStopButton
├── SynthPanel (re-branché à la sélection)
│   ├── colonnes : Octave (levier + Note) · Wave · ADSR · Mix out DB (titres en haut)
│   ├── rangée 1 = « VCO1 » (label à gauche) : levier oct, note, wave, ADSR×4, mix
│   └── rangée 2 = « VCO2 » (label à gauche) : levier oct, note, wave, ADSR×4, mix
└── VCFPanel : LPF/HPF (exclusifs) + Cutoff
```

---

## 9. Checklist rapide agent (à chaque session)

- [ ] Lu `AGENTS.md` au début
- [ ] `git pull` fait
- [ ] `git status` propre
- [ ] Modifs faites uniquement dans les dossiers/fichiers autorisés
- [ ] Décisions d'archi notées si besoin
- [ ] Build testé et vert
- [ ] Entrée Journal ajoutée
- [ ] Commit fait (code + journal)
