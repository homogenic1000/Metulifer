import type { BaseLayoutProps } from 'fumadocs-ui/layouts/shared';
import { uiTranslations } from 'fumadocs-ui/i18n';
import { i18n } from '@/lib/i18n';

/**
 * Traductions de l'interface Fumadocs (recherche, sommaire, thème, pagination…).
 * Aucun pack officiel FR n'existe encore, on fournit les clés à la main.
 */
export const translations = i18n
  .translations()
  .extend(uiTranslations())
  .add({
    fr: {
      displayName: 'Français',
      'Search(search trigger)': 'Rechercher',
      'Search(search dialog)': 'Rechercher dans la documentation',
      'No results found(search dialog)': 'Aucun résultat',
      'Search failed(search dialog)': 'La recherche a échoué',
      'Search failed, showing results for {query}(search dialog)':
        'Recherche échouée, résultats pour {query}',
      'On this page(table of contents)': 'Sur cette page',
      'No Headings(table of contents)': 'Aucun titre',
      'Table of Contents(inline table of contents)': 'Sommaire',
      'Previous Page(pagination)': 'Page précédente',
      'Next Page(pagination)': 'Page suivante',
      'Last updated on(page footer)': 'Dernière mise à jour le',
      'Edit on GitHub(edit page)': 'Modifier sur GitHub',
      'Toggle Theme(theme switcher)(aria-label)': 'Changer de thème',
      'Light(theme switcher)(aria-label)': 'Clair',
      'Dark(theme switcher)(aria-label)': 'Sombre',
      'System(theme switcher)(aria-label)': 'Système',
      'Open Search(search trigger)(aria-label)': 'Ouvrir la recherche',
      'Close Search(search dialog)(aria-label)': 'Fermer la recherche',
      'Choose a language(language switcher)': 'Choisir une langue',
      'Choose a language(language switcher)(aria-label)': 'Choisir une langue',
      'Toggle Menu(mobile menu)(aria-label)': 'Ouvrir le menu',
      'Open Sidebar(sidebar)(aria-label)': 'Ouvrir la barre latérale',
      'Close Sidebar(sidebar)(aria-label)': 'Fermer la barre latérale',
      'Close Sidebar(aria-label)': 'Fermer la barre latérale',
      'Collapse Sidebar(sidebar)(aria-label)': 'Replier la barre latérale',
      'Hide Sidebar(sidebar)': 'Masquer la barre latérale',
      'Show Sidebar(sidebar)': 'Afficher la barre latérale',
      'Copy Text(code block)(aria-label)': 'Copier le code',
      'Copied Text(code block)(aria-label)': 'Code copié',
      'Page Not Found(404 page)': 'Page introuvable',
      'Back to Home(404 page)': "Retour à l'accueil",
      'The page you are looking for might have been removed, had its name changed, or is temporarily unavailable.(404 page)':
        "La page recherchée a peut-être été supprimée, renommée, ou est temporairement indisponible.",
      'Copy Markdown(page actions)': 'Copier le Markdown',
      'Copied Markdown(page actions)': 'Markdown copié',
      'View as Markdown(page actions)': 'Voir en Markdown',
      'Open(page actions)': 'Ouvrir',
      'Layout Tab(layout tab trigger)': 'Onglet de section',
    },
    en: {
      displayName: 'English',
    },
  });

export function baseOptions(locale: string): BaseLayoutProps {
  return {
    nav: {
      title: (
        <span className="font-semibold tracking-tight">
          Metulifer
          <span className="ml-2 text-fd-muted-foreground text-xs font-normal">
            {locale === 'fr' ? 'documentation' : 'docs'}
          </span>
        </span>
      ),
      url: `/${locale}/docs`,
    },
    links: [
      {
        text: locale === 'fr' ? 'Accueil' : 'Home',
        url: `/${locale}`,
      },
    ],
  };
}
