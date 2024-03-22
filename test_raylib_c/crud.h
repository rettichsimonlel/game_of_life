#ifndef CRUD_H
#define CRUD_H

int save_state(char *, char *, int (*)[]);
char **load_file_names(char *);
int load_state(char *, char *, int (*)[]);

#endif // CRUD_H
