#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int *mem;
int mems = 1024;

#define checkoob(addr) \
if (addr >= mems) { \
  printf("Address out of bounds!\n"); \
  return 1; \
}

#define checke \
if (errors > 0) { \
	return 1; \
}

#define abo \
int a = eval(args[1]); \
int b = eval(args[2]); \
int o = evall(args[3]); \
checkoob(o) \
checke

#define rexec(a) \
if (exec(a) > 0) { \
  return 1; \
}

#define rargs(x) \
if (x != step) { \
	printf("Invalid arguments!\n"); \
	return 1; \
}

typedef struct {
	char *name;
	int addr;
} var;

typedef struct {
	char *name;
	
	var *args;
	int argc;
	
	char *code;
} func;

var **vars;
int varc = 0;

func **funcs;
int funcc = 0;

int isnum(char *txt) {
	return (strspn(txt, "-0123456789 \n") == strlen(txt));
}

int errors = 0;

int eval(char *);
char *strclone(char *);

int cpos(char c, char *txt) {
	for (int i = 0; i < strlen(txt); i ++) {
		if (txt[i] == c) {
			return i;
		}
	}
	return -1;
}

int clast(char c, char *txt) {
	for (int i = strlen(txt); i >= 0; i --) {
		if (txt[i] == c) {
			return i;
		}
	}	
	return -1;
}

int evall(char *txt) {
    if (txt[0] == '(') {
    	txt ++;
    }
	txt[strcspn(txt, "\n)")] = 0;
	
	if (isnum(txt)) {
		printf("Cant set constants!\n");
		errors ++;
		return 0;
	}
	
	int indp = cpos('[', txt);
	if (indp > -1) {
		if (indp == 0) {
			printf("Cannot set constants!\n");
			errors ++;
			return 0;
		}

		int addro = 0;
		if (txt[0] == '&') {
			addro = 1;
		}
		
		char old = txt[indp];
		txt[indp] = 0;
		int lefta = evall(txt + addro);
		txt[indp] = old;
		
		int rpos = clast(']', txt);
		if (rpos < 0) {
			printf("Unclosed square brackets!\n");
			errors ++;
			return 0;
		}
		txt[rpos] = 0;
		int ind = eval(txt + indp + 1);
		
		if (addro) {
			return lefta + ind;
		}
		return mem[lefta + ind];
	}
	
	if (txt[0] == '*') {
		// deref
		int addr = eval(txt + 1);
		checkoob(addr)
		return addr;
	}
	
	for (int i = 0; i < varc; i ++) {
		// address of var
		if (!strcmp(vars[i]->name, txt)) {
			return vars[i]->addr;
		}
	}
	
	errors ++;
	printf("Invalid expression! (L)\n");
	// printf("%s\n", txt);
	return 0;
}

int eval(char *txt) {
	if (txt[0]== '(') {
		txt ++;
	}
	txt[strcspn(txt, "\n)\r")] = '\0';

	if (isnum(txt)) {
		return atoi(txt);
	}

    //int indp = cpos('(', txt);
    //if (indp > -1) {
    //    int rpos = clast(')', txt);
    //    if (rpos < 0) {
    //        printf("Unclosed parentheses!\n");
    //        errors ++;
    //        return 0;
    //    }
    //
    //    txt[indp] = 0;
    //    char *fn = strclone(txt);
    //    txt[indp] = '(';
    //
    //
    //
    //    free(fn);
    //}

	if (txt[0] == '&') {
		// address of
		return evall(txt + 1);
	}

	return mem[evall(txt)];
}

char *strclone(char *txt) {
	int l = strlen(txt) + 1;
	char *r = malloc(l);
	memcpy(r, txt, l);
	return r;
}

int exec(char *txtin) {
    char *txt = txtin + strspn(txtin, " \n");
    size_t inpl = strlen(txt);
    if (inpl == 0) {
      return 0;
    }

    if (txt[0] == ';') {
    	return 0;
    }
    
    if (txt[0] == '(') {
      if (strspn(txt, "() ") == inpl) {
        return 0;
      }
      size_t am = 1;
      for (size_t i = 0; i < inpl; i ++) {
        if (txt[i] == ',') {
          am ++;
        }
      }
      // example: (set 0 10, out 0)
      char **cmds = malloc(sizeof(char *) * am);
      for (size_t i = 0; i < am; i ++) {
        cmds[i] = malloc(50);
      }
      int cmdp = 0;
      int cp = 0;
      int ind = 0;
      for (size_t i = 0; i < inpl; i ++) {
        char c = txt[i];
        if (c == '(') {
          ind ++;
          if (ind == 1) {
            continue;
          }
        }
        else if (c == ',' && ind == 1) {
          cmds[cmdp][cp] = 0;
          cmdp ++;
          cp = 0;
          continue;
        }
        else if (c == ')') {
          ind --;
          if (ind == 0) {
            break;
          }
          if (ind < 0) {
            printf("Invalid syntax!\n");
            return 1;
          }
        }
        cmds[cmdp][cp] = c;
        cp ++;
      }
      cmds[cmdp][cp] = 0;
      
      for (size_t i = 0; i < am; i ++) {
        rexec(cmds[i])
        free(cmds[i]);
      }
      free(cmds);
      return 0;
    }

    char args[4][30];
    int step = 0;
    int off = 0;
    int ind = 0;
    for (size_t i = 0; i < inpl; i++) {
      char c = txt[i];
      if (c == '(') {
      	ind ++;
      }
      else if (c == ')') {
      	ind --;
      }
      else if (c == ' ' && ind == 0) {
        if (step == 3) {
          break;
        }
        args[step][off] = '\0';
        step ++;
        off = 0;
        continue;
      }
      args[step][off] = c;
      off ++;
    }
    
    if (!strcmp(args[0], "in")) {
        int addr = evall(args[1]);
        checke
        if (scanf("%i", &mem[addr]) < 1) {
            return 1;
        }
    }
    else if (!strcmp(args[0], "cin")) {
        int addr = evall(args[1]);
        checke
        if (scanf("%c", (char *)&mem[addr]) < 1) {
            return 1;
        }
    }
    else if (!strcmp(args[0], "out")) {
      rargs(1)
      printf("%i\n", eval(args[1]));
      checke
    }
    else if (!strcmp(args[0], "set")) {
      rargs(2)
      mem[evall(args[1])] = eval(args[2]);
      checke
    }
    else if (!strcmp(args[0], "add")) {
      rargs(3)
      abo
      mem[o] = a + b;
    }
    else if (!strcmp(args[0], "sub")) {
      rargs(3)
      abo
      mem[o] = a - b;
    }
    else if (!strcmp(args[0], "while")) {
      rargs(2)
      while (eval(args[1]) > 0) {
        checke
        rexec(args[2])
      }
    }
    else if (!strcmp(args[0], "if")) {
      rargs(2)
      if (eval(args[1]) > 0) {
        rexec(args[2])
      }
      checke
    }
    else if (!strcmp(args[0], "ifn")) {
      rargs(2)
      if (eval(args[1]) == 0) {
        rexec(args[2])
      }
      checke
    }
    else if (!strcmp(args[0], "ife")) {
        rargs(3)
        if (eval(args[1]) > 0) {
            rexec(args[2])
        } else {
            rexec(args[3])
        }
        checke
    }
    else if (!strcmp(args[0], "var")) {
      rargs(2)
      vars = realloc(vars, sizeof(var *) * (varc + 1));
      vars[varc] = malloc(sizeof(var));
      vars[varc]->name = strclone(args[1]);
      vars[varc]->addr = eval(args[2]);
      checke
      varc ++;
    }
    else {
      int v = eval(txt);
      checke
      printf("%i\n", v);
    }
    return 0;
}

int main(int argc, char **argv) {
    char *par = 0;
    for (int i = 1; i < argc; i ++) {
        if (argv[i][0] == '-') {
            par = argv[i] + 1;
        }
        else if (par == 0) {
            FILE* filePointer;
            filePointer = fopen(argv[i], "r");

            fseek(filePointer, 0L, SEEK_END);
            size_t size = ftell(filePointer);
            rewind(filePointer);

            char *buffer = malloc(size);

            mem = malloc(mems);
            vars = malloc(sizeof(var *));
            funcs = malloc(sizeof(func *));

            while(fgets(buffer, size, filePointer)) {
                if (exec(buffer) > 0) {
                    break;
                }
            }

            free(mem);

            for (int i = 0; i < varc; i ++) {
                free(vars[i]);
            }
            free(vars);

            for (int i = 0; i < funcc; i ++) {
                free(funcs[i]);
            }
            free(funcs);

            free(buffer);
            fclose(filePointer);
            return 0;
        }
        else if (!strcmp(par, "mem")) {
            mems = atoi(argv[i]);
        }
    }
    if (mems < 1) {
        printf("Memory size needs to be at least 1!\n");
        return 1;
    }

    mem = malloc(mems);
    vars = malloc(sizeof(var *));
    funcs = malloc(sizeof(func *));

    for (;;) {
        char inp[100];
        printf("> ");
        if (!fgets(inp, sizeof(inp), stdin)) {
            break;
        }
        if (exec(inp) > 0) {
            break;
        }
    }

    free(mem);

    for (int i = 0; i < varc; i ++) {
    	free(vars[i]);
    }
    free(vars);

    for (int i = 0; i < funcc; i ++) {
        free(funcs[i]);
    }
    free(funcs);

    return 0;
}
