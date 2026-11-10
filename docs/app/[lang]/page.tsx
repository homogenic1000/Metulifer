import type { Metadata } from 'next';
import { Landing } from '@/components/landing';
import { i18n } from '@/lib/i18n';

export const metadata: Metadata = {
  title: 'Metulifer — documentation',
};

export function generateStaticParams() {
  return i18n.languages.map((lang) => ({ lang }));
}

export default async function LangLandingPage({
  params,
}: {
  params: Promise<{ lang: string }>;
}) {
  const { lang } = await params;

  return <Landing locale={lang === 'en' ? 'en' : 'fr'} />;
}
