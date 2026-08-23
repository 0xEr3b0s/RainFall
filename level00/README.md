# RainFall - Level 0

## Passkey for level0
level0

## Passkey for level1
1fe8a524fa4bec01ca4ea2a869af2a02260d4a7d5fe7e7c24d8617e6dca12d3a

## Objectif

Exploiter le binaire `level0` (setuid level1) afin d'obtenir un shell et lire le fichier `/home/user/level1/.pass`.

---

## Analyse du binaire

### Désassemblage de `main`

```asm
0x08048ed4 <+20>:   call   0x8049710 <atoi>
0x08048ed9 <+25>:   cmp    $0x1a7,%eax          ; 0x1a7 = 423
0x08048ede <+30>:   jne    0x8048f58 <main+152> ; si différent → "No !"
...
; si égal à 423 :
call   strdup("/bin/sh")
call   getegid / geteuid
call   setresgid / setresuid
call   execv("/bin/sh", ...)
```

### Points clés

- Le programme prend un argument en ligne de commande
- Il convertit cet argument avec `atoi`
- Il compare le résultat à **423** (`0x1a7`)
- Si la valeur est différente → affichage de `"No !"` et sortie
- Si la valeur est **423** → élévation des privilèges + exécution de `/bin/sh`

### Problème

Il n’y a pas de vulnérabilité mémoire classique.
Il s’agit simplement de comprendre la condition et de fournir la bonne valeur.

---

## Solution

Il suffit de lancer le binaire avec l’argument `423` :

```bash
./level0 423
```

Le programme entre dans le `if`, élève les privilèges et lance un shell en tant que `level1`.

Une fois le shell obtenu :

```bash
id
cat /home/user/level1/.pass
```

---

## Source reconstruite

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    if (argc < 2 || atoi(argv[1]) != 423)
    {
        fwrite("No !\n", 1, 5, stderr);
        return 0;
    }

    char *shell = strdup("/bin/sh");

    gid_t egid = getegid();
    uid_t euid = geteuid();

    setresgid(egid, egid, egid);
    setresuid(euid, euid, euid);

    char *args[] = { shell, NULL };
    execv(shell, args);

    return 0;
}
```

---

## Résumé

| Étape | Description |
|-------|-------------|
| 1 | Analyse du désassemblage |
| 2 | Identification de la comparaison avec `0x1a7` (423) |
| 3 | Lancement de `./level0 423` |
| 4 | Shell obtenu en tant que level1 |

Technique : **compréhension de la logique du programme** (pas d’exploitation mémoire).
