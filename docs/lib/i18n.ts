import { defineI18n } from 'fumadocs-core/i18n';

/**
 * Metulifer docs i18n.
 * Les deux langues sont toujours préfixées : /fr/docs/... et /en/docs/...
 * (`hideLocale: 'never'`) — compatible export statique, sans middleware.
 */
export const i18n = defineI18n({
  defaultLanguage: 'fr',
  languages: ['fr', 'en'],
  hideLocale: 'never',
  parser: 'dir',
});
