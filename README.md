# DS3 Button Tester - PS Vita / PS TV

Application homebrew pour tester tous les boutons d'une manette **DualShock 3** sur **PS Vita** et **PS TV**.

## Fonctionnalités

- **D-Pad** : Haut, Bas, Gauche, Droite
- **Boutons face** : Triangle, Carré, Croix, Rond
- **Gâchettes** : L1, R1, L2, R2
- **Sticks** : L3, R3 (clic)
- **Sticks analogiques** : Visualisation en temps réel avec valeurs X/Y
- **Boutons centraux** : Select, Start, PS
- **Valeur brute** : Affichage hexadécimal des boutons pressés
- **Multi-port** : Lit le port 0 (Vita) ET port 1 (DS3 externe sur PS TV)

## Prérequis

1. **VitaSDK** installé : https://vitasdk.org/
2. **CMake** 3.0+
3. Une PS Vita ou PS TV avec **HENkaku/Enso** et **VitaShell**

## Compilation

```bash
# Configurer VitaSDK
export VITASDK=/usr/local/vitasdk
export PATH=$VITASDK/bin:$PATH

# Compiler
chmod +x build.sh
./build.sh
```

Le fichier `build/ds3_tester.vpk` sera généré.

## Installation

1. Transférer `ds3_tester.vpk` sur la PS Vita via USB ou FTP
2. Ouvrir **VitaShell**
3. Naviguer vers le fichier VPK
4. Appuyer sur **X** pour installer
5. L'application "DS3 Button Tester" apparaît sur le LiveArea

## Utilisation

- Lancer l'application depuis le LiveArea
- Brancher une manette DS3 (PS TV) ou utiliser les boutons Vita
- Chaque bouton s'allume en **vert** quand il est pressé
- Les sticks analogiques affichent leur position en temps réel
- **START + SELECT** simultanément pour quitter

## Structure du projet

```
ds3-tester/
├── CMakeLists.txt          # Configuration CMake pour VitaSDK
├── build.sh                # Script de compilation
├── README.md               # Ce fichier
├── src/
│   └── main.c              # Code source principal
└── sce_sys/
    ├── icon0.png           # Icône de l'application (128x128)
    └── livearea/
        └── contents/
            ├── bg.png      # Fond du LiveArea (840x500)
            ├── startup.png # Image de démarrage (280x158)
            └── template.xml # Template LiveArea
```

## Notes techniques

- Utilise le framebuffer direct (pas de GXM/GPU) pour la simplicité
- Police bitmap 5x7 intégrée (pas de dépendance externe)
- Compatible Vita (contrôles tactiles mappés) et PS TV (DS3 externe)
- `sceCtrlPeekBufferPositiveExt2` pour la lecture multi-port PS TV

## Licence

Libre d'utilisation. Projet homebrew éducatif.
