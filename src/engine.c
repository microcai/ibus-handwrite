/* vim:set et sts=4: */

#include <string.h>
#include "engine.h"
#include "handrecog.h"
#include "UI.h"

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
static gboolean ibus_handwrite_engine_commit_text(IBusHandwriteEngine * engine , int index);
//static void ibus_engine_set_cursor_location(IBusEngine *engine, gint x, gint y,
//		gint w, gint h);
//static void ibus_handwrite_engine_set_capabilities(IBusEngine *engine,
//		guint caps);
//static void ibus_handwrite_engine_page_up(IBusEngine *engine);
//static void ibus_handwrite_engine_page_down(IBusEngine *engine);
//static void ibus_handwrite_engine_cursor_up(IBusEngine *engine);
//static void ibus_handwrite_engine_cursor_down(IBusEngine *engine);
//static void ibus_handwrite_property_activate(IBusEngine *engine,
//		const gchar *prop_name, gint prop_state);
//static void ibus_handwrite_engine_property_show(IBusEngine *engine,
//		const gchar *prop_name);
//static void ibus_handwrite_engine_property_hide(IBusEngine *engine,
//		const gchar *prop_name);

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
	klass->commit_text = ibus_handwrite_engine_commit_text;
}

static void ibus_handwrite_engine_init(IBusHandwriteEngine *handwrite)
{
//	puts(__func__);
	handwrite->engine = ibus_handwrite_recog_new(IBUS_HANDWRITE_RECOG_ENGINE_LUCYKILA);
	ibus_handwrite_recog_load_table(handwrite->engine,IBUS_HANDWRITE_RECOG_TABLE_FROM_FILENAME,modelfile);
	//UI_buildui(handwrite);
}

static void ibus_handwrite_engine_destroy(IBusHandwriteEngine *handwrite)
{
	g_object_unref(handwrite->engine);
	ibus_handwrite_engine_disable(handwrite);
	IBUS_OBJECT_CLASS (parent_class)->destroy((IBusObject *) handwrite);
}

static void ibus_handwrite_engine_enable(IBusHandwriteEngine *engine)
{
	UI_buildui(engine);
}

static void ibus_handwrite_engine_disable(IBusHandwriteEngine *engine)
{
	UI_cancelui(engine);
	// 撤销绘图窗口，销毁点列表
	if (engine->drawpanel)
		gtk_widget_destroy(engine->drawpanel);
	engine->drawpanel = NULL;
	g_free(engine->currentstroke.points);
	engine->currentstroke.points = NULL;
}

static void ibus_handwrite_engine_focus_in(IBusHandwriteEngine *engine)
{
	UI_show_ui(engine);
}

static void ibus_handwrite_engine_focus_out(IBusHandwriteEngine *engine)
{
	UI_hide_ui(engine);
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
