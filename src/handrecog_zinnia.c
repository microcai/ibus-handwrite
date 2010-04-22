/*
 * handrecog_zinnia.c
 *
 *  Created on: 2010-2-4
 *      Author: cai
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <zinnia.h>
#include "engine.h"
#include "handrecog.h"

typedef struct _IbusHandwriteRecogZinnia IbusHandwriteRecogZinnia;
typedef struct _IbusHandwriteRecogZinniaClass IbusHandwriteRecogZinniaClass;

GType ibus_handwrite_recog_zinnia_get_type(void);

#define IBUS_HANDWRITE_RECOG_ZINNIA_GET_CLASS(obj) \
		G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_IBUS_HANDWRITE_RECOG_ZINNIA, IbusHandwriteRecogZinniaClass)
#define IBUS_HANDWRITE_RECOG_ZINNIA(obj) \
		G_TYPE_CHECK_INSTANCE_CAST(obj,G_TYPE_IBUS_HANDWRITE_RECOG_ZINNIA,IbusHandwriteRecogZinnia)


struct _IbusHandwriteRecogZinnia{
	IbusHandwriteRecog	parent;
	zinnia_recognizer_t	*recognizer;
};

struct _IbusHandwriteRecogZinniaClass{
	IbusHandwriteRecogClass parent;
	void (* parentdestroy)(IbusHandwriteRecog *object);
};

static void ibus_handwrite_recog_zinnia_init(IbusHandwriteRecogZinnia*obj);
static void ibus_handwrite_recog_zinnia_class_init(IbusHandwriteRecogZinniaClass* klass);

static int zinnia_open_model(IbusHandwriteRecogZinnia*obj)
{
	g_debug(("using zinnia!!!"));
	gchar * file = g_strdup_printf("%s/handwriting-zh_CN.model",TOMOE_DATA_DIR);
	if (!zinnia_recognizer_open(obj->recognizer, file))
	{
		fprintf(stderr,  "ERROR: %s\n", zinnia_recognizer_strerror(
				obj->recognizer));
		g_free(file);
		return -1;
	}
	g_free(file);
	return 0;
}

static gboolean ibus_handwrite_recog_zinnia_domatch(IbusHandwriteRecog*obj,int want)
{
	size_t i;

	size_t	ii;

	zinnia_recognizer_t *recognizer;

	zinnia_character_t *character;

	zinnia_result_t *result;

	LineStroke cl;

	obj->matched = g_array_set_size(obj->matched,0);

	recognizer = IBUS_HANDWRITE_RECOG_ZINNIA(obj)->recognizer;


	character = zinnia_character_new();

	zinnia_character_clear(character);
	zinnia_character_set_width(character, 200);
	zinnia_character_set_height(character, 200);


	for (ii=0; ii < obj->strokes->len ; ++ii)
	{
		cl = g_array_index(obj->strokes, LineStroke , ii);

		for (i = 0; i < (cl.segments); ++i)
		{
			zinnia_character_add(character, ii, cl.points[i].x, cl.points[i].y);
		}
	}

	result = zinnia_recognizer_classify( recognizer, character, want);

	g_print("char %p\n",result);

	if (result == NULL)
	{
		fprintf(stderr, "%s\n", zinnia_recognizer_strerror(recognizer));
		zinnia_character_destroy(character);
		return FALSE;
	}

	int result_num  = zinnia_result_size(result);

		g_print("char \n");

	for (i = 0; i < result_num; ++i)
	{
		MatchedChar matched;

		strcpy(matched.chr,zinnia_result_value(result, i));

		obj->matched = g_array_append_val(obj->matched,matched);

	}

	zinnia_result_destroy(result);
	zinnia_character_destroy(character);
	return TRUE;

}


static void ibus_handwrite_recog_zinnia_init(IbusHandwriteRecogZinnia*obj)
{
	obj->recognizer = zinnia_recognizer_new();
	zinnia_open_model(obj);

}

static void ibus_handwrite_recog_zinnia_destory(IbusHandwriteRecog*obj)
{
	IbusHandwriteRecogZinnia * thisobj = IBUS_HANDWRITE_RECOG_ZINNIA(obj);

	zinnia_recognizer_destroy(thisobj->recognizer);

	IBUS_HANDWRITE_RECOG_ZINNIA_GET_CLASS(obj)->parentdestroy(obj);
}

static void ibus_handwrite_recog_zinnia_class_init(IbusHandwriteRecogZinniaClass* klass)
{
	IbusHandwriteRecogClass * parent = (IbusHandwriteRecogClass*)(klass);
	klass->parentdestroy = parent->destroy;
	parent->destroy = ibus_handwrite_recog_zinnia_destory;
	parent->domatch =ibus_handwrite_recog_zinnia_domatch;
}

GType ibus_handwrite_recog_zinnia_get_type(void)
{
	static const GTypeInfo type_info =
	{ sizeof(IbusHandwriteRecogZinniaClass), (GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) ibus_handwrite_recog_zinnia_class_init, NULL, NULL,
			sizeof(IbusHandwriteRecogZinnia), 0,
			(GInstanceInitFunc) ibus_handwrite_recog_zinnia_init, };

	static GType type = 0;

	if (type == 0)
	{
		type = g_type_register_static(G_TYPE_IBUS_HANDWRITE_RECOG, "IbusHandwriteRecog_zinnia",
				&type_info, 0);

	}
	return type;
}
