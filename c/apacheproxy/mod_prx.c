#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_protocol.h"
#include "apr_strings.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <utmp.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/times.h>
#include <errno.h>

#include <semaphore.h>
//размер буферов для получения данных от producer используется только при передаче ответа клиенту при получения данных от клиента используется буфер с изменяющимся размером.
#define SIZE_BIG_BUFFER		8192 
//Размер типичного COOKIE посылаемого producer
#define	COOKIES_SIZE		65
//константа окончания заголовка
#define END_OF_HEAD		"\r\n\r\n"
//константа определяющая имя кукиеса используемого в producer
#define producer_COOKIES		"PRXSESSIONID="
//константа определяющая какое поле считать типом содержимого
#define CONTENT_TYPE		"Content-Type: "
//весия модуля
#define VERSION			"mod_prx=2.1.15 alfa version!"
//путь для которого требуется создавать индификатор соединения
#define	INCLUDE_PATH		"/prx/baza"
//шаблон запроса GET
#define simplereqGET  		"GET %s HTTP/1.0\r\nCookie: %s\r\nUser-Agent: mod_prx \r\nAccept: */*\r\nHost: %s:%d\r\nConnection: Keep-Alive\r\n\r\n"
//шаблон запроса POST
#define simplereqPOST 		"POST %s HTTP/1.1\r\nCookie: %s\r\nUser-Agent: mod_prx \r\nAccept: */*\r\nReferer: http://localhost/test.html\\r\nHost: %s\r\nContent-Length: %d\r\nConnection: Keep-Alive\r\n\r\n"
//символы не в встречающиеся внутри поля заголовков(разделитель полей)
#define EXCLUDECHAR		"\t\n\r"
#define MaxExt		3
#define MAXSTRING		20
//количество сохраненных кукиесов
#ifndef DEFAULT_MODPRX_MAXCOOKIES
#define DEFAULT_MODPRX_MAXCOOKIES	3
#endif

//значения по умолчанию
#ifndef DEFAULT_MODPRX_SIDSAVEPATH
#define DEFAULT_MODPRX_SIDSAVEPATH "/tmp/cookies"
#endif

#ifndef DEFAULT_MODPRX_SemName
#define DEFAULT_MODPRX_SemName "semaphore"
#endif

#ifndef DEFAULT_MODPRX_DestinationPort
#define DEFAULT_MODPRX_DestinationPort 8972
#endif

#ifndef DEFAULT_MODPRX_DestinationIP
#define DEFAULT_MODPRX_DestinationIP "127.0.0.1"
#endif

#ifndef DEFAULT_MODPRX_EXT_ORIG
#define DEFAULT_MODPRX_EXT_ORIG "ext"
#endif

#ifndef DEFAULT_MODPRX_EXT_EXCH
#define DEFAULT_MODPRX_EXT_EXCH "prx"
#endif

#ifndef DEFAULT_Semaphore_count
#define DEFAULT_Semaphore_count 1
#endif
//////
//описания функций даны далее
static void *create_modprx_config(apr_pool_t *p, server_rec *s);
static void mod_prx_register_hooks (apr_pool_t *p);
static const char *set_modprx_DESTINATIONPORT(cmd_parms *parms, void *mconfig, const char *arg);
static const char *set_modprx_DESTINATIONIP(cmd_parms *parms, void *mconfig, const char *arg);
static const char *set_modprx_SemName(cmd_parms *parms, void *mconfig, const char *arg);
static const char *set_modprx_SIDSAVEPATH(cmd_parms *parms, void *mconfig, const char *arg);
static const char *set_modprx_EXT_EXCH(cmd_parms *parms, void *mconfig, const char *arg);
static const char *set_modprx_EXT_ORIG(cmd_parms *parms, void *mconfig, const char *arg);
static const char *set_modprx_SemCount(cmd_parms *parms, void *mconfig, const char *arg);
static const char *set_modprx_CookiesCount(cmd_parms *parms, void *mconfig, const char *arg);
static char * create_unique_id();
void lock_W(int fd);
void lock_R(int fd);
void unlock(int fd);
int out_debug(apr_table_t * table,request_rec *r,char * id,char * name);
struct str_cookies
{
	char data[COOKIES_SIZE];
	int count;
};
//блокировка файла при записи
void lock_W(int fd)
{
	struct flock lock;
	lock.l_type= F_WRLCK;
	lock.l_whence= SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	fcntl(fd,F_SETLKW,&lock);
}
//блокировка файла при чтении(мягкая позволяет другим потокам(процессам) считывать из файла информацию)
void lock_R(int fd)
{
	struct flock lock;
	lock.l_type= F_RDLCK;
	lock.l_whence= SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	fcntl(fd,F_SETLKW,&lock);
}
//убрать блокировку файла
void unlock(int fd)
{
	struct flock lock;
	lock.l_type= F_UNLCK;
	lock.l_whence= SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	fcntl(fd,F_SETLKW,&lock);
}

//определение директив используемых в модуле
static const command_rec mod_prx_cmds[] =
{
	AP_INIT_TAKE1(
			"ModulePRXSIDSavePath",
	(const char* (*)())set_modprx_SIDSAVEPATH,
	NULL,
	RSRC_CONF,
	"ModulePRXSIDSavePath <сторочка> -- путь куда сохраняются cookies."
		     ),
	AP_INIT_TAKE1(
			"ModulePRXDestinationIP",
	(const char* (*)())set_modprx_DESTINATIONIP,
	NULL,
	RSRC_CONF,
	"ModulePRXDestinationIP <число в скобках> -- IP сервера на который перенаправляют запросы."
		     ),
	AP_INIT_TAKE1(
			"ModulePRXDestinationPort",
	(const char* (*)())set_modprx_DESTINATIONPORT,
	NULL,
	RSRC_CONF,
	"ModulePRXDestinationPort <число в скобках> -- порт на который перенаправляют запросы."
		     ),
	AP_INIT_TAKE1(
			"ModulePRXSemaphoreName",
	(const char* (*)())set_modprx_SemName,
	NULL,
	RSRC_CONF,
	"ModulePRXLog <string> -- имя семофора синхронизации ограничение на количество запросов в еденицу времени."
		     ),
	AP_INIT_TAKE1(
			"ModulePRXSemaphoreCount",
	(const char* (*)())set_modprx_SemCount,
	NULL,
	RSRC_CONF,
	"ModulePRXSemaphoreCount \"<число в скобках>\" -- количество запросов в один момент времени."
		     ),
	AP_INIT_TAKE1(
			"ModulePRXCookiesCount",
	(const char* (*)())set_modprx_CookiesCount,
	NULL,
	RSRC_CONF,
	"ModulePRXCookiesCount \"<число в скобках>\" -- количество эмулируемых клиентов в момент времени."
		     ),
	AP_INIT_TAKE1(
			"ModulePRXEXT_ORIG",
	(const char* (*)())set_modprx_EXT_ORIG,
	NULL,
	RSRC_CONF,
	"ModulePRXEXT_ORIG <строка> -- оригинальное расширение."
		     ),
	AP_INIT_TAKE1(
			"ModulePRXEXT_EXCH",
	(const char* (*)())set_modprx_EXT_EXCH,
	NULL,
	RSRC_CONF,
	"ModulePRXEXT_EXCH <строка> -- расширение на которое нужно заменять."
		     ),

	{NULL}
};
//описание модуля в терминах сервера
module AP_MODULE_DECLARE_DATA prx_module =
{
	STANDARD20_MODULE_STUFF,//тип модуля
	NULL, // конфигурация директорий
	NULL, // объединение конфигурация директорий
	create_modprx_config, //создание структура конфигурации 
	NULL, // объединение серверов
	mod_prx_cmds, // структура конфигурации (директивы) 
	mod_prx_register_hooks, // регистрация функций
};
//структура хранения данных в модуле
typedef struct {
	int	DESTINATIONPORT;//порт пере направления
	char *	DESTINATIONIP;//имя сервера куда перенаправлять
	char *	SemName;//имя семафора
	char *	SIDSAVEPATH;//путь куда сохранять кукиес(индефикатор сессии)
	char *	EXT_EXCH[MaxExt][MAXSTRING];//измененное расширение
	char *	EXT_ORIG[MaxExt][MAXSTRING];//исходное расширение
	int 	SemCount;//количество запросов в один момент времени
	int 	CookiesCount;//количество сохраненных кукиесов
}mod_prx_config;
//создание уникального индефикатора сессии
static char * create_unique_id()
{
	time_t tm = 0;
	struct tms *tmsbuf = NULL;
	clock_t ticks;
	char base62_chars[63] = {"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};
	char yield1[7] = {"------"};
	char yield2[7] = {"------"};
	char yield3[3] = {"--"};
	char yield4[3] = {"--"};
	unsigned int value = 0;
	char *result =(char*)calloc(20,sizeof(char));
	signed int i;

	tm = time(NULL);
	tmsbuf = (struct tms *) malloc(sizeof(struct tms));
	ticks = times(tmsbuf);

	value = tm;
	for (i = 5; i >= 0; i--) {
		yield1[i] = base62_chars[value % 62];
		value = value / 62;
	}

	value = getpid();
	for (i = 5; i >= 0; i--) {
		yield2[i] = base62_chars[value % 62];
		value = value / 62;
	}

	value = ticks;
	for (i = 1; i >= 0; i--) {
		yield3[i] = base62_chars[value % 62];
		value = value / 62;
	}
	
	value = rand();
	for (i = 1; i >= 0; i--) {
		yield4[i] = base62_chars[value % 62];
		value = value / 62;
	}

	strcat(result, yield1);
	strcat(result, yield2);
	strcat(result, yield3);
	strcat(result, yield4);
	free(tmsbuf);

	return result;
}
//объединение бинарных данных(не символьных) указывается размер 
static void my_bincat(char **dest,size_t size_dest,const char *src,size_t size_src)//for binary cat.
{
	if(dest==NULL)return;
	if(*dest==NULL) return;
	if(src==NULL)return;
	if(size_src==0)return;
	size_t size=size_dest+size_src;
	*dest=(char*)realloc(*dest,size);
	if(*dest==NULL) 	return;	
	memcpy((*dest)+size_dest,src,size_src);
}
//объединение символьных строчек используя функцию бинарного объединения (реально созданный буфер на 1 байт больше (ставиться нолик для того чтобы можно было использовать в объединение символьных данных)(использует особенность этой функции - добавления нуля в конец буфера)
static void my_strcat(char **dest,const char *src)
{
	if(dest==NULL)return;
	if(src==NULL)return;
	//(использует особенность этой функции - добавления нуля в конец буфера)
	my_bincat(dest,strlen(*dest),src,strlen(src)+1);
}
//вывод содержимого таблиц
int out_debug(apr_table_t * table,request_rec *r,char * id,char * name) //out contents of table
{
	if(table==NULL)return -1;
	if(r==NULL)return -1;
	if(id==NULL)return -1;
	if(name==NULL)return -1;
	{
		apr_table_entry_t *pEntry = (apr_table_entry_t *)
        		((apr_array_header_t *)table)->elts;
		size_t n = ((apr_array_header_t *)table)->nelts;
		size_t i = 0;
		for (; i < n; i++) {
			ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
			"mod_prx: req_id=%s %s:%s = %s",id,name,pEntry[i].key,pEntry[i].val);
		};
	};
    return 0;
}
//метод отвечающий за работу запроса POST
static int metodPOST(char * req,request_rec *r,char * id,int sd,mod_prx_config *s_cfg,char *cookies,int port)
{
	if(req==NULL)return -1;
	if(r==NULL)return -1;
	if(id==NULL)return -1;
	if(sd==0)return -1;
	if(s_cfg==NULL)return -1;
	if(cookies==NULL)return -1;
	if(port==0)return -1;
//создания буфера для хранения данных полученных от клиента
	char *	messagePOST=(char *)calloc(2,sizeof(char));
//длина полученных данных
	size_t	lenforPOST=0;
	if(messagePOST==NULL)
	{
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: can't get data from post");//запись логов
		return -1;
	};
//получение данных от сервера(apache)
	{
		apr_bucket_brigade *bb;
		apr_status_t rv;
//создание указателя на внутренний буфер
		bb = apr_brigade_create(r->pool, r->connection->bucket_alloc);
		apr_size_t lendop=0; //переменная используеться только во время получения данных
		int seen_eos = 0;//конец потока
//??
		my_bincat(&messagePOST,lendop,"&",1);//глюк :->>
		lendop=lendop+1;
//??
		do {
			apr_bucket *bucket;
			rv = ap_get_brigade(r->input_filters,
			    bb, AP_MODE_READBYTES, APR_BLOCK_READ,HUGE_STRING_LEN);
			if (rv != APR_SUCCESS) 
				{
				break;
				}
#ifdef	APR_BRIGADE_FOREACH
			APR_BRIGADE_FOREACH(bucket, bb) 
#else
			for(bucket = APR_BRIGADE_FIRST(bb); bucket != APR_BRIGADE_SENTINEL(bb); bucket = APR_BUCKET_NEXT(bucket))
#warning This code consist error www.mail-archive.com/dev@httpd.apache.org/msg34917.html 
#endif
				{
//не окончание ли
				if (APR_BUCKET_IS_EOS(bucket)) 
					{
					seen_eos = 1;
					break;
					}
//пустой пакет				
				if (APR_BUCKET_IS_FLUSH(bucket)) 
					{
					continue;
					}
//времнные переменные для полученния данных от сервера
				apr_size_t len; //for send
				const char *data;
//получаем указатели на реальные данные
				apr_bucket_read(bucket,&data,
						&len,APR_BLOCK_READ);
//а вдруг пустые
				if(data==NULL)
					{
					ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
				     		"mod_prx: can't get data from post");//запись логов
					free(messagePOST);
					return -1;
					};
				if(messagePOST==NULL)
					{
					ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
						"mod_prx: can't alloc");//запись логов
					return -1;
					};
//обьеденение данных
				my_bincat(&messagePOST,lendop,data,len);//FIXME it not binary
				lendop=len+lendop;
				
				if(messagePOST==NULL)
					{
					ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
					     "mod_prx: can't alloc");//запись логов
					return -1;
					};
				}
//очистка пакета ( он нам более не нужен)
			apr_brigade_cleanup(bb);
//а не конец ли данных
		} while (!seen_eos);
//превод в реальный размер из-за глюка
	lenforPOST=lendop-1;//big gluc
	};
//создание из шаблона запроса к серверу
	size_t size=snprintf(NULL,0,simplereqPOST,
		req,cookies,s_cfg->DESTINATIONIP,lenforPOST);
	char *message=(char*)calloc(size+1,sizeof(char));
	if(message==NULL)
		{
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: can't alloc");//запись логов
		close(sd);
		free(message);
		return -1;
		};
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		"mod_prx: req_id=%s step=5",id);//запись логов
	snprintf(message,size+1,simplereqPOST,req,cookies,s_cfg->DESTINATIONIP,lenforPOST);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		"mod_prx: req_id=%s step=6",id);//запись логов
//передача запроса
	if (send(sd,message,strlen(message),0)== -1) 
		{
		free(message);
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0,
		r->server,"\t*module (%s)can  not send to port=%d*",
			s_cfg->DESTINATIONIP,port);
		//сервер очень занят наверное
		return -1;
		}
//передача данных запроса (+1 из-за глюка :->>)
	send(sd,messagePOST+1,lenforPOST,0);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0,
		r->server,"mod_prx: req_id=%s step=7",id);
	free(messagePOST);
//ну вот и передали запрос
	return 0;
}

//метод отвечающий за работу запроса GET
static int metodGET(char * req,request_rec *r,char * id,int sd,mod_prx_config *s_cfg,char *cookies,int port)
{
	if(req==NULL)return -1;
	if(r==NULL)return -1;
	if(id==NULL)return -1;
	if(sd==0)return -1;
	if(s_cfg==NULL)return -1;
	if(cookies==NULL)return -1;
	if(port==0)return -1;
//создаем запрос
	size_t size=snprintf(NULL,0,simplereqGET,req,cookies,s_cfg->DESTINATIONIP,port);
	char *message=(char*)calloc(size+1,sizeof(char));
	if(message==NULL)
	{
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: can't alloc");//запись логов
		close(sd);
		free(message);
		return -1;
	};
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		"mod_prx: req_id=%s step=5",id);
//запись логов
	snprintf(message,size+1,simplereqGET,req,cookies,s_cfg->DESTINATIONIP,port);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		"mod_prx: req_id=%s step=6",id);//запись логов
//передача запроса
	if (send(sd,message,strlen(message),0)== -1) 
	{
		free(message);
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
			"*module (%s)can  not send to port=%d*",
			s_cfg->DESTINATIONIP,port);
//сервер очень занят наверное
		return -1;
	}
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
			"mod_prx: req_id=%s step=7",id);
	free(message);
//ну вот и передали запрос
	return 0;
}
//полученеие сохраненного кукиеса
static	int	get_save_cookies(
	struct str_cookies **struct_for_cookies,
	char *name_of_file,
	int user_id,int max_count_cookies
)
{
int res;//количество байт
if(struct_for_cookies==NULL)return -1;
if(name_of_file==NULL)return -1;
if(user_id>max_count_cookies)
	user_id=0;
//индефикатор файла содержащего кукиес
	int file_cookies;
//буфер для кукиеса
	(*struct_for_cookies)=(struct str_cookies *)calloc(1,sizeof(struct str_cookies));
	if(struct_for_cookies==NULL)return -1;
//обнуление буфера
	memset((*struct_for_cookies)->data,0,COOKIES_SIZE);
	(*struct_for_cookies)->count=0;
	file_cookies=open(name_of_file,O_RDONLY);
	if(file_cookies>-1)
	{
		lock_R(file_cookies);
		lseek(file_cookies,user_id*sizeof(struct str_cookies),SEEK_SET);
		{
			res=read(file_cookies,(*struct_for_cookies),sizeof(struct str_cookies));
		}while(errno==EINTR&&res==-1);
		unlock(file_cookies);
		close(file_cookies);	
	}
	else
	{
		memset((*struct_for_cookies)->data,'0',COOKIES_SIZE-2);//FIXME not for all corectly
		file_cookies=open(name_of_file,O_RDWR|O_CREAT|O_TRUNC,S_IRWXU);
		if(file_cookies>-1)
			{
			lock_W(file_cookies);
			int count_cookies=0;
			for(;count_cookies<max_count_cookies;++count_cookies)
			{
				{
					res=write(file_cookies,*struct_for_cookies,sizeof(struct str_cookies));
				}while(errno==EINTR&&res==-1);
			};
			//в n раз больше( хранить в файле не один кукиес,а несколько)
			unlock(file_cookies);
			close(file_cookies);
		};
		return 1;
	};
	return 0;
}
//сохранение кукиеса
static	int	save_cookies(
		const struct str_cookies * struct_for_cookies,
		const char *name_of_file,
		int user_id,int max_count_cookies
)
{
int res;//количество байт
	if(struct_for_cookies==NULL)return -1;
	if(name_of_file==NULL)return -1;
	if(user_id>max_count_cookies)
		user_id=0;
//индефикатор файла содержащего кукиес
	int file_cookies;
	file_cookies=open(name_of_file,O_RDWR|O_CREAT,S_IRWXU);
	if(file_cookies>-1)
		{
		lock_W(file_cookies);
		lseek(file_cookies,user_id*sizeof(struct str_cookies),SEEK_SET);
		{
			res=write(file_cookies,struct_for_cookies,sizeof(struct str_cookies));
		}while(errno==EINTR&&res==-1);
		unlock(file_cookies);
		close(file_cookies);
		}
		else
		{
			return 1;
		};
	return 0;
}
//обработка запроса
static int	ReQ(char * req,request_rec *r,char * id,int user_id)
{
	if(req==NULL)return 0;
	if(id==NULL)return 0;
	if(r==NULL)return 0;
//буфер для кукиеса
	struct str_cookies *	cookies=NULL;
// данные подключения
	struct sockaddr_in addr; 
//сокет
	int sd;
//результат работы функций
	int res=0;
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		"mod_prx: req_id=%s step=1",id);
		//запись логов
//получение настроек модуля
	mod_prx_config *s_cfg =
		(mod_prx_config *)ap_get_module_config
		(r->server->module_config, &prx_module);
	if((void*)s_cfg==NULL)	return -1;
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		"mod_prx: req_id=%s step=2",id);//запись логов
//получение сохраненного cookies
	res=get_save_cookies(&cookies,s_cfg->SIDSAVEPATH,user_id,s_cfg->CookiesCount);

//для проверки работы записи save_cookies(cookies,s_cfg->SIDSAVEPATH,2,s_cfg->CookiesCount);

	if(res>0)
		ap_log_error(APLOG_MARK, LOG_INFO, 0,
			r->server," file not found=%s result=%d",s_cfg->SIDSAVEPATH,res);
	if(res<0)
		ap_log_error(APLOG_MARK, LOG_INFO, 0,
			r->server," unknow error in get cookies = %s ",s_cfg->SIDSAVEPATH);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0,
		 r->server,"mod_prx: req_id=%s step=3 read cookies = %s",id,cookies->data);
		//запись логов
	
//соединение с вторым сервером
	addr.sin_port = htons((unsigned short) s_cfg->DESTINATIONPORT);// порт подключения
	addr.sin_family = AF_INET;// тип адреса
	addr.sin_addr.s_addr = inet_addr(s_cfg->DESTINATIONIP);
	// создаем сокет
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
		return -1;
	// подключаемся к серверу
	if (connect(sd, (struct sockaddr *)(&addr), sizeof(addr)) == -1) 
	{
		ap_log_error(APLOG_MARK, APLOG_ERR, 0,
		r->server,"mod_prx: *server(%s) is very busy port=%d*",
		s_cfg->DESTINATIONIP,s_cfg->DESTINATIONPORT);
		//сервер очень занят наверное
			return -1;
	};
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,"mod_prx: req_id=%s cookies=%s step=4",id,cookies->data);//запись логов
	{
//генерим запрос
	if(r->method_number==M_POST)//NOT GET(POST!)
		{
		if(metodPOST(req,r,id,sd,s_cfg,cookies->data,s_cfg->DESTINATIONPORT)<0)return -1;
		}else
		if(r->method_number==M_GET)
			{
			if(metodGET(req,r,id,sd,s_cfg,cookies->data,
					s_cfg->DESTINATIONPORT)<0)
					return -1;
		}else
			{
			ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
			     "mod_prx: req_id=%s step=No GET or No POST",id);//запись логов
			return -1;
			}
		
	};
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		"mod_prx: req_id=%s step=8",id);//запись логов
//выделение гигантского буфера для получения ответа от сервера	
	char *resultbuffer=(char*)calloc(SIZE_BIG_BUFFER,sizeof(char));
	if(resultbuffer==NULL)
		{
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: can't alloc");//запись логов
		close(sd);
		free(resultbuffer);
		return -1;
		};
//вычитываем первую порцию данных
	size_t pos_last=0;
	{
		size_t _res=0;
//читаем результат запроса
		while((SIZE_BIG_BUFFER-1>pos_last)&&((_res=recv(sd,resultbuffer+pos_last,SIZE_BIG_BUFFER-1-pos_last,0))>0))
		{	
			pos_last=pos_last+_res;	
			resultbuffer[pos_last+1]=0;
		};
		pos_last++;
		resultbuffer[pos_last]=0;
	};
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0,
		 r->server,"mod_prx: req_id=%s step=9",id);//запись логов
//делим на заголовок и результат(+4 так как это последний \n
	char *body=strstr(resultbuffer,END_OF_HEAD)+strlen(END_OF_HEAD);
	if(body==NULL)
	{
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,"body not found");
		close(sd);
		free(resultbuffer);
		return -1;
	};
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0,
		r->server,"mod_prx: req_id=%s step=10",id);//запись логов
//получение типа conten_type
	char *content=NULL;
	content=strstr(resultbuffer,CONTENT_TYPE);//поиск типа 
	if(content!=NULL)
		{
		content=content+strlen(CONTENT_TYPE);
		size_t len_content=strprxn(content,EXCLUDECHAR);
		char *ContentBuffer=(char*)calloc(len_content+1,sizeof(char));
		if(ContentBuffer==NULL)
		{
			ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
			     "mod_prx: can't alloc");//запись логов
			close(sd);
			free(resultbuffer);
			return -1;
		};		
		strncpy(ContentBuffer,content,len_content);	
		ContentBuffer=apr_pstrdup(r->pool, ContentBuffer);
		ap_set_content_type(r,ContentBuffer);//conten set only work
		}
	else
		{
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
			"mod_prx: %s not found content-type",id);//запись логов
//content type по умолчанию
		ap_set_content_type(r,apr_pstrdup(r->pool,"text/html; charset=utf-8"));
		};
	r->no_cache=1;
	r->no_local_copy=1;
	apr_table_set(r->headers_out,"PRAGMA","no-cache");
	apr_table_set(r->headers_out,"cache-CONTROL","no-cache");
//передача результата клиенту
	ap_rwrite(body,pos_last+(resultbuffer-body)-1,r);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,"mod_prx: req_id=%s step=11",id);
//ищем кукиес
	char *cookie=strstr(resultbuffer,producer_COOKIES);
	if(cookie==NULL)
	{
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
				"mod_prx: PRXSESIONID not found");
	}
	else
	{
//cookies для логов и сохранения
		size_t lencookies=strprxn(cookie,EXCLUDECHAR);
		if(lencookies<COOKIES_SIZE-2)
			ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
				"mod_prx: req_id=%s error in write cookie size=%d!",id,lencookies);
		char *logcookie=(char * )calloc(lencookies+1,sizeof(char));
		if(logcookie!=NULL)
			{
//выделяем кукиес из заголовков
			strncpy(logcookie,cookie,lencookies);
			ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
				"mod_prx: req_id=%s %s",id,logcookie);
			ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
				"mod_prx: req_id=%s step=12",id);
//запись логов
			if(lencookies<COOKIES_SIZE-2)
			{
				ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
				     "mod_prx: error in size CooKie");//запись логов
				close(sd);
				free(resultbuffer);
				return -1;
			}
			ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
				"mod_prx: req_id=%s step=13",id);
				//запись логов
//узнаем изменился ли кукиес (не совсем коректно так как длина кукиеса может отличаться от COOKIES_SIZE-2 но передаваемый кукиес от producer именно такой длинны (всегда!)
			if(strncmp(cookies->data,logcookie,COOKIES_SIZE-2)!=0)
			{
				ap_log_error(APLOG_MARK, APLOG_DEBUG, 0,r->server,
					"mod_prx: change cookies for id=%s from %s to %s",
					id,cookies->data,logcookie);//запись логов
				struct str_cookies *	cookie_save=(struct str_cookies *)calloc(sizeof(struct str_cookies),1);
//скопировать в структуру записи кукиеса новый кукиес
				strncpy(cookie_save->data,logcookie,COOKIES_SIZE-2);
//увеличить счетчик количества изменений кукиеса
				cookie_save->count=cookies->count+1;
				if(save_cookies(cookie_save,s_cfg->SIDSAVEPATH,user_id,s_cfg->CookiesCount)!=0)
					ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
					     "mod_prx: I cann't write file=%s  ",
					     s_cfg->SIDSAVEPATH);//запись логов
				else
					ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
					     "mod_prx: cookies write ok  "
					     );//запись логов
				free(cookie_save);
			};
			free(logcookie);
		}else
		{
			ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
				"mod_prx: req_id=%s error in write cookie size=%d",id,lencookies);
		}
	};
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,"mod_prx: req_id=%s step=14",id);//запись логов
//проверка все ли вычитали если нет дочитываем
	if(pos_last==SIZE_BIG_BUFFER)
	{
		size_t _res=0;
		memset(resultbuffer,0,SIZE_BIG_BUFFER);
		while((_res=recv(sd,resultbuffer,SIZE_BIG_BUFFER-1,0))>0)
		{	
			ap_rwrite(resultbuffer,_res,r);
			memset(resultbuffer,0,SIZE_BIG_BUFFER);
		};
	};
	
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,"mod_prx: req_id=%s step=15",id);//запись логов
//закрыть все и освободить буфер	
	close(sd);
	free(resultbuffer);
	return 0;
}
//базовый хук обрабатывает запросы
static int mod_prx_method_handler (request_rec *r)
{
//номер разширения
	int extnumber;
	if(r->unparsed_uri==NULL)		
		return DECLINED;
//получение конфигурации.
	mod_prx_config *s_cfg =(mod_prx_config *)ap_get_module_config(r->server->module_config, &prx_module);
	if((void*)s_cfg==NULL)	return DECLINED;

//проверить длины
	for(extnumber=0;extnumber<MaxExt;++extnumber)
	{	
		if((s_cfg->EXT_ORIG)[extnumber]!=NULL)
		if((s_cfg->EXT_EXCH)[extnumber]!=NULL)
		{
			ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
				     "mod_prx: know extension %s-> %s",(char*)s_cfg->EXT_EXCH[extnumber],(char*)s_cfg->EXT_ORIG[extnumber]);
			if(strlen(r->unparsed_uri)<strlen((char*)(s_cfg->EXT_EXCH)[extnumber])+2)//2=*+.
				return DECLINED;		
				if(strlen((char*)(s_cfg->EXT_ORIG)[extnumber])!=strlen((char*)(s_cfg->EXT_EXCH)[extnumber]))
			{
				ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
				     "mod_prx: Size extenxion no equal");
				return DECLINED;
			};
		};
	};


//выделение буфера под запрос ( указан  SIZE_BIG_BUFFER так теоретически это ускорит перераспределение памяти 
	char *message=(char*)calloc(SIZE_BIG_BUFFER,sizeof(char));
	if(message==NULL)
		{
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: can't alloc");//запись логов
		free(message);
		return DECLINED;
		};
	char *res=NULL;
//копируем в буфер запрос
	my_strcat(&message,r->unparsed_uri);
	if(message==NULL)
		{
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: can't alloc");//запись логов
		free(message);
		return DECLINED;
		};

//ищем расширение
//более интеллектуальный поиск расширения с конца
	res=message+strprxn(message,"?&");
	if (res!=NULL)
	{
		for(extnumber=0;extnumber<MaxExt;++extnumber)
		{
		if((s_cfg->EXT_EXCH)[extnumber]!=NULL)
			if (*(res-strlen((char*)((s_cfg->EXT_ORIG)[extnumber]))-1)=='.')
				if((s_cfg->EXT_ORIG)[extnumber]!=NULL)
				if (strncmp(
					res-strlen((char*)((s_cfg->EXT_ORIG)[extnumber])),
					(char*)((s_cfg->EXT_ORIG)[extnumber]),
					strlen((char*)((s_cfg->EXT_ORIG)[extnumber]))
					)==0) 
				{
					ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     					"mod_prx: extension %s found",(char*)((s_cfg->EXT_ORIG)[extnumber]));//запись логов
					break;
				};
		};
		if(extnumber==MaxExt)
			{
				free(message);
			ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: can't find extension");//запись логов
				return DECLINED;
			}
			
	}
	else
	{
		free(message);
		return DECLINED;
	};

//заменяем в запросе расширение
	res=res-strlen((char*)(s_cfg->EXT_EXCH[extnumber]))-1;
	memcpy((char*)res + 1,(char*)(s_cfg->EXT_EXCH[extnumber]),strlen((char*)(s_cfg->EXT_EXCH[extnumber])));

	char url[50]="";
//копия запроса для логов
	strncpy(url,message,49);
//создание уникального индефикатора запроса
	char *unical=create_unique_id();
	ap_log_error(APLOG_MARK, LOG_INFO, 0, r->server,"mod_prx: req_id=%s Loaded '%s' version",unical,VERSION);
//добавлять идентификатор в запрос
	if(strncmp(message,INCLUDE_PATH,strlen(INCLUDE_PATH))==0)
	{
//есть ли параметр id
		if((strstr(message,"&id=")!=NULL)|(strstr(message,"?id=")!=NULL))	
		{
			ap_log_error(APLOG_MARK, LOG_INFO, 0, r->server,
			     "mod_prx: url=(%s...) found 'id'",
			     url
			    );//запись логов
			free(message);
			free(unical);
			return DECLINED;
		}else
//определяем есть ли в запросе параметры чтобы добавить в них свой случайный id запроса
		if(*(res+strlen((char*)(s_cfg->EXT_EXCH[extnumber]))+1)=='?')
		{
			my_strcat(&message,"&id=");
		}else
		{
			my_strcat(&message,"?id=");
		};
		my_strcat(&message,unical);
	}
	ap_log_error(APLOG_MARK, LOG_INFO, 0, r->server,
		     "mod_prx: req_id=%s url=(%s...) metod=(%d) metod_name=(%s)",
		     unical,url,r->method_number,r->method
		    );//запись логов
	if(message==NULL)
			{
				ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
				     "mod_prx: can't alloc");//запись логов
				free(message);
				return DECLINED;
			};
//работа с семофором ограничивающим количество запросов
	if(s_cfg->SemCount<1)
		{
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: error in count of proces");
		s_cfg->SemCount=1;	
		}else
		{
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
			     "mod_prx: req_id=%s MAX_Concurrency = %d",unical,s_cfg->SemCount);
		};
//непосредственно создание семофора или получение к нему доступа
	sem_t *sem_dostup=sem_open(s_cfg->SemName,O_CREAT,S_IRWXU,s_cfg->SemCount);
	if(sem_dostup==SEM_FAILED)
	{
		ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
	     "mod_prx: semaphore error name = %s errno= %d name=%s",s_cfg->SemName,errno,strerror(errno));//запись логов
		free(unical);
		free(message);
		return DECLINED;
	};
	sem_wait(sem_dostup);
///semaphore
	int concurrency=0;
	if(sem_getvalue(sem_dostup,&concurrency)==-1)
		{		
			ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
			     "mod_prx: can't getvalue of semaphore");//запись логов
			free(message);
			return DECLINED;
		}
//создание идентификатора пользователя
	int user_id=concurrency%s_cfg->CookiesCount;
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		"mod_prx: req_id=%s use user(kookies) = %d",unical,user_id);
	if(ReQ(message,r,unical,user_id)<0)
	{
//если не удачно то красиво выйдем
		sem_post(sem_dostup);
		free(unical);
		free(message);
		return DECLINED;
	};
	
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		"mod_prx: req_id=%s concurrency = %d",
		unical,s_cfg->SemCount-concurrency);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		"mod_prx: req_id=%s wait = %d",
		unical,-concurrency);
//semaphore
//вывод отладочной информации
	sem_post(sem_dostup);
	sem_close(sem_dostup);
	out_debug(r->headers_in,r,	unical,"headers_in");
	out_debug(r->headers_out,r,	unical,"headers_out");
	out_debug(r->err_headers_out,r,	unical,"err_headers_out");
	out_debug(r->subprocess_env,r,	unical,"subprocess_env");
	out_debug(r->notes,r,		unical,"notes");
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s args = %s",unical,r->args);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s hostname = %s",unical,r->hostname);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s the_request = %s",unical,r->the_request);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s content_type = %s",unical,r->content_type);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s method = %s",unical,r->method);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s status_line = %s",unical,r->status_line);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s protocol = %s",unical,r->protocol);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s assbackwards= %d",unical,r->assbackwards);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s proxyreq = %d",unical,r->proxyreq);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s header_only = %d",unical,r->header_only);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s proto_num = %d",unical,r->proto_num);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s status = %d",unical,r->status);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		     "mod_prx: req_id=%s method_number = %d",unical,r->method_number);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		    	"mod_prx: req_id=%s allowed = %d",unical,(int)r->allowed);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
			"mod_prx: req_id=%s bytes_sent = %d",unical,(int)r->bytes_sent);
	ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
		    	"mod_prx: req_id=%s remote_host = %s",
			unical,ap_get_remote_host(r->connection,
			r->per_dir_config, REMOTE_NAME,NULL));
///semaphore
	free(unical);
	free(message);

//return all ok
	return OK;
}

//функция регистрации хуков
static void mod_prx_register_hooks (apr_pool_t *p)
{
	ap_hook_handler(mod_prx_method_handler, NULL, NULL, APR_HOOK_LAST);
}
//вызывается при обработке соответствующего поля конфигурации
static const char *set_modprx_DESTINATIONPORT(cmd_parms *parms, void *mconfig, const char *arg)
{
	mod_prx_config *s_cfg =(mod_prx_config *)ap_get_module_config(parms->server->module_config, &prx_module);
	if((void*)s_cfg==NULL)	return NULL;
	// дублируем конфигурацию в внутреннюю структуру модуля
	s_cfg->DESTINATIONPORT =strtol( (char *) arg,NULL,10);
	// удачно
	return NULL;
}
//вызывается при обработке соответствующего поля конфигурации
static const char *set_modprx_DESTINATIONIP(cmd_parms *parms, void *mconfig, const char *arg)
{
	mod_prx_config *s_cfg =(mod_prx_config *)ap_get_module_config(parms->server->module_config, &prx_module);
	if((void*)s_cfg==NULL)	return NULL;
	// дублируем конфигурацию в внутреннюю структуру модуля
	s_cfg->DESTINATIONIP = (char *) arg;
	// удачно
	return NULL;
}
//вызывается при обработке соответствующего поля конфигурации
static const char *set_modprx_SemName(cmd_parms *parms, void *mconfig, const char *arg)
{
	mod_prx_config *s_cfg =(mod_prx_config *)ap_get_module_config(parms->server->module_config, &prx_module);
	if((void*)s_cfg==NULL)	return NULL;
	// дублируем конфигурацию в внутреннюю структуру модуля
	s_cfg->SemName = (char *) arg;
	// удачно
	return NULL;
}
//вызывается при обработке соответствующего поля конфигурации
static const char *set_modprx_SIDSAVEPATH(cmd_parms *parms, void *mconfig, const char *arg)
{
	mod_prx_config *s_cfg =(mod_prx_config *)ap_get_module_config(parms->server->module_config, &prx_module);
	if((void*)s_cfg==NULL)	return NULL;
	// дублируем конфигурацию в внутреннюю структуру модуля
	s_cfg->SIDSAVEPATH = (char *) arg;
	// удачно
	return NULL;
}
//вызывается при обработке соответствующего поля конфигурации
static const char *set_modprx_EXT_EXCH(cmd_parms *parms, void *mconfig, const char *arg)
{
int i;
	mod_prx_config *s_cfg =(mod_prx_config *)ap_get_module_config(parms->server->module_config, &prx_module);
	if((void*)s_cfg==NULL)	return NULL;
	// дублируем конфигурацию в внутреннюю структуру модуля
	for(i=0;i<MaxExt;++i)
	{
		int temp=strprxn(arg,";");
		if(temp>MAXSTRING-1)
			temp=MAXSTRING-1;
		strncpy((char*)(s_cfg->EXT_EXCH[i]),arg,temp);
		if(*(arg+temp)!=0)
		{
			arg+=temp;
			arg++;
		}
		else 
			break;
	}
	// удачно
	return NULL;
}
//вызывается при обработке соответствующего поля конфигурации
static const char *set_modprx_SemCount(cmd_parms *parms, void *mconfig, const char *arg)
{
	mod_prx_config *s_cfg =(mod_prx_config *)ap_get_module_config(parms->server->module_config, &prx_module);
	if((void*)s_cfg==NULL)	return NULL;
	// дублируем конфигурацию в внутреннюю структуру модуля
	s_cfg->SemCount = strtol( (char *) arg,NULL,10);
	// удачно
	return NULL;
}
//вызывается при обработке соответствующего поля конфигурации
static const char *set_modprx_EXT_ORIG(cmd_parms *parms, void *mconfig, const char *arg)
{
int i;
	mod_prx_config *s_cfg =(mod_prx_config *)ap_get_module_config(parms->server->module_config, &prx_module);
	if((void*)s_cfg==NULL)	return NULL;
	// дублируем конфигурацию в внутреннюю структуру модуля
	for(i=0;i<MaxExt;++i)
	{
		int temp=strprxn(arg,";");
		if(temp>MAXSTRING-1)
			temp=MAXSTRING-1;
		strncpy((char*)(s_cfg->EXT_ORIG[i]),arg,temp);
		if(*(arg+temp)!=0)
		{
			arg+=temp;
			arg++;
		}
		else 
			break;
	}
	// удачно
	return NULL;
}
static const char *set_modprx_CookiesCount(cmd_parms *parms, void *mconfig, const char *arg)
{
	mod_prx_config *s_cfg =(mod_prx_config *)ap_get_module_config(parms->server->module_config, &prx_module);
	if((void*)s_cfg==NULL)	return NULL;
	// дублируем конфигурацию в внутреннюю структуру модуля
	s_cfg->CookiesCount = strtol( (char *) arg,NULL,10);
	// удачно
	return NULL;
}
//создание конфигурацию по умолчанию
static void *create_modprx_config(apr_pool_t *pool, server_rec *s)
{
mod_prx_config *newcfg=NULL;
//выделение памяти под кофигурацию
	newcfg = (mod_prx_config *) apr_pcalloc(pool, sizeof(mod_prx_config));
	if((void*)newcfg==NULL)	return NULL;
//установка значений по умолчанию
	newcfg->SIDSAVEPATH 		= DEFAULT_MODPRX_SIDSAVEPATH;
	newcfg->SemName 		= DEFAULT_MODPRX_SemName;
	newcfg->DESTINATIONPORT 	= DEFAULT_MODPRX_DestinationPort;
	newcfg->DESTINATIONIP 	= DEFAULT_MODPRX_DestinationIP;
	strncpy((char*)((newcfg->EXT_EXCH))[0],DEFAULT_MODPRX_EXT_EXCH,MAXSTRING);
	strncpy((char*)((newcfg->EXT_ORIG)[0]),DEFAULT_MODPRX_EXT_ORIG,MAXSTRING);
	newcfg->SemCount		= DEFAULT_Semaphore_count;
	newcfg->CookiesCount		= DEFAULT_MODPRX_MAXCOOKIES;
//вернуть новую конфигурацию
	return (void *) newcfg;
}

