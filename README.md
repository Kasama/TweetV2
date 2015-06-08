# TweetV2
Second version of Tweet

Anatomia do arquivo de Tweet:

```
+------------------+     Topo da pilha reversa de remoção, usualmente, apenas um Inteiro.
|     StackTop     |     Incluir este tamanho no HEADER. Se -1, a pilha não existe.
|                  |
+------------------+  <- Begin of Register
|     TweetSize    |     Tamanho do registro a ser lido. O valor NÃO inclui TweetSize.
|                  |     Se negativo, o tweet é considerado removido logicamente.
+------------------+
|       Text       |     Texto, texto, texto.
|                  |     Com um UnitSeparator (/31) no final.
| UnitSeparator/31 |
+------------------+
|       User       |     Nome do autor daquele Tweet
|                  |     Com um UnitSeparator (/31) no final.
| UnitSeparator/31 |
+------------------+
|       Coord      |     Coordenadas do Tweet
|                  |     Com um UnitSeparator (/31) no final.
| UnitSeparator/31 |
+------------------+
|     Language     |     Idioma daquele Tweet
|                  |     Com um UnitSeparator (/31) no final.
| UnitSeparator/31 |
+------------------+
|     Favorite     |     Número de vezes que o tweet foi favoritado. Inteiro.
|                  |
+------------------+
|      Retweet     |     Número de vezes que o tweet Retwitado. Inteiro.
|                  |
+------------------+
|       Views      |     Número de vezes que o tweet Retwitado. Long.
|                  |
+------------------+  <- End Of Register
|     TweetSize    |
|        ...       |
|                  |

```
