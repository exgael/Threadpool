INF3173-231 TP3
=======================================

## Dépendances

Ubuntu:

```sh
  apt-get install build-essential cmake libpng-dev
```

### Compilation manuelle:

#### Methode 1

```sh
cmake -S . -B build
cd build
make 
make test
cd ..
```

#### Methode 2

```sh
cmake -S . -B build
cd build
make 
ctest
```

## Fonctionnement du programme

```sh
# Télécharger les images de test
./data/fetch.sh

# Ajouter le programme dans le PATH par simplicité
source ./build/env.sh

# Appel principal avec répertoire d’image d’entrée et de sortie
ieffect --input data/ --output results/
```

## Comment faire le fichier dist

Dans la racine du projet:

```sh
cmake -S . -B build
cd build
make dist
