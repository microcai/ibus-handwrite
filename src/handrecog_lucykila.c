/*
 * handrecog_lucykila.c - my first attempt to write my own online handwrite recognition engine
 *
 *  Created on: 2010-2-7
 *      Author: cai
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "engine.h"
#include "handrecog.h"
#include "global_var.h"

typedef struct _MATCHED MATCHED;
typedef struct _IbusHandwriteRecogLucyKila IbusHandwriteRecogLucyKila;
typedef struct _IbusHandwriteRecogLucyKilaClass IbusHandwriteRecogLucyKilaClass;

GType ibus_handwrite_recog_lucykila_get_type(void);


#define IBUS_HANDWRITE_RECOG_LUCYKILA_GET_CLASS(obj) \
		G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_IBUS_HANDWRITE_RECOG_LUCYKILA, IbusHandwriteRecogLucyKilaClass)
#define IBUS_HANDWRITE_RECOG_LUCYKILA(obj) \
		G_TYPE_CHECK_INSTANCE_CAST(obj,G_TYPE_IBUS_HANDWRITE_RECOG_LUCYKILA,IbusHandwriteRecogLucyKila)

struct _MATCHED{
  char  code[64-16];
  char  hanzi[16];
};

struct _IbusHandwriteRecogLucyKila
{
	IbusHandwriteRecog parent;
	GString * input; // 由笔画构成的，用来查笔画表的字符串
	void * start_ptr; //指向表的地址
	size_t items_count; //表项数
	size_t table_size; // 表大小
	size_t maped_size; // 分配的内存大小
};

struct _IbusHandwriteRecogLucyKilaClass
{
	IbusHandwriteRecogClass parent;
	void (* parentdestroy)(GObject *object);
};

static void ibus_handwrite_recog_lucykila_init(IbusHandwriteRecogLucyKila*obj);
static void ibus_handwrite_recog_lucykila_class_init(
		IbusHandwriteRecogLucyKilaClass* klass);

static char * nextline(char * ptr);
static gint mysort(gconstpointer a, gconstpointer b);

static int lucykila_open_table(IbusHandwriteRecogLucyKila*obj)
{
	struct stat state;
	char * ptr;
	const int max_length = 64; // 绝对够的，不够你找偶
	char *preserve, *ptr2;
	char * p;
	int preservesize;

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
	obj->table_size = obj->items_count*64;
	return 0;
}

void ibus_handwrite_recog_change_stroke(IbusHandwriteRecog* obj)
{
	GdkPoint startpoint;
	GdkPoint endpoint;

	IbusHandwriteRecogLucyKila * me;
	int i;

	me = IBUS_HANDWRITE_RECOG_LUCYKILA(obj);

	if(obj->strokes->len == 0)
	{
		me->input= g_string_truncate(me->input,0);
		return ;
	}

	LineStroke laststrok =	g_array_index(obj->strokes,LineStroke,obj->strokes->len-1);

	startpoint = laststrok.points[0];

	endpoint = laststrok.points[laststrok.segments - 1];

	//检测输入的笔画，h ? s ? p? z ? n?

	//有米有折点
	cairo_rectangle_int_t ret =
	{
			MIN(startpoint.x,endpoint.x), MIN(endpoint.y,startpoint.y),
			fabs(endpoint.x - startpoint.x), fabs(endpoint.y - startpoint.y)
	};


	if( abs(startpoint.x - endpoint.x ) > 17 && abs(startpoint.y - endpoint.y ) > 7  )
	{
		int init=0;

		printf("is z!!!?\n");
		cairo_region_t * rg = cairo_region_create_rectangle(&ret);
		for(i=1;i < laststrok.segments -1 ;++i)
		{
			if(!cairo_region_contains_point(rg,laststrok.points[i].x,laststrok.points[i].y))
			{
				init ++;
			}

			if(init>5)
			{
				printf("god z!!!\n");
				me->input = g_string_append_c(me->input,'z');
				cairo_region_destroy(rg);
				return ;
			}
		}
		cairo_region_destroy(rg);
	}

	printf("NO Z!");

	//米有折点

	//首先，比较 起点和终点的 斜率

	int x = startpoint.x - endpoint.x;
	int y = startpoint.y - endpoint.y;

	float xielv = ((float) startpoint.x - (float) endpoint.x)
			/ ((float) startpoint.y - (float) endpoint.y);

	if ((atan2(y, x) > 0) || (atan2(y, x) < -2.9))
	{
		printf("h ?\n");
		me->input = g_string_append_c(me->input,'h');
	}
	else if ((atan2(y, x) < -1.4 ) && (atan2(y, x) > -1.7 ))
	{
		printf("s  ?\n");
		me->input = g_string_append_c(me->input,'s');

	}
	else if (atan2(y, x) < -2)
	{
		printf("n ?\n");
		me->input = g_string_append_c(me->input,'n');
	}
	else if (atan2(y, x) < -0.5)
	{
		printf("p  ?\n");
		me->input = g_string_append_c(me->input,'p');
	}
}

static gboolean ibus_handwrite_recog_lucykila_domatch(IbusHandwriteRecog*obj,int want)
{
	IbusHandwriteRecogLucyKila * me;
	MATCHED mt;
	char * ptr, *start_ptr, *p;
	int i, size = 0;

	me = IBUS_HANDWRITE_RECOG_LUCYKILA(obj);

	if(me->input->len == 0)
		return 0;

	GArray * result  = g_array_new(TRUE,TRUE,sizeof(MATCHED));
		puts(__func__);

	for (i = 0 , ptr = me->start_ptr ; i < me->items_count ; ++i , ptr+=64)
	{
		if (memcmp(ptr, me->input->str, me->input->len) == 0)
		{
			memset(&mt, 0, 64);
			p = ptr;
			while (*p != ' ' && *p != '\t')
				++p;
			memcpy(mt.code, ptr, p - ptr);
			while (*p == ' ' || *p == '\t')
				++p;
			strcpy(mt.hanzi, p);
			result = g_array_append_vals(result, &mt , 1 );
			size++;
		}
	}

	puts(__func__);

	//调节顺序
	g_array_sort(result, mysort);

	//载入 matched
	MatchedChar mc;

	obj->matched = g_array_set_size(obj->matched,0);

	for( i =0; i < size ;++i)
	{
		mt = g_array_index(result,MATCHED,i);

		strcpy(mc.chr , mt.hanzi);
		obj->matched = g_array_append_val(obj->matched,mc);
	}

	g_array_free(result,TRUE);

	return size;
}

static void ibus_handwrite_recog_lucykila_init(IbusHandwriteRecogLucyKila*obj)
{
	obj->input = g_string_new("");
	obj->start_ptr; //指向表的地址
	obj->items_count = 0; //表项数
	obj->table_size = 0; // 表大小
	obj->maped_size = 0; // 分配的内存大小
	lucykila_open_table(obj);
}

static void ibus_handwrite_recog_lucykila_destory(GObject*obj)
{
	IbusHandwriteRecogLucyKila * thisobj = IBUS_HANDWRITE_RECOG_LUCYKILA(obj);
	g_string_free(thisobj->input, TRUE);
	munmap(thisobj->start_ptr, thisobj->maped_size);

	IBUS_HANDWRITE_RECOG_LUCYKILA_GET_CLASS(obj)->parentdestroy((GObject*)obj);
}

static void ibus_handwrite_recog_lucykila_class_init(
		IbusHandwriteRecogLucyKilaClass* klass)
{
	IbusHandwriteRecogClass * parent = (IbusHandwriteRecogClass*) (klass);

	parent->domatch = ibus_handwrite_recog_lucykila_domatch;
	parent->change_stroke = ibus_handwrite_recog_change_stroke;

	klass->parentdestroy = G_OBJECT_CLASS(klass)->finalize ;

	G_OBJECT_CLASS(klass)->finalize = ibus_handwrite_recog_lucykila_destory;

}

GType ibus_handwrite_recog_lucykila_get_type(void)
{
	static const GTypeInfo type_info =
	{ sizeof(IbusHandwriteRecogLucyKilaClass), (GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) ibus_handwrite_recog_lucykila_class_init, NULL,
			NULL, sizeof(IbusHandwriteRecogLucyKila), 0,
			(GInstanceInitFunc) ibus_handwrite_recog_lucykila_init, };

	static GType type = 0;

	if (type == 0)
	{
		type = g_type_register_static(G_TYPE_IBUS_HANDWRITE_RECOG,
				"IbusHandwriteRecog_LucyKila", &type_info, 0);

	}
	return type;
}

static char *
nextline(char * ptr)
{
	while (*ptr != '\n')
		++ptr;
	//  *ptr = 0;
	return *ptr ? ++ptr : NULL;
}

static gint
mysort(gconstpointer a, gconstpointer b)
{
  MATCHED * pa ,  *pb;
  pa = (MATCHED*) a;
  pb = (MATCHED*) b;
//  g_printf("match sort %s %s\n",pa->hanzi,pb->hanzi);

  return (strlen(pa->code) ) - (strlen(pb->code) ) ;
}
