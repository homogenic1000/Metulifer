import type { ReactNode } from 'react';
import type { Metadata } from 'next';
import './global.css';

export const metadata: Metadata = {
  title: {
    default: 'Metulifer',
    template: '%s · Metulifer',
  },
  description:
    'Metulifer — séquenceur numérique 6 pistes avec synthé dédié par séquence (2 VCO, 2 ADSR, filtre, reverb). Plugin audio JUCE par Saumure.',
};

export default function RootLayout({ children }: { children: ReactNode }) {
  return (
    <html lang="fr" suppressHydrationWarning>
      <body className="flex min-h-screen flex-col antialiased">{children}</body>
    </html>
  );
}
