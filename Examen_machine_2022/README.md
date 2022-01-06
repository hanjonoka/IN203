# Examen_machine_2022
Examen machine du 6 Janvier 2022 : compression d'image parallèle par série de Fourier

**Dans le Makefile, modifier les premières lignes en commentant ou décommentant pour prendre le fichier adapter à votre environnement !**

__Pour ceux en distanciel__ : Pour toute question, envoyer un mail à xjuvigny@gmail.com (plus rapide que juvigny@onera.fr)

# Programme séquentiel

Temps pour l'image tiny_lena_grapy.png car le programme est trop lent avec les autres images. Taux par défaut (10%)

| Temps DTF | Temps sélection coefs | Temps reconstitution |
| :-------: | :-------------------: | :------------------: |
| 0.625     | 0.00032               | 0.072                |

# Parallélisation OMP

On parallélise la fonction de transformée de Fourier et la fonction de reconstitution car ce sont celles qui prennent le plus de temps.
On parallélise sur les lignes dans l'espace des fréquences pour la DTF et sur les colonnes de la nouvelle image pour la reconstitution.

Taux par défaut (10%), small_lena_gray.png

| Nombre de threads | Temps DTF | Temps reconstitution | Speedup DTF | Speedup recons |
| :---------------: | :-------: | :------------------: | :---------: | :------------: |
| 1                 | 180.5     | 19.3                 | 1           | 1              |
| 2                 | 90.5      | 10.2                 | 1.99        | 1.89           |
| 3                 | 64.5      | 7.21                 | 2.79        | 2.76           |
| 4                 | 49.7      | 5.54                 | 3.63        | 3.48           |

Pour la DTF et la reconstitution on remarque que le speedup est proche du nombre de thread ce qui montre que le programme est bien parallélisé.

# Parallélisation MPI 1

On parallélise dans la fonction DTF : chaque process va s'occuper de height/nbp valeurs de n2. Ensuite, on somme les valeurs des DTF partielles avec une reduction.
On devrait découper l'image avec un MPI_scatter() avant de faire la transformée mais je ne l'ai pas fait par manque de temps.

Les speedup sont moins bons que précédemment mais restent corrects.

Taux par défaut (10%), small_lena_gray.png

| Nombre de threads | Temps DTF | Speedup |
| :---------------: | :-------: | :-----: |
| 1                 | 95.95     | 1       |
| 2                 | 51.14     | 1.87    |
| 3                 | 36.14     | 2.65    |
| 4                 | 28.22     | 3.4     |

NB : je suis passé branché sur secteur ce qui explique les temps beaucoup plus courts qu'avant.

# Parallélisation MPI 2

Chaque process a besoin d'une copie entière de l'image pour calculer la transformée de fourier.
Dans un premier temps, on parallélise uniquement la DTF puis on finit l'image sur le process 0.