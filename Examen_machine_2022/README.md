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
On parallélise sur les lignes pour la DTF et sur les colonnes pour la reconstitution.

Taux par défaut (10%), small_lena_gray.png

| Nombre de threads | Temps DTF | Temps reconstitution | Speedup DTF | Speedup recons |
| :---------------: | :-------: | :------------------: | :---------: | :------------: |
| 1                 | 180.5     | 19.3                 | 1           | 1              |
| 2                 | 90.5      | 10.2                 | 1.99        | 1.89           |
| 3                 | 64.5      | 7.21                 | 2.79        | 2.76           |
| 4                 | 49.7      | 5.54                 | 3.63        | 3.48           |

Pour la DTF et la reconstitution on remarque que le speedup est proche du nombre de thread ce qui montre que le programme est bien parallélisé.
