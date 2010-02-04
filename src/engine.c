/* vim:set et sts=4: */

#include <string.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include "engine.h"
#include "handrecog.h"

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
};

typedef void (* ibus_engine_callback)(IBusEngine *engine);

/* functions prototype */
static void ibus_handwrite_engine_class_init(IBusHandwriteEngineClass *klass);
static void ibus_handwrite_engine_init(IBusHandwriteEngine *engine);
static void ibus_handwrite_engine_destroy(IBusHandwriteEngine *engine);
static gboolean
ibus_handwrite_engine_process_key_event(IBusEngine *engine, guint keyval,
		guint keycode, guint modifiers);
static void ibus_handwrite_engine_focus_in(IBusHandwriteEngine *engine);
static void ibus_handwrite_engine_focus_out(IBusHandwriteEngine *engine);
static void ibus_handwrite_engine_reset(IBusHandwriteEngine *engine);
static void ibus_handwrite_engine_enable(IBusHandwriteEngine *engine);
static void ibus_handwrite_engine_disable(IBusHandwriteEngine *engine);
static void ibus_engine_set_cursor_location(IBusEngine *engine, gint x, gint y,
		gint w, gint h);
static void ibus_handwrite_engine_set_capabilities(IBusEngine *engine,
		guint caps);
static void ibus_handwrite_engine_page_up(IBusEngine *engine);
static void ibus_handwrite_engine_page_down(IBusEngine *engine);
static void ibus_handwrite_engine_cursor_up(IBusEngine *engine);
static void ibus_handwrite_engine_cursor_down(IBusEngine *engine);
static void ibus_handwrite_property_activate(IBusEngine *engine,
		const gchar *prop_name, gint prop_state);
static void ibus_handwrite_engine_property_show(IBusEngine *engine,
		const gchar *prop_name);
static void ibus_handwrite_engine_property_hide(IBusEngine *engine,
		const gchar *prop_name);

static IBusEngineClass *parent_class = NULL;

GType ibus_handwrite_engine_get_type(void)
{
	static GType type = 0;

	static const GTypeInfo type_info =
	{ sizeof(IBusHandwriteEngineClass), (GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) ibus_handwrite_engine_class_init, NULL, NULL,
			sizeof(IBusHandwriteEngine), 0,
			(GInstanceInitFunc) ibus_handwrite_engine_init, };

	if (type == 0)
	{
		type = g_type_register_static(IBUS_TYPE_ENGINE, "IBusHandwriteEngine",
				&type_info, (GTypeFlags) 0);
	}

	return type;
}

static void ibus_handwrite_engine_class_init(IBusHandwriteEngineClass *klass)
{
	//init global class data
	IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
	IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);

	parent_class = (IBusEngineClass *) g_type_class_peek_parent(klass);

	ibus_object_class->destroy
			= (IBusObjectDestroyFunc) ibus_handwrite_engine_destroy;

	engine_class->process_key_event = ibus_handwrite_engine_process_key_event;

	engine_class->disable
			= (ibus_engine_callback) ibus_handwrite_engine_disable;
	engine_class->enable = (ibus_engine_callback) ibus_handwrite_engine_enable;
	engine_class->focus_in
			= (ibus_engine_callback) ibus_handwrite_engine_focus_in;
	engine_class->focus_out
			= (ibus_engine_callback) ibus_handwrite_engine_focus_out;
	engine_class->reset = (ibus_engine_callback) ibus_handwrite_engine_reset;
}

static void ibus_handwrite_engine_init(IBusHandwriteEngine *handwrite)
{
//	puts(__func__);
	handwrite->engine = ibus_handwrite_recog_new(IBUS_HANDWRITE_RECOG_ENGINE_ZINNIA);
	ibus_handwrite_recog_load_table(handwrite->engine,IBUS_HANDWRITE_RECOG_TABLE_FROM_FILENAME,modelfile);
}

static void ibus_handwrite_engine_destroy(IBusHandwriteEngine *handwrite)
{
	g_object_unref(handwrite->engine);
	ibus_handwrite_engine_disable(handwrite);
	IBUS_OBJECT_CLASS (parent_class)->destroy((IBusObject *) handwrite);
}

static gboolean on_paint(GtkWidget *widget, GdkEventExpose *event,
		gpointer user_data)
{
	GdkGC *gc;

	IBusHandwriteEngine * engine;

	LineStroke cl;
	int i;

	MatchedChar * matched;

	engine = (IBusHandwriteEngine *) (user_data);

	gc = gdk_gc_new(widget->window);

	gdk_draw_rectangle(widget->window, gc,0,0,0,199,199);

	gdk_draw_rectangle(widget->window, gc,0,200,0,399,199);

	puts(__func__);

	//已经录入的笔画

	for (i = 0; i < engine->engine->strokes->len ; i++ )
	{
		printf("drawing %d th line, total %d\n",i,engine->engine->strokes->len);
		cl =  g_array_index(engine->engine->strokes,LineStroke,i);
		gdk_draw_lines(widget->window, gc, cl.points,cl.segments );
	}
	//当下笔画
	if (engine->currentstroke.points)
		gdk_draw_lines(widget->window, gc, engine->currentstroke.points,
				engine->currentstroke.segments);


	int munber = ibus_handwrite_recog_getmatch(engine->engine,&matched,0);

	//画10个侯选字
	for (i = 0; i < munber ; ++i)
	{

		char drawtext[32]={0};

		sprintf(drawtext,"%d.%s",i,matched[i].chr);

		PangoLayout * layout = gtk_widget_create_pango_layout(widget,drawtext);

		gdk_draw_layout(widget->window, gc, (i % 5) * 40 + 3, 205 + (20 * (i / 5)),	layout);
		g_object_unref(layout);
	}

	g_object_unref(gc);
	return TRUE;

}

static gboolean on_mouse_move(GtkWidget *widget, GdkEventMotion *event,
		gpointer user_data)
{
	IBusHandwriteEngine * engine;

	engine = (IBusHandwriteEngine *) (user_data);

	if (engine->mouse_state == GDK_BUTTON_PRESS) // 鼠标按下状态
	{
		if ((event->x > 0) && (event->y > 0) && (event->x < 199) && (event->y
				< 199))
		{
			engine->currentstroke.points
					= g_renew(GdkPoint,engine->currentstroke.points,engine->currentstroke.segments +1  );

			engine->currentstroke.points[engine->currentstroke.segments].x
					= event->x;
			engine->currentstroke.points[engine->currentstroke.segments].y
					= event->y;
			engine->currentstroke.segments++;
			printf("move, x= %lf, Y=%lf, segments = %d \n",event->x,event->y,engine->currentstroke.segments);
		}
	}
	else
	{
//	printf("move start, x = %lf y = %lf \n",event->x_root -engine->lastpoint.x,event->y_root - engine->lastpoint.y);
		gtk_window_move(GTK_WINDOW(widget),event->x_root -engine->lastpoint.x,event->y_root - engine->lastpoint.y);
	}

	gdk_window_invalidate_rect(widget->window, 0, TRUE);

	return FALSE;
}

static gboolean on_button(GtkWidget* widget, GdkEventButton *event, gpointer user_data)
{
	IBusHandwriteEngine * engine;
	LineStroke * token;

	engine = (IBusHandwriteEngine *) (user_data);

	switch (event->type)
	{

	case GDK_BUTTON_PRESS:
		if(event->button != 1)
		{
			engine->mouse_state = 0;
			engine->lastpoint.x = event->x;
			engine->lastpoint.y = event->y;
			return ;
		}

		if ((event->x > 0) && (event->y > 0) && (event->x < 199) && (event->y
				< 199))
		{

			engine->mouse_state = GDK_BUTTON_PRESS;

			g_print("mouse clicked\n");

			engine->currentstroke.segments = 1;

			engine->currentstroke.points = g_new(GdkPoint,1);

			engine->currentstroke.points[0].x = event->x;
			engine->currentstroke.points[0].y = event->y;

		}
		break;
	case GDK_BUTTON_RELEASE:
		engine->mouse_state = GDK_BUTTON_RELEASE;

		ibus_handwrite_recog_append_stroke(engine->engine,engine->currentstroke);

		ibus_handwrite_recog_domatch(engine->engine,10);

		g_print("mouse released\n");

		gdk_window_invalidate_rect(event->window, 0, TRUE);

		break;
	}
}

static void ibus_handwrite_engine_enable(IBusHandwriteEngine *engine)
{
	GdkPixmap * pxmp;
	GdkGC * gc;
	GdkColor black, white;

	GdkColormap* colormap = gdk_colormap_get_system();

	gdk_color_black(colormap, &black);
	gdk_color_white(colormap, &white);

	g_object_unref(colormap);

	if (!engine->drawpanel)
	//建立绘图窗口, 建立空点
	{
		engine->drawpanel = gtk_window_new(GTK_WINDOW_POPUP);
		gtk_window_move((GtkWindow*) engine->drawpanel, 500, 550);
		gtk_widget_add_events(GTK_WIDGET(engine->drawpanel),
				GDK_BUTTON_MOTION_MASK | GDK_BUTTON_RELEASE_MASK| GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK);
		g_signal_connect_after(G_OBJECT(engine->drawpanel),"motion_notify_event",G_CALLBACK(on_mouse_move),engine);
		g_signal_connect(G_OBJECT(engine->drawpanel),"expose-event",G_CALLBACK(on_paint),engine);
		g_signal_connect(G_OBJECT(engine->drawpanel),"button-release-event",G_CALLBACK(on_button),engine);
		g_signal_connect(G_OBJECT(engine->drawpanel),"button-press-event",G_CALLBACK(on_button),engine);

		gtk_window_resize(GTK_WINDOW(engine->drawpanel), 200, 250);

		gtk_widget_show(engine->drawpanel);


		pxmp = gdk_pixmap_new(NULL, 200, 250, 1);
		gc = gdk_gc_new(GDK_DRAWABLE(pxmp));

		gdk_gc_set_foreground(gc, &black);

		gdk_draw_rectangle(GDK_DRAWABLE(pxmp),gc,1,0,0,200,250);

		gdk_gc_set_foreground(gc, &white);

		gdk_draw_arc(GDK_DRAWABLE(pxmp), gc, 1, 0, 0, 30, 30, 0, 360 * 64);
		gdk_draw_arc(GDK_DRAWABLE(pxmp), gc, 1,200 - 30,0,30,30,0,360 * 64);
		gdk_draw_arc(GDK_DRAWABLE(pxmp),gc,1,200- 30,250 - 30,30,30,0,360 * 64);
		gdk_draw_arc(GDK_DRAWABLE(pxmp),gc,1,0,250 - 30,30,30,0,360 * 64);
		gdk_draw_rectangle(GDK_DRAWABLE(pxmp),gc,1,0,15,200 ,250 - 30);
		gdk_draw_rectangle(GDK_DRAWABLE(pxmp),gc,1,15,0,200 - 30,250);
		gdk_window_shape_combine_mask(engine->drawpanel->window,pxmp,0,0);
		g_object_unref(gc);
		g_object_unref(pxmp);
		gtk_window_set_opacity(GTK_WINDOW(engine->drawpanel),0.62);
		//	engine->GdkPoints = NULL;
	}
	//	gtk_widget_show_all(engine->drawpanel);
}

static void ibus_handwrite_engine_disable(IBusHandwriteEngine *engine)
{ // 撤销绘图窗口，销毁点列表
	if (engine->drawpanel)
		gtk_widget_destroy(engine->drawpanel);
	engine->drawpanel = NULL;
	g_free(engine->currentstroke.points);
	engine->currentstroke.points = NULL;
}

static void ibus_handwrite_engine_focus_in(IBusHandwriteEngine *engine)
{
	GdkCursor* cursor;

	printf("%s \n", __func__);
	if (engine->drawpanel)
	{
		gtk_widget_show(engine->drawpanel);

		cursor = gdk_cursor_new(GDK_PENCIL);
		if (cursor)
		{
			gdk_window_set_cursor(engine->drawpanel->window, cursor);
			gdk_cursor_unref(cursor);
		}
	}
}

static void ibus_handwrite_engine_focus_out(IBusHandwriteEngine *engine)
{
	if (engine->drawpanel)
	{
		gtk_widget_hide(engine->drawpanel);
	}
	printf("%s \n", __func__);

}
static void ibus_handwrite_engine_reset(IBusHandwriteEngine *engine)
{
	printf("%s \n", __func__);
//	ibus_handwrite_engine_disable(engine);
}

static gboolean ibus_handwrite_engine_commit_text(IBusHandwriteEngine * engine , int index)
{
	IBusText *text;
	MatchedChar * matched;

	int number = ibus_handwrite_recog_getmatch(engine->engine,&matched,0);

	if(number > index )
	{
		text =ibus_text_new_from_string(matched[index].chr);
		ibus_engine_commit_text(IBUS_ENGINE(engine),text);
		g_object_unref(text);
		ibus_handwrite_recog_clear_stroke(engine->engine);
		engine->needclear = TRUE;
		engine->currentstroke.segments = 0;
		g_free(engine->currentstroke.points);
		engine->currentstroke.points = NULL;
		return TRUE;
	}
	engine->needclear = FALSE;
	return FALSE;
}

static gboolean ibus_handwrite_engine_process_key_event(IBusEngine *engine,
		guint keyval, guint keycode, guint modifiers)
{
	IBusHandwriteEngine *handwrite = (IBusHandwriteEngine *) engine;
	MatchedChar * matched;

	if (!modifiers)
		return FALSE;

	if(!handwrite->engine->strokes->len )
		return FALSE;


	gdk_window_invalidate_rect(handwrite->drawpanel->window,0,0);

	switch (keyval)
	{
	case IBUS_BackSpace:
		if(handwrite->needclear)
			return FALSE;
		if (handwrite->engine->strokes->len)
		{
			ibus_handwrite_recog_remove_stroke(handwrite->engine, 1);
			return TRUE;
		}
		//ibus_handwrite_engine_disable(handwrite);
		return FALSE;
	case IBUS_space:
		return ibus_handwrite_engine_commit_text(handwrite,0);

	case IBUS_0 ... IBUS_9:

	case IBUS_KP_0 ... IBUS_KP_9:
		return ibus_handwrite_engine_commit_text(handwrite,
				(keyval > IBUS_KP_0) ? (keyval - IBUS_KP_0) : (keyval - IBUS_0));
	}
	return FALSE;
}
