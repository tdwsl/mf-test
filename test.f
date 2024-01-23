
( simple test program for mfa )

: space # 20 emit ;
: cr # a emit ;

: puts dup c@ if emit 1+ puts ; then 2drop ;

: .digit dup # a - -if drop # [ "a a - ] + else drop # "0 + then emit ;

: . dup 2/ 2/ 2/ 2/ ? if . else drop then
  # f and .digit ;

: msg " Hello, world!" { a 0 } ;

: arr [ 4 5 4 13 2a ] ;

: parr !a a@+ 1- for a@+ . space next cr ;

# a # 10 # 20 + emit emit
# msg puts

# 2213 . # a emit

# abcdef . cr

# a # 3 /mod . space . cr
# a # 3 mod . space # a # 3 / . cr

# a begin dup . space 1- ? 0until drop cr
# a for i . space next cr

# arr parr

