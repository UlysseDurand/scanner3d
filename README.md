# Scanner3d

This project was a collaboration of Ulysse Durand, Loïc Thomas, Gabin Jobert--Rollin and Yann Prost.

It was in the context of the Science of Engineering project part of the baccalaureate in 12th grade in 2018-2019.

A wooden box with a rotating platform was build. The object to scan was placed on the rotating platform, a focal lens with a light source sent parallel light rays on the object and its shadow was cast on a screen recorded by a webcam. In the end, the shadows of the object under different angles were processed to rebuild the 3d object.

Two approches were used to rebuild the 3d object

- The first one consisted of a 3d grid of points, at each shadow of the object, points out of the shadow were removed
(completed).

- The second one consisted of making the intersection of an extrusion of the shadows rotated with their respective angles (uncompleted).

You can find a summary document [here](https://gitlab.com/ulysse_durand/scanner3d/-/blob/main/readme/Scanner3D.pdf?ref_type=heads).

Conception d'un scanner 3d, un objet dont l'ombre est prise en photo sous plusieurs angle est reconstitué en format .obj.
Merci à Yann PROST, Loïc THOMAS et Gabin JOBERT-ROLLIN pour la conception materielle du scanner, l'acquisition d'images, et le pré-traitement des données.
Ceci est le fruit d'un projet de terminale de Sciences de l'ingénieur.

Tout est expliqué ici : ![document explicatif](https://github.com/UlysseDurand/Scanner3d/blob/main/readme/Scanner3D.pdf)

L'objet : ![figure5](https://github.com/UlysseDurand/Scanner3d/blob/main/readme/figures/5.jpg?raw=true)

L'entrée des programmes : ![figure6](https://github.com/UlysseDurand/Scanner3d/blob/main/readme/figures/6.jpg?raw=true)

La sortie du programme 1 : ![figure9](https://github.com/UlysseDurand/Scanner3d/blob/main/readme/figures/9.jpg?raw=true)

L'entrée du programme 2 : ![entree.gif](https://github.com/UlysseDurand/Scanner3d/blob/main/readme/entree.gif?raw=true)

La sortie du programme 2 : ![sortie.gif](https://github.com/UlysseDurand/Scanner3d/blob/main/readme/sortie.gif?raw=true)

