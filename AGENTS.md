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

```
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
| `Octave` / `Wave` (colonnes, knobs 86 px) | 2 VCOs par séquence | **rangée 1 = VCO1**, **rangée 2 = VCO2** (labels à gauche) ; Wave = 4 positions sine/saw/square/triangle |
| ADSR ×2 | 2 ADSRs | titre unique « ADSR » ; par rangée : 4 knobs A/D/S/R (31 px) |
| `Mix out DB` | niveaux VCO | 2 knobs (86 px) : mix VCO1 (rangée 1), mix VCO2 (rangée 2) |
| `VCF` | filtre | 2 boutons exclusifs **LPF / HPF** + 1 gros knob (**cutoff**) |
| `LFO` ×2 + `reverb` | **hors périmètre** | **apparus dans le wireframe 2026-10-07** (structure VCF) — implémentation reportée (sémantique à valider) |

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
│   ├── colonnes : Octave · Wave · ADSR · Mix out DB (titres en haut)
│   ├── rangée 1 = « VCO1 » (label à gauche) : oct, wave, ADSR×4, mix
│   └── rangée 2 = « VCO2 » (label à gauche) : oct, wave, ADSR×4, mix
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
