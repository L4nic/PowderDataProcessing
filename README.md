# PowderDataProcessing

**Version 2.0**  
Une application Qt pour le traitement automatisé de données issues de la diffraction des rayons X.

Développé par Y. Filinchuk & Loïc Rochez – UCLouvain

---

## 🧪 Description

**PowderDataProcessing** est un outil graphique intuitif permettant d'analyser et transformer efficacement des fichiers expérimentaux liés à la diffraction des rayons X.  
Les modules disponibles couvrent différentes tâches : ajout d'incertitudes statistiques (sigmas), normalisation, mise à l'échelle, et extraction de facteurs depuis des fichiers `.seq`.

---

## 🖥️ Interface & Fonctionnalités

L'application se compose d'une interface graphique simple permettant de :
- Sélectionner les fichiers d'entrée/sortie,
- Saisir les paramètres nécessaires (distance détecteur-échantillon, plages de 2Theta, etc.),
- Lancer le traitement en un clic.

### Modules disponibles :
- **Add Sigmas**
- **Add Sigmas LEO**
- **Add Sigmas Series**
- **Normalize**
- **Scale Series**
- **Seq Scales**
- **Shorter Buffer**

👉 Pour une explication détaillée de chaque module, consultez le fichier [`help.txt`](./help.txt).

---

## ⚙️ Compilation

Ce projet utilise **CMake** et **Qt6**.

### 🔧 Prérequis :
- Qt6 (Widgets)
- CMake >= 3.5
- Un compilateur C++ compatible C++17

### 💡 Exemple de compilation sous Linux :
```bash
mkdir build
cd build
cmake ..
make
