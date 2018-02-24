Linguagem
=========
*Runtime* funciona baseado numa pilha implícita que contém valores `double`.
Todas as operações funcionam nessa pilha, que possui um tamanho máximo definido
em tempo de compilação.

Funções
-------
Funções são do tipo `void (pilha_t *)`, sendo a pilha dinâmica e sequencial:
uma `struct { double *v; int s; }` contendo um vetor de números e seu tamanho.

Funções podem receber quantos argumentos quiser pela pilha e empilhar quantos
retornos quiser. Cuidado com *overflow* e *underflow*!

Instruções que estiverem fora de definições de funções serão adicionadas à
função `main`, implicitamente criada pelo compilador.


Instruções
----------
Instrução      | Pilha anterior | Pilha atual
-------------- | -------------- | -----------
dup            | [..., x]       | [..., x, x]
rot            | [..., x, y]    | [..., y, x]
add            | [..., x, y]    | [..., (x + y)]
sub            | [..., x, y]    | [..., (x - y)]
mul            | [..., x, y]    | [..., (x * y)]
div            | [..., x, y]    | [..., (x / y)]
mod            | [..., x, y]    | [..., (x % y)]
push *CONST*   | [...]          | [..., *CONST*]
call *FUNC*    | [...]          | Depende da função
print          | [..., x]       | [...] (e printa o valor de `x`)
print *STRING* | [...]          | [...] (e printa o valor de *STRING*)


Gramática
---------
```
Programa <- HashBang? S ((DefFunção / Instrução) S)* !.

HashBang <- "#!" [^\n]*

Instrução <- (I"DUP"
            / I"ROT"
            / I"ADD"
            / I"SUB"
            / I"MUL"
            / I"DIV"
            / I"MOD"
            / I"PUSH" EspaçoArg Constante
            / I"CALL" EspaçoArg NomeFunção
            / I"PRINT" (EspaçoArg String)?
            )
            EOI

DefFunção <- I"FUNCTION" EspaçoArg NomeFunção EOI (Instrução S)* I"END"
NomeFunção <- [_\w] [_\w\d]*

Constante <- \d+  # por enquanto
String <- "\"" (!"\"" Char)* "\""
Char <- "\\" [abfnrtv\'\"\\]
      / .

EspaçoArg <- (" " / "\t")*
EOI <- (" " / "\t" / Comentário)* (";" / EOL / !.)
EOL <- "\n"
Comentário <- "#" [^\n]*
S <- (\s / Comentário)*
```
