/*
 * handrecog_lucykila.c - my first attempt to write my own online handwrite recognition engine
 *
 *  Created on: 2010-2-7
 *      Author: cai
 */

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <glib.h>

#include "engine.h"
#include "handrecog.h"

typedef struct _IbusHandwriteRecogLucyKila IbusHandwriteRecogLucyKila;
typedef struct _IbusHandwriteRecogLucyKilaClass IbusHandwriteRecogLucyKilaClass;

GType ibus_handwrite_recog_lucykila_get_type(void);

#define G_TYPE_IBUS_HANDWRITE_RECOG_LUCYKILA (ibus_handwrite_recog_lucykila_get_type())

#define IBUS_HANDWRITE_RECOG_LUCYKILA_GET_CLASS(obj) \
		G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_IBUS_HANDWRITE_RECOG_LUCYKILA, IbusHandwriteRecogLucyKilaClass)
#define IBUS_HANDWRITE_RECOG_LUCYKILA(obj) \
		G_TYPE_CHECK_INSTANCE_CAST(obj,G_TYPE_IBUS_HANDWRITE_RECOG_LUCYKILA,IbusHandwriteRecogLucyKila)


struct _IbusHandwriteRecogLucyKila{
	IbusHandwriteRecog	parent;
	GString		*		input; // 由笔画构成的，用来查笔画表的字符串
	void		* 		start_ptr; //指向表的地址
	size_t				items_count; //表项数
	size_t				table_size; // 表大小
	size_t				maped_size; // 分配的内存大小
};

struct _IbusHandwriteRecogLucyKilaClass{
	IbusHandwriteRecogClass parent;
	void (* parentdestroy)(IbusHandwriteRecog *object);
};

static void ibus_handwrite_recog_lucykila_init(IbusHandwriteRecogLucyKila*obj);
static void ibus_handwrite_recog_lucykila_class_init(IbusHandwriteRecogLucyKilaClass* klass);


static char * nextline(char * ptr);

static int lucykila_open_table(IbusHandwriteRecogLucyKila*obj, int way, ...)
{
	struct stat state;
	char * ptr;
	const int max_length = 64; // 绝对够的，不够你找偶
	char *preserve, *ptr2;
	char * p;
	int preservesize;

	puts(__func__);

	//打开表
	int f = open(tablefile, O_RDONLY);
	if (f < 0)
		return -1;

	fstat(f, &state);
	//映射进来
	preserve = ptr = (char*) mmap(0, state.st_size, PROT_WRITE | PROT_READ,
			MAP_PRIVATE, f, 0);
	if (!ptr)
	{
		close(f);
		return -1;
	}
	close(f);
	//优化数据文件，其实就是使得每一行都一样长

	preservesize = 1024 * 1024;

	//预先申请 1 M 内存，不够了再说
	ptr2 = obj->start_ptr = mmap(0, preservesize, PROT_WRITE | PROT_READ,
			MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	if (!preserve)
	{
		munmap(preserve, state.st_size);
		return -1;
	}

	obj->items_count = 0;

	//进入循环，一行一行的扫描 :)
	while ((ptr = nextline(ptr)) && ((ptr - preserve) < state.st_size))
	{
		memcpy(ptr2, ptr, 64); //直接拷贝过去就可以了
		nextline(ptr2)[-1] = 0;
		ptr2 += 64;
		obj->items_count++;
	}

	munmap(preserve, state.st_size);

	obj->maped_size = preservesize;
	obj->table_size = ptr2 - preserve;
	return 0;
}


static gboolean ibus_handwrite_recog_lucykila_domatch(IbusHandwriteRecog*obj,int want)
{
	return TRUE;
}

static void ibus_handwrite_recog_lucykila_init(IbusHandwriteRecogLucyKila*obj)
{
	obj->input = g_string_new("");
	obj->start_ptr; //指向表的地址
	obj->items_count = 0; //表项数
	obj->table_size = 0; // 表大小
	obj->maped_size = 0; // 分配的内存大小
}

static void ibus_handwrite_recog_lucykila_destory(IbusHandwriteRecog*obj)
{
	IbusHandwriteRecogLucyKila * thisobj = IBUS_HANDWRITE_RECOG_LUCYKILA(obj);
	g_string_free(thisobj->input,TRUE);
	munmap(thisobj->start_ptr,thisobj->maped_size);

	IBUS_HANDWRITE_RECOG_LUCYKILA_GET_CLASS(obj)->parentdestroy(obj);
}

static void ibus_handwrite_recog_lucykila_class_init(IbusHandwriteRecogLucyKilaClass* klass)
{
	IbusHandwriteRecogClass * parent = (IbusHandwriteRecogClass*)(klass);
	parent->load_table = (int (*)(IbusHandwriteRecog*, int way, ...)) lucykila_open_table;
	klass->parentdestroy = parent->destroy;
	parent->destroy = ibus_handwrite_recog_lucykila_destory;
	parent->domatch =ibus_handwrite_recog_lucykila_domatch;
}

GType ibus_handwrite_recog_lucykila_get_type(void)
{
	static const GTypeInfo type_info =
	{ sizeof(IbusHandwriteRecogLucyKilaClass), (GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) ibus_handwrite_recog_lucykila_class_init, NULL, NULL,
			sizeof(IbusHandwriteRecogLucyKila), 0,
			(GInstanceInitFunc) ibus_handwrite_recog_lucykila_init, };

	static GType type = 0;

	if (type == 0)
	{
		type = g_type_register_static(G_TYPE_IBUS_HANDWRITE_RECOG, "IbusHandwriteRecog_LucyKila",
				&type_info, 0);

	}
	return type;
}

static char *
nextline(char * ptr)
{
  while (*ptr != '\n')
    ++ptr;
//  *ptr = 0;
  return *ptr?++ptr:NULL;
}
