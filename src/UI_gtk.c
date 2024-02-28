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
		GdkPoint point = cl->points[i];
		cairo_line_to(cr, point.x, point.y);
	}

	cairo_stroke(cr);

	return TRUE;
}

static gboolean paint_lines(GtkWidget *widget, cairo_t * cr, IBusHandwriteEngine * engine)
{
	GtkStyleContext* stylectx;

	LineStroke cl;
	int i;

	puts(__func__);

	/* render frame border */
	stylectx = gtk_widget_get_style_context(widget);
	gtk_render_frame(stylectx, cr, 0, 0, WIDTH, HEIGHT);

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

static void regen_loopuptable(GtkWidget * widget, IBusHandwriteEngine * engine)
{
	int i;
	MatchedChar *matched;
	char drawtext[32]={0};
	GtkWidget * bt;

	gtk_container_foreach(GTK_CONTAINER(widget),(GtkCallback)gtk_widget_destroy,0);

	int munber = ibus_handwrite_recog_getmatch(engine->engine,&matched,0);

	//画10个侯选字
	for (i = 0; i < MIN(munber,10) ; ++i)
	{
		sprintf(drawtext,"%d.%s",i,matched[i].chr);

		bt = gtk_button_new_with_label(drawtext);

		g_object_set(G_OBJECT(bt), "expand", TRUE, NULL);

		gtk_grid_attach(GTK_GRID(widget), bt, i%5, i/5, 1, 1);

		gtk_widget_show(bt);

		void clicked(GtkButton *button, IBusHandwriteEngine *engine)
		{
			ibus_handwrite_engine_commit_text(engine,GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button),"order")));
		}

		g_object_set_data(G_OBJECT(bt),"order",GINT_TO_POINTER(i));
		g_signal_connect(bt,"clicked",G_CALLBACK(clicked),engine);

		gtk_widget_show(bt);
	}
}


static gboolean on_mouse_move(GtkWidget *widget, GdkEventMotion *event,
		gpointer user_data)
{
	IBusHandwriteEngine * engine;

	engine = (IBusHandwriteEngine *) (user_data);

	GdkCursorType ct ;

	gint width,height;

	GdkWindow * window;

	gtk_window_get_size(GTK_WINDOW(engine->drawpanel),&width,&height);

	ct = event->y < (height - PANELHEIGHT) ?  GDK_PENCIL:GDK_CENTER_PTR;

	if( event->state & (GDK_BUTTON2_MASK |GDK_BUTTON3_MASK ))
		ct = GDK_FLEUR;

	if(event->state & GDK_BUTTON2_MASK )
		ct = GDK_BOTTOM_RIGHT_CORNER;

	window = gtk_widget_get_window(widget);

	gdk_window_set_cursor(window,gdk_cursor_new(ct));

	if (engine->mouse_state == GDK_BUTTON_PRESS) // 鼠标按下状态
	{
		gdk_window_set_cursor(window,gdk_cursor_new(ct));

		engine->currentstroke.points
				= g_renew(GdkPoint,engine->currentstroke.points,engine->currentstroke.segments +1  );

		engine->currentstroke.points[engine->currentstroke.segments].x
				= event->x;
		engine->currentstroke.points[engine->currentstroke.segments].y
				= event->y;
		engine->currentstroke.segments++;
		printf("move, x= %lf, Y=%lf, segments = %d \n",event->x,event->y,engine->currentstroke.segments);

		gtk_widget_queue_draw(widget);
	}
	else if(event->state & GDK_BUTTON2_MASK)
	{
		// change size
		width += event->x - engine->lastpoint.x;
		height += event->y - engine->lastpoint.y;

		gtk_window_resize(GTK_WINDOW(engine->drawpanel),width,height);

		g_debug("set size to %d,%d",width,height);

		widget_realize(engine->drawpanel,engine);

		engine->lastpoint.x = event->x;
		engine->lastpoint.y = event->y;
	}
	else if( event->state & (GDK_BUTTON2_MASK |GDK_BUTTON3_MASK ))
	{
		gtk_window_move(GTK_WINDOW(engine->drawpanel),event->x_root -engine->lastpoint.x,event->y_root - engine->lastpoint.y);
	}
	return FALSE;
}

static gboolean on_button(GtkWidget* widget, GdkEventButton *event, gpointer user_data)
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

		engine->currentstroke.points = g_new(GdkPoint,1);

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
		regen_loopuptable(engine->lookuppanel,engine);

		break;
	default:
		return FALSE;
	}
	return TRUE;
}

void UI_buildui(IBusHandwriteEngine * engine)
{
	//建立绘图窗口, 建立空点
	if (!engine->drawpanel)
	{
		engine->drawpanel = gtk_window_new(GTK_WINDOW_POPUP);

                gtk_widget_set_tooltip_markup(GTK_WIDGET(engine->drawpanel),
                                              _("<b>Hint:</b>\n"
                                                "Left mouse key to draw strokes.\n"
                                                "Holding right mouse key to move the widget.\n"
                                                ));


		gtk_window_set_position(GTK_WINDOW(engine->drawpanel),GTK_WIN_POS_MOUSE);

		GtkWidget * vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_box_set_homogeneous(GTK_BOX(vbox), FALSE);

		gtk_container_add(GTK_CONTAINER(engine->drawpanel),vbox);

		GtkWidget * drawing_area = gtk_drawing_area_new();

                g_signal_connect(G_OBJECT(drawing_area),"draw",G_CALLBACK(paint_lines),engine);

		gtk_box_pack_start(GTK_BOX(vbox),drawing_area,TRUE,TRUE,FALSE);

		gtk_widget_set_size_request(drawing_area, WIDTH, HEIGHT - PANELHEIGHT);

		engine->lookuppanel = gtk_grid_new();

		gtk_box_pack_end(GTK_BOX(vbox),engine->lookuppanel,FALSE,TRUE,FALSE);
		gtk_widget_set_size_request(engine->lookuppanel, WIDTH, PANELHEIGHT);

		gtk_widget_add_events(GTK_WIDGET(drawing_area),GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK| GDK_BUTTON_PRESS_MASK);


		g_signal_connect(G_OBJECT(engine->drawpanel),"realize",G_CALLBACK(widget_realize),engine);
		g_signal_connect(G_OBJECT(drawing_area),"motion_notify_event",G_CALLBACK(on_mouse_move),engine);
		g_signal_connect(G_OBJECT(drawing_area),"button-release-event",G_CALLBACK(on_button),engine);
		g_signal_connect(G_OBJECT(drawing_area),"button-press-event",G_CALLBACK(on_button),engine);
	}
	gtk_widget_show_all(engine->drawpanel);
}

void UI_show_ui(IBusHandwriteEngine * engine)
{
	printf("%s \n", __func__);
	if (engine->drawpanel)
	{
		gtk_widget_show_all(engine->drawpanel);
	}
}

void UI_hide_ui(IBusHandwriteEngine * engine)
{
	if (engine->drawpanel)
	{
		gtk_widget_hide(engine->drawpanel);
	}
}

void UI_cancelui(IBusHandwriteEngine* engine)
{
	// 撤销绘图窗口，销毁点列表
	if (engine->drawpanel)
		gtk_widget_destroy(engine->drawpanel);
	engine->drawpanel = NULL;
}

static void widget_realize(GtkWidget *widget, gpointer user_data)
{
	cairo_t * cr;
	cairo_surface_t * surface;
	cairo_region_t * region;
	GdkWindow * window;
	const int R = 5;
	gint width,height;

	window = gtk_widget_get_window(widget);

	gtk_widget_set_opacity(widget, 0.62);

	gtk_window_get_size(GTK_WINDOW(widget),&width,&height);

	surface = gdk_window_create_similar_surface
	    (window, CAIRO_CONTENT_ALPHA, width, height);
	cr = cairo_create(surface);

	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);

	cairo_move_to(cr, 0, R);
	cairo_arc(cr, R, R, R, M_PI, -M_PI/2);
	cairo_line_to(cr, WIDTH - R, 0);
	cairo_arc(cr, WIDTH - R, R, R, -M_PI/2, 0);
	cairo_line_to(cr, WIDTH, HEIGHT - R);
	cairo_arc(cr, WIDTH - R, HEIGHT - R, R, 0, M_PI/2);
	cairo_line_to(cr, R, HEIGHT);
	cairo_arc(cr, R, HEIGHT - R, R, M_PI/2, M_PI);
	cairo_close_path(cr);
	cairo_fill(cr);

	region = gdk_cairo_region_create_from_surface(surface);

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	/* gtk_widget_reset_shapes(widget); */
	gdk_window_shape_combine_region(window, region, 0, 0);
	gdk_window_input_shape_combine_region(window, region, 0, 0);

	cairo_region_destroy(region);
}
