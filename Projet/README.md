# lscpu
Architecture:                    x86_64
CPU op-mode(s):                  32-bit, 64-bit
Byte Order:                      Little Endian
Address sizes:                   39 bits physical, 48 bits virtual
CPU(s):                          8
On-line CPU(s) list:             0-7
Thread(s) per core:              2
Core(s) per socket:              4
Socket(s):                       1


# Question 2.1 : 
le fichier source avec les chrono est simulation_timed.cpp

On constate que pour chaque pas, le temps consacré à la simulation est d'environ 0.0165 secondes et le temps total est de 0.040s. Seul 40% du temps est consacré au calcul et le reste à l'affichage.

# Question 2.2 :
simulation_sync_affiche_mpi.cpp

On constate que le programme accélère par rapport à la version non parallélisée. Cependant le facteur limitant est le temps d'affichage. Le process 1 qui fait les calculs attends que le process 0 ait fini d'afficher le jour précédent avant de continuer ses calculs. Le calcul de la propagation de l'épidémie est limité par la vitesse d'affichage. Un pas de calcul est égal à un pas d'affichage.

# Question 2.3 :
simulation_async_affiche_mpi.cpp

On constate encore une accélération par rapport à la question précédente. En effet le process 1 n'attends plus que le jours soit affiché avant de calculer le suivant. Le process 0 n'affiche pas tous les jours mais en saute certains s'il est trop lent. Le facteur limitant est maintenant le temps de calcul des jours.

# Question 2.4 :
simulation_async_omp.cpp

On parallélise avec omp la boucle sur les individus qui les contamine et les fait se déplacer car c'est là que le programme prend le plus de temps.
Avec une parallélisation "naïve" les résultats sont différents des précédents. En effet, les individus ne sont plus traités dans l'ordre dans lequel ils sont dans la population, mais plutôt dans un ordre qui dépend du découpage fait par omp et de la vitesse de traitement des threads. En conséquent, le moteur de génération pseudo aléatoire des objets grippe et agents ne génèrent plus les même nombres pour les même individus lors de l'appel de la métode estContaminé(). Il faut donc modifier individu.cpp pour que les tirages aléatoires des durées d'incubation et d'infection soient faites par les individus et non par l'objet grippe ou agent. Ainsi, quelque soit l'ordre dans lequel sont traités les individus, on a le même résultat.


Avec un nombre fixe d'individus de 100,000 au total, on constate un speedup de 1.63 si on utilise deux threads omp. Avec 3 et 4 threads on constate des speedup de seulement 1.33 et 1.51. Cela s'explique probablement par le fait que les informations doivent circuler entre les UC et que l'utilisation du cache est moins optimale. C'est donc avec 2 threads que la parallélisation est la plus efficace.

| 100,000 individus       | simulation.exe | affiche_sync | affich_async | async_omp(1) | async_omp(2) | async_omp(3) | asycn_omp(4) |
| :---------------:       | :------------: | :----------: | :----------: | :----------: | :----------: | :----------: | :----------: |
| temps pour 300 pas      | 13.6527        | 11.6128      | 6.2562       | 6.3192       | 3.8659       | 4.7591       | 4.1791       |
| speedup /simulation.exe | 1              | 1.18         | 2.18         | 2.16         | 3.53         | 2.87         | 3.23         |
| speedup /async_omp(1)   |                |              |              | 1            | 1.63         | 1.33         | 1.51         |