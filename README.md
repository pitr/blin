# blin
micro assembler

```
> 3+4

# assembly
push 4
vbroadcastss xmm0, [rsp]
pop rax
push 3
vbroadcastss xmm1, [rsp]
pop rax
xmm0, xmm1

# result
7 7 7 7

> !+2

# assembly
mov rax, [iit]
movdqu xmm0, [rax]
push 2
vbroadcastss xmm1, [rsp]
pop rax
xmm0, xmm1

# result
2 3 4 5

>
```
