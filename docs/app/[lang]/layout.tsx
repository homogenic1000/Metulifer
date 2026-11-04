import type { ReactNode } from 'react';
import { RootProvider } from 'fumadocs-ui/provider/next';
import { i18nProvider } from 'fumadocs-ui/i18n';
import { translations } from '@/lib/layout.shared';
import { LangSync } from '@/components/lang-sync';

const basePath = process.env.NEXT_PUBLIC_BASE_PATH ?? '';

export default async function LangLayout({
  params,
  children,
}: {
  params: Promise<{ lang: string }>;
  children: ReactNode;
}) {
  const { lang } = await params;

  return (
    <RootProvider
      i18n={i18nProvider(translations, lang)}
      search={{
        options: {
          type: 'static',
          api: `${basePath}/api/search`,
        },
      }}
    >
      <LangSync lang={lang} />
      {children}
    </RootProvider>
  );
}
