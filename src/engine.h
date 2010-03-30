/* vim:set et sts=4: */
#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <ibus.h>
#include <gtk/gtk.h>
#include "handrecog.h"

#define IBUS_TYPE_HANDWRITE_ENGINE	\
	(ibus_handwrite_engine_get_type ())

GType ibus_handwrite_engine_get_type(void);

#define IBUS_HANDWRITE_ENGINE_GET_CLASS(obj)	((IBusHandwriteEngineClass*)(IBUS_ENGINE_GET_CLASS(obj)))


typedef struct _IBusHandwriteEngine IBusHandwriteEngine;
typedef struct _IBusHandwriteEngineClass IBusHandwriteEngineClass;

struct _IBusHandwriteEngine
{
	IBusEngine parent;

	/* members */
	GtkWidget * drawpanel;
	GdkPoint lastpoint;
	guint mouse_state;
	IbusHandwriteRecog * engine;
	LineStroke currentstroke;
	gboolean	needclear;

};

struct _IBusHandwriteEngineClass
{
	IBusEngineClass parent;
	gboolean (*commit_text)(IBusHandwriteEngine * engine , int index);
};


typedef struct _RESULTCHAR RESULTCHAR;

extern gchar * tablefile;
#endif
