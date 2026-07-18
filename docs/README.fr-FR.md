# Accessible OBS Studio 1.0

Accessible OBS Studio est un module d’accessibilité pour OBS Studio 32, 64 bits, sous Windows 10 et 11. Il est destiné aux utilisateurs aveugles du clavier et d’un lecteur d’écran, et a été testé avec JAWS et NVDA. Une clé API OpenAI et Internet ne sont nécessaires que pour les fonctions OpenAI.

## Installation

Installez l’édition 64 bits d’OBS Studio 32.0 ou version ultérieure, puis exécutez `AccessibleOBSStudio-1.0-Setup.exe`. Si OBS Studio est absent, endommagé ou antérieur à 32.0, le programme propose d’ouvrir la [page officielle de téléchargement d’OBS](https://obsproject.com/download), puis se ferme sans apporter de modifications. Vous pouvez aussi mettre à jour une ancienne version avec Aide > Rechercher des mises à jour dans OBS Studio. OBS Studio 32.x est pris en charge. Avec OBS Studio 33 ou version ultérieure, le programme avertit d’une incompatibilité possible et propose la [page du dernier module](https://tiflo.info/aobs) avant d’autoriser un choix explicite d’installation malgré tout. Si OBS Studio est en cours d’exécution, le programme demande de le fermer complètement, puis de choisir Réessayer ; il ne ferme jamais OBS automatiquement. Le module est installé dans `C:\ProgramData\obs-studio\plugins\accessible-obs-studio`. Les composants Microsoft WebView2 et Visual C++ manquants ne sont ajoutés qu’après ces vérifications, sans remplacer les fichiers OBS ou Qt. Sur la page finale, la case **Ouvrir le fichier Lisez-moi dans le navigateur web** ouvre la documentation HTML française.

## Raccourcis clavier par défaut

- F3 : description de base du canevas, limitée à 80 caractères.
- Maj+F3 : description détaillée.
- Alt+F3 : lecture du texte visible, sans traduction ni commentaire.
- Ctrl+F3 : description des personnes et des arrière-plans.
- F4 : Vérification visuelle de la diffusion ou de l’enregistrement pour les problèmes de mise en page, caméra, éclairage, cadrage, netteté, grain, apparence, vêtements, arrière-plan et objets indésirables.
- Ctrl+M : placer le focus sur les commandes multimédias visibles.
- F5, F7, F8 : démarrer ou arrêter la diffusion, l’enregistrement ou la caméra virtuelle.
- F6 / Maj+F6 : zone principale suivante / précédente.
- Ctrl+0 à Ctrl+5 : canevas, scènes, sources, mélangeur audio, transitions ou commandes.
- Ctrl+` (touche sous Échap) : ouvrir la console de volume accessible.

La commande **Accessible OBS Studio : Ouvrir l’éditeur de raccourcis clavier** ouvre directement le même éditeur et n’a pas de raccourci par défaut. Son identifiant interne reste inchangé, ce qui conserve toute attribution existante.

Par défaut, Accessible OBS Studio impose que tous les raccourcis clavier d’OBS ne fonctionnent que lorsque OBS est l’application active. Il maintient **Paramètres > Avancé > Comportement du focus des raccourcis clavier** sur **Désactiver les raccourcis clavier lorsque la fenêtre principale n’a pas le focus** et rétablit cette valeur si elle change. Pour rendre le contrôle à OBS, cochez puis enregistrez **Autoriser OBS Studio à gérer le fonctionnement des raccourcis clavier hors d’OBS** dans l’éditeur. Le module cesse alors d’intervenir.

Au premier démarrage et après un changement de profil, le module compare ses attributions prévues aux attributions existantes. La boîte de dialogue n’apparaît qu’en cas de conflit réel. Vous pouvez supprimer uniquement les attributions en conflit et appliquer les raccourcis Accessibility par défaut, ou conserver les attributions existantes ; les raccourcis par défaut en conflit restent alors non attribués. **Ne plus me demander pour cette version** s’applique à tous les profils, mais une nouvelle version ou compilation déclenche un nouveau contrôle.

## Éditeur de raccourcis clavier

Ouvrez **Outils > Accessible OBS Studio** pour ouvrir directement l’éditeur de raccourcis clavier. Les flèches parcourent la liste des commandes ; Tab passe entre la commande sélectionnée, la case de contrôle des raccourcis OBS —décochée par défaut— et les boutons. Entrée ou **Ajouter ou modifier** ouvre la boîte d’attribution. Entrée et OK vérifient immédiatement les doublons. En cas de conflit, l’autre commande est nommée : Non revient à la saisie, Oui réattribue le raccourci. Utilisez **Paramètres API OpenAI** dans cet éditeur pour configurer OpenAI.

## Mélangeur et commandes multimédias

Ctrl+3 place le focus sur le mélangeur OBS standard. Le module ne numérote plus ses curseurs et n’y installe plus de filtre d’événements global. Ctrl+` ouvre la console accessible modale : Gauche et Droite changent de source, Haut et Bas modifient le volume de 1 dB, Origine règle 0 dB, Espace active ou désactive le son, et 1 à 0 sélectionnent les dix premières sources.

À son ouverture, la console prend le contrôle exclusif des sources, volumes et états de coupure disponibles à cet instant. Ses modifications sont appliquées immédiatement à OBS, mais elle ne surveille pas les changements effectués pendant la session au moyen du mélangeur natif, d’un contrôleur externe, d’un changement de scène ou d’une autre méthode. Ne manipulez pas le mélangeur ailleurs pendant que la console est ouverte ; fermez-la puis rouvrez-la pour charger les changements externes.

## Description du canevas

Les cinq modes capturent le canevas rendu par OBS. Seule la réponse la plus récente utilise le rôle ARIA alert agressif ; la question de l’utilisateur n’est jamais répétée. Les cinq modes acceptent des questions complémentaires.

Dans la description de base, **Description détaillée** est toujours disponible, **Lire le texte** uniquement si du texte a été détecté, **Personnes et arrière-plans** uniquement si des personnes ont été détectées, et **Corrections suggérées** uniquement si un problème peut réellement être corrigé automatiquement. Ces actions réutilisent l’image déjà envoyée.

Le mode **Personnes et arrière-plans** donne la priorité aux personnes visibles, puis décrit leur arrière-plan immédiat. Les détails sans rapport concernant l’interface, le texte ou la scène sont omis sauf s’ils influencent directement la présentation d’une personne.

**Vérification visuelle** contrôle uniquement l’aspect de la diffusion ou de l’enregistrement. Les contrôles visuels existants concernant la mise en page OBS, les captures vides, la caméra, l’éclairage, le plein écran Zoom, le cadrage, le flou, une lentille potentiellement sale, le grain, l’apparence, les vêtements, l’arrière-plan et les objets indésirables sont conservés. Le contenu verbal est ignoré : langue, orthographe, grammaire, traduction, formulation, faits, nombres, sujet, ton, pertinence, sous-titres et légendes. Le texte n’est signalé comme objet visuel que s’il est trop petit, coupé, flou, peu contrasté, masqué ou s’il masque des éléments visuels importants. Une boîte de dialogue ou d’erreur n’est signalée que si elle masque du contenu ou révèle un problème visible de capture ou de mise en page, jamais à cause de son message. La correction automatique reste limitée à une liste fixe de transformations OBS réversibles. **Vérifier à nouveau** capture une nouvelle image et signale les améliorations, dégradations, changements et problèmes visuels restants.

La clé API est validée avant l’enregistrement, conservée dans le Gestionnaire d’informations d’identification Windows et jamais affichée. Sa suppression demande confirmation et affiche un message de réussite.

## Confidentialité et licence

Les fonctions du canevas envoient à OpenAI l’image capturée, la langue d’OBS, des instructions de sécurité fixes et les questions complémentaires. Il n’y a ni télémétrie ni publicité. Copyright (C) 2026 [Tiflo.Info](https://tiflo.info). GNU GPL version 2 ou ultérieure ; voir [LICENSE.txt](../LICENSE.txt). [English](../README.md).
