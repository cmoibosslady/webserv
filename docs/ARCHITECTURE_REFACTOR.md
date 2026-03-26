# Refactor architecture (webserv)

## Problèmes actuels observés

- `ClientConnection` hérite de `Parser` **et** `Response` (`multiple inheritance`) alors que ce sont 3 responsabilités différentes.
- La décision de routage (`find_type_request`) et la construction de réponse (`prepareResponse`) sont mélangées dans la connexion.
- Le parsing HTTP est fragile (headers parsés avec un split sur `:` global, peu de validation, état implicite).
- `TCPServer` pilote trop de logique applicative (lecture, routage, CGI, envoi) dans un seul flux.

## Cible (clean et modulable)

Pipeline strict:

1. Read socket bytes
2. `HttpRequestParser` -> `HttpRequest`
3. `Router` -> `RouteMatch`
4. `BodyDecoder` (selon `Content-Type`) -> `DecodedBody`
5. `Handler` (métier)
6. `ResponseBuilder` -> bytes HTTP

## Modules recommandés

- `HttpRequestParser` : start-line, headers, body, chunked/content-length
- `Router` : map `(method, path)` -> handler
- `BodyDecoder` : `x-www-form-urlencoded`, `json`, `multipart/form-data`
- `Handler` : logique métier (upload, login, delete, static, autoindex, cgi)
- `ResponseBuilder` : status line, headers, body

## Contrats de données

### `HttpRequest`
- method
- target (path + query)
- version
- headers
- rawBody

### `DecodedBody`
- kind: none | form | json | multipart
- formFields
- jsonValue
- parts (multipart)

### `RequestContext`
- serverConfig*
- locationConfig*
- client fd

## Migration incrémentale (sans casser le projet)

### Étape 1 — Séparer l’orchestration
- Créer `RequestProcessor` appelé par `TCPServer::build_client_response`.
- `RequestProcessor` prend un `ClientConnection` et retourne un `ProcessingResult`.
- Garder le comportement existant, déplacer seulement le flux décisionnel.

### Étape 2 — Découpler `ClientConnection`
- Remplacer l’héritage `Parser`/`Response` par composition:
  - `ClientConnection` contient `HttpRequestParser parser_;`
  - `ClientConnection` contient `ResponseBuilder response_;`

### Étape 3 — Introduire `Router`
- Table de routes par méthode:
  - `GET /...`
  - `POST /...`
  - `DELETE /...`
- Retourne `405` si route existe mais méthode non autorisée.

### Étape 4 — Introduire `BodyDecoder`
- Registry `content-type -> decoder`.
- `415` si type non supporté, `400` si payload invalide.

### Étape 5 — Spécialiser les handlers
- `StaticHandler`, `UploadHandler`, `CgiHandler`, `DeleteHandler`, `AutoIndexHandler`.
- `RequestProcessor` ne fait plus de logique métier.

## Règles de propreté

- Une classe = une responsabilité.
- Ne pas logger le body brut en prod.
- Toutes les erreurs HTTP passent par un mapping unique.
- APIs internes stables: `HttpRequest`, `HttpResponse`, `RouteMatch`, `DecodedBody`.

## Priorité immédiate (gros gain / faible risque)

1. Créer `RequestProcessor` et déplacer la logique de switch `request_type`.
2. Ajouter un vrai `Router` minimal (exact match d’abord).
3. Garder `CGIControler` tel quel au début (refactor plus tard).

## Arborescence suggérée

- `include/http/HttpRequest.hpp`
- `include/http/HttpResponse.hpp`
- `include/http/HttpRequestParser.hpp`
- `include/routing/Router.hpp`
- `include/routing/RouteMatch.hpp`
- `include/body/BodyDecoder.hpp`
- `include/handlers/IHandler.hpp`
- `include/handlers/StaticHandler.hpp`
- `include/handlers/UploadHandler.hpp`
- `srcs/http/...`
- `srcs/routing/...`
- `srcs/body/...`
- `srcs/handlers/...`
