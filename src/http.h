#define ERR_404 	"404: File not found"
#define ERR_403 	"403: Access Denied"
#define ERR_CONN	"Cannot estabilish connection"
#define ERR_HEADER	"Invalid HTTP header"
#define ERR_EOF		"EOF from client"
#define ERR_READ	"Read error"
#define ERR_URL		"Invalid URL"
#define ERR_DST		"Cannot open destination file"
#define ERR_UNKNOW	"Unknown error"

int getfile(char *file, char *outfile);
bool send_request(int fd, char *path);
int process_header(char *data, int *ret);
