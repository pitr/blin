#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "blin.h"

#define P printf
#define panic(...) {P("panic: ");P(__VA_ARGS__);P("\n");exit(1);}
#define DO(n,x) {u8 i=0,_n=(n);for(;i<_n;++i){x;}}
#define push(h) {**mem=h;++*mem;}

u8 pre[] = { 0x57 }; // push rdi
u8 post[] = { 0x5f, 0xf3, 0x0f, 0x7f, 0x07, 0xc3 }; // pop rdi, write xmm0 to rdi

i32 iit[] = { 0, 1, 2, 3 };

#define NUMREG 8
u8 registers[NUMREG] = {1}; // register reference counter up to xmm7, xmm0 is taken

u8 getreg()
{
    for (u8 i = 0; i < NUMREG; ++i) {
        if(registers[i] == 0) {
            registers[i]++;
            return i;
        }
    }
    panic("all registers are used up");
}

void freereg(u8 reg)
{
    if(reg == 0) {
        panic("cannot free xmm0");
    }
    if(registers[reg] == 0) {
        panic("register already free");
    }
    registers[reg]--;
}

typedef enum {in_imm, in_reg, in_nil} in_t;

// _compile: parse line, compile to memory, ensure result is in register o
void _compile(u8 **mem, u8 *line, u8 o, i32 in, in_t t)
{
    while(*line == ' ') {
        line++;
    }

    u8 c = *line;
    line++;

    if (c == '\0' || c == ')') {
        if (t == in_imm) {
            P("push %d\n", in);
            push(0x6a);push(in);
            P("vbroadcastss xmm%d, [rsp]\npop rax\n", o);
            push(0xc4);push(0xe2);push(0x79);push(0x18);push(0x04 + (8*o));push(0x24);push(0x58);
        }
        return;
    } else if (c >= '0' && c <= '9') {
        int n = c-'0';
        if (t == in_imm) {
            n+=10*in;
        }
        _compile(mem, line, o, n, in_imm);
    } else if (c == '!') {
        if (t != in_nil) {
            panic("val! syntax is not supported");
        }
        P("mov rax, [iit]\n");
        push(0x48);push(0xb8);
        i32 *a = &iit[0];
        memcpy(*mem, &a, 8);
        *mem += 8;
        P("movdqu xmm%d, [rax]\n", o);
        push(0xf3);push(0x0f);push(0x6f);push(o*8);
        _compile(mem, line, o, o, in_reg);
    } else if (c == '+' || c == '-' || c == '*' || c == '|' || c == '&' || c == '=' || c == '<') {
        u8 c2 = *line;
        if(c2=='/' || c2=='\'') line++;
        else c2 = ' ';

        if(t == in_nil) {
            if(c=='-') panic("-expr is not supported");

            _compile(mem, line, o, in, t);
            P("%c%c xmm%d, xmm%d\n", c, c2, o, o);

            push(0x66);push(0x0f);
            if (c=='*' || c=='|' || (c=='+' && c2 != ' ')) {push(0x38);}
            push(c=='+' ? (c2=='/' ? 0x02 : c2=='\'' ? 0x06 : 0xfe) : c=='*' ? 0x40 : c=='-' ? 0xfa : c=='|' ? 0x1e : c=='&' ? 0xdb : c=='=' ? 0x76 : c=='<' ? 0x66 : 0x00);
            push(0xc0 | (o<<3) | o);
        } else if (t == in_imm) {
            _compile(mem, line, o, in, in_nil);
            u8 left = getreg();

            P("push %d\n", in);
            push(0x6a);push(in);
            P("vbroadcastss xmm%d, [rsp]\npop rax\n", left);
            push(0xc4);push(0xe2);push(0x79);push(0x18);push(0x04 + (8*left));push(0x24);push(0x58);

            P("%c%c xmm%d, xmm%d\n", c, c2, o, left);
            push(0x66);push(0x0f);
            if (c=='*' || c=='|' || (c=='+' && c2 != ' ')) {push(0x38);}
            push(c=='+' ? (c2=='/' ? 0x02 : c2=='\'' ? 0x06 : 0xfe) : c=='*' ? 0x40 : c=='-' ? 0xfa : c=='|' ? 0xeb : c=='&' ? 0xdb : c=='=' ? 0x76 : c=='<' ? 0x66 : 0x00);
            if (c=='-') push(0xc0 | (left<<3) | o)
            else push(0xc0 | (o<<3) | left)

            freereg(left);
        } else {
            u8 right = getreg();
            _compile(mem, line, right, in, in_nil);
            P("%c%c xmm%d, xmm%d\n", c, c2, o, right);
            push(0x66);push(0x0f);
            if (c=='*' || c=='|' || (c=='+' && c2 != ' ')) {push(0x38);}
            push(c=='+' ? (c2=='/' ? 0x02 : c2=='\'' ? 0x06 : 0xfe) : c=='*' ? 0x40 : c=='-' ? 0xfa : c=='|' ? 0xeb : c=='&' ? 0xdb : c=='=' ? 0x76 : c=='<' ? 0x66 : 0x00);
            if (c=='-') push(0xc0 | (right<<3) | o)
            else push(0xc0 | (o<<3) | right)
            freereg(right);
        }
    } else {
        panic("unknown op: %c", c);
    }
}

#define BUF 1024

ptr compile(u8 *line)
{
    ptr m = mmap(0, sizeof(pre)+BUF+sizeof(post), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ptr buf = m;

    memcpy(buf, pre, sizeof(pre));
    buf += sizeof(pre);

    u8 code[] = { 0x66, 0x0f, 0xfe, 0xc1, 0x66, 0x0f, 0x38, 0x40, 0xc1 };
    memcpy(buf, code, sizeof(code));
    _compile(&buf, line, 0, 0, in_nil);

    memcpy(buf, post, sizeof(post));

    return m;
}

typedef void (*func)(i32 *);

void run(ptr m, i32 result[4]) {
    func f = (func)m;
    f(&result[0]);
}
