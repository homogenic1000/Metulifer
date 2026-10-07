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

---

## 8. Spec UI — Figma validé

> Source : Figma « VST » node `121:16` « FRAME DE MATHE » (1280×720), validée le 2026-10-06.
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
| SynthPanel (786×237) | synthé de la **séquence sélectionnée** | re-branché au clic |
| VCO1 / VCO2 | 2 VCO par séquence | chacun : knob **Octave** + knob **Wave** (4 positions : sine/saw/square/triangle) |
| ADSR ×2 | 2 ADSRs | 4 knobs chacun (A/D/S/R), taille 31px |
| `Mix out DB` | niveaux VCO | 2 knobs = **volume individuel VCO1 / VCO2** |
| `VCF` | filtre | 2 boutons exclusifs **LPF / HPF** + 1 gros knob (**cutoff**) |
| `LFO` ×2 + `reverb` | **hors périmètre** | à designer plus tard |

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
│   ├── VCO1 : Octave, Wave   ├── VCO2 : Octave, Wave
│   ├── ADSR1 ×4              ├── ADSR2 ×4
│   └── Mix : VCO1, VCO2
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
