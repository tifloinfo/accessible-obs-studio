# Accessible Studio 1.1.3

Accessible Studio est un module d’accessibilité pour OBS Studio 32, 64 bits, sous Windows 10 et 11. Il est destiné aux utilisateurs aveugles du clavier et d’un lecteur d’écran, et a été testé avec JAWS et NVDA. Une clé API OpenAI et Internet ne sont nécessaires que pour les fonctions OpenAI.

Accessible Studio est un module d’accessibilité tiers et indépendant pour OBS Studio. Il n’est ni développé, ni affilié, ni parrainé, ni approuvé par le projet OBS ou ses contributeurs. OBS et OBS Studio sont des marques déposées de Wizards of OBS LLC.

## Installation

Installez l’édition 64 bits d’OBS Studio 32.0 ou version ultérieure, puis exécutez `AccessibleStudio-1.1.3-Setup.exe`. Si OBS Studio est absent, endommagé ou antérieur à 32.0, le programme propose d’ouvrir la [page officielle de téléchargement d’OBS](https://obsproject.com/download), puis se ferme sans apporter de modifications. Vous pouvez aussi mettre à jour une ancienne version avec Aide > Rechercher des mises à jour dans OBS Studio. OBS Studio 32.x est pris en charge. Avec OBS Studio 33 ou version ultérieure, le programme avertit d’une incompatibilité possible et propose la [page du dernier module](https://github.com/tifloinfo/accessible-studio/releases/latest) avant d’autoriser un choix explicite d’installation malgré tout. Si OBS Studio est en cours d’exécution, le programme demande de le fermer complètement, puis de choisir Réessayer ; il ne ferme jamais OBS automatiquement. Le module est installé dans `C:\ProgramData\obs-studio\plugins\accessible-studio`. Les composants Microsoft WebView2 et Visual C++ manquants ne sont ajoutés qu’après ces vérifications, sans remplacer les fichiers OBS ou Qt. Sur la page finale, la case **Ouvrir le fichier Lisez-moi dans le navigateur web** ouvre la documentation HTML française.

Si la version publiée Accessible OBS Studio 1.0 est installée, elle est désinstallée automatiquement et seuls ses paramètres propres au module, ses raccourcis clavier, ses données en cache et sa clé API OpenAI enregistrée sont supprimés. Les autres paramètres et contenus d’OBS sont conservés.

## Raccourcis clavier par défaut

- F3 : description de base du canevas, limitée à 80 caractères.
- Maj+F3 : description détaillée.
- Alt+F3 : lecture du texte visible, sans traduction ni commentaire.
- Ctrl+F3 : description des personnes et des arrière-plans.
- F4 : Vérification visuelle de la diffusion ou de l’enregistrement pour les problèmes de mise en page, caméra, éclairage, cadrage, netteté, grain, apparence, vêtements, arrière-plan et objets indésirables.
- Ctrl+M : placer le focus sur les commandes multimédias visibles.
- F5, F7, F8 : démarrer ou arrêter la diffusion, l’enregistrement ou la caméra virtuelle.
- Alt+F2 : afficher l’état de la diffusion, de l’enregistrement, de la caméra virtuelle et du mode Studio.
- Alt+F7 : suspendre ou reprendre l’enregistrement.
- F6 / Maj+F6 : zone principale suivante / précédente.
- Ctrl+0 à Ctrl+5 : canevas, scènes, sources, mélangeur audio, transitions ou commandes.
- Ctrl+` (touche sous Échap) : ouvrir la console de volume accessible.
- Ctrl+I : démarrer ou arrêter Audible Meter.
- Ctrl+Maj+D : démarrer ou interrompre Sound Doctor.

Lorsque NVDA est le seul lecteur d’écran actif détecté, le module annonce explicitement le nom localisé de la zone après un changement réussi avec F6, Maj+F6 ou Ctrl+0 à Ctrl+5. Cette annonce supplémentaire est supprimée avec JAWS, Narrateur, un lecteur inconnu ou plusieurs lecteurs détectés simultanément.

La commande **.Ouvrir l’éditeur de raccourcis clavier** ouvre directement le même éditeur et n’a pas de raccourci par défaut. Son identifiant interne reste inchangé, ce qui conserve toute attribution existante.

Par défaut, Accessible Studio impose que tous les raccourcis clavier d’OBS ne fonctionnent que lorsque OBS est l’application active. Il maintient **Paramètres > Avancé > Comportement du focus des raccourcis clavier** sur **Désactiver les raccourcis clavier lorsque la fenêtre principale n’a pas le focus** et rétablit cette valeur si elle change. Pour rendre le contrôle à OBS, cochez puis enregistrez **Autoriser OBS Studio à gérer le fonctionnement des raccourcis clavier hors d’OBS** dans l’éditeur. Le module cesse alors d’intervenir.

Au premier démarrage et après un changement de profil, le module compare ses attributions prévues aux attributions existantes. La boîte de dialogue n’apparaît qu’en cas de conflit réel. Vous pouvez supprimer uniquement les attributions en conflit et appliquer les raccourcis Accessibility par défaut, ou conserver les attributions existantes ; les raccourcis par défaut en conflit restent alors non attribués. **Ne plus me demander pour cette version** s’applique à tous les profils, mais une nouvelle version ou compilation déclenche un nouveau contrôle.

## Menu Accessible Studio

**Outils > Accessible Studio** ouvre désormais un menu accessible. **Outils audio** contient la console de volume accessible, Audible Meter, Sound Doctor et **Paramètres audio avancés**. **Outils vidéo** contient les descriptions courte et détaillée du canevas, Lire le texte, Personnes et arrière-plans, Vérification visuelle et la gestion des clés API OpenAI. Le menu principal contient également l’éditeur de raccourcis clavier et **Ouvrir le manuel d’utilisation**. Le manuel utilise la langue de l’interface OBS si elle est installée, sinon l’anglais.

## Éditeur de raccourcis clavier

Ouvrez **Outils > Accessible Studio > Éditeur de raccourcis clavier**. Les flèches parcourent la liste des commandes ; Tab passe entre la commande sélectionnée, la case de contrôle des raccourcis OBS —décochée par défaut— et les boutons. Entrée ou **Ajouter ou modifier** ouvre la boîte d’attribution. Entrée et OK vérifient immédiatement les doublons. En cas de conflit, l’autre commande est nommée : Non revient à la saisie, Oui réattribue le raccourci.

## Mélangeur et commandes multimédias

Ctrl+3 place le focus sur le mélangeur OBS standard. Le module ne numérote plus ses curseurs et n’y installe plus de filtre d’événements global. Ctrl+` ouvre la console accessible modale : Gauche et Droite changent de source, Haut et Bas modifient le volume de 1 dB et Origine règle 0 dB. Espace bascule en toute sécurité le contrôle audio et la sortie Programme ensemble, Ctrl+Espace bascule uniquement le contrôle audio et Maj+Espace uniquement la sortie. Chaque source possède aussi des boutons distincts pour la sortie et le contrôle audio. Les touches 1 à 0 sélectionnent les dix premières sources.

La console actualise depuis OBS la liste des sources, le volume, la sortie et le contrôle audio deux fois par seconde ; les changements effectués avec le mélangeur natif, un contrôleur externe ou un changement de scène apparaissent donc pendant la session. Ses propres modifications sont appliquées immédiatement. Sous OBS 32.2 et versions ultérieures, la coupure et le contrôle audio sont indépendants ; sous OBS 32.0 et 32.1, la console traduit les mêmes commandes vers les anciens états Contrôle uniquement, Contrôle et sortie, et coupé.

Lorsque le focus se trouve dans les commandes multimédias, Gauche et Droite reculent ou avancent de 5 secondes. Maj+Gauche et Maj+Droite reculent ou avancent d’une minute ; Page précédente recule de 5 minutes et Page suivante avance de 5 minutes. En dehors des commandes multimédias, ces touches conservent leur fonction normale.

La console de volume n’affiche initialement que les sources de la sortie Programme actuelle ; les sources propres à l’Aperçu sont exclues en mode Studio. Activez avec Entrée le bouton non défini par défaut **Afficher toutes les sources** pour afficher toutes les sources du mélangeur, les sources actives en premier. Le même bouton devient ensuite **Afficher uniquement les sources actives**. Espace n’active pas ce bouton d’affichage.

Les changements de diffusion, d’enregistrement, de pause, de caméra virtuelle et de mode Studio sont annoncés au lecteur d’écran. Alt+F2 affiche les **Informations d’état**, notamment « reconnexion » et « enregistrement suspendu ». Alt+F7 suspend ou reprend un enregistrement.

## Description du canevas

Les cinq modes capturent le canevas rendu par OBS. Chaque nouvelle réponse initiale ou complémentaire est annoncée une fois par une région active ARIA assertive ; la question de l’utilisateur n’est jamais répétée. Les cinq modes acceptent des questions complémentaires.

**Copier le dernier résultat** copie uniquement la réponse la plus récente dans le Presse-papiers, sans titres, messages d’état ni échanges précédents.

Dans la description de base, **Description détaillée** est toujours disponible, **Lire le texte** uniquement si du texte a été détecté, **Personnes et arrière-plans** uniquement si des personnes ont été détectées, et **Corrections suggérées** uniquement si un problème peut réellement être corrigé automatiquement. Ces actions réutilisent l’image déjà envoyée.

Le mode **Personnes et arrière-plans** donne la priorité aux personnes visibles, puis décrit leur arrière-plan immédiat. Les détails sans rapport concernant l’interface, le texte ou la scène sont omis sauf s’ils influencent directement la présentation d’une personne.

**Vérification visuelle** contrôle uniquement l’aspect de la diffusion ou de l’enregistrement. Les contrôles visuels existants concernant la mise en page OBS, les captures vides, la caméra, l’éclairage, le plein écran Zoom, le cadrage, le flou, une lentille potentiellement sale, le grain, l’apparence, les vêtements, l’arrière-plan et les objets indésirables sont conservés. Le contenu verbal est ignoré : langue, orthographe, grammaire, traduction, formulation, faits, nombres, sujet, ton, pertinence, sous-titres et légendes. Le texte n’est signalé comme objet visuel que s’il est trop petit, coupé, flou, peu contrasté, masqué ou s’il masque des éléments visuels importants. Une boîte de dialogue ou d’erreur n’est signalée que si elle masque du contenu ou révèle un problème visible de capture ou de mise en page, jamais à cause de son message. La correction automatique reste limitée à une liste fixe de transformations OBS réversibles. **Vérifier à nouveau** capture une nouvelle image et signale les améliorations, dégradations, changements et problèmes visuels restants.

Lors du choix d’une correction automatique, la liste ne contient que les sources capables de produire de la vidéo ; s’il n’y en a qu’une, elle est sélectionnée automatiquement. Pour un contenu mal positionné, **Ajuster au canevas** est prioritaire. Après approbation, une nouvelle image est capturée uniquement pour vérifier si l’agrandissement a produit un flou, un grain, du bruit ou une pixellisation inutilisables. Si la qualité n’est pas acceptable ou ne peut pas être confirmée, l’ajustement est automatiquement annulé et **Centrer complètement** est appliqué. **Étirer à l’écran** n’est jamais proposé.

La clé API est validée avant l’enregistrement, conservée dans le Gestionnaire d’informations d’identification Windows et jamais affichée. Sa suppression demande confirmation et affiche un message de réussite.

## Audible Meter

Pour des réponses techniques détaillées sur Audible Meter, la correction automatique, les tonalités de la console et Sound Doctor, consultez la [FAQ des fonctions audio](Sound-Features-FAQ.fr-FR.html).

Ctrl+I démarre ou arrête Audible Meter ; les avertissements automatiques sont activés au départ. I active ou désactive ensemble les avertissements automatiques d’entrée et de sortie. Pendant un avertissement de sortie, Maj+I réduit prudemment toutes les sources responsables ; le volume n’est jamais augmenté et chaque correction est limitée à 12 dB. Ctrl+Maj+I restaure la correction automatique la plus récente si le niveau n’a pas été modifié ensuite par un autre moyen. Les avertissements avant curseur ne sont pas corrigés automatiquement. H annonce le niveau actuel de la dernière source focalisée dans la console, J la source actuellement la plus forte et son niveau, K le niveau actif typique de la source sélectionnée et L la source au niveau actif typique le plus élevé. I, Maj+I, Ctrl+Maj+I, H, J, K et L ne sont jamais interceptées pendant la saisie.

Alt+1 à Alt+9 passent aux neuf premières scènes dans l’ordre affiché ; Alt+0 passe à la dixième. Les scènes au-delà des dix premières n’ont pas de raccourci numérique par défaut.

**Outils > Accessible Studio > Outils audio > Paramètres audio avancés** ouvre une boîte de dialogue de type Paramètres OBS. Utilisez la liste des catégories à gauche et les flèches pour passer entre **Audible Meter** et **Sound Doctor**. La page Audible Meter contient les avertissements de sortie et la liste d’arrêt. La page Sound Doctor contient la variation dynamique minimale, le rapport maximal du compresseur, la portée des recommandations de limiteur et le plafond recommandé. Appliquer enregistre sans fermer ; OK enregistre et ferme ; Annuler abandonne les modifications effectuées depuis le dernier Appliquer.

Un avertissement avant curseur persistant a la priorité exclusive. Aucun son n’est émis pendant la première décision ; Oui est le choix par défaut et démarre ensuite la tonalité grave. Non ou Échap ajoute la source à la liste d’arrêt. En l’absence de signal, une annonce survient après 2 secondes, se répète 10 secondes plus tard, puis une boîte de dialogue avec Oui par défaut apparaît après 10 secondes supplémentaires. Après une correction réussie, la tonalité s’arrête et le message de niveau sûr doit être confirmé par OK. Lorsque la console est ouverte, la détection avant curseur continue en arrière-plan ; les problèmes sont mis en attente et présentés à sa fermeture. Si l’utilisateur tente d’ouvrir la console pendant un réglage, Oui poursuit sans l’ouvrir ; Non ou Échap ajoute la source à la liste et ouvre la console.

## Sound Doctor

Ctrl+Maj+D ou **Outils > Accessible Studio > Outils audio > Sound Doctor** démarre **Sound Doctor**. Pendant la surveillance, la même commande ouvre une confirmation avec Oui par défaut pour interrompre le processus et supprimer les mesures. **Ne plus afficher ce message** enregistre l’interruption immédiate uniquement avec Oui. Non ou Échap poursuit la surveillance. Sound Doctor observe les sources audio actives pendant au moins deux minutes et ne conserve en mémoire que des mesures de taille fixe ; aucun son n’est enregistré. Si une diffusion ou un enregistrement est en cours, le rapport reste masqué jusqu’à l’arrêt des deux.

Le rapport WebView2 accessible au clavier analyse les niveaux habituels, les crêtes, la dynamique, les risques d’écrêtage et les compresseurs ou limiteurs existants. Chaque recommandation justifiée possède une case **Appliquer automatiquement cette modification**, décochée au départ. **Terminer** applique uniquement les changements cochés ; Échap n’en applique aucun. Les nouveaux filtres portent visiblement le nom **Sound Doctor – Compressor** ou **Sound Doctor – Limiter**, comportent un marqueur interne et peuvent être annulés avec la commande Annuler d’OBS.

## Confidentialité et licence

Les fonctions du canevas envoient à OpenAI l’image capturée, la langue d’OBS, des instructions de sécurité fixes et les questions complémentaires. Il n’y a ni télémétrie ni publicité. Copyright (C) 2026 [Tiflo.Info](https://tiflo.info). GNU GPL version 2 ou ultérieure ; voir [LICENSE.txt](../LICENSE.txt). [English](../README.md).
