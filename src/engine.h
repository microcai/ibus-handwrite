/* vim:set et sts=4: */
#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <ibus.h>
#include <gtk/gtk.h>

#define IBUS_TYPE_HANDWRITE_ENGINE	\
	(ibus_handwrite_engine_get_type ())

GType ibus_handwrite_engine_get_type(void);

#define IBUS_HANDWRITE_ENGINE_GET_CLASS(obj)	((IBusHandwriteEngineClass*)(IBUS_ENGINE_GET_CLASS(obj)))

typedef struct _RESULTCHAR RESULTCHAR;

extern char modelfile[1024] ; //= "data/handwriting-zh_CN.model";
extern char tablefile[1024] ; //= "data/table.txt";
#endif
