# FauteuilSousMarin

## Le projet 

Projet de Fauteuil sousmarin pour [Odysseus 3.1](https://odysseus31.com/ "Odysseus 3.1"), association à Lyon.

L'idée est de répliquer les mouvements du fauteuil spatial de la NASA dans les années 1980 sous l'eau.

<img src="./space-mmu.jpg" width="800"><br><br>

Sous l'eau, pas de gravité, donc il est possible de simuler le fonctionnement d'un fauteuil spatial, pour se déplacer comme dans l'espace.

### Pour qui ?

* Pour les spationautes en formations (entrainement)
* Pour des personnes handicapées, qui seront prise en charge dans leurs déplacement sous l'eau.

## Principe du projet

Un fauteuil (cadre en tube soudé) acceuille le plongeur, son bloc d'air, deux joysticks, les moteurs et les batteries.

Le système est controllé par deux joysticks, inspirés directement des commandes de la NASA.
Une commande permet la translation du fauteuil sur les 3 axes (main gauche).
Une commande permet la rotation du fauteuil sur les 3 axes (main droite).
<img src="./MMU 02.jpg" width="800"><br><br>

Une commande à distance (Monitor box) permet le contrôle du fauteuil à distance, en parallèle ou en remplacement des commandes sur le fauteuil. Cette commande permet aussi d'armer le fauteuil lors de la mise à l'eau, pour éviter des salades de doigts (ou cheveux, ceinture de plomb, ...) avec les propulseurs.

Le fauteuil selon deux modes :
* plongée (normal) : quand l'ordre (joystick) s'arrête, l'action correspondante s'arrête
* espace (space) : quand l'ordre (joystick) s'arrête, l'action continue, comme dans l'espace. Le système est a améliorer, pour être plus réaliste. 

**DANGER**
Le surnom du mode 'espace' c'est 'Vicieux', vu les marches avant plein gaz (malgré l'arrêt de la commande) dans les murs de la piscine qui a servi aux tests. Bah oui, dans l'espace, quand tu as mis gaz avant toutes, il te faut gaz arrière toute pour compenser la première commande....

## Electronique

Un microcontrolleur Arduino gère le 'SpaceChair', en utilisant les entrées des joysticks et en dirigeant les propulseurs. Un mode 'SpaceMode' permet de simuler

Il contrôle ensuite 6 ESC (contrôlleur de moteur brushless (triphasé)) comme en modélisme.
Il y a 6 moteurs, 3 par axes et 2 par sens (avant/arrière). Je l'ai conçu pour équilibrer l'effet gyroscopique (moteur sens horaire et contra sens horaire sur chaque axe) et permettre plus de finesse.

## Annexes

### Le fauteuil MMU

Nasa: Manned Maneuvering Unit ou MMU, utilisé en 1984 !
Retiré car trop dangereux (24,4m/s de vitesse, donc en cas d'erreur, c'est bye bye la station et bonjour l'effet **vers l'infini et au delà**).
Depuis, passage au Simplified Aid for EVA Rescue : SAFER.
Il fait un delta de 3,3 m/s

MMU: 24 petites tuyères, chacune pouvant créer une poussée de 7,5 kg
3-axes Translatio controlleur (main gauche)
3-axes Rotation controlleur (main gauche)
Commandes simple ou mutli-axes en continues ou impulsions.

Son mode d'emploi : http://large.stanford.edu/courses/2014/ph240/clark1/docs/mcr-78-617.pdf

Un site qui en parle : http://capcomespace.net/dossiers/espace_US/shuttle/sts/EMU_MMU/MMU.htm

### Liste des composants
Voir aussi dans le dossier Datasheets !

* 6 moteurs, en 3 lots de CW et CWW https://www.aliexpress.com/item/32965239368.html
* 6 controlleurs 90A étanche : https://hobbyking.com/fr_fr/hobbyking-90a-boat-esc-4a-sbec.html?queryID=&objectID=48599&indexName=hbk_live_magento_fr_fr_products
* 1 télécommande de programmation : https://hobbyking.com/fr_fr/hobbyking-boat-esc-programming-card.html
* 4 joysticks 3 axes :  JH-D300X-R4 https://www.aliexpress.com/item/32992844960.html
* 6 potentiometres IP68 Vishay P13SM : https://www.vishay.com/docs/51064/p13sm.pdf
* 2 Arduino Micro
