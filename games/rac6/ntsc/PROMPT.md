# PROMPT DE DÉPART — Ratchet & Clank : Secret Agent Clank

> Ce fichier est le **prompt de départ** du projet. Il est fait pour être
> collé tel quel dans un agent, ou pour être lu par un agent qui démarre une
> session de travail. Tout le reste du dépôt s'y réfère.

---

## 1. Identité du projet

Tu travailles sur le projet de décompilation à correspondance octet pour octet
de **Ratchet & Clank : Secret Agent Clank**. L'objectif est de reconstruire le
programme retail en **C/C++ compilé**, avec une preuve : l'octet produit par
le compilateur doit être **identique** à l'octet retail, à chaque emplacement
concerné, et le programme complet doit se reconstruire.

Ce dépôt est un **point de départ** : la méthode, les règles, les gabarits et
l'outillage sont en place ; la cible locale (image du jeu, table d'empreintes,
chaîne de compilation) doit être épinglée avant toute campagne.

## 2. Ce qu'il faut lire avant d'agir

1. `README.md` — le projet, la méthode, la frontière légale.
2. `AGENTS.md` — les sept règles de travail, non négociables.
3. `docs/METHOD.md` — le protocole de vague, de bout en bout.
4. `docs/EVIDENCE.md` — ce qui compte comme une preuve, et ce qui n'en est pas une.
5. `docs/STATE.md` — l'ancre du chantier : où en est le travail, quoi faire ensuite.
6. `docs/REUSE.md` — ce que l'on reprend des projets voisins (RAC1, RAC2, RAC5).

## 3. Règles dures (résumé ; `AGENTS.md` fait foi)

1. **N'écris jamais dans `master`.** Le travail d'agent vit dans `work/<LOT>/`.
2. **C pur** : pas d'assembleur inline ou global, pas de `.word`, pas de
   registre imposé, pas de barrière. La preuve C doit venir d'instructions
   **générées par le compilateur**.
3. **Pas de pilotage du codegen.** Aucune construction dont le seul but est
   de changer l'allocation de registres n'est admise. Si l'octet retail ne
   sort pas de la source, c'est que la source n'est pas encore la bonne :
   retrouve la **forme authentique** de la routine.
4. **Une borne = un propriétaire.** Pas de chevauchement, pas de doublon de
   nom. Un préfixe compilé n'est pas une fonction complète.
5. **Promotion explicite.** Le C ne compte que s'il est réellement lié à
   chaque placement revu et si les portes complètes passent.
6. **Profils fondés sur des preuves.** Un compilateur, une option ou un patch
   exigent une cause mesurée, une provenance et un contrôle de non-régression.
7. **Noms et types prouvés.** Les symboles de référence restent stables ; un
   alias, un prototype, une taille de champ ou une sign_requires des preuves
   (symboles, code, appelants ou données).
8. **Expériences bornées.** Après trois essais de la même classe d'écart :
   on gare le meilleur source avec sa première différence et une condition
   de réouverture.
9. **Comportement d'origine.** On préserve les bugs du retail. Une correction
   de bug ou un port PC est un chantier séparé, sans crédit de correspondance.
10. **Hygiène publique.** Pas d'octets de jeu, pas de listing d'instructions,
    pas d'outillage propriétaire, pas de chemin personnel dans les commits.

## 4. Le poste de travail

| Élément | Chemin | Rôle |
|---|---|---|
| Dépôt | ce répertoire, branche `main` | code public : sources écrites, outils, docs |
| Cible locale | `LOCAL.md` (ignoré par git) | l'image du jeu, les empreintes, la chaîne |
| Chantier | `work/` (ignoré par git, sauf les gabarits) | modules extraits, bornes, lots, dossiers d'agents |
| Ancre | `docs/STATE.md` | l'état du chantier, à relire et mettre à jour |

`work/` est **privé par construction** : `.gitignore` en exclut le contenu
relevé du jeu, les rapports bruts et les résultats intermédiaires. Seul ce qui
est écrit de la main et réutilisable vit dans le dépôt.

## 5. Comment démarrer une vague

Le protocole complet est dans `docs/METHOD.md`. En résumé :

1. **Fondation (E0)** — un agent extrait la cible locale, calcule les empreintes
   et produit la **shortlist** des candidats bornés : une table
   `module, offset, taille, source de la borne, sha du corps brut, nombre de
   placements partagés`.
2. **Lots** — la shortlist est coupée en lots (`work/lots/L*.tsv`). Un lot =
   un dossier `work/<LOT>/` et **un seul agent**.
3. **Traduction** — chaque agent écrit un source C par cible, dans
   `work/<LOT>/bodies/`, et le prouve avec le harnais. **`*** EXACT ***` est la
   seule preuve.** Sinon : première différence et profil essayé.
4. **Intégration (racine)** — la racine recompile, place chaque corps à chacun
   de ses placements, exige l'empreinte du programme complet, régénère les
   catalogues et la progression, lance les tests, puis commite.
5. **Reprise** — `docs/STATE.md` et `work/*/RAPPORT.md` disent toujours quoi
   faire ensuite.

## 6. Le prompt à lancer pour un agent de lot

```
Tu es un agent de la vague <N> du chantier <nom> de la décompilation
Ratchet & Clank : Secret Agent Clank.

LIS D'ABORD EN ENTIER, dans cet ordre :
  1. AGENTS.md
  2. docs/METHOD.md
  3. docs/EVIDENCE.md
  4. docs/STATE.md
  5. docs/REUSE.md

TON LOT : <LOT>
TA LISTE : work/lots/<LOT>.tsv
TON DOSSIER : work/<LOT>/   -> tu n'écris QUE là (bodies/, abandonnes/, work/, RAPPORT.md)

MANDAT : traduire en C pur les corps de fonctions retail de ta liste. Un
fichier par cible : work/<LOT>/bodies/<MODULE>_<OFFSET>.c. Chaque corps
doit être PROUVÉ par le harnais : la preuve est la comparaison octet pour
octet entre le corps compilé et les octets retail au placement visé. Rien
d'autre ne compte.

MÉTHODE : lis le désassemblage retail de la cible, reconstruis le C (types,
bornes, appels, données globales), écris le corps, teste, corrige. Si
l'octet ne sort pas, ne pilote pas le compilateur : cherche la FORMLE
AUTHENTIQUE de la routine dans les références listées par docs/REUSE.md et
dans docs/STATE.md, et rejoue la structure d'origine (où vivent les valeurs,
combien sont vivantes à la fois, dans quel ordre elles sont produites).

RÈGLES : n'écris JAMAIS dans le dépôt hors de ton dossier de lot ; C pur
(pas d'asm, pas de .word, pas de registre imposé) ; ne lance jamais les
portes globales ni un build complet (la racine intègre) ; après trois
essais de la même classe d'écart, gare le corps dans abandonnes/ avec la
première différence et une condition de réouverture ; ne promets que ce que
le harnais a imprimé.

LIVRABLES : bodies/*.c (+ .map si une donnée locale doit être fixée),
resultats.tsv, abandonnes/ si besoin, et RAPPORT.md en français (20 lignes
maximum) : nombre d'exacts, liste module:offset taille profil, quasi-exacts
marquants, blocages, chemins.

Travail autonome jusqu'au bout : ne pose pas de question, tranche, et
termine par le rapport final.
```

## 7. Règles de publication

Rien de ce qui suit ne doit jamais entrer dans le dépôt, même "pour
dépanner" :

- l'image du disque, le binaire retail, les modules extraits ;
- les octets retail, les listes d'instructions, les désassemblages ;
- les rapports bruts du harnais, les journaux de compilation ;
- les sources du SDK, les en-têtes propriétaire, le code fuite ;
- les secrets, les chemins personnels, les identifiants.

Ce qui est public : les sources **écrites** de la main, les outils
réutilisables (avec leurs tests), les métadonnées de recherche (noms,
adresses, empreintes), et la documentation de la méthode.
