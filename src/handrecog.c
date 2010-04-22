/*
 * handrecog.c - Handwrite recognize engine
 *
 *  Created on: 2010-2-2
 *      Author: cai
 */

#include <stdarg.h>
#include <string.h>
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
	obj->strokes = g_array_set_size(obj->strokes,0);

	if (IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->change_stroke)
		IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->change_stroke(obj);
}

void ibus_handwrite_recog_append_stroke(IbusHandwriteRecog*obj,LineStroke stroke)
{
	LineStroke s;
	s.segments = stroke.segments;

	if (stroke.segments)
	{
		s.points = g_new(GdkPoint,s.segments);
		memcpy(s.points, stroke.points, s.segments * sizeof(GdkPoint));

		obj->strokes = g_array_append_val(obj->strokes,s);

		if (IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->change_stroke)
			IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->change_stroke(obj);
	}
}

void ibus_handwrite_recog_remove_stroke(IbusHandwriteRecog*obj,int number)
{
	int i;
	for( i =obj->strokes->len - number;i< obj->strokes->len ; i++)
	{
		g_free(g_array_index(obj->strokes,LineStroke,i).points);
	}

	obj->strokes = g_array_remove_range(obj->strokes,obj->strokes->len - number,number);

	if(IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->change_stroke)
		IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->change_stroke(obj);
}

gboolean ibus_handwrite_recog_domatch(IbusHandwriteRecog*obj,int want)
{
	return IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->domatch(obj,want);
}

IbusHandwriteRecog* ibus_handwrite_recog_new(GType enginetype)
{
	return IBUS_HANDWRITE_RECOG(g_object_new(enginetype,NULL));
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

static void ibus_handwrite_recog_destory(GObject * gobj)
{
	IbusHandwriteRecog*obj = (IbusHandwriteRecog*)gobj;
	int i;
	puts(__func__);

	g_array_free(obj->matched, TRUE);

	obj->matched = NULL;

	for( i= 0; i < obj->strokes->len ; ++i)
	{
		g_free(g_array_index(obj->strokes,LineStroke,i).points);
	}
	g_array_free(obj->strokes, TRUE);

	IBUS_HANDWRITE_RECOG_GET_CLASS(obj)->destroy(gobj);
}

static void ibus_handwrite_recog_class_init(IbusHandwriteRecogClass* klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	klass->change_stroke = NULL;
	klass->domatch = NULL;
	klass->destroy =  gobject_class->finalize;
	gobject_class->finalize = ibus_handwrite_recog_destory;
}


