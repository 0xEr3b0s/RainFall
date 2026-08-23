# Rainfall

> Un projet d'exploitation binaire du cursus Cybersécurité de 42.

## Présentation

**Rainfall** est un exercice dans l'art de l'exploitation logicielle. Il expose
une succession de programmes vulnérables — des binaires C et C++ compilés — dont
chacun doit être compris, disséqué, puis finalement subverti. La tâche n'a rien
de purement théorique : chaque niveau est une porte close, et la clef de chacune
se trouve au niveau qui le précède.

Le projet met à disposition une machine virtuelle dédiée, sur laquelle réside une
série d'utilisateurs, de `level0` jusqu'à `level9`, accompagnés de plusieurs
étapes `bonus`. Chaque utilisateur possède un **binaire SUID** — un programme qui
s'exécute avec les privilèges de son propriétaire plutôt qu'avec ceux de
l'appelant. En découvrant puis en exploitant une faille au sein d'un binaire
donné, l'on obtient un interpréteur revêtant l'identité de l'utilisateur suivant,
et l'on peut dès lors lire le mot de passe qui ouvre le passage vers le niveau
ultérieur.

La progression est ainsi une chaîne : la compréhension procure l'accès, l'accès
procure le secret suivant, et ce secret procure le défi d'après — chaque échelon
plus exigeant que le précédent.

## Objectifs

Le projet est conçu pour cultiver, chez le praticien, une intuition de la manière
dont les programmes défaillent, ainsi que la discipline nécessaire pour tourner
cette défaillance à son avantage. Au fil de l'exercice, l'on apprend à :

- Lire et interpréter des binaires compilés au niveau de l'assembleur, en
  l'absence de toute source.
- Reconnaître, dans le code natif, les signatures des familles classiques de
  vulnérabilités.
- Raisonner sur l'agencement de la mémoire — la pile, le tas, et les segments
  intermédiaires.
- Façonner des entrées précises qui détournent l'exécution d'un programme vers une
  fin de son propre choix.
- Manier avec aisance les instruments consacrés de l'analyse statique et
  dynamique.

## Notions abordées

À travers ses niveaux, Rainfall parcourt un vaste territoire de techniques
d'exploitation, notamment mais non exclusivement :

- **Les débordements de tampon** — le dépassement d'une allocation de taille fixe
  et la corruption de la mémoire adjacente.
- **Les vulnérabilités de chaîne de format** — l'abus d'un spécificateur de
  conversion non gardé pour lire et écrire à des adresses arbitraires.
- **Le contrôle de l'adresse de retour** — la mainmise sur le retour d'une
  fonction et la redirection du flux d'exécution.
- **Le retour à la libc et la réutilisation de code** — la composition d'un
  exploit à partir de la machinerie déjà présente dans le programme.
- **La manipulation de la GOT et de la PLT** — le détournement des appels
  indirects d'un programme au travers de ses tables de liaison.
- **Les entrailles des objets C++** — l'exploitation des tables de méthodes
  virtuelles et des particularités de la mémoire objet, dans les étapes plus
  tardives et plus complexes.
- **La manipulation de l'environnement et des arguments** — le placement de
  données choisies à portée d'un processus.

## Méthode

La méthode prescrite relève de la patience plutôt que de la force. Pour chaque
binaire :

1. **Reconnaissance** — établir la nature du fichier, ses protections et ses
   permissions.
2. **Analyse statique** — lire le désassemblage, reconstituer la logique du
   programme, et localiser le point de faiblesse.
3. **Analyse dynamique** — observer le programme en cours d'exécution, confirmer
   l'hypothèse, et mesurer les contours exacts de la faille.
4. **Exploitation** — construire l'entrée qui transforme la compréhension en
   accès.
5. **Ascension** — recouvrer le mot de passe suivant et poursuivre.

## Outillage

Le travail se conduit sur un système **x86 32 bits** et repose sur l'appareil
coutumier de la discipline :

- `gdb` — le débogueur, pour l'inspection dynamique de l'exécution et de la
  mémoire.
- `objdump` — pour le désassemblage statique et la lecture des en-têtes de
  section.
- `ltrace` et `strace` — pour le traçage des appels de bibliothèque et des appels
  système.
- `checksec` — pour l'énumération des mesures défensives d'un binaire.
- **Python** — pour la rédaction des charges utiles et l'agencement d'une entrée
  précise.

## Contenu du dépôt

Ce dépôt documente la traversée du projet par son auteur. Conformément aux
principes de l'École et à la simple prudence, **il ne contient aucun mot de passe
de niveau ni aucun code d'exploitation fonctionnel**. Ce qu'il offre, en revanche,
est un relevé de méthode : des notes sur le raisonnement appliqué, sur les notions
étudiées, et sur la manière dont chaque classe de vulnérabilité fut abordée.

## Note d'éthique

Les techniques répétées ici sont les instruments du praticien en sécurité
offensive, étudiés dans un cadre maîtrisé et autorisé, à seule fin d'apprendre à
défendre. Elles ne doivent être appliquées que sur des systèmes pour lesquels l'on
détient une autorisation explicite. Savoir comment une serrure se crochète relève
du métier de serrurier ; ce n'est point une licence pour entrer chez autrui.

---

*Partie intégrante du cursus Cybersécurité de 42 — la chaîne d'exploitation
binaire.*
