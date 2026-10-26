#define UP_HTTP_HOST 	"foryou.pl"
#define UP_HTTP_PATH 	"/~esio/c"
#define UP_HTTP_PORT	80

#define UP_INFOFILE	"update"

#define UP_LOCKFILE	".knb-update"

#define UP_VERSION_STR	"01" /* VERSION (string) */
#define UP_VERSION_INT	"02" /* VERSION (int) */
#define UP_FREEBSD_STR	"03" /* SIZE: freebsd*/
#define UP_LINUX_STR	"04" /*	      linux */
#define UP_SUNOS_STR	"05" /*	      sunos */
#define UP_WINDOWS_STR	"06" /*       cygwin */

#define UP_SRC_STR	"10" /*       sources */

#define UP_TARFILE	"update.tar.gz"

void end_update();
bool check_update();
bool set_variable3(char *var, char *args, char *error);
bool load_info(char *file);
bool fetch_info(char *file);
bool fetch_package();
void upgrade(char *file);
bool preupdate();
bool update();
bool fork_and_go();
int get_index_by_uname(char *dupa);
