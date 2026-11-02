#ifndef MTD_UUID_H
#define MTD_UUID_H
void uuid_generate_random(unsigned char out[16]);
void uuid_unparse_upper(const unsigned char uu[16], char *out);
#endif
