'use client';

import { useEffect } from 'react';

/**
 * Le layout racine est partagé par toutes les langues (export statique),
 * on aligne donc l'attribut `lang` du document sur la langue de l'URL.
 */
export function LangSync({ lang }: { lang: string }) {
  useEffect(() => {
    document.documentElement.lang = lang;
  }, [lang]);

  return null;
}
