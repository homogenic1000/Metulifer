import { createFromSource } from 'fumadocs-core/search/server';
import { source } from '@/lib/source';

// Index de recherche pré-rendu : la recherche s'exécute dans le navigateur,
// aucun serveur Node n'est nécessaire (export statique).
export const revalidate = false;

export const { staticGET: GET } = createFromSource(source);
