# webserv

Implémentation d’un serveur HTTP en C++98 (projet Common Core 42).

## Fonctionnalités

- Serveur non bloquant multi-clients (poll).
- Parsing de configuration type Nginx simplifié.
- Méthodes HTTP gérées : `GET`, `POST`, `DELETE`.
- Fichiers statiques, autoindex, pages d’erreur.
- Upload de fichiers.
- Exécution CGI (`.pl`, `.py`, `.sh`) avec gestion de timeout.
- Gestion basique des connexions persistantes (keep-alive).

## Arborescence utile

- `srcs/` : implémentations C++.
- `include/` : headers.
- `conf/webserv.conf` : configuration principale d’exemple.
- `var/www/` : contenus statiques et pages d’erreur.
- `var/cgi-bin/` : scripts CGI de test.

## Compilation

Prérequis:

- `c++` compatible C++98
- `make`

Build:

```bash
make
```

Nettoyage:

```bash
make clean
make fclean
```

## Lancement

Le binaire attend **obligatoirement** un fichier de configuration :

```bash
./webserv conf/webserv.conf
```

Ports utilisés par la config fournie :

- `8080` : contenu statique
- `8081` : upload (`POST` / `DELETE`)
- `8082` : CGI

Arrêt serveur : `Ctrl+C`.

## Tests rapides

### 1) Page statique

```bash
curl -i http://127.0.0.1:8080/
```

### 2) Upload (selon config)

```bash
curl -i -X POST --data-binary @README.md http://127.0.0.1:8081/
```

### 3) CGI

```bash
./test_cgi.sh
```

Ou avec URL explicite :

```bash
./test_cgi.sh http://127.0.0.1:8082
```

## Notes

- Projet académique 42, orienté apprentissage de la programmation réseau bas niveau.
- Le comportement exact dépend du fichier de configuration chargé.