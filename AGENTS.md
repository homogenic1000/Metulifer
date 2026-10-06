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

**Setup une seule fois** (ajoute l'alias `Projucer` au shell — à faire par chaque dev) :

```bash
printf '\nalias Projucer="$HOME/JUCE/Projucer.app/Contents/MacOS/Projucer"\n' >> ~/.zshrc && source ~/.zshrc
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
| _à remplir_ | APVTS pour les paramètres | Pattern standard JUCE, UI↔DSP propre | — |
| _à remplir_ | Structure classes (Voice, Sequencer, …) | À définir ensemble avant coding | — |

---

## 8. Checklist rapide agent (à chaque session)

- [ ] Lu `AGENTS.md` au début
- [ ] `git pull` fait
- [ ] `git status` propre
- [ ] Modifs faites uniquement dans les dossiers/fichiers autorisés
- [ ] Décisions d'archi notées si besoin
- [ ] Build testé et vert
- [ ] Entrée Journal ajoutée
- [ ] Commit fait (code + journal)
