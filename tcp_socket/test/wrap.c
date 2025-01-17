#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

void sys_err(const char* msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int Socket(int domain,int type,int protocol)
{
	int n;
	if((n = socket(domain,type,protocol)) == -1)
		sys_err("socket");
	return n;
}

int Bind(int sockfd,const struct sockaddr* addr,socklen_t addrlen)
{
	int n;
	if((n = bind(sockfd,addr,addrlen)) == -1)
		sys_err("bind");
	return n;
}

int Listen(int sockfd,int backlog)
{
	int n;
	if((n = listen(sockfd,backlog)) == -1)
		sys_err("listen");
	return n;
}

int Accept(int sockfd,struct sockaddr* addr,socklen_t* addrlen)
{
	int n;
again:
	if((n = accept(sockfd,addr,addrlen)) < 0)
	{
		if(errno == ECONNABORTED || (errno == EINTR))
			goto again;
		else
			sys_err("accept");
	}
	return n;
}

int Connect(int sockfd,const struct sockaddr* addr,socklen_t addrlen)
{
	int n;
	if((n = connect(sockfd,addr,addrlen)) == -1)
		sys_err("connect");
	return n;
}

ssize_t Read(int fd,void* buf,size_t count)
{
	ssize_t size;
again:
	if((size = read(fd,buf,count)) == -1)
	{
		if(errno == EINTR)
			goto again;
		else
			sys_err("read");
	}
	return size;
}

ssize_t Write(int fd,void* buf,size_t count)
{
	ssize_t size;
again:
	if((size = write(fd,buf,count)) == -1)
	{
		if(errno == EINTR)
			goto again;
		else
			sys_err("write");
	}
	return size;
}

int Close(int fd)
{
	int n;
	if((n = close(fd)) == -1)
		sys_err("close");
	return n;
}

ssize_t readn(int fd,char* vptr,size_t n)
{
	size_t nleft;//还没有读的的大小
	size_t nread;//已经读的大小
	char* ptr;
	nleft = n;
	ptr = vptr;
	while(nleft > 0)
	{
		if((nread = read(fd,ptr,nleft)) == -1)
		{
			if(errno == EINTR)
				nread = 0;
			else
				return -1;
		}
		else if(nread == 0)
			break;
		nleft -= nread;
		ptr += nread;
	}
	return n - nleft;
}

ssize_t writen(int fd,const char* vptr,size_t n)
{
	size_t nleft;
	size_t nwrite;
	const char* ptr;
	nleft = n;
	ptr = vptr;
	while(nleft > 0)
	{
		if((nwrite = write(fd,ptr,nleft)) == -1)
		{
			if(errno == EINTR)//被信号打扰
				nwrite = 0;
			else 
				return -1;
		}
		else if(nwrite == 0)
			break;
		nleft -= nwrite;
		ptr += nwrite;
	}
	return n - nleft;
}

ssize_t my_read(int fd,char* ptr)
{
	static int read_cnt;
	static char* read_ptr;
	static char read_buf[100];
	

	if(read_cnt <= 0)
	{
again:
		if((read_cnt = read(fd,read_buf,sizeof(read_buf))) == -1)
		{
			if(errno == EINTR)
				goto again;
			else
				return -1;
		}
		else if(read_cnt == 0)
			return 0;
		read_ptr = read_buf;
	}
	read_cnt--;
	*ptr = *read_ptr;
	read_ptr++;
	return 1;
}

ssize_t Readline(int fd,void* vptr,size_t maxlen)
{
	int ret;
	char c,*ptr;
	int i;
	ptr = vptr;
	for(i = 0;i<maxlen;i++)
	{
		if((ret = my_read(fd,&c)) == 1)
		{
			*ptr = c;
			ptr++;
			if(c == '\n')
				break;
		}
		else if(ret == 0)
		{
			*ptr++ = '\0';
			return i;
		}
		else
			return -1;
	}
	*ptr = 0;
	return i + 1;
}
