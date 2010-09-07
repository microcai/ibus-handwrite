/* vim:set et sts=4: */

#include <string.h>
#include "engine.h"
#include "handrecog.h"
#include "UI.h"
#include "global_var.h"

#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)


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

//static void ibus_engine_set_cursor_location(IBusEngine *engine, gint x, gint y,
//		gint w, gint h);
//static void ibus_handwrite_engine_set_capabilities(IBusEngine *engine,
//		guint caps);
//static void ibus_handwrite_engine_page_up(IBusEngine *engine);
//static void ibus_handwrite_engine_page_down(IBusEngine *engine);
//static void ibus_handwrite_engine_cursor_up(IBusEngine *engine);
//static void ibus_handwrite_engine_cursor_down(IBusEngine *engine);
static void ibus_handwrite_property_activate(IBusEngine *engine,
		const gchar *prop_name, guint prop_state);
//static void ibus_handwrite_engine_property_show(IBusEngine *engine,
//		const gchar *prop_name);
//static void ibus_handwrite_engine_property_hide(IBusEngine *engine,
//		const gchar *prop_name);

//static IBusEngineClass *ibus_handwrite_engine_parent_class = NULL;

G_DEFINE_TYPE(IBusHandwriteEngine,ibus_handwrite_engine,IBUS_TYPE_ENGINE)

static void ibus_handwrite_engine_class_init(IBusHandwriteEngineClass *klass)
{
	//init global class data
	IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
	IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);

//	parent_class = (IBusEngineClass *) g_type_class_peek_parent(klass);

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

	engine_class->property_activate = ibus_handwrite_property_activate;

	klass->commit_text = ibus_handwrite_engine_commit_text;
}

static void ibus_handwrite_engine_init(IBusHandwriteEngine *handwrite)
{
//	puts(__func__);
	G_TYPE_IBUS_HANDWRITE_RECOG_ZINNIA;
	G_TYPE_IBUS_HANDWRITE_RECOG_LUCYKILA;
	//先注册2个引擎
	handwrite->engine_type = G_TYPE_IBUS_HANDWRITE_RECOG_LUCYKILA ;

#ifdef WITH_ZINNIA
	if( strcmp(lang,"jp") ==0 || strcmp(lang,"ja")==0 )
	{
		handwrite->engine_type = G_TYPE_IBUS_HANDWRITE_RECOG_ZINNIA;
	}
#endif

	handwrite->engine = ibus_handwrite_recog_new(handwrite->engine_type);
}

static void ibus_handwrite_engine_destroy(IBusHandwriteEngine *handwrite)
{
	g_object_unref(handwrite->engine);
	ibus_handwrite_engine_disable(handwrite);
	IBUS_OBJECT_CLASS (ibus_handwrite_engine_parent_class)->destroy((IBusObject *) handwrite);
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

	IBusPropList * pl = ibus_prop_list_new();

	IBusProperty * p = ibus_property_new("choose-color", PROP_TYPE_NORMAL,
			ibus_text_new_from_static_string(_("color")), GTK_STOCK_COLOR_PICKER,
			ibus_text_new_from_static_string(_("click to set color")), TRUE, TRUE,
			PROP_STATE_UNCHECKED, NULL);

	ibus_prop_list_append(pl, p);

#ifdef WITH_ZINNIA
	if( strcmp(lang,"jp") ==0 || strcmp(lang,"ja"))
	{
		extern char icondir[4096];

		gchar * iconfile = g_strdup_printf("%s/switch.svg",icondir);

		g_debug("icon file is %s",iconfile);

		p = ibus_property_new("choose-engine", PROP_TYPE_NORMAL,
				ibus_text_new_from_static_string(_("engine")), iconfile,
				ibus_text_new_from_static_string(_("click to set engine")), TRUE, TRUE,
				PROP_STATE_UNCHECKED, NULL);

		g_free(iconfile);

		ibus_prop_list_append(pl, p);
	}
#endif

	ibus_engine_register_properties(IBUS_ENGINE(engine), pl);

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

void ibus_handwrite_property_activate(IBusEngine *engine,const gchar *prop_name, guint prop_state)
{
	IBusHandwriteEngine *  handwrite = IBUS_HANDWRITE_ENGINE(engine);

	if(g_strcmp0(prop_name,"choose-engine")==0)
	{
		g_debug("change engine");

		g_object_unref(handwrite->engine);

		if (handwrite->engine_type == G_TYPE_IBUS_HANDWRITE_RECOG_ZINNIA)
			handwrite->engine_type = G_TYPE_IBUS_HANDWRITE_RECOG_LUCYKILA;
		else
			handwrite->engine_type = G_TYPE_IBUS_HANDWRITE_RECOG_ZINNIA;

		handwrite->engine = ibus_handwrite_recog_new(handwrite->engine_type);

	}else if(g_strcmp0(prop_name,"choose-color")==0)
	{
		g_debug("color choose");

		GtkWidget * dialog = gtk_color_selection_dialog_new(prop_name);


		GtkWidget * color_sel = gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(dialog));

		gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(color_sel),handwrite->color);

		gtk_dialog_run(GTK_DIALOG(dialog));

		gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(color_sel),handwrite->color);

		gtk_widget_destroy(dialog);
	}
}

gboolean ibus_handwrite_engine_commit_text(IBusHandwriteEngine * engine , int index)
{
	MatchedChar * matched;

	int number = ibus_handwrite_recog_getmatch(engine->engine,&matched,0);

	if(number > index )
	{
		ibus_engine_commit_text(IBUS_ENGINE(engine),ibus_text_new_from_string(matched[index].chr));
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

	gtk_widget_queue_draw(handwrite->drawpanel);

	if (!modifiers)
		return FALSE;

	if(!handwrite->engine->strokes->len )
		return FALSE;

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

	case IBUS_Escape:
		ibus_handwrite_recog_clear_stroke(handwrite->engine);
		return TRUE;
	}
	return FALSE;
}
