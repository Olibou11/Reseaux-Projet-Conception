# Reseaux-Projet-Conception

## Introduction
Le but du projet est de simuler une attaque de type cheval de Troie. Pour faire exécuter des lignes de commandes Shell (sous Windows) en arrière-plan sous forme de communication réseau (comme une communication Client et Serveur). Le “pirate” doit être en mesure d’exécuter plusieurs commandes sur l’appareil de la “cible” et de voir la réponse du système d’exploitation (le résultat de la ligne de commande) en recevant un fichier texte contenant la réponse obtenue avec l’appareil de la “cible”.

## Avant de débuter
Dans ce projet, nous avons décidé d’utiliser que les ressources internes de Visual Studio, alors il n’est pas nécessaire d’ajouter des extensions externes.
Il est important de noter que notre programme fonctionne uniquement avec les appareils sous Windows 10. Le problème est dans l’utilisation de la méthode FindWindow qui ne fonctionne pas correctement avec les versions plus récentes telle que Windows 11. En outre, le programme n’est pas en mesure de  trouver la fenêtre du terminal CMD, alors il est impossible de communiquer avec ce dernier. Selon le blog de Microsoft, le problème ne semble pas avoir été résolu depuis son apparition. En somme, nous vous conseillons de vérifier la version du système d’exploitation de votre victime avant d’utiliser notre programme. D’ailleurs, comme la méthode n’est pas utilisée du côté client, toutes les versions sont compatibles pour celui-ci.


## Étapes d’utilisation
- Lancer l'exécutable du serveur sur la victime.
- Lancer l'exécutable du client sur l'hôte.
- À partir du client, entrer n’importe quelle commande.
- Le client reçoit un fichier (.txt) crypté du serveur. Il est déchiffré et affiché dans la console de ce dernier. Le message peut être une erreur comme une information utile. 
- Fermer la fenêtre d’exécution du client pour quitter.
- Au besoin, ouvrez la fenêtre de nouveau pour vous y reconnecter sans problème.
- Tant que l’exécutable est en fonction sur la victime, il est possible de s’y reconnecter.
