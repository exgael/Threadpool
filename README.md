INF3173-231 TP3
=======================================

## Dépendances

Ubuntu:

```sh
  apt-get install build-essential cmake libpng-dev
```


Il est également recommandé d'utiliser un IDE, tel que QtCreator, pour programmer la solution.

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

## Comment faire la remise

Dans la racine du projet:

```sh
cmake -S . -B build
cd build
make dist
```

Il faut choisir le fichier compresser en `tar.gz`

## Date echeance

~mercredi 12 avril 2023, 23:59~

samedi 15 avril 2023, 23:59 
