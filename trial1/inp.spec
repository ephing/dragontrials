_(T|F) BOOL true
_A FORALL false
_E EXISTS false
[(] LPAREN false
[)] RPAREN false
[a-zA-Z] PROP true
[&] AND false
[|] OR false
^ XOR false
~ NOT false
-> IF false
<-> IFF false
\_|\t (SKIP)
. (ERR) "Bad input"