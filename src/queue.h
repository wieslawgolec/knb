int get_index_by_queue(char *nick);
void exchange_queue(int s1, int s2);
void sort_queue(int type);
bool rem_queue(int index);
bool clear_queue();
bool add_queue(char *nick, int prio);
void ins_queue(void);
void build_queue(void);
void build_squeue(void);
