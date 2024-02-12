
( caesar cipher )

: shift ( k c -- c )
  dup # "a - ? +if drop dup # [ "z 1 + ] - ? -if drop
    # "a - + # 1a mod # "a + ;
  then then drop
  dup # "A - ? +if drop dup # [ "Z 1 + ] - ? -if drop
    # "A - + # 1a mod # "A + ;
  then then drop
  swap drop ;

: str " Hello, world!" { 0 } ;

: .shift ( k s -- )
  over over c@ ? if shift emit 1+ .shift ; then
  2drop 2drop ;

# 7 # str .shift # a emit

