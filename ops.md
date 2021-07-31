ops


num      vpbroadcastd - fill

+        paddd
-        psubd
*        pmulld
  (max)  pmaxud
  (min)  pminud
| (abs)  pabsd
  (sign) psignd (b<0 => -a; b=0 => 0; b>0 => a)
  (+/)   phaddd (a[1]+a[0] ; a[3]+a[2] ; b[1]+b[0] ; b[3]+b[2])
  (-')   phsubd (a[1]-a[0] ; a[3]-a[2] ; b[1]-b[0] ; b[3]-b[2])

=        pcmpeqd (a=b)
>        pcmpgtd (a>b)

&        pand
(andnot) pandn (!a & b)
|        por
^ (xor)  pxor
~        ptest (sets CF/ZF)
 (<<)    pslld
 (>>)    psrld
