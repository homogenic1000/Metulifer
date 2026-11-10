import type { Metadata } from 'next';
import { Landing } from '@/components/landing';

export const metadata: Metadata = {
  title: 'Metulifer — documentation',
};

export default function LandingPage() {
  return <Landing locale="fr" />;
}
