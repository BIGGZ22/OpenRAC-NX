# Vague N — <chantier> (ROOT, <date>)

> Ancre de la vague. État général : `docs/STATE.md`. Départ de la vague :
> `master` = `<commit>`.

## Départ

- Ce qui est déjà apparié, avec ses chiffres exacts.
- Ce qui reste ouvert, et d'où vient la file de la vague.

## Lot fondateur E0 — livré

- Modules extraits, empreintes, manifestes.
- Bornes : une entrée bornée par une borne, avec la preuve de la borne.
- Shortlist : `module, offset, taille, source de la borne, sha du corps
  brut, modules partagés, placements partagés`, **classée par rendement**
  (nombre de placements qui portent le même corps brut).
- Outillage : le harnais par fonction, et sa preuve qu'il sait dire non.

## Lots de traduction (N)

| lot | modules | cibles |
|---|---|---|
| L01 | | |

Brief : `work/templates/BRIEF-COMMUN.md`. Listes : `work/lots/L*.tsv`.

## Intégration (ROOT)

1. Recopier les corps exacts de `work/<LOT>/bodies/` vers `src/`.
2. Recalculer les empreintes, écrire la table des placements.
3. Lancer la porte du programme complet, puis les tests et le garde du dépôt.
4. Un seul écrivain dans le clone : le ROOT seul.

## Reprise immédiate si interruption

1. `ls work/*/RAPPORT.md`, `cat work/*/resultats.tsv`.
2. Un corps exact → `work/<LOT>/bodies/` ; le revérifier avant d'intégrer.
3. Ne pas relancer E0 : modules, bornes et harnais sont livrés.
