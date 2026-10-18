/*
 * UI_gtk.c -- provide UI via gtk
 *
 *  Created on: 2010-2-4
 *      Author: cai
 */


#include <math.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <gdk/gdk.h>

#include "engine.h"
#include "UI.h"

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) gettext_noop (String)

#define WIDTH 260
#define HEIGHT 260
#define PANELHEIGHT 60

static void widget_realize(GtkWidget *widget, gpointer user_data);

static gboolean _draw_lines(cairo_t * cr, LineStroke * cl)
{
	if (0 == cl->segments)
		return FALSE;

	int i;
	for (i = 0; i < cl->segments; ++i) {
		graphene_point_t point = cl->points[i];
		cairo_line_to(cr, point.x, point.y);
	}

	cairo_stroke(cr);

	return TRUE;
}

    static void clicked(GtkButton *button, IBusHandwriteEngine *engine)
		{
			ibus_handwrite_engine_commit_text(engine,GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button),"order")));
		}

static gboolean paint_lines(GtkWidget *widget, cairo_t * cr, IBusHandwriteEngine * engine)
{
	GtkStyleContext* stylectx;

	LineStroke cl;
	int i;

	puts(__func__);

	/* render frame border */
	//謄gtk_render_frame deprciated omiting

	/* set line attributes */
	cairo_set_line_width(cr,3.0);
	cairo_set_line_cap(cr,CAIRO_LINE_CAP_ROUND);
	cairo_set_line_join(cr,CAIRO_LINE_JOIN_ROUND);

	gdk_cairo_set_source_rgba (cr, engine->color);

	//已经录入的笔画
	for (i = 0; i < engine->engine->strokes->len ; i++ )
	{
		printf("drawing %d th line, total %d\n",i,engine->engine->strokes->len);
		cl =  g_array_index(engine->engine->strokes,LineStroke,i);
		_draw_lines(cr, &cl);
	}
	//当下笔画
	if ( engine->currentstroke.segments && engine->currentstroke.points )
		_draw_lines(cr, &(engine->currentstroke));

	return TRUE;
}

static void regen_lookuptable(GtkGrid * widget, IBusHandwriteEngine * engine)
{
	int i;
	MatchedChar *matched;
	char drawtext[32]={0};
	GtkWidget * bt;

    GtkWidget* child;
    for(child = gtk_widget_get_first_child(GTK_WIDGET(widget));
        child != NULL;
        child = gtk_widget_get_next_sibling(child)){
       gtk_grid_remove(widget,child); 
    }

	int munber = ibus_handwrite_recog_getmatch(engine->engine,&matched,0);

	//画10个侯选字
	for (i = 0; i < MIN(munber,10) ; ++i)
	{
		sprintf(drawtext,"%d.%s",i,matched[i].chr);

		bt = gtk_button_new_with_label(drawtext);

		g_object_set(G_OBJECT(bt), "expand", TRUE, NULL);

		gtk_grid_attach(GTK_GRID(widget), bt, i%5, i/5, 1, 1);

		gtk_widget_set_visible(bt,TRUE);


		g_object_set_data(G_OBJECT(bt),"order",GINT_TO_POINTER(i));
		g_signal_connect(bt,"clicked",G_CALLBACK(clicked),engine);

		gtk_widget_set_visible(bt,TRUE);
	}
}


static gboolean on_drag(GtkWidget *widget,GdkEvent* event,
		gpointer user_data)
{
	IBusHandwriteEngine * engine;

	engine = (IBusHandwriteEngine *) (user_data);

	//GdkCursorType ct ;

	gint width,height;

	GtkWindow * window;

	gtk_widget_get_size_request(engine->drawpanel,&width,&height);

	//ct = event->y < (height - PANELHEIGHT) ?  GDK_PENCIL:GDK_CENTER_PTR;

    GdkModifierType mods = gdk_event_get_modifier_state(event);


	if(mods & GDK_BUTTON2_MASK )
		gtk_widget_set_cursor_from_name(widget,"se-resize");

    
	if( mods & GDK_BUTTON2_MASK | GDK_BUTTON3_MASK )
	    gtk_widget_set_cursor_from_name(widget,"all-resize");

    GtkRoot *root = gtk_widget_get_root(widget);
	window = GTK_WINDOW(root);

    double event_x;
    double event_y;
    gdk_event_get_position( event, &event_x,&event_y);

	if (mods == GDK_BUTTON1_MASK) // 鼠标按下状态
	{
		//gdk_window_set_cursor(window,gdk_cursor_new(ct));
        
		engine->currentstroke.points
				= g_renew(graphene_point_t,engine->currentstroke.points,engine->currentstroke.segments +1  );
       

                
		engine->currentstroke.points[engine->currentstroke.segments].x = event_x;
        engine->currentstroke.points[engine->currentstroke.segments].y = event_y;

        
		engine->currentstroke.segments++;
		printf("move, x= %lf, Y=%lf, segments = %d \n",event_x,event_y,engine->currentstroke.segments);

		gtk_widget_queue_draw(widget);
	}
	else{
        GdkDevice* device = gdk_event_get_device(event); 
        GdkToplevel* toplevel = GDK_TOPLEVEL(root);
        guint32 time = gdk_event_get_time(event);



        if(mods & GDK_BUTTON2_MASK)
	    {
            gdk_toplevel_begin_resize(toplevel,GDK_SURFACE_EDGE_SOUTH_WEST,device,2,event_x,event_y,time);

	    }
	    else if( mods & (GDK_BUTTON3_MASK ))
	    {
            gdk_toplevel_begin_move(toplevel, device,3,event_x,event_y,time);
        }
    }

	return FALSE;
}
/**
static gboolean on_button(GtkWidget* widget, GdkButtonEvent *event, gpointer user_data)
{
	IBusHandwriteEngine * engine;

	engine = (IBusHandwriteEngine *) (user_data);

	switch (event->type)
	{

	case GDK_BUTTON_PRESS:
		if(event->button != 1)
		{
			engine->mouse_state = 0;
			engine->lastpoint.x = event->x;
			engine->lastpoint.y = event->y;
			return TRUE;
		}

		engine->mouse_state = GDK_BUTTON_PRESS;

		g_print("mouse clicked\n");

		engine->currentstroke.segments = 1;

		engine->currentstroke.points = g_new(graphene_point_t,1);

		engine->currentstroke.points[0].x = event->x;
		engine->currentstroke.points[0].y = event->y;

		break;
	case GDK_BUTTON_RELEASE:
		engine->mouse_state = GDK_BUTTON_RELEASE;

		ibus_handwrite_recog_append_stroke(engine->engine,engine->currentstroke);

		engine->currentstroke.segments = 0;
		g_free(engine->currentstroke.points);

		engine->currentstroke.points = NULL;

		ibus_handwrite_recog_domatch(engine->engine,10);

		g_print("mouse released\n");

		gtk_widget_queue_draw(widget);
		regen_lookuptable(GTK_GRID(engine->lookuppanel),engine);

		break;
	default:
		return FALSE;
	}
	return TRUE;
}
*/

void UI_buildui(IBusHandwriteEngine * engine)
{
    
    GtkGesture* gesture;
	//建立绘图窗口, 建立空点
	if (!engine->drawpanel)
	{
		engine->drawpanel = gtk_window_new();
        gtk_window_set_decorated(GTK_WINDOW(engine->drawpanel),FALSE);

        gtk_widget_set_tooltip_markup(GTK_WIDGET(engine->drawpanel),
                                      _("<b>Hint:</b>\n"
                                      "Left mouse key to draw strokes.\n"
                                      "Holding right mouse key to move the widget.\n"
                                      ));


		GtkWidget * vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_box_set_homogeneous(GTK_BOX(vbox), FALSE);

		gtk_window_set_child(GTK_WINDOW(engine->drawpanel),vbox);

		GtkWidget * drawing_area = gtk_drawing_area_new();

                g_signal_connect(G_OBJECT(drawing_area),"draw",G_CALLBACK(paint_lines),engine);

		gtk_box_prepend(GTK_BOX(vbox),drawing_area);

		gtk_widget_set_size_request(drawing_area, WIDTH, HEIGHT - PANELHEIGHT);

		engine->lookuppanel = gtk_grid_new();

		gtk_box_append(GTK_BOX(vbox),engine->lookuppanel);
		gtk_widget_set_size_request(engine->lookuppanel, WIDTH, PANELHEIGHT);

		//gtk_widget_add_events(GTK_WIDGET(drawing_area),GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK| GDK_BUTTON_PRESS_MASK);

        gesture = gtk_gesture_drag_new();
        g_signal_connect(drawing_area,"drag-start",G_CALLBACK(on_drag),engine);
		g_signal_connect(drawing_area,"drag-update",G_CALLBACK(on_drag),engine);
        g_signal_connect(drawing_area,"drag-end", G_CALLBACK(on_drag),engine);
	}
	gtk_widget_set_visible(engine->drawpanel,TRUE);
}

void UI_show_ui(IBusHandwriteEngine * engine)
{
	printf("%s \n", __func__);
	if (engine->drawpanel)
	{
		gtk_widget_set_visible(engine->drawpanel,TRUE);
	}
}

void UI_hide_ui(IBusHandwriteEngine * engine)
{
	if (engine->drawpanel)
	{
		gtk_widget_set_visible(engine->drawpanel,FALSE);
	}
}

void UI_cancelui(IBusHandwriteEngine* engine)
{
	// 撤销绘图窗口，销毁点列表
	if (engine->drawpanel)
		gtk_window_destroy(GTK_WINDOW(engine->drawpanel));
	engine->drawpanel = NULL;
}
//deleting relize because round corners are default in gnome and themeing is done in css
