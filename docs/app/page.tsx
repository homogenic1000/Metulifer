import Link from 'next/link';
import type { Metadata } from 'next';

export const metadata: Metadata = {
  title: 'Metulifer — documentation',
};

const content = {
  fr: {
    lang: 'Français',
    title: 'Metulifer',
    subtitle: 'Séquenceur modulaire à pas & double synthé',
    description:
      'Six séquences indépendantes, chacune avec son propre synthé : 2 VCO, 2 ADSR, un filtre, une reverb et deux LFO câblables.',
    cta: 'Lire la documentation',
    alt: 'Read the documentation in English',
    altHref: '/en/docs',
    cards: [
      ['Démarrer', 'Prérequis, build Projucer/Xcode et premier son en cinq minutes.'],
      ['Guide utilisateur', 'Séquenceur, synthé, filtre, reverb, LFO et câbles de modulation.'],
      ['Architecture', 'Processor/Editor, APVTS, moteur de séquence, DSP et persistance.'],
    ],
  },
  en: {
    lang: 'English',
    title: 'Metulifer',
    subtitle: 'Modular step sequencer & dual synth',
    description:
      'Six independent sequences, each with its own synth: 2 VCOs, 2 ADSRs, a filter, a reverb and two patchable LFOs.',
    cta: 'Read the documentation',
    alt: 'Lire la documentation en français',
    altHref: '/fr/docs',
    cards: [
      ['Get started', 'Requirements, Projucer/Xcode build and your first sound in five minutes.'],
      ['User guide', 'Sequencer, synth, filter, reverb, LFO and modulation cables.'],
      ['Architecture', 'Processor/Editor, APVTS, sequencer engine, DSP and state.'],
    ],
  },
} as const;

export default function LandingPage() {
  const copy = content.fr;

  return (
    <main className="mx-auto flex w-full max-w-5xl flex-1 flex-col justify-center gap-10 px-6 py-16">
      <header className="space-y-4">
        <p className="text-fd-primary text-xs font-semibold uppercase tracking-[0.2em]">
          Saumure · JUCE audio plugin
        </p>
        <h1 className="text-4xl font-bold tracking-tight sm:text-6xl">{copy.title}</h1>
        <p className="text-fd-muted-foreground text-lg">{copy.subtitle}</p>
        <p className="max-w-2xl text-fd-muted-foreground">{copy.description}</p>
      </header>

      <div className="flex flex-wrap items-center gap-4">
        <Link
          href="/fr/docs"
          className="bg-fd-primary text-fd-primary-foreground rounded-lg px-5 py-3 text-sm font-semibold transition hover:opacity-90"
        >
          {copy.cta} (FR)
        </Link>
        <Link
          href="/en/docs"
          className="border-fd-border hover:bg-fd-accent rounded-lg border px-5 py-3 text-sm font-semibold transition"
        >
          Read the docs (EN)
        </Link>
      </div>

      <ul className="grid gap-4 sm:grid-cols-3">
        {copy.cards.map(([title, text]) => (
          <li key={title} className="border-fd-border bg-fd-card rounded-xl border p-5">
            <h2 className="mb-2 text-sm font-semibold">{title}</h2>
            <p className="text-fd-muted-foreground text-sm leading-relaxed">{text}</p>
          </li>
        ))}
      </ul>

      <footer className="text-fd-muted-foreground flex flex-wrap gap-4 text-xs">
        <span>Metulifer · VST3 / AU / Standalone · macOS 12+</span>
        <Link href={content.en.altHref} className="underline underline-offset-4">
          {content.en.alt}
        </Link>
      </footer>
    </main>
  );
}
