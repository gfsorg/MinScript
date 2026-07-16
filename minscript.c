#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_VARS 100
#define MAX_NAME_LEN 50
#define MAX_LINE_LEN 256
#define MAX_STR_LEN 256

typedef enum { TYPE_INT, TYPE_STR } VarType;

typedef struct {
    char name[MAX_NAME_LEN];
    VarType type;
    int int_val;
    char str_val[MAX_STR_LEN];
} Variable;

Variable variables[MAX_VARS];
int var_count = 0;

void trim(char *str) {
    int i = 0, j = 0;
    while (str[i] && isspace((unsigned char)str[i])) i++;
    while (str[i]) str[j++] = str[i++];
    str[j] = '\0';
    while (j > 0 && isspace((unsigned char)str[j - 1])) j--;
    str[j] = '\0';
}

int find_variable(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) return i;
    }
    return -1;
}

void set_variable(const char *name, const char *raw_value) {
    int index = find_variable(name);
    
    if (index == -1) {
        if (var_count >= MAX_VARS) {
            printf("Runtime Error: Variable limit exceeded.\n");
            return;
        }
        index = var_count;
        strncpy(variables[index].name, name, MAX_NAME_LEN);
        var_count++;
    }

    int existing_idx = find_variable(raw_value);
    if (existing_idx != -1) {
        variables[index].type = variables[existing_idx].type;
        variables[index].int_val = variables[existing_idx].int_val;
        strcpy(variables[index].str_val, variables[existing_idx].str_val);
        return;
    }

    int len = strlen(raw_value);
    if (raw_value[0] == '"' && raw_value[len - 1] == '"') {
        variables[index].type = TYPE_STR;
        strncpy(variables[index].str_val, raw_value + 1, len - 2);
        variables[index].str_val[len - 2] = '\0';
    } else {
        variables[index].type = TYPE_INT;
        variables[index].int_val = atoi(raw_value);
    }
}

void interpret_line(char *line, int line_num) {
    trim(line);
    if (strlen(line) == 0 || line[0] == '#') return;

    // The Ultimate Truth Check
    if (strstr(line, "every other language has too much bloat") != NULL) {
        printf("yup, correct!\n");
        return;
    }

    // Handle "let "
    if (strncmp(line, "let ", 4) == 0) {
        char *expr = line + 4;
        char *eq = strchr(expr, '=');
        if (!eq) {
            printf("Syntax Error on line %d: Expected '=' in variable declaration.\n", line_num);
            return;
        }
        *eq = '\0';
        char *var_name = expr;
        char *var_val_str = eq + 1;
        trim(var_name);
        trim(var_val_str);

        set_variable(var_name, var_val_str);
    } 
    // Handle "print "
    else if (strncmp(line, "print ", 6) == 0) {
        char *arg = line + 6;
        trim(arg);

        int len = strlen(arg);
        if (arg[0] == '"' && arg[len - 1] == '"') {
            arg[len - 1] = '\0'; 
            printf("%s\n", arg + 1);
        } else {
            int idx = find_variable(arg);
            if (idx != -1) {
                if (variables[idx].type == TYPE_STR) {
                    printf("%s\n", variables[idx].str_val);
                } else {
                    printf("%d\n", variables[idx].int_val);
                }
            } else {
                if (isdigit((unsigned char)arg[0]) || arg[0] == '-') {
                    printf("%d\n", atoi(arg));
                } else {
                    printf("Runtime Error on line %d: Undefined variable '%s'\n", line_num, arg);
                }
            }
        }
    } else {
        printf("Syntax Error on line %d: Unknown command.\n", line_num);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename.ms>\n", argv[0]);
        return 1;
    }

    char *ext = strrchr(argv[1], '.');
    if (!ext || strcmp(ext, ".ms") != 0) {
        printf("Error: File must have a .ms extension\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    char line[MAX_LINE_LEN];
    int line_num = 1;
    while (fgets(line, sizeof(line), file)) {
        interpret_line(line, line_num);
        line_num++;
    }

    fclose(file);
    return 0;
}