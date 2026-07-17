# Accessible OBS Studio 1.0

[![Logo Tiflo.Info : des vagues bleues au-dessus du nom Tiflo.info et de la devise russe « Ferme les yeux et regarde »](../assets/tiflo-info-logo.jpg)](https://tiflo.info)

Accessible OBS Studio est un module d’accessibilité pour OBS Studio sous Windows. Il est destiné aux personnes aveugles qui utilisent le clavier et un lecteur d’écran, notamment JAWS ou NVDA.

## Configuration requise et installation

Le module nécessite Windows 10 ou 11 64 bits et une version compatible d’OBS Studio 32 64 bits. Seules les fonctions OpenAI exigent une clé API personnelle et une connexion Internet.

Fermez OBS, puis exécutez `AccessibleOBSStudio-1.0-Setup.exe`. Le programme d’installation place le module dans `C:\ProgramData\obs-studio\plugins\accessible-obs-studio` et vérifie Microsoft WebView2 ainsi que Visual C++. Seuls les composants absents sont téléchargés directement depuis Microsoft ; une connexion Internet est alors nécessaire. Aucun fichier OBS ou Qt n’est remplacé. Pour désinstaller le module, utilisez Applications installées dans Windows. Les paramètres sont conservés par défaut.

## Raccourcis par défaut

- F3 : capturer et analyser le canevas avec OpenAI.
- Ctrl+M : placer le focus sur les commandes multimédias natives visibles.
- F5 : démarrer ou arrêter la diffusion.
- F6 / Maj+F6 : zone principale suivante / précédente.
- F7 : démarrer ou arrêter l’enregistrement.
- F8 : activer ou désactiver la caméra virtuelle.
- Ctrl+accent grave : ouvrir la console de volume accessible. Il s’agit de la touche physique juste sous Échap ; son caractère dépend de la disposition du clavier.
- Ctrl+0 à Ctrl+5 : aperçu vidéo, scènes, sources, mélangeur audio, transitions ou commandes.

La commande **Accessible OBS Studio: Open Accessible OBS Studio** ouvre l’éditeur de raccourcis. Elle peut recevoir un raccourci, mais aucun n’est attribué par défaut.

Au premier démarrage, le module désactive les raccourcis OBS lorsque la fenêtre principale d’OBS n’a pas le focus, uniquement si vous n’avez jamais défini ce réglage vous-même. Toute modification ultérieure est respectée.

## Éditeur de raccourcis

Ouvrez **Outils > Accessible OBS Studio**. Recherchez une commande, puis appuyez sur Entrée ou activez **Ajouter ou modifier**. La configuration OpenAI est accessible avec **API Settings** dans cette fenêtre. La boîte « Hot Key » gère plusieurs raccourcis par commande. **Ajouter un autre raccourci** crée une attribution supplémentaire. Entrée valide et Échap annule. Tab, Maj+Tab, Entrée, Échap, Alt+F4, les combinaisons avec la touche Windows et les commandes système réservées ne sont pas capturées.

Les changements ne sont inscrits dans OBS qu’après activation de OK dans la fenêtre principale. Suppr ou **Supprimer** efface toutes les attributions de la commande sélectionnée. Si des changements ne sont pas enregistrés, la fermeture propose Enregistrer, Ignorer les modifications et Annuler.

## Audio et médias

Ctrl+3 place le focus sur le mélangeur audio. Les réglages de volume visibles sont numérotés dans l’ordre d’affichage. Les touches 1 à 9 sélectionnent les neuf premières sources et 0 la dixième. Les flèches et les autres commandes restent celles d’OBS. Ctrl+M place le focus sur les commandes multimédias lorsque celles-ci sont affichées ; le module ne remplace aucune touche multimédia d’OBS.

Ctrl+accent grave ouvre la **console de volume accessible** modale. Le module mémorise le contrôle OBS précédemment ciblé et le restaure après Échap. La console présente toutes les sources audio actuellement actives dans le mélangeur OBS, y compris les sources pertinentes dans les groupes ou scènes imbriquées et les périphériques globaux. Gauche et Droite changent de source, Haut et Bas modifient le volume de 1 dB, Origine rétablit 0 dB et Espace active ou coupe le son. Les touches 1 à 9 sélectionnent les neuf premières sources et 0 la dixième. Les changements sont immédiats et annoncés par JAWS et NVDA. La commande et son raccourci peuvent être modifiés dans l’éditeur.

## Analyse du canevas et actions

La clé API est facultative. Enregistrez-la uniquement si nécessaire avec **API Settings** dans l’éditeur de raccourcis. La boîte de dialogue accepte Tab et Maj+Tab ; Entrée dans le champ de la clé active **Enregistrer la clé**. Le format et l’authentification OpenAI sont vérifiés avant le stockage chiffré dans le Gestionnaire d’identifiants Windows. Sans clé, seules les fonctions OpenAI sont bloquées ; toutes les autres restent disponibles.

F3 ne nécessite pas de placer le focus sur l’aperçu. Un bref clic confirme le démarrage de la requête. Le focus reste inchangé pendant la capture. La fenêtre de résultat reçoit ensuite le focus et, à sa fermeture, le contrôle OBS précédent est restauré s’il existe encore. Le module ne reprend jamais le focus à une autre application.

Les questions complémentaires doivent porter directement sur l’image capturée. Le texte de l’image et les questions sont traités comme des données non fiables. Les questions sans rapport sont refusées. La conversation prend fin à la fermeture de la fenêtre ou lors d’une nouvelle capture.

**Actions suggérées** présente une liste limitée de transformations OBS réversibles. Si aucune source n’est sélectionnée, une liste de sources accessible apparaît. Chaque action possède sa propre case et son niveau de risque. Rien n’est exécuté avant l’activation de **Appliquer la sélection**. Le module utilise les actions natives et l’historique Annuler d’OBS. La diffusion, l’enregistrement, l’audio, les identifiants, les commandes arbitraires et les réglages de sortie sont exclus.

## Analyse de compatibilité

Pour une version majeure d’OBS non testée, l’avertissement propose **Annuler**, **Exécuter quand même** et **Analyser la compatibilité**. L’analyse associe des contrôles locaux en lecture seule à des sources OBS officielles et classe le risque comme Faible, Moyen, Élevé ou Inconnu. Elle ne constitue jamais une garantie.

Un rapport réussi est enregistré pour les versions exactes d’OBS et du module, ainsi que pour l’architecture. Il est ensuite réaffiché sans nouvel appel OpenAI, dans une fenêtre WebView2 accessible, et copié dans le Presse-papiers.

## Confidentialité, coût et licence

L’analyse du canevas transmet à OpenAI le canevas, la langue d’OBS, une invite de sécurité fixe et les questions valides. L’analyse de compatibilité transmet les versions, les dépendances et un manifeste fixe, mais aucune scène, aucun média ni identifiant. Les frais d’API incombent au propriétaire de la clé. Le module ne contient ni télémétrie ni publicité.

Copyright (C) 2026 [Tiflo.Info](https://tiflo.info). GNU GPL version 2 ou ultérieure ; voir [LICENSE.txt](../LICENSE.txt). [English](../README.md).
