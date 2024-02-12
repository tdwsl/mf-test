
( core words )

: +   { 83 ef 04 } { 03 07 } ;inline
: -   { 83 ef 04 } { f7 d8 03 07 } ;inline
: and { 83 ef 04 } { 23 07 } ;inline
: or  { 83 ef 04 } { 0b 07 } ;inline
: xor { 83 ef 04 } { 33 07 } ;inline
: emit { 51 } { 89 F9 AB B8 04 00 00 00 BB 01 00 00 00 BA 01 00 00 00 CD 80 }
  { 59 } { 83 ef 08 } { 8b 07 } ;
: drop { 83 ef 04 } { 8b 07 } ;inline
: 2drop { 83 ef 08 } { 8b 07 } ;inline
: swap { 87 47 fc } ;inline
: c@ { 8a 00 25 ff 00 00 00 } ;inline
: @ { 8b 00 } ;inline
: !a { 83 EF 04 89 C1 8B 07 } ;inline
: @a { AB 89 C8 } ;inline
: a!+ { 89 01 83 C1 04 83 EF 04 8B 07 } ;inline
: a@+ { AB 8B 01 83 C1 04 } ;inline
: dup { ab } ;inline
: over { ab 8b 47 f8 } ;inline
: 1+ { 40 } ;inline
: 1- { 48 } ;inline
: ! { 8B 5F FC 89 18 83 EF 08 8B 07 } ;inline
: c! { 8B 5F FC 88 18 83 EF 08 8B 07 } ;inline
: * { 83 ef 04 f7 27 } ;inline
: /mod { 31 D2 89 C3 8B 47 FC F7 F3 89 57 FC } ;inline
: / { 31 D2 89 C3 83 EF 04 8B 07 F7 F3 } ;inline
: mod { 31 D2 89 C3 83 EF 04 8B 07 F7 F3 89 D0 } ;inline
: 2/ { d1 e8 } ;inline
: 2* { d1 e0 } ;inline
: i { ab 8b 04 24 } ;inline
: bye { b8 01 00 00 00 cd 80 } ;inline

