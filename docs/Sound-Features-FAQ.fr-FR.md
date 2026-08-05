# FAQ des fonctions audio

Ce document décrit Sound Doctor, Audible Meter et la console de volume accessible d’Accessible Studio 1.1.3.

## Vue d’ensemble

### Quelle est la différence entre ces fonctions ?

**Audible Meter** surveille continuellement les sources actives et signale les problèmes de niveau par des tonalités et des annonces du lecteur d’écran. La **console de volume accessible** permet de consulter et modifier au clavier le volume, la sortie et le contrôle audio d’OBS. **Sound Doctor** exécute un diagnostic distinct, analyse des statistiques de niveau et propose des filtres facultatifs de compression et de limitation. Audible Meter et Sound Doctor ne peuvent pas fonctionner simultanément.

### Mon son est-il enregistré, transmis ou conservé ?

Non. Ces fonctions lisent uniquement les niveaux numériques fournis par OBS. Elles ne capturent aucune forme d’onde, ne créent aucun enregistrement, n’envoient aucun son sur le réseau et n’écrivent ni historique ni rapport sur disque. Les statistiques occupent une mémoire de taille fixe et sont supprimées à l’arrêt. Seuls les réglages ordinaires et les identités placées dans la liste d’arrêt avant curseur sont conservés.

## Sound Doctor

### Que fait exactement Sound Doctor ?

Démarrez-le avec Ctrl+Maj+D ou via **Outils > Accessible Studio > Outils audio > Sound Doctor**. Après confirmation, il observe les sources actives pendant 120 secondes tandis que vous les utilisez normalement. Une source doit fournir au moins 45 secondes de signal actif pour recevoir une recommandation de traitement. Si une diffusion ou un enregistrement reste actif, la mesure continue et le rapport attend l’arrêt des deux.

Pour chaque source, Sound Doctor conserve des distributions de taille fixe du niveau RMS, des crêtes et du temps actif, ainsi que son identité, son niveau initial et une empreinte de la chaîne de filtres. Il examine aussi les compresseurs et limiteurs existants. Il ne reconnaît ni paroles ni contenu et n’envoie aucune mesure à un service d’IA.

### Comment décide-t-il qu’un compresseur peut aider ?

La compression n’est envisagée que pour les types probablement en direct, comme les microphones et entrées audio directes. Sound Doctor compare la médiane RMS active, passage typique, au 95e centile, passage fort. Leur différence est la variation dynamique.

Il recommande un compresseur si la source fournit 45 secondes actives, si son curseur et ses filtres n’ont pas changé, si elle est probablement en direct, si aucun compresseur même désactivé n’existe et si la variation atteint le minimum configuré.

Le seuil proposé vaut médiane plus 5 dB, arrondi et limité entre −24 et −10 dB. Attaque : 6 ms ; relâchement : 100 ms ; gain de sortie : 0 dB. Le rapport s’adapte : 2:1 sous 10 dB, 2,5:1 de 10 à moins de 14 dB, puis le maximum configuré à partir de 14 dB. Les valeurs par défaut sont 8 dB de variation minimale et 3:1 au maximum.

### Peut-il distinguer sûrement la parole de la musique ?

Non. Les niveaux décrivent l’intensité et la dynamique, pas le contenu. Le type de source n’est qu’un indice d’entrée probablement en direct. Un microphone peut transporter de la musique et une capture d’application de la parole. Le rapport conseille donc de ne pas sélectionner la compression pour une source principalement musicale.

### Quand recommande-t-il un limiteur ?

Par défaut, pour toute entrée probablement en direct ayant fourni 45 secondes actives, restée inchangée et sans limiteur existant. Aucun écrêtage ni crête proche du plafond n’est exigé : il s’agit d’une protection préventive de type mur contre de futures crêtes. Le limiteur est proposé en dernier, avec un plafond par défaut de −3 dBFS et un relâchement de 60 ms. Le son sous le plafond reste inchangé.

Le réglage avancé peut étendre cette politique à toutes les sources suffisamment actives. Cela renforce la sécurité, mais peut être inutile pour des médias ou musiques déjà maîtrisés.

### Les recommandations de compresseur et limiteur sont-elles liées ?

Non. Elles sont évaluées séparément. Une source peut recevoir aucune, une ou les deux. Le compresseur réduit la variation dynamique ; le limiteur final impose un plafond de crête strict.

### Que contient encore le rapport ?

Il indique les réglages des compresseurs et limiteurs existants, avertit si un limiteur n’est pas dernier, signale les crêtes répétées avant curseur proches de l’écrêtage et explique les données insuffisantes ou changements survenus pendant le test. Un son écrêté avant OBS exige de réduire le gain du périphérique ou de l’application.

### Sound Doctor modifie-t-il automatiquement quelque chose ?

Pas simplement en s’exécutant. Chaque proposition possède une case **Appliquer automatiquement cette modification**, décochée au départ. **Terminer** applique uniquement les éléments cochés. Sound Doctor revérifie auparavant la source, son volume et ses filtres et ignore toute proposition périmée. Les nouveaux filtres portent un nom Sound Doctor visible, sont ordonnés intentionnellement et prennent en charge Annuler et Rétablir d’OBS. Échap ferme sans appliquer.

### Puis-je modifier la politique ?

Ouvrez **Outils > Accessible Studio > Outils audio > Paramètres audio avancés**, puis la page **Sound Doctor** :

- **Variation dynamique minimale pour la compression :** 6–15 dB ; 8 dB par défaut. Une valeur basse produit davantage de recommandations.
- **Rapport maximal recommandé :** 2,5:1, 3:1 ou 3,5:1 ; 3:1 par défaut.
- **Sources admissibles au limiteur :** entrées probablement en direct seulement par défaut, ou toutes les sources actives.
- **Plafond recommandé :** −12 à −1 dBFS ; −3 dBFS par défaut.

La session de 120 secondes et les 45 secondes actives minimales restent fixes. Appliquer enregistre sans fermer, OK enregistre et ferme, Annuler abandonne les modifications depuis la dernière application.

## Audible Meter et correction automatique

### Que surveille Audible Meter ?

Ctrl+I démarre ou arrête son unique mode sans fenêtre. Il suit toutes les sources actives, utilise la crête après curseur pour les avertissements de sortie, la crête avant curseur pour les captures en direct admissibles et des résumés RMS pour les niveaux typiques. Il fonctionne avec ou sans diffusion ou enregistrement.

### Que signifie le signal sonore automatique ?

La tonalité automatique aiguë signifie qu’au moins une source a passé la durée réelle configurée au niveau d’avertissement de sortie ou au-dessus. La durée par défaut est 1,5 seconde. Le niveau par défaut suit le mode OBS : −9 dBFS en crête d’échantillon ou −2 dBFS en crête vraie. Une baisse de moins d’une seconde n’efface pas entièrement l’exposition ; une seconde sous la limite réarme l’avertissement. C’est un avertissement et non une mesure précise ; le lecteur d’écran nomme la source la plus problématique.

La tonalité grave signifie qu’après votre accord pour régler une entrée admissible, son signal avant curseur reste dans le rouge. Réduisez le gain du microphone, de l’interface, de l’entrée Windows ou de l’application. Le curseur OBS ne peut réparer cette distorsion.

### Que signifient les deux tonalités de la console ?

Lorsque Audible Meter fonctionne et que la console est ouverte, la source focalisée produit une tonalité moyenne dans la zone jaune de sortie et une tonalité aiguë dans la zone rouge. Vert, silence, source indisponible ou focus hors d’un contrôle de source : aucun son. Ce sont des mesures immédiates, non des avertissements retardés, et I ne les désactive pas.

En crête d’échantillon, le jaune commence à −20 dBFS et le rouge à −9 dBFS. En crête vraie, le jaune commence à −13 dBFS et le rouge à −2 dBFS.

### Audible Meter baisse-t-il mon son tout seul ?

Non. Maj+I pendant la tonalité aiguë demande explicitement une correction. Pour chaque source responsable, la cible est le niveau d’avertissement moins la marge de sécurité. Audible Meter la compare à la plus haute crête de l’incident, arrondit la réduction nécessaire au 0,5 dB supérieur et baisse le curseur OBS sans dépasser le maximum configuré. Il n’augmente jamais une source et ne change rien pour un simple avertissement avant curseur.

Les valeurs par défaut sont une marge de 3 dB et une réduction maximale de 12 dB. C’est une protection prudente, pas un mixage artistique.

### Puis-je annuler une correction ?

Oui. Ctrl+Maj+I restaure la dernière correction Maj+I. Une source n’est restaurée que si son curseur correspond encore à la valeur automatique ; une source modifiée ensuite est ignorée. Démarrer ou arrêter le compteur, changer de collection ou enregistrer de nouveaux réglages efface ce niveau unique de restauration.

### Comment modifier le comportement ?

Sur la page **Audible Meter** des paramètres audio avancés :

- **Niveau d’avertissement de sortie :** −30 à 0 dBFS ; par défaut −9 dBFS en crête d’échantillon ou −2 dBFS en crête vraie.
- **Durée au-dessus :** 0,1–30 secondes ; 1,5 seconde par défaut.
- **Marge de sécurité :** 0–12 dB ; 3 dB par défaut.
- **Réduction automatique maximale :** 0,5–30 dB ; 12 dB par défaut.
- **Liste d’arrêt avant curseur :** retirez une exception pour vérifier à nouveau cette entrée inchangée.

Ces réglages ne modifient ni les zones OBS fixes, ni les tonalités de mesure, ni H/J/K/L, ni la limite rouge avant curseur.

### Puis-je désactiver temporairement les avertissements ?

Appuyez sur I pendant le fonctionnement. Leur réactivation recommence le chronométrage. Les tonalités de la console restent actives. Les lettres ne sont jamais interceptées dans un contrôle modifiable.

### À quoi servent H, J, K et L ?

- H annonce le niveau et la zone actuels de la dernière source focalisée dans la console.
- J annonce la source active actuellement la plus forte.
- K annonce le niveau RMS actif typique de la source sélectionnée pour la session.
- L annonce la source au niveau RMS typique le plus fort de la session.

### Pourquoi un avertissement avant curseur est-il prioritaire ?

Il signale un dommage possible avant le curseur OBS. La boîte de décision est silencieuse. Oui démarre le guidage ; la tonalité grave continue tant que l’entrée est rouge. Un niveau actif sain et soutenu résout l’incident. Le silence n’est pas un succès : annonces après 2 et 12 secondes, puis question après 22 secondes. Non ou Échap enregistre la source inchangée dans la liste d’arrêt. Une identité ou configuration modifiée est réévaluée.

### Que permet la console de volume accessible ?

Ctrl+Accent grave l’ouvre. Gauche/Droite sélectionnent, Haut/Bas changent de 1 dB, Origine fixe 0 dB, 1–9 sélectionnent les neuf premières sources et 0 la dixième. Espace bascule ensemble contrôle audio et sortie, Ctrl+Espace le contrôle seul, Maj+Espace la sortie seule. La console montre normalement les sources actives du programme ; **Afficher toutes les sources** ajoute les inactives. Les changements sont immédiats et la console n’élève pas seule une source normale au-dessus de 0 dB.

### Quel est le flux de travail le plus sûr ?

Exécutez Sound Doctor avec une parole et un son représentatifs, examinez chaque proposition et laissez la compression musicale décochée sauf effet voulu. Gardez Audible Meter actif, utilisez les tonalités de la console pour l’inspection immédiate, corrigez les problèmes avant curseur au périphérique ou à l’application d’origine et n’utilisez Maj+I que pour demander une réduction prudente.
