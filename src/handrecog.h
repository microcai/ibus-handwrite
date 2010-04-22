/*
 * handrecog.c
 *
 *  Created on: 2010-2-4
 *      Author: cai
 */

#ifndef HANDRECOG_C_
#define HANDRECOG_C_

#include <glib.h>
#include <gdk/gdk.h>

typedef struct _LineStroke LineStroke;
typedef struct _MatchedChar MatchedChar;
typedef struct _IbusHandwriteRecog IbusHandwriteRecog;
typedef struct _IbusHandwriteRecogClass IbusHandwriteRecogClass;

struct _LineStroke
{
	int		segments; //包含有的段数目
	GdkPoint* points; //包含的用来构成笔画的点
};

struct _MatchedChar{
	char	chr[8];
};


struct _IbusHandwriteRecog{
	GObject parent;
	GArray  * matched;
	GArray  * strokes;
};

struct _IbusHandwriteRecogClass{
	GObjectClass parent;

	void (*dispose)(GObject*);
	guint singal[2];

	/* signals */
    void (* destroy)        (GObject*object);

    /*numbers*/
	void (*change_stroke)(IbusHandwriteRecog*);
	gboolean (*domatch)(IbusHandwriteRecog*,int want);
};

GType ibus_handwrite_recog_get_type(void) G_GNUC_CONST;
GType ibus_handwrite_recog_lucykila_get_type(void) G_GNUC_CONST;
GType ibus_handwrite_recog_zinnia_get_type(void) G_GNUC_CONST;

#ifdef WITH_ZINNIA
#define G_TYPE_IBUS_HANDWRITE_RECOG_ZINNIA (ibus_handwrite_recog_zinnia_get_type())
#else
#define G_TYPE_IBUS_HANDWRITE_RECOG_ZINNIA G_TYPE_IBUS_HANDWRITE_RECOG_LUCYKILA
#endif

#define G_TYPE_IBUS_HANDWRITE_RECOG_LUCYKILA (ibus_handwrite_recog_lucykila_get_type())
#define G_TYPE_IBUS_HANDWRITE_RECOG (ibus_handwrite_recog_get_type())
#define IBUS_HANDWRITE_RECOG_GET_CLASS(obj) G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_IBUS_HANDWRITE_RECOG, IbusHandwriteRecogClass)
#define IBUS_HANDWRITE_RECOG(obj) \
		G_TYPE_CHECK_INSTANCE_CAST(obj,G_TYPE_IBUS_HANDWRITE_RECOG,IbusHandwriteRecog)
enum{
	IBUS_HANDWRITE_RECOG_TABLE_FROM_FILENAME,
	IBUS_HANDWRITE_RECOG_TABLE_FROM_FILE,
	IBUS_HANDWRITE_RECOG_TABLE_FROM_FD,
	IBUS_HANDWRITE_RECOG_TABLE_FROM_MEMORY
};


IbusHandwriteRecog* ibus_handwrite_recog_new( GType   engine );

/*
 *  if way = IBUS_HANDWRITE_RECOG_TABLE_FROM_FILE, then pass filename
 *  if way = IBUS_HANDWRITE_RECOG_TABLE_FROM_MEMORY, then pass start point and length
 *  if way = IBUS_HANDWRITE_RECOG_TABLE_FROM_FD, then pass file descriptor
 */

void ibus_handwrite_recog_clear_stroke(IbusHandwriteRecog*obj);
void ibus_handwrite_recog_append_stroke(IbusHandwriteRecog*obj,LineStroke stroke);
void ibus_handwrite_recog_remove_stroke(IbusHandwriteRecog*obj,int number);
guint ibus_handwrite_recog_getmatch(IbusHandwriteRecog*obj,MatchedChar ** matchedchars, int munbers_skip);
gboolean ibus_handwrite_recog_domatch(IbusHandwriteRecog*,int want);


#endif /* HANDRECOG_C_ */
