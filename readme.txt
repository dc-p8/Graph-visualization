Voici les commandes disponibles :
[Maj + V] : supression d'un vertex
[Maj + E] : supression d'un edge
[V] : ajout d'un vertex
[E] : ajout d'un edge
[M] : déplacement d'un vertex
[Echap] : Mode normal
[S] : stress
[R] : reset
[C] : Activer/désactiver répultion du centre de masse
[+] : augmenter l'amplificateur de mouvements
[-] : diminuer l'amplicicateur de mouvements
[Q] : Quitter

Il est possible d'avoir un résultat accéptable en lançant juste le programme, toutefois il est conseillé d'intéragir avec lui.

En effet, la force de répultion du centre de masse (activée par défaut) peut empecher certains rapprochements, il y donc des croisements inutiles et facilement contournables.

Il est alors conseillé, pour un résultat optimal, de procéter en deux étapes :


La première étape va permettre de faire resortir la topologie principale du graphe en ne prennant en compte que les forces de répultions entre chaque couple de vertex, et les forces d'attraction pour chaque couple de vertex relié.
L'epace entre chaque vertex va alors augmenter, ce qui donnera une impression d'expention de l'espace.
Cette phase peut prendre du temps, on peut alors amplifier les mouvements en appuyant sur la touche '+'.

Une fois qu'on a la topoligie générale du graphe, on constate qu'il est relativement illisible (trop espacé, on ne voit pas les edges).

<screen1.png>

On va alors procéder à l'étape 2.
En appuyant sur la touche 'c' (centre), on va ajouter une force de répultion au centre de masse, représenté par le cercle transparent.
Cela pour effet de propulser tous les vertex sur l'extérieur.

<screen2.png>

Comme cette répultion est plus faire que la répultion entre chaque couple de vertex, les vertex se situant entre la bordure extérieur vont pouvoir prendre l'espace disponible

<screen3.png>

Après quelques secondes, des ajustements ont été fait en prenant en compte la force de répultion centrale.

<screen4.png>

On peut commencer dirrectement à la phase 2 (fixer re_center = true), mais la force centrale pourra alors empêcher certains rapprochements et le résultat sera moins logique (des croisements inutiles et facilement évitables).

<screen5.png