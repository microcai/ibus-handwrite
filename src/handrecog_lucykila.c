/*
 * handrecog_lucykila.c - my first attempt to write my own online handwrite recognition engine
 *
 *  Created on: 2010-2-7
 *      Author: cai
 */

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
};

struct _IbusHandwriteRecogLucyKilaClass{
	IbusHandwriteRecogClass parent;
	void (* parentdestroy)(IbusHandwriteRecog *object);
};

static void ibus_handwrite_recog_lucykila_init(IbusHandwriteRecogLucyKila*obj);
static void ibus_handwrite_recog_lucykila_class_init(IbusHandwriteRecogLucyKilaClass* klass);

static int lucykila_open_table(IbusHandwriteRecogLucyKila*obj,int way,void * start_ptr,int length)
{
	if(way==IBUS_HANDWRITE_RECOG_TABLE_FROM_MEMORY)
	{
		//TODO
	}
	else return 1;
}


static gboolean ibus_handwrite_recog_lucykila_domatch(IbusHandwriteRecog*obj,int want)
{
	return FALSE;
}


static void ibus_handwrite_recog_lucykila_init(IbusHandwriteRecogLucyKila*obj)
{
}

static void ibus_handwrite_recog_lucykila_destory(IbusHandwriteRecog*obj)
{
	IbusHandwriteRecogLucyKila * thisobj = IBUS_HANDWRITE_RECOG_LUCYKILA(obj);

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
