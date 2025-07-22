#include "common.h"


char * trimstr (char * s, char c) {
	char *p;
	assert (s);
	
	p = s + strlen (s) - 1;
	* (p + 1) = '\0';
	
	return s;
}

void remove_specialstr(char *src, char *dst) {
	int i, len, r = 0;
	char nc;
	
	if ((len = strlen(src)) <= 0) {
		return;
	}
		
	for (i=0; i<len; i++) {
		nc = *(src +i);
		if (nc == 0x28 
			|| nc == 0x29
			|| (nc > 0x60 && nc < 0x7b)) {
			
			dst[r] = nc;
			r++;
		}
	}
	
	return;
}

int path_prefix(char *dest, char *src) {
	int i, srclen;
	
	assert (dest);
	assert (src);
				
	srclen = strlen(src);
	if (srclen > FILENAME_MAX)
		return -1;
	
	strcpy(dest, src);
	
	for (i = srclen - 1; i >= 0 ; i--) {
		if (src[i]=='/') {
			dest[i+1] = '\0';
			break;	
		}
	}
	
	return 0;
}

int compare(const void *a, const void *b) {
	int comp1, comp2;
				
	assert(a != NULL);
	assert(b != NULL);
	
	comp1 = *(int *)a;
	comp2 = *(int *)b;
			
	return (comp1-comp2);
}
 
int maxval(int val[], size_t n) 
{	
	int tmp	[n];
	
	assert (n > 0);	
	
	memcpy (tmp, val, sizeof (tmp));
	
	qsort (tmp, n, sizeof(int), compare);		
	return tmp [n-1];
}

int maxidx (int val[], size_t n)
{
	int max, i;
	
	max = maxval (val, n);
	
	for (i = 0; i < n; i ++)
		if (val [i] == max)
			return i;
			
	assert (0);
	return 0;
}

int minval(int val[], size_t n) 
{	
	int tmp [n];
	
	assert (n > 0);
	
	memcpy (tmp, val, sizeof (tmp));
	
	qsort (tmp, n, sizeof(int), compare);
	return tmp [0];
}

int minidx (int val[], size_t n)
{
	int min, i;
	
	min = minval (val, n);
	
	for (i = 0; i < n; i ++)
		if (val [i] == min)
			return i;
			
	assert (0);
	return 0;
}

int sum (int val[], size_t n) 
{
	int i, sum;
	
	assert (val);
	assert (n > 0);	
		
	for (i=0, sum=0; i < n; i++)
		sum += val[i];
	
	return sum;
}


void swap_byte (char *a, char *b) {
	char tmp;
	
	tmp = *a;
	*a = *b;
	*b = tmp;	
}

int ipstr2no (char *ipstr) {
	int ipno = 0, pos = 0;
	char buf[32];
	char *p1, *pp;

	pp = ipstr;
	
	return ipno;
}

void no2ipstr (int ipno, char *ipstr) {
	int p1, p2, p3, p4;

	return;
}

void set_signal_handler(void (*func)(int), int sig){
	struct sigaction act;
		
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);	
	sigaddset(&act.sa_mask, sig);
	act.sa_flags = SA_RESTART;
	sigaction(sig, &act, NULL);

  if (sigaction(sig, &act, (struct sigaction *) NULL) != 0) {
    fprintf(stderr, "error setting signal handler for %d (%s)\n", sig, strsignal(sig));
    exit(EXIT_FAILURE);
  }
  
}

void sig_block(int sig) {
	sigset_t set;
	
	sigemptyset(&set);
	sigaddset(&set, sig);
	if (pthread_sigmask(SIG_BLOCK, &set, NULL)	< 0)
		 printf("Failed to set sigmask.\n");
}

void sig_unblock(int sig) {
	sigset_t set;
	
	sigemptyset(&set);
	sigaddset(&set, sig);
	if (pthread_sigmask(SIG_UNBLOCK, &set, NULL) < 0)
		printf("Failed to unset sigmask.\n");
}

int set_thread_schedule(pthread_t tid, int policy, int priority)
{
	int  maxp, minp;
	struct sched_param scp;

	maxp = sched_get_priority_max(policy);
	minp = sched_get_priority_min(policy);

	if ((maxp < 0) || (minp < 0))
		return -1;
		
	scp.sched_priority = 
		minp + (((float)(maxp - minp) / (float)100) * priority);
			
	if (pthread_setschedparam(tid, policy, &scp)) {
		perror ("pthread_setschedparam");
		return -1;
	}
	else
		return  0;
}

int set_fd_set (fd_set * set, int fd[], size_t n) {
	size_t i = 0;
	
	if (set && fd) 
	{
		FD_ZERO (set);
		
		for (i = 0; i < n; i ++)
			if (fd [i] >= 0)
				FD_SET (fd [i], set);

		return maxval (fd, n);
	}
	return 0;
}

int random_seed (int size, void * buf)
{
    int fd = -1;
	int	ret = 0;

	fd = open ("/dev/urandom", O_RDONLY);
	if (fd < 0)
		fd = open ("/dev/random", O_RDONLY);

    if (fd < 0) {
		ret = -1;
		goto out;
    }

    if (read(fd, buf, size) != size) {
		ret = -1;
		goto out;
    }

	ret = 0;

out:
	if (fd >= 0)
		close (fd);

    return ret;
}


char *
str_skipwhite (char * p)
{
	assert (p);

	while (*p == ' ' || *p =='\t')
		p ++;

	return p;
}

char *
str_getword (char *buf, int bufsz, char *p)
{
	char	*p2 = NULL;
	int		tot = 0;

	assert (buf);
	assert (p);
	assert (bufsz > 0);

	p2 = str_skipwhite (p);
	while (! isspace (*p2) && *p2 != '\0' && tot < (bufsz - 1)) {
		buf [tot] = *p2;
		tot ++;
		p2 ++;
	}

	buf [tot] = '\0';
	return p2;
}

char *
str_replace_needle (char *dest, int destsz, char * src, char *needle, char * new_needle)
{
	char	*p = NULL;
	char	*p1 = NULL;

	if (destsz <= 0 || !dest || ! src || !needle || ! new_needle)
		return NULL;

	snprintf (dest, destsz, src);

	p = strstr (dest, needle);
	if (! p)
		return dest;

	p1 = p + strlen (needle);
	*p = 0;

	STRCAT_BUF (dest, destsz, new_needle);
	STRCAT_BUF (dest, destsz, src + (p1 - p));
	return dest;
}

#include "iconv.h"


int iconv_codepage_conv(char* out_buf, int out_bufsz, char* in_page, char* out_codepage, char* in_codepage)
{
	int bConv = 0;

	memset(out_buf, 0, out_bufsz);
	iconv_t cd = iconv_open(out_codepage, in_codepage);
	if (cd != (iconv_t)(-1))
	{
		// todo .. 
	}

	return bConv;
}

