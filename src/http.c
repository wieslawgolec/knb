#include "prots.h"

int getfile(char *file, char *outfile)
{
    int i, f, fd = 0, len, ret, result;
    bool header;
    char buf[8192];
    
    sprintf(buf, "%s/%s", UP_HTTP_PATH, file);
    
    if((f = open(outfile, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU)) < 1)
    {
	result = 1;
	goto end;
    }
    if((fd = do_connect(me.update.ip, me.update.port, "", 0)) < 1)
    {
    	result = 2;
	goto end; 
    }
    if(!send_request(fd, buf))
    {
        result = 3;
	goto end;
    }	    

    header = true;
    do
    {
	len = recv(fd, buf, sizeof(buf), 0);

	if(header) /* sprawdzay, czy byl jzu naglowek */
	{
	    i = process_header(buf, &ret); /* sprawdzamy wartosc result i koneic naglwoka */

	    if(i)
	    {
		header = false;    
		if(ret != 200)
		{
		    result = -ret;
		    goto end;
		}
		result = write(f, buf + i, len - i);
	    }
	    else
		result = write(f, buf, len);
	}
	else
	    result = write(f, buf, len);

    }while(len > 0);

    result = 0;
    end:	
	if(f) close(f);
	if(fd) close(fd);
    return result;
}	

bool send_request(int fd, char *path)
{
    char buf[MAX_LEN];
    
    sprintf(buf, "GET %s HTTP/1.0\r\nUser-Agent: Mozilla/5.0 (compatybile: %s-%s-%s)\r\nHost: %s:%d\r\n\r\n",
    path, KNB_SHORT, KNB_VERSION_STR, me.uname_str, me.update.name, me.update.port);   

    if(write(fd, buf, sizeof(buf)) == -1)
	return false;
    return true;
}

int process_header(char *data, int *ret)
{
    char buf[5];
    char *n; 
    int i, j, k;
    
    for(i = 0, k = 0, j = 0; i < strlen(data) && j != 2 && k < 4; i++)
    {
	if(j == 1)
	{
	    if(data[i] < '0' || data[i] > '9')
		j = 2;
	    else
		buf[k++] = data[i];
	}
	if(data[i] == ' ') j = 1;
    }
    buf[k] = '\0';
    
    *ret = 0;
    if(_isnumber(buf)) 
	*ret = atoi(buf);
    
    if((n = strstr(data, "\r\n\r\n")) != NULL)
	return (int) abs(data - n) + 4;
	
    return 0;
}
