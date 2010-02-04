/*
 * caicai_recg.c - caicai recognize engine
 *
 *  Created on: 2010-2-2
 *      Author: cai
 */

#include <stdarg.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include "engine.h"
#include "handrecog.h"

static void ibus_handwrite_recog_init(IbusHandwriteRecog*obj);
static void ibus_handwrite_recog_class_init(IbusHandwriteRecogClass* klass);

guint ibus_handwrite_recog_getmatch(IbusHandwriteRecog* recog,MatchedChar ** matchedchars,  int munbers_skip)
{
	if(recog->matched)
	{
		*matchedchars = & g_array_index(recog->matched,MatchedChar,munbers_skip);
		return recog->matched->len - munbers_skip;
	}
	else
	{
		*matchedchars = NULL;
		return 0;
	}
}

void ibus_handwrite_recog_clear_stroke(IbusHandwriteRecog*obj)
{
	g_array_set_size(obj->strokes, 0);
	if (IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->change_stroke)
		IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->change_stroke(obj);
}

void ibus_handwrite_recog_append_stroke(IbusHandwriteRecog*obj,LineStroke stroke)
{
	obj->strokes = g_array_append_val(obj->strokes,stroke);
	if(IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->change_stroke)
		IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->change_stroke(obj);
}

void ibus_handwrite_recog_remove_stroke(IbusHandwriteRecog*obj,int number)
{
	obj->strokes = g_array_remove_range(obj->strokes,obj->strokes->len - number,number);
	if(IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->change_stroke)
		IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->change_stroke(obj);
}

gboolean ibus_handwrite_recog_domatch(IbusHandwriteRecog*obj,int want)
{
	return IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->domatch(obj,want);
}

gboolean ibus_handwrite_recog_load_table(IbusHandwriteRecog* obj, int way, ...)
{
	va_list ap;
	int fd = -1;
	FILE * file = NULL;
	gchar * filename = NULL;
	void * start_ptr = NULL;
	int len = -1;
	int child_ret  ;

	va_start(ap,way);
	switch(way){
	case IBUS_HANDWRITE_RECOG_TABLE_FROM_FILENAME:
		filename = va_arg(ap,gchar*);
		if ((child_ret = IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->load_table(obj,
				way, filename)) != 1)
			break;
		file = fopen(filename,"r");
		if(!file)
			return FALSE;
	case IBUS_HANDWRITE_RECOG_TABLE_FROM_FILE:
		if(!file) file = va_arg(ap,FILE*);
		if ((child_ret = IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->load_table(obj,
				way, file)) != 1)
			break;

		va_start(ap,way);
		if (file != va_arg(ap,FILE*))
		{
			fclose(file);
			file = NULL;
			fd = open(filename,O_WRONLY);
		}else
		{
			fd = fileno(file);
		}
	case IBUS_HANDWRITE_RECOG_TABLE_FROM_FD:
		if(fd ==-1)	fd = va_arg(ap,int);

		if ((child_ret = IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->load_table(obj,
				way, fd)) != 1)
			break;
		return -1;
	case IBUS_HANDWRITE_RECOG_TABLE_FROM_MEMORY:
		start_ptr = va_arg(ap,void*);
		len = va_arg(ap,int);
		return IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->load_table(obj,way,start_ptr,len);

	}
	return child_ret == 0;
}

IbusHandwriteRecog* ibus_handwrite_recog_new(ENGINEYTPE enginetype)
{
	GType engine[] =
	{ ibus_handwrite_recog_zinnia_get_type(), G_TYPE_INVALID };

	return IBUS_HANDWRITE_RECOG(g_type_create_instance(engine[enginetype]));
}


GType ibus_handwrite_recog_get_type(void)
{
	static const GTypeInfo type_info =
	{ sizeof(IbusHandwriteRecogClass), (GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) ibus_handwrite_recog_class_init, NULL, NULL,
			sizeof(IbusHandwriteRecog), 0,
			(GInstanceInitFunc) ibus_handwrite_recog_init, };

	static GType type = 0;

	if (type == 0)
	{

		type = g_type_register_static(G_TYPE_OBJECT, "IbusHandwriteRecog",
				&type_info, G_TYPE_FLAG_ABSTRACT);

	}
	return type;
}


static void ibus_handwrite_recog_init(IbusHandwriteRecog*obj)
{
	obj->matched =g_array_new(TRUE,TRUE,sizeof(MatchedChar));
	obj->strokes = g_array_sized_new(TRUE,TRUE,sizeof(LineStroke),0);
}

static void ibus_handwrite_recog_destory(IbusHandwriteRecog*obj)
{
	int i;
	puts(__func__);

	g_array_free(obj->matched, TRUE);

	obj->matched = NULL;

	for( i= 0; i < obj->strokes->len ; ++i)
	{
		g_free(g_array_index(obj->strokes,LineStroke,i).points);
	}
	g_array_free(obj->strokes, TRUE);
}

static void my_dispose(GObject * obj)
{
	g_signal_emit(obj,IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->singal[0],0);
	IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->dispose(obj);
}


static void ibus_handwrite_recog_class_init(IbusHandwriteRecogClass* klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	klass->dispose = gobject_class->dispose;

	gobject_class->dispose = my_dispose ;

	klass->load_table = NULL;
	klass->change_stroke = NULL;
	klass->domatch = NULL;
	klass->destroy = ibus_handwrite_recog_destory ;

    klass->singal[0] =  g_signal_new("destroy", G_TYPE_FROM_CLASS (gobject_class),
    		G_SIGNAL_RUN_LAST,
    		G_STRUCT_OFFSET (IbusHandwriteRecogClass, destroy), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


