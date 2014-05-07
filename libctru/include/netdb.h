#ifndef NETDB_H
#define NETDB_H

struct hostent {
   char * h_name;
   char ** h_aliases;
   int h_addrtype;
   int h_length;
   char ** h_addr_list;
};


#ifdef __cplusplus
extern "C" {
#endif

	struct hostent * gethostbyname(const char * name);

#ifdef __cplusplus
};
#endif


#endif // NETDB_H
