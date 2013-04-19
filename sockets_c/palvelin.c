#include <signal.h>     // For watchdog signal
#include <sys/socket.h> // Socket interface
#include <netinet/in.h> // inetaddr data structures
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h> // Booleans
#include <gmp.h> 
#include <nettle/hmac.h>
#include <nettle/sha.h>
#include "hajap.h" // Helpful functions are declared here.
#include "util.h" // HASH functions from SRPesimerkki
#include "srp.h" // SRP implementation from SRPesimerkki

/* Watchdog stops the program after this time */
#define MAX_RUNTIME 60*60  // hour in seconds

#define BACKLOG 5 // Max number of connections in queue

#define DIGEST_ASCII (SHA256_DIGEST_SIZE*2+1)

/* Username "database" and SRP stuff, generated with SRPesimerkki */
#include "prime.inc"
#include "sv.inc"

/* Protocol and version */
const char p_ident[10] = "HAJAP2012";
const char p_version = '1';


/* Originally from client code's udp_send */
/* Version byte added */
struct udp_recv 
{   
   uint16_t id; // id, 2 bytes
   char version; // version, 1 byte
   char direction; // direction, 1 byte
   uint32_t sid; //mpz_get_ui(vars->sid), 4 bytes
   uint32_t tid; // 4 bytes
   uint16_t prefix; // 2 bytes
   uint32_t nro; // 4 bytes
   char hmac_sha256[DIGEST_ASCII]; // N bytes
   uint16_t maclen; // 2 bytes
};

/* Originally from client code udp_recv */
/* Version byte removed */
struct udp_send {
   uint16_t id; // id, 2 bytes
   char direction; // direction, 1 byte
   uint32_t sid; // sid, 4 bytes
   uint32_t tid; // tid+1, 4 bytes
   uint16_t ans_len; // length of the answer, 2 bytes
   char* answer; // answer, N bytes
   char hmac_sha256[DIGEST_ASCII]; // N bytes
   uint16_t maclen; // 2 bytes
};


/* Watchdog to keep runaway processes at bay */
static void watchdog(int signro) {
   exit(signro);  // program stops when signal comes
}

/* Print usage information and exit with code 0. */
void print_usage() {
   printf("Tarvittava komentoriviparametri: --portti N  \n");
   exit(0);
}

/* Build a TCP frame for sending. */
void create_TCP_frame(unsigned char* frame, const int frame_len, const unsigned char* command) {
   /* Change frame length to network byte order */
   uint16_t frame_len_net = htons(frame_len);
   
   /* Construct the frame according to protocol */
   memcpy(&frame[0], &frame_len_net, 2);  // Frame length (whole frame)
   memcpy(&frame[2], p_ident, 9);         // Protocol name
   memcpy(&frame[11], &p_version, 1);     // Protocol version
   memcpy(&frame[12], command, frame_len-12 ); // Command (header always 12 bytes )
}


/* Check incoming frame for protocol policy. */
int check_frame(const char* msg, const unsigned char* recvline)  {
   size_t p = strlen(p_ident);
   char p_ident_local[p+1];
   p_ident_local[p]='\0'; // add 0 so strcmp works
   char p_version_local;
   char command[3] = "00\0"; // add 0, so the string can end and we can compare
   
   memcpy(p_ident_local, recvline, p);
   memcpy(&p_version_local, &recvline[p], 1);
   memcpy(&command, &recvline[p+1], 2);
   
   if( strcmp(p_ident_local, p_ident) != 0 ) {
      printf("frame is not in protocol: ident wrong \n");
      return 1; /* Failed */    
   }   
   
   if( p_version_local != p_version ) {
      printf("frame is not in protocol: version wrong \n");
      return 1; /* Failed */
   }
   
   if( strcmp(command, msg) != 0 ) {
      printf("frame is not in protocol: command wrong: %s \n", command);
      return 1; /* Failed */  
   }
  
   return 0; /* Success */
}


/* Calculate and return length of mpz_t type variable.
	 Also create ASCII-hex string of the variable.
	 Important: buf needs to be at least MAXLINE characters.
	 This seems a bit odd way to do it, but what the heck. */
int get_var_len(mpz_t val, unsigned char* buf) {
   int count = gmp_snprintf((char*)buf, MAXLINE, "%Zx", val);
   if(count < 0) err_quit("gmp_snprintf()");
   return count;
}

/* Get mpz number-values and save in srp_var struct variables. */
void get_mpz_val(mpz_t val, const size_t val_len, 
		 const unsigned char* recvline, const size_t pos) {
 
   unsigned char buf[val_len+1];
   buf[val_len] = '\0';
   
   memcpy(buf, &recvline[pos], val_len);

   if( gmp_sscanf((char*)buf, "%Zx", val ) != 1 ) err_quit("sscanf()");
}

/*
 * Receives command S0 and checks if OK
 * 
 * bytes 2    1    1-64
 * data "S0"  len  username
 * 
 * Username needs to exist in the database
*/ 
bool receiveTCPcommandS0( int sockfd ) {
   /* get first 2 bytes that indicate the length of the frame */
   uint16_t frame_len;
   if( readn(sockfd, &frame_len, 2) != 2) err_sys("readn error");

   /* convert frame length to host byte order */
   frame_len = ntohs(frame_len) - 2; // we already got 2 bytes
   unsigned char recvline[frame_len];

   /* get the rest of the frame */
   if( readn(sockfd, &recvline, frame_len) != frame_len) err_sys("readn error");

   printf("frame received, length %d \n", frame_len ); 
   
   /* check frame against protocol */ 
   if( check_frame("S0", recvline) != 0){
      printf("frame S0 is not in protocol \n");
      return false;
   }
   
   /* get username length */
   unsigned char uname_len = recvline[14-2];

   /* get username */
   char uname[uname_len+1];
   memcpy(&uname, &recvline[15-2], uname_len);
   uname[uname_len] = '\0'; // end string with null character for strcmp to work
   
   /* check if username in "user database" */
   if( strcmp( uname, SRP_user ) != 0 ) {
      printf("username %s not in database \n", uname);
      return false;
   }
   
   printf("Received username: %s \n", uname);

   /* Success */
   return true;
}

/*
 * Sends command S1 to client
 *  
 * bytes 2    2   n 2   n 2   n 2   n
 * data "S1"  len N len g len s len B
 */
bool sendTCPcommandS1( int sockfd ) {
   /* SRP: Read N and g specified by prime.inc */
   mpz_t Nval, gval;
   mpz_init( Nval );
   mpz_init( gval );
   mpz_import( Nval, DHp_size, 1, 1, 0, 0, DHp);
   mpz_import( gval, DHg_size, 1, 1, 0, 0, DHg);
   set_N( Nval );
   set_g( gval );
   
   /* SRP: get salt of the username from "user database".            */
   /* We just get the one salt we've generated for this work.        */
   /* We know from receiveTCPcommandS0 that the username is correct. */
   /* In a real program we would take a salt according to username.  */
   mpz_t salt, verifier;
   mpz_init( salt );
   mpz_init( verifier );
   mpz_import( salt, SRP_s_size, 1, 1, 0, 0, SRP_s_data);
   mpz_import( verifier, SRP_v_size, 1, 1, 0, 0, SRP_v_data);
   server_set_v( salt, verifier );

   /* SRP: Calculate connection parameter B */
   mpz_t B ; mpz_init( B );
   server_init( B );

   /* Make a string of N, s, g and B and calculate their length   */
   /* Length already known for N, g and s, so we could also ignore it... */
   /* ...but whatever. At least now the variable names are consistent */
   unsigned char N_buf[MAXLINE];       
   unsigned char s_buf[MAXLINE];
   unsigned char g_buf[MAXLINE];
   unsigned char B_buf[MAXLINE];
   int N_len = get_var_len(Nval, N_buf);
   int s_len = get_var_len(salt, s_buf);
   int g_len = get_var_len(gval, g_buf);
   int B_len = get_var_len(B, B_buf);
   uint16_t N_len_net = htons(N_len); // convert the lengths
   uint16_t s_len_net = htons(s_len); // to network byte order
   uint16_t g_len_net = htons(g_len); // for sending
   uint16_t B_len_net = htons(B_len);
   
   /* Length of the command and the whole frame. Header = 12 bytes. */
   uint16_t command_len = 2 + 2 + N_len + 2 + g_len + 2 + s_len + 2 + B_len;
   uint16_t frame_len = 12 + command_len;
   
   /* Initialize buffers for frame and command */
   unsigned char frame[frame_len];
   unsigned char command[command_len];
   
   /* Create the command */
   memcpy(command, "S1", 2);                   /* This is command S1 */
   memcpy(&command[2], &N_len_net, 2);                    /* N length */
   memcpy(&command[4], N_buf, N_len);                     /* N        */
   memcpy(&command[4+N_len], &g_len_net, 2);              /* g length */
   memcpy(&command[6+N_len], g_buf, g_len);               /* g        */
   memcpy(&command[6+N_len+g_len], &s_len_net, 2);        /* s length */
   memcpy(&command[8+N_len+g_len], s_buf, s_len);         /* s        */
   memcpy(&command[8+N_len+g_len+s_len], &B_len_net, 2);  /* B length */
   memcpy(&command[10+N_len+g_len+s_len], B_buf, B_len);  /* B        */

   /* Create frame for the command. */
   create_TCP_frame(frame, frame_len, command);
   
   /* Send the frame to client. */
   if( writen(sockfd, frame, frame_len) != (ssize_t)frame_len) err_sys("writen error");

   /* Success! */
   return true;
}



/*
 * Receives command S2 and checks if OK
 *
 * bytes 2    2    n  2    n
 * data "S2"  len  A  len  M1
 *
 * Return false if command not in protocol or if M1 is not correct
 * 
 * To fix in a real program:
 * Possible crash if client sends too long "length" information
 * Should do a sanity check for all received lengths...
 */
bool receiveTCPcommandS2( int sockfd, mpz_t* K ) {
   /* get first 2 bytes that indicate the length of the frame */
   uint16_t frame_len;
   if( readn(sockfd, &frame_len, 2) != 2) err_sys("readn error");
   
   /* convert frame length to host byte order */
   frame_len = ntohs(frame_len) - 2; /* we already got 2 bytes */
   unsigned char recvline[frame_len];
   
   /* get the rest of the frame */
   if( readn(sockfd, &recvline, frame_len) != frame_len) err_sys("readn error");
   
   /* check frame against protocol */
   if( check_frame("S2", recvline) != 0) {
      printf("frame S2 is not in protocol \n");
      return false;
   }
      
   uint16_t A_len_net, M1_len_net; /* lengths in network byte order */
   
   /* init, get and set variables A and M1 */
   
   /* A */
   mpz_t A;
   mpz_init( A );
   memcpy(&A_len_net, &recvline[12], 2);  // get length of A
   ssize_t A_len = ntohs(A_len_net);      // host byte order
   get_mpz_val(A, A_len, recvline, 12+2); // get the value to A
   server_setA(A);                        // set A
   
   /* M1 */
   mpz_t M1; 
   mpz_init( M1 );
   memcpy(&M1_len_net, &recvline[12+2+A_len], 2);
   ssize_t M1_len = ntohs(M1_len_net);
   get_mpz_val(M1, M1_len, recvline, 12+2+A_len+2);

   /* Calculate the key K */
   /* For this we need to know A and B */
   /* mpz_t K;    --- K initialized in main */
   mpz_init( *K );
   server_K( *K );

   /* Verify M1 against our own */
   /* Calculates also server's M1 */
   if( !server_verifyM1(M1) ){
      printf("M1 from client is wrong.\n");
      return false;
   }

   /* Success */
   return true;
}

/*
 * Sends command S3 to client
 *  
 * bytes 2    2   n  4  
 *      "S3"  len M2 SID
 */
bool sendTCPcommandS3( int sockfd, uint32_t SID ) {   
   /* SRP: Server's own verifier M2, to be sent to client */
   mpz_t M2; mpz_init( M2 );
   server_M2( M2 );
   
   /* Print stuff for debug */
   printf( "Session ID: %d\n", SID );
   
   /* Make a string of M2 and calculate it's length   */
   unsigned char M2_buf[MAXLINE];       
   int M2_len = get_var_len(M2, M2_buf);
   uint16_t M2_len_net = htons(M2_len); // network byte order

   uint32_t SID_net = htonl(SID); // network byte order - long
   
   /* Length of the command and the whole frame. Header = 12 bytes. */
   uint16_t command_len = 2 + 2 + M2_len + 4;
   uint16_t frame_len = 12 + command_len;
   
   /* Initialize buffers for frame and command */
   unsigned char frame[frame_len];
   unsigned char command[command_len];
   
   /* Create the command */
   memcpy(command, "S3", 2);    /* This is command S3 */
   memcpy(&command[2], &M2_len_net, 2);  /* M2 length */
   memcpy(&command[4], M2_buf, M2_len);  /* M2        */
   memcpy(&command[4+M2_len], &SID_net, 4);  /* SID       */

   /* Create frame for the command. */
   create_TCP_frame(frame, frame_len, command);
   
   /* Send the frame to client. */
   if( writen(sockfd, frame, frame_len) != (ssize_t)frame_len) err_sys("writen error");

   /* Success! */
   return true;
}


/* Authorizes connection via TCP commands */
bool authConnection( int sockfd, uint32_t SID, mpz_t* K ) {
   if( !receiveTCPcommandS0( sockfd ) ) {
      return false;
   }
   
   if( !sendTCPcommandS1( sockfd ) ) {
      return false;
   }
   
   if( !receiveTCPcommandS2( sockfd, K ) ) {
      return false;
   }
   
   if( !sendTCPcommandS3( sockfd, SID ) ) {   
      return false;
   }
   
   return true;
}


/* Authorization via TCP */
void auth( unsigned int port, uint32_t* SID, mpz_t* K ) {
   /* Initialize the TCP connection */
   printf("Opening TCP port %d for listening \n", port);
   
   /* Create a TCP socket */
   /* PF_INET --> IPv4, SOCK_STREAM --> TCP */ 
   int sockfd = socket(PF_INET, SOCK_STREAM, 0);
   if( sockfd < 0 ) err_sys("socket error");

   /* Create struct for server address and zero it. */
   struct sockaddr_in server_addr;
   memset((char *)&server_addr, 0, sizeof(server_addr));

   /* Set rest of the server address struct */
   server_addr.sin_family      = AF_INET;  /* IPv4 */
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
   server_addr.sin_port        = htons(port);
   
   /* Bind the socket to the listen port. */
   if( bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 ) {
      err_sys("bind error");
   }

   /* Prepare socket for incoming connections
    * Necessary only for stream oriented data modes (TCP) */
   if( listen(sockfd, BACKLOG ) < 0) err_sys("listen error");
   
   /* Client address is prepared */
   struct sockaddr_in client_addr;
   socklen_t client_len = sizeof(client_addr);

   /* Loop until authorization is ready */
   bool authReady = false;
   while(!authReady) {
      
      /* The initial descriptor remains a listening descriptor and accept() can be 
       * called at any time until it is closed.
       * accept() blocks until connection is received. */
      int newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
      if( newsockfd < 0 ) err_sys("accept error");

      /* Make up some session ID */
      /* To fix for real program:
       * do some actual session id, for example based on IP and current time? */
      *SID = 12345;      
      
      /* Authorize connection (HAJAP2012 protocol) */
      authReady = authConnection( newsockfd, *SID, K);

      /* TCP connection is closed even if the authorization was successful */
      close(newsockfd); 
   }
   
   /* TCP listening socket is closed, authorization is ready */ 
   close(sockfd);  
}


/* Same as in asiakas.c */
/* Terminate a string with zero. */
static inline void add_terminating_zero(char* string, int length) {
   string[length] = '\0';
}


/*
 * Create hmac-sha256 hash.
 * Take either udp_send or udp_recv struct, one on them is NULL
 * and operate on one of them which is not NULL.
 */
/* Originally from asiakas.c - recv and send swapped... */
void create_hmac(struct udp_send* send_params, struct udp_recv* recv_params, mpz_t K, char* ascii_hex) {
   	 
   // binary digest
   unsigned char digest[SHA256_DIGEST_SIZE];
   // passwd
   char t[MAXLINE];
   int count = gmp_sprintf(t, "%Zx", K);
   char passwd[count];
   memcpy(passwd, t, count);
   
   // assuming params are not null
		 
   // count the hmac
   struct hmac_sha256_ctx buf;
   
   // hmac passwd
   hmac_sha256_set_key(&buf, count, (uint8_t*)passwd);
   
   if(send_params != NULL) { 
      hmac_sha256_update(&buf, 2, (uint8_t*)&send_params->id);
      hmac_sha256_update(&buf, 1, (uint8_t*)&p_version);
      hmac_sha256_update(&buf, 1, (uint8_t*)&send_params->direction);
      hmac_sha256_update(&buf, 4, (uint8_t*)&send_params->sid);
      hmac_sha256_update(&buf, 4, (uint8_t*)&send_params->tid);
      hmac_sha256_update(&buf, 2, (uint8_t*)&send_params->ans_len);
      hmac_sha256_update(&buf, send_params->ans_len, (uint8_t*)send_params->answer);
   }
   else if(recv_params != NULL) {
      hmac_sha256_update(&buf, 2, (uint8_t*)&recv_params->id);
      hmac_sha256_update(&buf, 1, (uint8_t*)&recv_params->version);
      hmac_sha256_update(&buf, 1, (uint8_t*)&recv_params->direction);
      hmac_sha256_update(&buf, 4, (uint8_t*)&recv_params->sid);
      hmac_sha256_update(&buf, 4, (uint8_t*)&recv_params->tid);
      hmac_sha256_update(&buf, 2, (uint8_t*)&recv_params->prefix);
      hmac_sha256_update(&buf, 4, (uint8_t*)&recv_params->nro);
   }
   else err_quit("create_hmac: no structure passed");
   
   // get binary digest
   hmac_sha256_digest(&buf, SHA256_DIGEST_SIZE, digest);
   
   // turn binary digest into ascii-hex
   for(int i = 0; i < SHA256_DIGEST_SIZE; ++i)
     sprintf(ascii_hex + i * 2, "%02x", digest[i]);
   
   // put the ending zero
   add_terminating_zero(ascii_hex, DIGEST_ASCII-1);   
}

/* Sends UDP reply to client */
/* According to the phone number they sent */
void send_udp_reply(int sockfd, uint32_t SID, uint32_t TID, mpz_t K, 
		    char* answer, uint16_t answer_len,
		    struct sockaddr_in client_addr) {
   struct udp_send params;
   
   params.id = htonl(5010); // 2 bytes
   params.direction = 'A';  // 1 byte, going to client
   params.sid = htonl(SID); // 4 bytes session ID
   params.tid = htonl(TID+1); // TID is the transmission ID from client, 4 bytes
   params.ans_len = htons(answer_len); // 2 bytes
   params.answer = answer;

   create_hmac(&params, NULL, K, params.hmac_sha256); // N bytes
   
   uint16_t maclen_host = strlen(params.hmac_sha256);
   params.maclen = htons(maclen_host); // 2 bytes
   
   size_t len = 2+1+1+4+4+2+answer_len+2+maclen_host;
   unsigned char sendline[len];
   
   memcpy(sendline, &params.id, 2);
   memcpy(&sendline[2], &p_version, 1);
   memcpy(&sendline[2+1], &params.direction, 1);
   memcpy(&sendline[2+1+1], &params.sid, 4);
   memcpy(&sendline[2+1+1+4], &params.tid, 4);
   memcpy(&sendline[2+1+1+4+4], &params.ans_len, 2);
   memcpy(&sendline[2+1+1+4+4+2], params.answer, answer_len);
   memcpy(&sendline[2+1+1+4+4+2+answer_len], &params.maclen, 2);
   memcpy(&sendline[2+1+1+4+4+2+answer_len+2], params.hmac_sha256, maclen_host);
   
   // just in case ;)
   if(sizeof sendline > 2000) {
      err_quit("Too big answer to send");
   }
   
   // Send to client via UDP
   socklen_t cli_addr_len = sizeof(client_addr);   
   if( sendto( sockfd, sendline, len, 0, (struct sockaddr*)&client_addr, cli_addr_len) < 0) {
      close(sockfd);
      err_sys("UDP sendto error");
   }
}

void getAnswerToQuery( uint16_t prefix, uint32_t nro, char* answer) {
   if( prefix == 1234 && nro == 123456 ) {
      memcpy(&answer[0], "Jaska Jokunen ", 14);
   }
   else {
      memcpy(&answer[0], "Soita 020202  ", 14);
   }  
}
 
/* Receives and parses UDP query from client */
bool parse_udp_query(int sockfd, uint32_t SID, mpz_t K) {
   struct udp_recv params;
   uint16_t id_net, maclen_net;
   uint32_t sid_net, tid_net;
   char recvline[MAXLINE];

   /* UDP client address */
   struct sockaddr_in client_addr;
   socklen_t cli_addr_len = sizeof(client_addr);

   /* Receive UDP packet */
   if( recvfrom(sockfd, recvline, MAXLINE, 0, (struct sockaddr*)&client_addr, &cli_addr_len) < 0 ) {
      close(sockfd);
      err_quit("UDP recvfrom error");
   }
     
   memcpy(&id_net, recvline, 2); // id, 2 bytes
   params.id = ntohs(id_net);
   memcpy(&params.version, &recvline[2], 1); // ver, 1 byte
   memcpy(&params.direction, &recvline[2+1], 1); // direction, 1 byte
   memcpy(&sid_net, &recvline[2+1+1], 4); // sid, 4 bytes
   params.sid = ntohl(sid_net); /* long */
   memcpy(&tid_net, &recvline[2+1+1+4], 4); // tid, 4 bytes
   params.tid = ntohl(tid_net); /* long */
   memcpy(&params.prefix, &recvline[2+1+1+4+4], 2); // prefix, 2 bytes
   params.prefix = ntohs(params.prefix);
   memcpy(&params.nro, &recvline[2+1+1+4+4+2], 4); // phone number, 4 bytes
   params.nro = ntohl(params.nro); /* long */
   memcpy(&maclen_net, &recvline[2+1+1+4+4+2+4], 2);
   params.maclen = ntohs(maclen_net);

   char hmac_sha256[params.maclen+1];
   add_terminating_zero(hmac_sha256, params.maclen);
   memcpy(hmac_sha256, &recvline[2+1+1+4+4+2+4+2], params.maclen);
   
   // check received data
   if( params.id != 5010 ||
       params.version != p_version ||
       params.direction != 'P' ||
       params.sid != SID ) {
      printf("UDP query broken or not according to protocol \n");
      return false;
   }
        
   // calculate hmac based on received data
   char hmac_sha256_calc[DIGEST_ASCII];
   create_hmac(NULL, &params, K, hmac_sha256_calc);
   
   // check the hmacs
   if( strncmp(hmac_sha256, hmac_sha256_calc, params.maclen) != 0 ){
      printf("UDP query has incorrect hash \n");
      return false;
   }
   
   // If it's all ok, print the query
   printf("Query received - prefix: %d - number: %d\n", params.prefix, params.nro);

   /* Get answer from "database" */
   char answer[14]; // Answers always 14 chars long
   getAnswerToQuery( params.prefix, params.nro, answer );
   
   /* Time to reply to the query... */
   send_udp_reply(sockfd, SID, params.tid, K, answer, 14, client_addr);
   
   printf("Replied to UDP query\n");
   return true;
}



/* UDP communication part */
void serveUDP( unsigned int port, uint32_t SID, mpz_t K ) {
   /* Initialize the UDP connection */
   printf("Opening UDP port %d for listening \n", port);
   
   /* Create a UDP socket */
   /* PF_INET --> IPv4, SOCK_DGRAM --> UDP */
   int sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if( sockfd < 0 ) err_sys("UDP socket error");
   
   /* Create struct for server address and zero it. */
   struct sockaddr_in server_addr;
   memset((char *)&server_addr, 0, sizeof(server_addr));
   
   /* Set rest of the server address struct */
   server_addr.sin_family      = AF_INET;  /* IPv4 */
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   server_addr.sin_port        = htons(port);
   
   /* Bind the socket to the listen port. */
   if( bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1 ) {   
      close(sockfd);
      err_sys("UDP bind error");
   }
   
   /* Get query from client, and answer */
   if( !parse_udp_query( sockfd, SID, K ) ) {
      close(sockfd);
      err_quit("parseUDPquery failed");
   }
   
   close(sockfd);
   printf("Connection closed - only one query possible in this version :)\n");
}



int main( int argc, char *argv[] ) {
   
   /* Server port */
   unsigned int port = 0;
   
   /* Watchdog code */
   if( signal(SIGALRM, watchdog) == SIG_ERR ) exit(2); /* Watchdog caused error */
   alarm( MAX_RUNTIME );  /* Program stops after MAX_RUNTIME seconds */
   
   /* Parsing command line parameters */
   if( argc == 3 && strcmp( argv[1], "--portti" ) == 0 ) {
      port = atoi( argv[2] );
      if( port == 0 ) print_usage(); /* Port number not valid */
   }
   else print_usage(); /* Command line parameters not valid */
   
   /* K variable for this session */
   mpz_t K;

   /* Session ID for this session*/
   uint32_t SID;
   
   /* Authorization via TCP - returns K and SID */
   auth( port, &SID, &K );
  
   /* Authorization is done */
   printf("Authorization complete. \n");

   /* UDP communication */
   serveUDP( port, SID, K );
   
   return 0;
}
