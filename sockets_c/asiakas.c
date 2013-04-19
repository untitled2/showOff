#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <nettle/hmac.h>

/* Helpful functions are declared here. */
#include "hajap.h"
#include "util.h"
#include "srp.h"

/*
 * There must be fixed number of command
 * line params for this program - 11.
 */
#define OPNUMBER	11
#define SA struct sockaddr
#define DIGEST_ASCII (SHA256_DIGEST_SIZE*2+1)

// Remove this later and reorganize order of functions
const char p_ident[10] = "HAJAP2012";
const char p_version = '1';
// structure for srp variables
struct srp_var {
	mpz_t N, g, s, B, A, K, M1, M2, sid;
};

struct udp_send {
	uint16_t id; // id, 2 bytes
	char direction; // direction, 1 byte
	uint32_t sid; //mpz_get_ui(vars->sid), 4 bytes
	uint32_t tid; // 4 bytes
	uint16_t prefix; // 2 bytes
	uint32_t nro; // 4 bytes
	char hmac_sha256[DIGEST_ASCII]; // N bytes
	uint16_t maclen; // 2 bytes
};

struct udp_recv {
	uint16_t id; // id, 2 bytes
	char version; // version, 1 byte
	char direction; // direction, 1 byte
	uint32_t sid; // sid, 4 bytes
	uint32_t tid; // tid+1, 4 bytes
	uint16_t ans_len; // length of the answer, 2 bytes
	char* answer; // answer, N bytes
	uint16_t maclen; // 2 bytes
};

void create_frame(unsigned char*, const int, const unsigned char*, const int);
void parse_S1(const unsigned char*, struct srp_var*, const char*);
void parse_S3(const unsigned char*, struct srp_var*);

/* Terminate a string with zero. */
static inline void add_terminating_zero(char* string, int length) {

	string[length] = '\0';
}

/* Initialize srp variables. */
void init_var(struct srp_var* vars) {

	mpz_init(vars->N);
	mpz_init(vars->g);
	mpz_init(vars->s);
	mpz_init(vars->B);
	mpz_init(vars->A);
	mpz_init(vars->K);
	mpz_init(vars->M1);
	mpz_init(vars->M2);
	mpz_init(vars->sid);
}

/* Get mpz number-values and save in srp_var struct variables. */
static inline void get_mpz_val(mpz_t val, const size_t val_len, const unsigned char* recvline, const size_t pos) {

	unsigned char buf[val_len+1];
	add_terminating_zero((char*)buf, val_len);

	memcpy(buf, &recvline[pos], val_len);
	if( gmp_sscanf((char*)buf, "%Zx", val ) != 1 )
		err_quit("sscanf()");
}

/*
 * Create hmac-blabla thing.
 * Take either udp_send or udp_recv struct, one on them is NULL
 * and operate on one of them which is not NULL.
 */
void create_hmac(struct udp_send* send_params, struct udp_recv* recv_params, mpz_t K, char* ascii_hex) {

	// binary digest
	unsigned char digest[SHA256_DIGEST_SIZE];
	// passwd
	char t[MAXLINE];
	int count = gmp_sprintf(t, "%Zx", K);
	char passwd[count];
	memcpy(passwd, t, count);

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
		hmac_sha256_update(&buf, 2, (uint8_t*)&send_params->prefix);
		hmac_sha256_update(&buf, 4, (uint8_t*)&send_params->nro);
	}
	else if(recv_params != NULL) {
		hmac_sha256_update(&buf, 2, (uint8_t*)&recv_params->id);
		hmac_sha256_update(&buf, 1, (uint8_t*)&recv_params->version);
		hmac_sha256_update(&buf, 1, (uint8_t*)&recv_params->direction);
		hmac_sha256_update(&buf, 4, (uint8_t*)&recv_params->sid);
		hmac_sha256_update(&buf, 4, (uint8_t*)&recv_params->tid);
		hmac_sha256_update(&buf, 2, (uint8_t*)&recv_params->ans_len);
		hmac_sha256_update(&buf, recv_params->ans_len, (uint8_t*)recv_params->answer);
	}
	else
		err_quit("create_hmac: no structure passed");

	// get binary digest
	hmac_sha256_digest(&buf, SHA256_DIGEST_SIZE, digest);

	// turn binary digest into ascii-hex
	for(int i = 0; i < SHA256_DIGEST_SIZE; ++i)
		sprintf(ascii_hex + i * 2, "%02x", digest[i]);

	// put the ending zero
	add_terminating_zero(ascii_hex, DIGEST_ASCII-1);
	
}

/* Split query to prefix and nro. */
void split(const char* query, uint16_t* prefix, uint32_t* nro) {

	char p[4], n[6];

	// prefix
	for(int i=0; i<4; i++)
		p[i] = query[i];
	*prefix = atoi(p);

	// nro
	for(int i=5; i<11; i++)
		n[i-5] = query[i];
	*nro = atoi(n);
}

void parse_udp_reply(int sockfd, struct udp_send* sent_params, mpz_t K) {

	struct udp_recv params;
	uint16_t id_net, ans_len_net, ans_len, maclen_net, maclen;
	uint32_t sid_net, tid_net;
	char recvline[MAXLINE];

	Read(sockfd, recvline, MAXLINE);

	// parse here, ugly code
	memcpy(&id_net, recvline, 2); // id, 2 bytes
	params.id = id_net;
	memcpy(&params.version, &recvline[2], 1); // ver, 1 byte
	memcpy(&params.direction, &recvline[2+1], 1); // direction, 1 byte
	memcpy(&sid_net, &recvline[2+1+1], 4); // sid, 4 bytes
	params.sid = sid_net;
	memcpy(&tid_net, &recvline[2+1+1+4], 4); // tid, 4 bytes
	params.tid = tid_net;
	memcpy(&ans_len_net, &recvline[2+1+1+4+4], 2); // len, 2 bytes
	params.ans_len = ans_len_net;
	ans_len = ntohs(ans_len_net);
	char answer[ans_len+1];
	add_terminating_zero(answer, ans_len);
	memcpy(answer, &recvline[2+1+1+4+4+2], ans_len);
	params.answer = answer;
	memcpy(&maclen_net, &recvline[2+1+1+4+4+2+params.ans_len], 2);
	params.maclen = maclen_net;
	maclen = ntohs(maclen_net);
	char hmac_sha256[maclen+1];
	add_terminating_zero(hmac_sha256, maclen);
	memcpy(hmac_sha256, &recvline[2+1+1+4+4+2+ans_len+2], maclen);

	// check received data
	assert(sent_params->id == params.id);
	assert(p_version == params.version);
	assert(params.direction == 'A');
	assert(sent_params->sid == params.sid);
	assert(sent_params->tid == (params.tid-1));
	
	// calculate hmac based on received data
	char hmac_sha256_calc[DIGEST_ASCII];
	create_hmac(NULL, &params, K, hmac_sha256_calc);

	// check the hmacs
	assert( strncmp(hmac_sha256, hmac_sha256_calc, maclen) == 0);

	// if it's all ok, print the answer
	printf("%s\n", answer);
}

/*
 * Construct and send a query to server.
 * Then receive the answer.
 */
void query_server(int sockfd, const char* query, struct srp_var* vars) {

	struct udp_send params;
	uint16_t prefix;
	uint32_t nro;

	// get prefix and nro from the query to separate vars
	split(query, &prefix, &nro);

	params.id = htonl(5010); // 2 bytes
	params.direction = 'P'; // 1 byte
	params.sid = mpz_get_ui(vars->sid); // 4 bytes
	params.tid = htonl(111111111); // 4 bytes
	params.prefix = htons(prefix); // 2 bytes
	params.nro = htonl(nro); // 4 bytes
	create_hmac(&params, NULL, vars->K, params.hmac_sha256); // N bytes

	uint16_t maclen_host = strlen(params.hmac_sha256);
	params.maclen = htons(maclen_host); // 2 bytes

	size_t len = 2+1+1+4+4+2+4+2+maclen_host;
	unsigned char sendline[len];

	// sudoku
	memcpy(sendline, &params.id, 2);
	memcpy(&sendline[2], &p_version, 1);
	memcpy(&sendline[2+1], &params.direction, 1);
	memcpy(&sendline[2+1+1], &params.sid, 4);
	memcpy(&sendline[2+1+1+4], &params.tid, 4);
	memcpy(&sendline[2+1+1+4+4], &params.prefix, 2);
	memcpy(&sendline[2+1+1+4+4+2], &params.nro, 4);
	memcpy(&sendline[2+1+1+4+4+2+4], &params.maclen, 2);
	memcpy(&sendline[2+1+1+4+4+2+4+2], params.hmac_sha256, maclen_host);

	// just in case ;)
	assert(sizeof sendline < 2000);

	// send to server
	Write(sockfd, sendline, len);

	// get answer from server and parse it
	parse_udp_reply(sockfd, &params, vars->K);
}

/* Recv and check incoming frame for protocol policy. */
void recv_frame(int sockfd, struct srp_var* vars, const char* msg, const char* passwd) {

	ssize_t frame_len;
	uint16_t frame_len_net;
	size_t p = strlen(p_ident);
	char p_ident_local[p+1];
	add_terminating_zero(p_ident_local, p);
	char p_version_local;
	char s1[3] = "00\0"; // add 0, so the string can end and we can compare it

	// get first 2 bytes that indicate the length of the frame
	Readn(sockfd, &frame_len_net, 2);

	frame_len = ntohs(frame_len_net) - 2; // we already got 2 bytes
	unsigned char recvline[frame_len]; // the block we will be receiving
	
	// get the rest of the frame
	Readn(sockfd, recvline, frame_len); 

	memcpy(p_ident_local, recvline, p);
	memcpy(&p_version_local, &recvline[p], 1);
	memcpy(&s1, &recvline[p+1], 2);

	if( strcmp(p_ident_local, p_ident) != 0 ||
			(p_version_local != p_version) ||
			strcmp(s1, msg) != 0) {
		err_quit("recv_frame: frame isn't constructed right");
	}
	else {
		if( strcmp(msg, "S1") == 0)
			parse_S1(recvline, vars, passwd); // call parse for S1
		else if( strcmp(msg, "S3") == 0)
			parse_S3(recvline, vars); // call parser for S3
		else
			err_quit("recv_frame: unknown string to parse");
	}
}

/* Parse first message that came from server. */
void parse_S1(const unsigned char* recvline, struct srp_var* vars, const char* passwd) {

	printf("S1 received\n");

	assert(passwd != NULL);

	uint16_t N_len_net, g_len_net, s_len_net, B_len_net; // network byte order
	ssize_t N_len, g_len, s_len, B_len; // host byte order
	size_t p = strlen(p_ident);

	// set all variables

	// N
	memcpy(&N_len_net, &recvline[p+1+2], 2);
	N_len = ntohs(N_len_net);
	get_mpz_val(vars->N, N_len, recvline, p+1+2+2);
	set_N(vars->N);

	// g
	memcpy(&g_len_net, &recvline[p+1+2+2+N_len], 2);
	g_len = ntohs(g_len_net);
	get_mpz_val(vars->g, g_len, recvline, p+1+2+2+N_len+2);
	set_g(vars->g);

	// s
	memcpy(&s_len_net, &recvline[p+1+2+2+N_len+2+g_len], 2);
	s_len = ntohs(s_len_net);
	get_mpz_val(vars->s, s_len, recvline, p+1+2+2+N_len+2+g_len+2);
	client_set_v(vars->s, (unsigned const char*)passwd);

	// B
	memcpy(&B_len_net, &recvline[p+1+2+2+N_len+2+g_len+2+s_len], 2);
	B_len = ntohs(B_len_net);
	get_mpz_val(vars->B, B_len, recvline, p+1+2+2+N_len+2+g_len+2+s_len+2);
	client_setB(vars->B);

	// A, K, M1
	client_init(vars->A);
	client_K(vars->K);
	client_M1(vars->M1);
}

/* Parse second message, that comes from server. */
void parse_S3(const unsigned char* recvline, struct srp_var* vars) {

	printf("S3 received\n");

	uint16_t M2_len_net;
	uint16_t M2_len;
	size_t p = strlen(p_ident);
	uint32_t sid_net;
	uint32_t sid_ui;
	
	// M2
	memcpy(&M2_len_net, &recvline[p+1+2], 2);
	M2_len = ntohs(M2_len_net);
	get_mpz_val(vars->M2, M2_len, recvline, p+1+2+2);

	// get the key
	memcpy(&sid_net, &recvline[p+1+2+2+M2_len], 4);
	sid_ui = ntohl(sid_net);
	mpz_set_ui(vars->sid, sid_ui);
}

/* Calculate and return length of mpz_t variable. */
static inline int get_var_len(mpz_t val, unsigned char* buf) {

	int count = gmp_snprintf((char*)buf, MAXLINE, "%Zx", val);
	if(count < 0)
		err_quit("gmp_snprintf()");

	return count;
}

/* Authenticate with the server. */
void auth(int sockfd, const char* uname, const char* passwd, struct srp_var* vars) {

	assert(uname != NULL);

	// initialize srp variables
	init_var(vars);

 /*
	* Create first command, then frame, and send it to server.
  * Magic numbers are for protocol structure lengths.
	*/
	unsigned char uname_len = strlen(uname);
	size_t command1_len = 2 + 1 + uname_len;
	unsigned char command1[command1_len];
	size_t frame_len = sizeof(uint16_t) + strlen(p_ident) + sizeof(char) + command1_len;
	unsigned char frame1[frame_len];

	/* Create first command. */
	memcpy(command1, "S0", 2);
	memcpy(&command1[2], &uname_len, 1);
	memcpy(&command1[3], uname, uname_len);

	/* Create frame for the first command. */
	create_frame(frame1, frame_len, command1, command1_len);

	assert(frame1 != NULL);

	/* Send the first frame to server. */
	Writen(sockfd, frame1, frame_len); 

	/* Receive and parse first message that came from server. */
	recv_frame(sockfd, vars, "S1", passwd);

	/* Create second command. Use new variables not to get confused. */
	// get lengths of A and M1
	unsigned char A_buf[MAXLINE], M1_buf[MAXLINE];
	size_t A_len = get_var_len(vars->A, A_buf);
	size_t M1_len = get_var_len(vars->M1, M1_buf);

	uint16_t A_len_net = htons(A_len);
	uint16_t M1_len_net = htons(M1_len);
	size_t command2_len = 2 + 2 + A_len + 2 + M1_len;
	unsigned char command2[command2_len];
	size_t frame2_len = sizeof(uint16_t) + strlen(p_ident) + sizeof(char) + command2_len;
	unsigned char frame2[frame2_len];

	memcpy(command2, "S2", 2);

	memcpy(&command2[2], &A_len_net, 2);
	memcpy(&command2[2+2], A_buf, A_len);

	memcpy(&command2[2+2+A_len], &M1_len_net, 2);
	memcpy(&command2[2+2+A_len+2], M1_buf, M1_len);

	/* Create frame for the second command. */
	create_frame(frame2, frame2_len, command2, command2_len);

	/* Send the second frame to server. */
	Writen(sockfd, frame2, frame2_len);

	/* Receive and parse second message, that came from server. */
	recv_frame(sockfd, vars, "S3", NULL);

	/* Check M2. */
	if(client_verifyM2(vars->M2))
		printf("auth OK\n");
	else
		err_quit("auth failed");
}

/* Build a frame we send to server. */
void create_frame(unsigned char* frame, const int frame_len, const unsigned char* command, const int command_len) {

	// if an error occurred, we see correct error message
	assert( sizeof(uint16_t) == 2);
	assert( sizeof(char) == 1);

	int s = sizeof(uint16_t);
	int p = strlen(p_ident);
	int c = sizeof(char);
	uint16_t frame_len_net = htons(frame_len);

	/* Construct the frame. */
	memcpy(frame, &frame_len_net, s);
	memcpy(&frame[s], p_ident, p);
	memcpy(&frame[s+p], &p_version, c);
	memcpy(&frame[s+p+c], command, command_len);
}

/*
 * Open a TCP connection to server.
 * Or make a connected UDP socket.
 */
void open_connection(int* sockfd, const char* serv_ip, const char* serv_port, const char* protocol) {

	struct sockaddr_in servaddr;

	/* Null the structure. */
	bzero(&servaddr, sizeof servaddr);

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(serv_port));

	/* Turn ip address of the server to network byte order. */
	if(inet_pton(AF_INET, serv_ip, &servaddr.sin_addr) <= 0)
		err_quit("inet_pton  error for %s", serv_ip);

	// make TCP or UDP socket
	if( strcmp(protocol, "TCP") == 0) {
		if( (*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			err_sys("tcp socket error");

		if( connect(*sockfd, (SA *) &servaddr, sizeof servaddr) < 0)
			err_sys("tcp connect error");
	}
	else if( strcmp(protocol, "UDP") == 0) {
		if( (*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
			err_sys("tcp socket error");
		if( connect(*sockfd, (SA *) &servaddr, sizeof servaddr) < 0)
			err_sys("udp connect error");
	}
	else
		err_quit("open_connection: unknown protocol");

	/* Connection succeed, return to main(). */
}

/* Print usage information and exit with EXIT_CODE. */
void print_usage(const char* program_name) {
	fprintf(stdout, "Usage: %s OPTIONS\n", program_name);
	fprintf(stdout,
					"	-o	--palvelin N		Palvelimen ip osoite.\n"
					"	-p	--portti P		Palvelimen portti.\n"
					"	-t	--tunnus K		Käyttäjätunnus.\n"
					"	-s	--salasana S		Salasana.\n"
					"	-k	--kysely PPPP-NNNNNN	Palvelimelta kysyttävä puhelinnumero (Prefix ja Numero).\n");

	exit(0);
}

/* Main program entry point. ARGC contains number of argument list
	 elements; ARGV is an array of pointers to them. */
int main(int argc, char* argv[]) {

	int tcp_sockfd, udp_sockfd;
	int next_option;
	struct srp_var vars; // structure for srp variables

	/* String listing valid short options. */
	const char* const short_options = "ho:p:t:s:k:";

	/* Array describing valid long options. */
	const struct option long_options[] = {
		{"palvelin",	1, NULL, 'o'},
		{"portti",		1, NULL, 'p'},
		{"tunnus",    1, NULL, 't'},
		{"salasana",  1, NULL, 's'},
		{"kysely",    1, NULL, 'k'},
		{ NULL,				0, NULL,  0 }		/* Required at the end of the array. */
	};

	/* Server ip address. */
	const char* serv_ip = NULL;

	/* Server port. */
	const char* serv_port = NULL;

	/* Username. */
	const char* uname = NULL;

	/* Password. */
	const char* passwd = NULL;

	/* Query to send to the server. */
	const char* query = NULL;


	do {

		next_option = getopt_long(argc, argv, short_options, long_options, NULL);

		switch(next_option) {
			case 'o':
				serv_ip = optarg;
				break;
			case 'p':
				serv_port = optarg;
				break;
			case 't':
				uname = optarg;
				break;
			case 's':
				passwd = optarg;
				break;
			case 'k':
				query = optarg;
				break;
			case -1:
				break;
			case '?':
				print_usage(argv[0]);
			default:
				abort();
		}
	} while(next_option != -1);

	/* There must be 11 arguments EXACTLY. */
	if(argc != OPNUMBER) {
		printf("Too few or too many arguments.\n");
		print_usage(argv[0]);
	}

	// check parameters
	

	/* Open a TCP connection to server. */
	open_connection(&tcp_sockfd, serv_ip, serv_port, "TCP");

	/* Authenticate. */
	auth(tcp_sockfd, uname, passwd, &vars);

	assert(mpz_get_ui(vars.sid) != 0);

	/* Close connection. */
	close(tcp_sockfd);

	/* Open UDP connection to server. */
	open_connection(&udp_sockfd, serv_ip, serv_port, "UDP");

	/* Query to server. */
	query_server(udp_sockfd, query, &vars);

	/* Close connections. */
	close(udp_sockfd);


	return 0;
}
