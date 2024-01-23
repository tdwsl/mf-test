#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORYSZ 1024*1024
#define HEADERSZ 0x1000
#define MAXWORDS 1200
#define MAXSTRING 32768
#define MAXINLINE 4096
#define CSZ 256
#define ESZ 16
#define ISZ 256

#define DATA 0x0805a000
#define ORG 0x08049000

unsigned char header[HEADERSZ];
unsigned char memory[MEMORYSZ];
unsigned nmemory = 0;

char *words[MAXWORDS];
unsigned worda[MAXWORDS];
unsigned char wordn[MAXWORDS];
unsigned char inln[MAXINLINE];
int ninln = 0;
int nwords = 0;

char strings[MAXSTRING];
char *stringp = strings;

unsigned lastRet;

unsigned cs[CSZ];
unsigned cp = 0;
unsigned is[ISZ];
unsigned ip = 0;
unsigned es[ESZ];
unsigned ep = 0;

char *addName(char *name) {
    char *s;
    for(s = strings; s < stringp; s += strlen(s)+1)
        if(!strcmp(s, name)) return s;
    s = stringp;
    strcpy(stringp, name);
    stringp += strlen(stringp)+1;
    return s;
}

void loadHeader(const char *filename) {
    FILE *fp;
    fp = fopen(filename, "rb");
    if(!fp) { printf("failed to open binary %s\n", filename); exit(1); }
    fread(header, 1, HEADERSZ, fp);
    fclose(fp);
}

int number(char *s, int *n) {
    *n = 0;
    do {
        *n <<= 4;
        if(*s >= '0' && *s <= '9') *n += *s-'0';
        else if(*s >= 'a' && *s <= 'f') *n += *s-'a'+10;
        else if(*s >= 'A' && *s <= 'F') *n += *s-'A'+10;
        else return 0;
    } while(*++s);
    return 1;
}

int findWord(char *s) {
    int i;
    for(i = nwords-1; i >= 0; i--)
        if(!strcmp(words[i], s)) return i;
    return -1;
}

void getNext(char *buf, FILE *fp) {
    char c;
    while((c = fgetc(fp)) <= 32 && !feof(fp));
    if(c <= 32) { *buf = 0; return; }
    do *(buf++) = c; while((c = fgetc(fp)) > 32 && !feof(fp));
    *buf = 0;
}

int value(char *s, FILE *fp) {
    int n;
    if((n = findWord(s)) != -1 && !wordn[n]) return worda[n];
    else if(number(s, &n)) return n;
    else if(s[0] == '"' && s[1]) return s[1];
    else if(!strcmp(s, "here")) return nmemory+ORG;
    else if(!strcmp(s, "org")) return ORG;
    else if(!strcmp(s, "[")) {
        getNext(s, fp);
        es[ep] = value(s, fp);
        while(strcmp(s, "]")) {
            if(!strcmp(s, "+")) { ep--; es[ep] += es[ep+1]; }
            else if(!strcmp(s, "-")) { ep--; es[ep] -= es[ep+1]; }
            else if(!strcmp(s, "*")) { ep--; es[ep] *= es[ep+1]; }
            else if(!strcmp(s, "/")) { ep--; es[ep] /= es[ep+1]; }
            else if(!strcmp(s, "mod")) { ep--; es[ep] %= es[ep+1]; }
            else if(!strcmp(s, "and")) { ep--; es[ep] &= es[ep+1]; }
            else if(!strcmp(s, "or")) { ep--; es[ep] |= es[ep+1]; }
            else if(!strcmp(s, "xor")) { ep--; es[ep] ^= es[ep+1]; }
            else es[++ep] = value(s, fp);
            getNext(s, fp);
        }
        return es[ep];
    }
    printf("%s ?\n", s);
}

void asmFile(const char *filename) {
    FILE *fp;
    int i, j;
    char prevCall;
    char buf[200];
    fp = fopen(filename, "r");
    if(!fp) { printf("failed to open source %s\n", filename); exit(1); }
    prevCall = 0;
    for(;;) {
        getNext(buf, fp);
        if(!buf[0]) break;
        if((i = findWord(buf)) != -1) {
            if(wordn[i]) {
                prevCall = 0;
                for(j = 0; j < inln[worda[i]]; j++)
                    memory[nmemory++] = inln[worda[i]+1+j];
            } else {
                memory[nmemory++] = 0xe8;
                *(int*)&memory[nmemory] = worda[i]-ORG-nmemory-4;
                nmemory += 4;
                prevCall = 1;
            }
        } else if(!strcmp(buf, ";")) {
            if(prevCall) memory[nmemory-5] = 0xe9;
            else memory[nmemory++] = 0xc3;
            prevCall = 0;
            lastRet = nmemory;
        } else if((prevCall = 0) | !strcmp(buf, ";inline")) {
            worda[nwords-1] -= ORG;
            j = ninln;
            inln[ninln++] = nmemory-worda[nwords-1];
            for(i = worda[nwords-1]; i < nmemory; i++)
                inln[ninln++] = memory[i];
            nmemory = worda[nwords-1];
            worda[nwords-1] = j;
            wordn[nwords-1] = 1;
        } else if(!strcmp(buf, "[")) {
            for(;;) {
                getNext(buf, fp);
                if(!strcmp(buf, "]")) break;
                *(unsigned*)&memory[nmemory] = value(buf, fp);
                nmemory += 4;
            }
        } else if(!strcmp(buf, "{")) {
            for(;;) {
                getNext(buf, fp);
                if(!strcmp(buf, "}")) break;
                memory[nmemory++] = value(buf, fp);
            }
        } else if(!strcmp(buf, "\"")) {
            while((i = fgetc(fp)) != '"')
                memory[nmemory++] = i;
        } else if(!strcmp(buf, "(")) {
            while(fgetc(fp) != ')');
        } else if(!strcmp(buf, ":")) {
            getNext(buf, fp);
            if((i = findWord(buf)) == -1) {
                i = nwords++; words[i] = addName(buf);
            }
            worda[i] = nmemory+ORG;
            wordn[i] = 0;
        } else if(!strcmp(buf, "constant")) {
            getNext(buf, fp);
            if((i = findWord(buf)) == -1) {
                i = nwords++; words[i] = addName(buf);
            }
            getNext(buf, fp);
            worda[i] = value(buf, fp);
            wordn[i] = 0;
        } else if(!strcmp(buf, "?")) {
            memory[nmemory++] = 0x09;
            memory[nmemory++] = 0xc0;
        } else if(!strcmp(buf, "if")) {
            memory[nmemory++] = 0x0f;
            memory[nmemory++] = 0x84;
            cs[cp++] = nmemory;
            nmemory += 4;
        } else if(!strcmp(buf, "0if")) {
            memory[nmemory++] = 0x0f;
            memory[nmemory++] = 0x85;
            cs[cp++] = nmemory;
            nmemory += 4;
        } else if(!strcmp(buf, "-if")) {
            memory[nmemory++] = 0x0f;
            memory[nmemory++] = 0x83;
            cs[cp++] = nmemory;
            nmemory += 4;
        } else if(!strcmp(buf, "else")) {
            memory[nmemory++] = 0xe9;
            *(int*)&memory[cs[cp-1]] = nmemory-cs[cp-1];
            cs[cp-1] = nmemory;
            nmemory += 4;
        } else if(!strcmp(buf, "then")) {
            cp--;
            *(int*)&memory[cs[cp]] = nmemory-cs[cp]-4;
        } else if(!strcmp(buf, "begin")) {
            is[ip++] = nmemory;
        } else if(!strcmp(buf, "until")) {
            memory[nmemory++] = 0x0f;
            memory[nmemory++] = 0x84;
            *(int*)&memory[nmemory] = is[--ip]-nmemory-4;
            nmemory += 4;
        } else if(!strcmp(buf, "0until")) {
            memory[nmemory++] = 0x0f;
            memory[nmemory++] = 0x85;
            *(int*)&memory[nmemory] = is[--ip]-nmemory-4;
            nmemory += 4;
        } else if(!strcmp(buf, "again")) {
            memory[nmemory++] = 0xe9;
            *(int*)&memory[nmemory] = is[--ip]-nmemory-4;
            nmemory += 4;
        } else if(!strcmp(buf, "for")) {
            *(int*)&memory[nmemory] = 0x04ef8350;
            nmemory += 4;
            memory[nmemory++] = 0x8b;
            memory[nmemory++] = 0x07;
            is[ip++] = nmemory;
        } else if(!strcmp(buf, "next")) {
            memory[nmemory++] = 0xff;
            memory[nmemory++] = 0x0c;
            memory[nmemory++] = 0x24;
            memory[nmemory++] = 0x0f;
            memory[nmemory++] = 0x89;
            *(int*)&memory[nmemory] = is[--ip]-nmemory-4;
            nmemory += 4;
            memory[nmemory++] = 0x83;
            memory[nmemory++] = 0xc4;
            memory[nmemory++] = 0x04;
        } else if(!strcmp(buf, "#")) {
            getNext(buf, fp);
            memory[nmemory++] = 0xab;
            memory[nmemory++] = 0xb8;
            *(unsigned*)&memory[nmemory] = value(buf, fp);
            nmemory += 4;
        } else {
            printf("%s ?\n", buf);
            exit(1);
        }
    }
    fclose(fp);
}

void saveFile(const char *filename) {
    FILE *fp;
    fp = fopen(filename, "wb");
    if(!fp) { printf("failed to open output %s\n", filename); exit(1); }
    fwrite(header, 1, HEADERSZ, fp);
    fwrite(memory, 1, nmemory, fp);
    fclose(fp);
}

int main(int argc, char **args) {
    int i, start;
    if(argc < 3) {
        printf("usage: mfa <file1,file2,...> <out>\n");
        return 1;
    }
    loadHeader("header.bin");
    memory[nmemory++] = 0xbf;
    *(unsigned*)&memory[nmemory] = DATA;
    nmemory += 4;
    memory[nmemory++] = 0xe9;
    start = nmemory;
    nmemory += 4;
    lastRet = nmemory;
    asmFile("core.f");
    for(i = 1; i < argc-1; i++)
        asmFile(args[i]);
    *(int*)&memory[start] = lastRet-start-4;
    memory[nmemory++] = 0xb8;
    *(unsigned*)&memory[nmemory] = 1;
    nmemory += 4;
    memory[nmemory++] = 0xcd;
    memory[nmemory++] = 0x80;
    saveFile(args[argc-1]);
    printf("compiled %d bytes\n", nmemory+HEADERSZ);
    return 0;
}
