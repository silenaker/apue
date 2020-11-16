#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fort.h"
#include "ll.h"

#define int2binstr(n, bs) to_binary_str((char*)&n, sizeof(int), bs)
#define arr_len(arr) (sizeof(arr) / sizeof((arr)[0]))

#define ADDR_TYPE_STRLEN 8
#define ADDR_LEN_STRLEN 8

struct hostent_p {
  char* name;
  char* aliases;
  char* addr_list;
  char addr_type[ADDR_TYPE_STRLEN];
  char addr_len[ADDR_LEN_STRLEN];
};

struct netent_p {
  char* name;
  char* aliases;
  char addr_type[ADDR_TYPE_STRLEN];
  char address[INET_ADDRSTRLEN];
};

struct protoent_p {
  char* name;
  char* aliases;
  char protocol[8];
};

struct servent_p {
  char* name;
  char* aliases;
  char* protocol;
  char port[8];
};

struct addrinfo_p {
  char* flags;
  char* family;
  char* sock_type;
  char* protocol;
  char* hostname;
  char address[INET6_ADDRSTRLEN];
  char port[8];
};

char* to_binary_str(char* byte, int len, char* str) {
  unsigned char mask = 128;
  char* p = str;

  for (int i = 0; i < len; i++) {
    for (int j = 0; j < 8; j++) {
      *p = (mask >> j) & (*(byte + i)) ? 49 : 48;  // 1 and 0 character
      p++;
    }
    if (i < len - 1) {
      *p = 32;  // space character
      p++;
    }
  }
  *p = 0;  // NULL (string terminator)
  return str;
}

void init_hostent_p(struct hostent_p* entp, struct hostent* ent) {
  char* aliases;
  char* addr_list;
  char** p;
  char addr[INET_ADDRSTRLEN];
  int addr_list_num;

  // 获取别名
  aliases = NULL;
  for (p = ent->h_aliases; *p; p++) {
    if (aliases) {
      aliases = realloc(aliases, strlen(aliases) + strlen(*p) + 2);
      strcat(aliases, ",");
      strcat(aliases, *p);
    } else {
      aliases = malloc(strlen(*p) + 1);
      strcpy(aliases, *p);
    }
  }
  if (!aliases) {
    aliases = malloc(1);
    aliases[0] = '\0';
  }

  // 获取地址
  addr_list_num = 0;
  for (p = ent->h_addr_list; *p; p++) {
    addr_list_num++;
  }
  if (addr_list_num > 0) {
    addr_list = malloc(addr_list_num * (INET_ADDRSTRLEN));
    addr_list[0] = '\0';
  } else {
    addr_list = malloc(1);
    addr_list[0] = '\0';
  }
  for (p = ent->h_addr_list; *p; p++) {
    inet_ntop(AF_INET, *p, addr, INET_ADDRSTRLEN);
    if (addr_list[0]) {
      strcat(addr_list, ",");
    }
    strcat(addr_list, addr);
  }

  entp->aliases = aliases;
  entp->addr_list = addr_list;
  entp->name = malloc(strlen(ent->h_name) + 1);
  strcpy(entp->name, ent->h_name);
  snprintf(entp->addr_type, ADDR_TYPE_STRLEN, "%d", ent->h_addrtype);
  snprintf(entp->addr_len, ADDR_LEN_STRLEN, "%d", ent->h_length);
}

void destroy_hostent_p(struct hostent_p* entp) {
  free(entp->name);
  free(entp->aliases);
  free(entp->addr_list);
  entp->name = NULL;
  entp->aliases = NULL;
  entp->addr_list = NULL;
}

void init_netent_p(struct netent_p* entp, struct netent* ent) {
  char* aliases;
  char** alias;

  aliases = NULL;
  for (alias = ent->n_aliases; *alias; alias++) {
    if (aliases) {
      aliases = realloc(aliases, strlen(aliases) + strlen(*alias) + 2);
      strcat(aliases, ",");
      strcat(aliases, *alias);
    } else {
      aliases = malloc(strlen(*alias) + 1);
      strcpy(aliases, *alias);
    }
  }
  if (!aliases) {
    aliases = malloc(1);
    aliases[0] = 0;
  }

  entp->aliases = aliases;
  entp->name = malloc(strlen(ent->n_name) + 1);
  strcpy(entp->name, ent->n_name);
  snprintf(entp->addr_type, ADDR_TYPE_STRLEN, "%d", ent->n_addrtype);
  snprintf(entp->address, INET_ADDRSTRLEN, "%d", ent->n_net);
}

void destroy_netent_p(struct netent_p* entp) {
  free(entp->name);
  free(entp->aliases);
  entp->name = NULL;
  entp->aliases = NULL;
}

void init_protoent_p(struct protoent_p* entp, struct protoent* ent) {
  char* aliases;
  char** alias;

  aliases = NULL;
  for (alias = ent->p_aliases; *alias; alias++) {
    if (aliases) {
      aliases = realloc(aliases, strlen(aliases) + strlen(*alias) + 2);
      strcat(aliases, ",");
      strcat(aliases, *alias);
    } else {
      aliases = malloc(strlen(*alias) + 1);
      strcpy(aliases, *alias);
    }
  }
  if (!aliases) {
    aliases = malloc(1);
    aliases[0] = 0;
  }

  entp->aliases = aliases;
  entp->name = malloc(strlen(ent->p_name) + 1);
  strcpy(entp->name, ent->p_name);
  snprintf(entp->protocol, 8, "%d", ent->p_proto);
}

void destroy_protoent_p(struct protoent_p* entp) {
  free(entp->name);
  free(entp->aliases);
  entp->name = NULL;
  entp->aliases = NULL;
}

void init_servent_p(struct servent_p* entp, struct servent* ent) {
  char* aliases;
  char** alias;

  aliases = NULL;
  for (alias = ent->s_aliases; *alias; alias++) {
    if (aliases) {
      aliases = realloc(aliases, strlen(aliases) + strlen(*alias) + 2);
      strcat(aliases, ",");
      strcat(aliases, *alias);
    } else {
      aliases = malloc(strlen(*alias) + 1);
      strcpy(aliases, *alias);
    }
  }
  if (!aliases) {
    aliases = malloc(1);
    aliases[0] = 0;
  }

  entp->aliases = aliases;
  entp->name = malloc(strlen(ent->s_name) + 1);
  entp->protocol = malloc(strlen(ent->s_proto) + 1);
  strcpy(entp->name, ent->s_name);
  strcpy(entp->protocol, ent->s_proto);
  snprintf(entp->port, 8, "%d", ntohs(ent->s_port));
}

void destroy_servent_p(struct servent_p* entp) {
  free(entp->name);
  free(entp->aliases);
  free(entp->protocol);
  entp->name = NULL;
  entp->aliases = NULL;
  entp->protocol = NULL;
}

void init_addrinfo_p(struct addrinfo_p* aip, struct addrinfo* ai) {
  struct sockaddr_in* sinp;
  struct sockaddr_in6* sinp6;

  // handle flags
  if (ai->ai_flags == 0) {
    aip->flags = "0";
  } else {
    if (ai->ai_flags & AI_PASSIVE) {
      aip->flags = "passive";
    } else if (ai->ai_flags & AI_CANONNAME) {
      aip->flags = "canon";
    } else if (ai->ai_flags & AI_NUMERICHOST) {
      aip->flags = "numhost";
    } else if (ai->ai_flags & AI_NUMERICSERV) {
      aip->flags = "numserv";
    } else if (ai->ai_flags & AI_V4MAPPED) {
      aip->flags = "v4mapped";
    } else if (ai->ai_flags & AI_ALL) {
      aip->flags = "all";
    }
  }

  // handle protocol
  switch (ai->ai_protocol) {
    case 0:
      aip->protocol = "default";
      break;
    case IPPROTO_TCP:
      aip->protocol = "TCP";
      break;
    case IPPROTO_UDP:
      aip->protocol = "UDP";
      break;
    case IPPROTO_RAW:
      aip->protocol = "raw";
      break;
    default:
      aip->protocol = "unknown";
  }

  // handle family
  switch (ai->ai_family) {
    case AF_INET:
      aip->family = "inet";
      break;
    case AF_INET6:
      aip->family = "inet6";
      break;
    case AF_UNIX:
      aip->family = "unix";
      break;
    case AF_UNSPEC:
      aip->family = "unspecified";
      break;
    default:
      aip->family = "unknown";
  }

  // handle socket type
  switch (ai->ai_socktype) {
    case SOCK_STREAM:
      aip->sock_type = "stream";
      break;
    case SOCK_DGRAM:
      aip->sock_type = "datagram";
      break;
    case SOCK_SEQPACKET:
      aip->sock_type = "seqpacket";
      break;
    case SOCK_RAW:
      aip->sock_type = "raw";
      break;
    default:
      aip->sock_type = "unknown";
  }

  aip->hostname = ai->ai_canonname ? ai->ai_canonname : "-";

  switch (ai->ai_family) {
    case AF_INET:
      sinp = (struct sockaddr_in*)ai->ai_addr;
      // alternative function: inet_ntoa (only for ipv4)
      // strcpy(aip->address, inet_ntoa(sinp->sin_addr));
      inet_ntop(AF_INET, &sinp->sin_addr, aip->address, INET6_ADDRSTRLEN);
      snprintf(aip->port, 8, "%d", ntohs(sinp->sin_port));
      break;

    case AF_INET6:
      sinp6 = (struct sockaddr_in6*)ai->ai_addr;
      inet_ntop(AF_INET6, &sinp6->sin6_addr, aip->address, INET6_ADDRSTRLEN);
      snprintf(aip->port, 8, "%d", ntohs(sinp6->sin6_port));
      break;

    default:
      strcpy(aip->address, "");
      strcpy(aip->port, "");
      break;
  }
}

void get_all_hosts() {
  struct hostent* ent;
  struct ll_head* list;
  struct ll_node* node;
  ft_table_t* table;
  struct hostent_p* entp;

  printf("get all hosts:\n");

  // open or rewind host database file, optional
  sethostent(1);
  // get host entry
  list = ll_construct();
  while ((ent = gethostent())) {
    entp = malloc(sizeof(struct hostent_p));
    init_hostent_p(entp, ent);
    ll_append(list, entp, sizeof(struct hostent_p));
  }
  // close host database file
  endhostent();

  // render table
  table = ft_create_table();
  ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
  ft_write_ln(table, "Host Name", "Aliases", "Address Type", "Address Length",
              "Address List");
  for (node = list->next; node; node = node->next) {
    entp = node->buf;
    ft_write_ln(table, entp->name, entp->aliases, entp->addr_type,
                entp->addr_len, entp->addr_list);
  }
  printf("%s\n", ft_to_string(table));

  // free memory
  for (node = list->next; node; node = node->next) {
    destroy_hostent_p(node->buf);
    free(node->buf);
  }
  ll_destroy(list);
  ft_destroy_table(table);
}

void get_host_by_name() {
  char input[LINE_MAX];
  struct hostent* ent = NULL;
  struct hostent_p entp;
  ft_table_t* table;

  printf("get host by name:\n");

  // get host name from terminal input
  for (int i = 0; !ent && i < 3; i++) {
    printf("please input host name: ");
    fflush(stdout);
    gets(input);
    ent = gethostbyname(input);
    if (!ent) {
      printf("can not find host by name: %s\n", input);
    }
  }

  if (ent) {
    init_hostent_p(&entp, ent);
    // render table
    table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table, "Host Name", "Aliases", "Address Type", "Address Length",
                "Address List");
    ft_write_ln(table, entp.name, entp.aliases, entp.addr_type, entp.addr_len,
                entp.addr_list);
    printf("%s\n", ft_to_string(table));
    ft_destroy_table(table);
  } else {
    printf("try terminate!\n\n");
  }
}

void get_host_by_addr() {
  char input[LINE_MAX];
  struct hostent* ent = NULL;
  struct hostent_p entp;
  struct in_addr addr;
  ft_table_t* table;

  printf("get host by address:\n");

  // get host address from terminal input
  for (int i = 0; !ent && i < 3; i++) {
    printf("please input host address: ");
    fflush(stdout);
    gets(input);
    addr.s_addr = inet_addr(input);
    ent = gethostbyaddr(&addr, sizeof(struct in_addr), AF_INET);
    if (!ent) {
      printf("can not find host by address: %s\n", input);
    }
  }

  if (ent) {
    init_hostent_p(&entp, ent);
    // render table
    table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table, "Host Name", "Aliases", "Address Type", "Address Length",
                "Address List");
    ft_write_ln(table, entp.name, entp.aliases, entp.addr_type, entp.addr_len,
                entp.addr_list);
    printf("%s\n", ft_to_string(table));
    ft_destroy_table(table);
  } else {
    printf("try terminate!\n\n");
  }
}

void get_all_networks() {
  struct netent* ent;
  struct ll_head* list;
  struct ll_node* node;
  struct netent_p* entp;
  struct ft_table* table;

  printf("get all networks: \n");

  // open or rewind network database file, optional
  setnetent(1);
  list = ll_construct();
  // get network entry
  while ((ent = getnetent())) {
    entp = malloc(sizeof(struct netent_p));
    init_netent_p(entp, ent);
    ll_append(list, entp, sizeof(struct netent_p));
  }
  // close network database file
  endnetent();

  // render table
  table = ft_create_table();
  ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
  ft_write_ln(table, "Network Name", "Aliases", "Address Type", "Address");
  for (node = list->next; node; node = node->next) {
    entp = node->buf;
    ft_write_ln(table, entp->name, entp->aliases, entp->addr_type,
                entp->address);
  }
  printf("%s\n", ft_to_string(table));

  // free memory
  for (node = list->next; node; node = node->next) {
    destroy_netent_p(node->buf);
    free(node->buf);
  }
  ll_destroy(list);
  ft_destroy_table(table);
}

void get_network_by_addr() {
  char input[LINE_MAX];
  struct netent* ent = NULL;
  struct netent_p entp;
  struct in_addr addr;
  ft_table_t* table;

  printf("get network by address:\n");

  // get network address from terminal input
  for (int i = 0; !ent && i < 3; i++) {
    printf("please input network address: ");
    fflush(stdout);
    gets(input);
    inet_pton(AF_INET, input, &addr);
    ent = getnetbyaddr(addr.s_addr, AF_INET);
    if (!ent) {
      printf("can not find network by address: %s\n", input);
    }
  }

  if (ent) {
    init_netent_p(&entp, ent);
    // render table
    table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table, "Network Name", "Aliases", "Address Type", "Address");
    ft_write_ln(table, entp.name, entp.aliases, entp.addr_type, entp.address);
    printf("%s\n", ft_to_string(table));
    ft_destroy_table(table);
  } else {
    printf("try terminate!\n\n");
  }
}

void get_network_by_name() {
  char input[LINE_MAX];
  struct netent* ent = NULL;
  struct netent_p entp;
  ft_table_t* table;

  printf("get network by name:\n");

  // get network name from terminal input
  for (int i = 0; !ent && i < 3; i++) {
    printf("please input network name: ");
    fflush(stdout);
    gets(input);
    ent = getnetbyname(input);
    if (!ent) {
      printf("can not find network by name: %s\n", input);
    }
  }

  if (ent) {
    init_netent_p(&entp, ent);
    // render table
    table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table, "Network Name", "Aliases", "Address Type", "Address");
    ft_write_ln(table, entp.name, entp.aliases, entp.addr_type, entp.address);
    printf("%s\n", ft_to_string(table));
    ft_destroy_table(table);
  } else {
    printf("try terminate!\n\n");
  }
}

void get_all_protocols() {
  struct protoent* ent;
  struct ll_head* list;
  struct ll_node* node;
  struct protoent_p* entp;
  struct ft_table* table;

  printf("get all protocols: \n");

  // open or rewind protocol database file, optional
  setprotoent(1);
  list = ll_construct();
  // get protocol entry
  while ((ent = getprotoent())) {
    entp = malloc(sizeof(struct protoent_p));
    init_protoent_p(entp, ent);
    ll_append(list, entp, sizeof(struct protoent_p));
  }
  // close protocol database file
  endprotoent();

  // render table
  table = ft_create_table();
  ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
  ft_write_ln(table, "Protocol Name", "Aliases", "Protocol Number");
  for (node = list->next; node; node = node->next) {
    entp = node->buf;
    ft_write_ln(table, entp->name, entp->aliases, entp->protocol);
  }
  printf("%s\n", ft_to_string(table));

  // free memory
  for (node = list->next; node; node = node->next) {
    destroy_protoent_p(node->buf);
    free(node->buf);
  }
  ll_destroy(list);
  ft_destroy_table(table);
}

void get_protocol_by_number() {
  char input[LINE_MAX];
  struct protoent* ent = NULL;
  struct protoent_p entp;
  ft_table_t* table;

  printf("get protocol by number:\n");

  // get protocol number from terminal input
  for (int i = 0; !ent && i < 3; i++) {
    printf("please input protocol number: ");
    fflush(stdout);
    gets(input);
    ent = getprotobynumber(atoi(input));
    if (!ent) {
      printf("can not find protocol entry by number: %s\n", input);
    }
  }

  if (ent) {
    init_protoent_p(&entp, ent);
    // render table
    table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table, "Protocol Name", "Aliases", "Protocol Number");
    ft_write_ln(table, entp.name, entp.aliases, entp.protocol);
    printf("%s\n", ft_to_string(table));
    ft_destroy_table(table);
  } else {
    printf("try terminate!\n\n");
  }
}

void get_protocol_by_name() {
  char input[LINE_MAX];
  struct protoent* ent = NULL;
  struct protoent_p entp;
  ft_table_t* table;

  printf("get protocol by name:\n");

  for (int i = 0; !ent && i < 3; i++) {
    printf("please input protocol name: ");
    fflush(stdout);
    gets(input);
    ent = getprotobyname(input);
    if (!ent) {
      printf("can not find protocol entry by name: %s\n", input);
    }
  }

  if (ent) {
    init_protoent_p(&entp, ent);
    // render table
    table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table, "Protocol Name", "Aliases", "Protocol Number");
    ft_write_ln(table, entp.name, entp.aliases, entp.protocol);
    printf("%s\n", ft_to_string(table));
    ft_destroy_table(table);
  } else {
    printf("try terminate!\n\n");
  }
}

void get_all_services() {
  struct servent* ent;
  struct ll_head* list;
  struct ll_node* node;
  struct servent_p* entp;
  struct ft_table* table;

  printf("get all services: \n");

  // open or rewind services database file, optional
  setservent(1);
  list = ll_construct();
  // get service entry
  while ((ent = getservent())) {
    entp = malloc(sizeof(struct servent_p));
    init_servent_p(entp, ent);
    ll_append(list, entp, sizeof(struct servent_p));
  }
  // close service database file
  endservent();

  // render table
  table = ft_create_table();
  ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
  ft_write_ln(table, "Service Name", "Aliases", "Port", "Protocol");
  node = list->next;
  for (int i = 0; node && i < 200; i++) {
    entp = node->buf;
    ft_write_ln(table, entp->name, entp->aliases, entp->port, entp->protocol);
    node = node->next;
  }
  printf("%s", ft_to_string(table));
  if (node) {
    printf("table has too many entries, only print first 200 rows\n\n");
  }

  // free memory
  for (node = list->next; node; node = node->next) {
    destroy_servent_p(node->buf);
    free(node->buf);
  }
  ll_destroy(list);
  ft_destroy_table(table);
}

void get_service_by_port() {
  char port[LINE_MAX];
  char proto[LINE_MAX];
  struct servent* ent = NULL;
  struct servent_p entp;
  struct ft_table* table;

  printf("get service by port:\n");

  // get port number from terminal input
  for (int i = 0; !ent && i < 3; i++) {
    printf("please input port number: ");
    fflush(stdout);
    gets(port);
    printf("please input protocol: ");
    fflush(stdout);
    gets(proto);
    // 注意：port 为 network byte order
    ent = getservbyport(htons(atoi(port)), proto);
    if (!ent) {
      printf("can not find service entry by port number: %s and protocol: %s\n",
             port, proto);
    }
  }

  if (ent) {
    init_servent_p(&entp, ent);
    // render table
    table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table, "Service Name", "Aliases", "Port", "Protocol");
    ft_write_ln(table, entp.name, entp.aliases, entp.port, entp.protocol);
    printf("%s\n", ft_to_string(table));
    ft_destroy_table(table);
  } else {
    printf("try terminate!\n\n");
  }
}

void get_service_by_name() {
  char name[LINE_MAX];
  char proto[LINE_MAX];
  struct servent* ent = NULL;
  struct servent_p entp;
  struct ft_table* table;

  printf("get service by name:\n");

  // get service name from terminal input
  for (int i = 0; !ent && i < 3; i++) {
    printf("please input service name: ");
    fflush(stdout);
    gets(name);
    printf("please input protocol: ");
    fflush(stdout);
    gets(proto);
    ent = getservbyname(name, proto);
    if (!ent) {
      printf("can not find service entry by name: %s and protocol: %s\n", name,
             proto);
    }
  }

  if (ent) {
    init_servent_p(&entp, ent);
    // render table
    table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table, "Service Name", "Aliases", "Port", "Protocol");
    ft_write_ln(table, entp.name, entp.aliases, entp.port, entp.protocol);
    printf("%s\n", ft_to_string(table));
    ft_destroy_table(table);
  } else {
    printf("try terminate!\n\n");
  }
}

void get_all_addresses() {
  struct addrinfo *ailist, *ai;
  struct addrinfo hint;
  int error;
  char hostname[LINE_MAX];
  char servicename[LINE_MAX];
  struct ll_head* list;
  struct ll_node* node;
  struct addrinfo_p* aip;
  struct ft_table* table;

  printf("get address info:\n");
  printf("please input host name: ");
  fflush(stdout);
  gets(hostname);
  printf("please input service name: ");
  fflush(stdout);
  gets(servicename);

  // get all addresses by host name, service name and hint
  memset(&hint, 0, sizeof(hint));
  hint.ai_flags = AI_CANONNAME;
  if ((error = getaddrinfo(hostname, servicename, &hint, &ailist)) != 0) {
    errx(1, "getaddrinfo error: %s", gai_strerror(error));
  }
  list = ll_construct();
  for (ai = ailist; ai; ai = ai->ai_next) {
    aip = malloc(sizeof(struct addrinfo_p));
    init_addrinfo_p(aip, ai);
    ll_append(list, aip, sizeof(struct addrinfo_p));
  }

  // render table
  table = ft_create_table();
  ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
  ft_write_ln(table, "Host Name", "Host Address", "Service Port",
              "Address Family", "Socket Type", "Protocol", "Flags");
  for (node = list->next; node; node = node->next) {
    aip = node->buf;
    ft_write_ln(table, aip->hostname, aip->address, aip->port, aip->family,
                aip->sock_type, aip->protocol, aip->flags);
  }
  printf("%s\n", ft_to_string(table));

  // free memory
  freeaddrinfo(ailist);
  for (node = list->next; node; node = node->next) {
    free(node->buf);
  }
  ll_destroy(list);
  ft_destroy_table(table);
}

/*
 * man getaddrinfo example 1
 * The following code tries to connect to ``www.kame.net'' service ``http'' via
 * a stream socket.  It loops through all the addresses available, regardless of
 * address family.  If the destination resolves to an IPv4 address, it will use
 * an PF_INET socket.  Similarly, if it resolves to IPv6, an PF_INET6 socket is
 * used. Observe that there is no hardcoded reference to a particular address
 * family. The code works even if getaddrinfo() returns addresses that are not
 * IPv4/v6.
 */
void connect_to() {
  struct addrinfo *ailist, *ai;
  struct addrinfo hint;
  int error;
  int fd;
  char hostname[LINE_MAX];
  char servicename[LINE_MAX];
  const char* cause = NULL;
  struct addrinfo_p aip;
  struct ft_table* table;

  printf("connect to:\n");
  printf("please input host name: ");
  fflush(stdout);
  gets(hostname);
  printf("please input service name: ");
  fflush(stdout);
  gets(servicename);

  // get all addresses by host name, service name and hint
  memset(&hint, 0, sizeof(hint));
  hint.ai_family = PF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;
  if ((error = getaddrinfo(hostname, servicename, &hint, &ailist)) != 0) {
    errx(1, "%s", gai_strerror(error));
  }

  // create socket and try to connect
  fd = -1;
  for (ai = ailist; ai; ai = ai->ai_next) {
    fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (fd < 0) {
      cause = "socket";
      continue;
    }

    if (connect(fd, ai->ai_addr, ai->ai_addrlen) < 0) {
      cause = "connect";
      close(fd);
      fd = -1;
      continue;
    }

    printf("successfully connected:\n");
    init_addrinfo_p(&aip, ai);
    table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table, "Host Name", "Host Address", "Service Port",
                "Address Family", "Socket Type", "Protocol", "Flags");
    ft_write_ln(table, aip.hostname, aip.address, aip.port, aip.family,
                aip.sock_type, aip.protocol, aip.flags);
    printf("%s\n", ft_to_string(table));
    break;
  }

  if (fd < 0) {
    err(1, "%s", cause);
  }
  freeaddrinfo(ailist);
}

/*
 * man getaddrinfo example 2
 * The following example tries to open a wildcard listening socket onto service
 * ``http'', for all the address families available.
 */
#define MAXSOCK 128

void listen_to() {
  struct addrinfo *ailist, *ai;
  struct addrinfo hint;
  int error;
  int fd;
  char hostname[LINE_MAX];
  char servicename[LINE_MAX];
  const char* cause = NULL;
  int n;
  int fds[MAXSOCK];
  struct addrinfo_p aip;
  struct ft_table* table;

  printf("listen to:\n");
  printf("please input service name: ");
  fflush(stdout);
  gets(servicename);

  memset(&hint, 0, sizeof(hint));
  hint.ai_family = PF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;
  hint.ai_flags = AI_PASSIVE;
  if ((error = getaddrinfo(NULL, servicename, &hint, &ailist)) != 0) {
    errx(1, "%s", gai_strerror(error));
  }

  n = 0;
  for (ai = ailist; ai && n < MAXSOCK; ai = ai->ai_next) {
    fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (fd < 0) {
      cause = "socket";
      continue;
    }
    if (bind(fd, ai->ai_addr, ai->ai_addrlen) < 0) {
      cause = "bind";
      close(fd);
      continue;
    }
    listen(fd, 5);
    printf("successfully listened:\n");
    init_addrinfo_p(&aip, ai);
    table = ft_create_table();
    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    ft_write_ln(table, "Host Name", "Host Address", "Service Port",
                "Address Family", "Socket Type", "Protocol", "Flags");
    ft_write_ln(table, aip.hostname, aip.address, aip.port, aip.family,
                aip.sock_type, aip.protocol, aip.flags);
    printf("%s\n", ft_to_string(table));
    fds[n] = fd;
    n++;
  }

  if (n == 0) {
    err(1, "%s", cause);
  }
  freeaddrinfo(ailist);
}

void print_byte_order() {
  int number;
  int str_len = sizeof(int) * 8 + 1;
  char binstr[str_len];

  // 初始化表格数据
  int nums[] = {1, 2, 3, 256, 257, 258, 65536, 65537, 65538};
  char* numstrs[] = {"1",   "2",     "3",     "256",  "257",
                     "258", "65536", "65537", "65538"};
  char numbinstrs[arr_len(nums)][36];
  for (int i = 0; i < 9; i++) {
    int2binstr(nums[i], (char*)&numbinstrs[i]);
  }

  printf("processor architecture byte order (big-endian or little-endian):\n");

  ft_table_t* table = ft_create_table();
  /* Set "header" type for the first row */
  ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
  ft_write_ln(table, "Number", "Binary String");
  ft_write_ln(table, numstrs[0], numbinstrs[0]);
  ft_write_ln(table, numstrs[1], numbinstrs[1]);
  ft_write_ln(table, numstrs[2], numbinstrs[2]);
  ft_write_ln(table, numstrs[3], numbinstrs[3]);
  ft_write_ln(table, numstrs[4], numbinstrs[4]);
  ft_write_ln(table, numstrs[5], numbinstrs[5]);
  ft_write_ln(table, numstrs[6], numbinstrs[6]);
  ft_write_ln(table, numstrs[7], numbinstrs[7]);
  ft_write_ln(table, numstrs[8], numbinstrs[8]);
  printf("%s\n", ft_to_string(table));
  ft_destroy_table(table);
}

int main(int argc, char* argv[]) {
  print_byte_order();
  get_all_hosts();
  get_host_by_addr();
  get_host_by_name();
  get_all_networks();
  get_network_by_addr();
  get_network_by_name();
  get_all_protocols();
  get_protocol_by_number();
  get_protocol_by_name();
  get_all_services();
  get_service_by_port();
  get_service_by_name();
  get_all_addresses();
  connect_to();
  listen_to();
}
