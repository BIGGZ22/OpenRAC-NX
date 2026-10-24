# BRIEF COMMUN — vague N, <chantier> (<date>)

À lire en entier avant toute action. Commun à tous les agents de la vague.

## 1. Contexte

Ce que la vague doit produire, en une phrase. Ce qui est déjà apparié, et
ce qui reste ouvert.

## 2. Où est quoi

| Élément | Chemin | Rôle |
|---|---|---|
| Dépôt | ce dépôt | **LECTURE SEULE** pour un agent de lot |
| Cible locale | `LOCAL.md` | l'image, les empreintes, la chaîne |
| Modules retail | `work/E0/modules/` | la cible extraite |
| Bornes | `work/E0/bounds/` | les candidats bornés |
| Harnais | `work/E0/harness.py` | compile, lie, compare au retail |
| Ta liste | `work/lots/<LOT>.tsv` | tes cibles |
| **Ton dossier** | `work/<LOT>/` | **tu n'écris que là** |

## 3. Harnais

```
python3 work/E0/harness.py <MODULE> <OFFSET> <src.c> [--size N]
```

La comparaison octet pour octet du corps compilé avec les octets retail est
la **seule preuve**. Sinon, le harnais affiche la première différence (offset
+ octets) et le profil essayé. Codes de retour : 0 = exact, 1 = mismatch
mesuré, 2 = refus. **Un refus n'est pas un essai.**

## 4. Règles dures

1. **N'écris jamais dans le dépôt** hors de `work/<LOT>/`.
2. **C pur** : pas d'asm inline/global, pas de `.word`, pas de registre
   imposé, pas de barrière.
3. **Pas de pilotage du codegen** : si l'octet ne sort pas, cherche la forme
   authentique de la routine, pas une construction qui déplace un registre.
4. **Ne lance jamais** la porte globale ni un build complet — le ROOT intègre.
5. **Borne ta cible** : l'extent vient de ta liste ; vérifie le retour réel
   avant de conclure.
6. Après trois essais de la même classe d'écart : gare le corps dans
   `abandonnes/` avec la première différence et une condition de réouverture.
7. Aucune promesse : seulement ce que le harnais a imprimé.

## 5. Livrable du lot

```
bodies/<MODULE>_<OFFSET>.c
resultats.tsv        module<TAB>offset<TAB>taille<TAB>profil<TAB>as<TAB>statut
abandonnes/          corps garés + NOTE (1re diff, condition de réouverture)
RAPPORT.md
```

## 6. Rapport final au ROOT

**Français, 20 lignes maximum** : nombre d'exacts, liste
`module:offset taille profil`, quasi-exacts marquants (n/total), blocages,
chemins.
