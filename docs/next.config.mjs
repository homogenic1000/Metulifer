import { createMDX } from 'fumadocs-mdx/next';

const withMDX = createMDX();

// Déployé sur https://<user>.github.io/Metulifer -> NEXT_PUBLIC_BASE_PATH=/Metulifer
// En local (ou sur un domaine racine), laisser vide.
const basePath = process.env.NEXT_PUBLIC_BASE_PATH ?? '';

/** @type {import('next').NextConfig} */
const config = {
  reactStrictMode: true,
  // Site 100 % statique : aucun serveur Node requis.
  output: 'export',
  trailingSlash: true,
  images: { unoptimized: true },
  ...(basePath ? { basePath, assetPrefix: basePath } : {}),
};

export default withMDX(config);
