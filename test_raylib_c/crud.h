#ifndef CRUD_H
#define CRUD_H

int save_state(char *, char *, int (*)[]);
char **load_file_names(char *);
char **delete_stuff(char *, char *);
int load_state(char *, char *, int (*)[]);

#endif // CRUD_H
