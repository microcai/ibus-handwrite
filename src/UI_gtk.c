/*
 * UI_gtk.c -- provide UI via gtk
 *
 *  Created on: 2010-2-4
 *      Author: cai
 */


#include <gtk/gtk.h>

#include "engine.h"
#include "UI.h"

#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) gettext_noop (String)


static void widget_realize(GtkWidget *widget, gpointer user_data);

static gboolean paint_lines(GtkWidget *widget, GdkEventExpose *event,IBusHandwriteEngine * engine)
{
	GdkGC *gc;
	GdkWindow * window;
	GdkColormap * cmap;
	GtkStyle* style;

	LineStroke cl;
	int i;

	MatchedChar * matched;

	puts(__func__);


	style = gtk_style_copy(widget->style);

	style = gtk_style_attach(style,widget->window);

	gtk_paint_shadow(style,widget->window,GTK_STATE_ACTIVE,GTK_SHADOW_ETCHED_OUT,NULL,widget,NULL,0,0,200,200);

	gtk_style_detach(style);

	window = widget->window;

	cmap= gtk_widget_get_colormap(widget);
	gdk_colormap_alloc_color(cmap,engine->color,FALSE,TRUE);

	gc = gdk_gc_new(window);
	gdk_gc_set_line_attributes(gc,3,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gdk_gc_set_foreground(gc,engine->color);

	//已经录入的笔画

	for (i = 0; i < engine->engine->strokes->len ; i++ )
	{
		printf("drawing %d th line, total %d\n",i,engine->engine->strokes->len);
		cl =  g_array_index(engine->engine->strokes,LineStroke,i);
		gdk_draw_lines(window, gc, cl.points,cl.segments );
	}
	//当下笔画
	if ( engine->currentstroke.segments && engine->currentstroke.points )
		gdk_draw_lines(window, gc, engine->currentstroke.points,
				engine->currentstroke.segments);

	g_object_unref(gc);

	gdk_colormap_free_colors(cmap,engine->color,1);
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

		gtk_table_attach_defaults(GTK_TABLE(widget),bt,i%5,i%5+1,i/5,i/5+1);

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

	guint width,height;

	gtk_window_get_size(GTK_WINDOW(engine->drawpanel),&width,&height);


	ct = event->y < (height-50) ?  GDK_PENCIL:GDK_CENTER_PTR;

	if( event->state & (GDK_BUTTON2_MASK |GDK_BUTTON3_MASK ))
		ct = GDK_FLEUR;

	if(event->state & GDK_BUTTON2_MASK )
		ct = GDK_BOTTOM_RIGHT_CORNER;

	gdk_window_set_cursor(widget->window,gdk_cursor_new(ct));

	if (engine->mouse_state == GDK_BUTTON_PRESS) // 鼠标按下状态
	{
		gdk_window_set_cursor(widget->window,gdk_cursor_new(ct));

		engine->currentstroke.points
				= g_renew(GdkPoint,engine->currentstroke.points,engine->currentstroke.segments +1  );

		engine->currentstroke.points[engine->currentstroke.segments].x
				= event->x;
		engine->currentstroke.points[engine->currentstroke.segments].y
				= event->y;
		engine->currentstroke.segments++;
		printf("move, x= %lf, Y=%lf, segments = %d \n",event->x,event->y,engine->currentstroke.segments);

		gtk_widget_queue_draw(widget);
	    while (gtk_events_pending ())
	        gtk_main_iteration ();

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
	int i;
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

		GtkWidget * vbox = gtk_vbox_new(FALSE,0);

		gtk_container_add(GTK_CONTAINER(engine->drawpanel),vbox);

		GtkWidget * drawing_area = gtk_drawing_area_new();

                g_signal_connect(G_OBJECT(drawing_area),"expose-event",G_CALLBACK(paint_lines),engine);

		gtk_box_pack_start(GTK_BOX(vbox),drawing_area,TRUE,TRUE,FALSE);

		gtk_widget_set_size_request(drawing_area,200,200);

		engine->lookuppanel = gtk_table_new(2,5,TRUE);

		gtk_box_pack_end(GTK_BOX(vbox),engine->lookuppanel,FALSE,TRUE,FALSE);
		gtk_widget_set_size_request(engine->lookuppanel,200,50);

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
	GdkCursor* cursor;

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
		gtk_widget_hide_all(engine->drawpanel);
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
	GdkPixmap * pxmp;
	GdkGC * gc;
	GdkColor black, white;
	int R = 5;
	guint	width,height;

	//二值图像，白就是 1
	white.pixel = 1;
	black.pixel = 0;

	gtk_window_set_opacity(GTK_WINDOW(widget), 0.62);

	gtk_window_get_size(GTK_WINDOW(widget),&width,&height);

	pxmp = gdk_pixmap_new(NULL, width, height, 1);
	gc = gdk_gc_new(GDK_DRAWABLE(pxmp));

	gdk_gc_set_foreground(gc, &black);

	gdk_draw_rectangle(GDK_DRAWABLE(pxmp), gc, 1, 0, 0, width, height);

	gdk_gc_set_foreground(gc, &white);

	gdk_draw_arc(GDK_DRAWABLE(pxmp), gc, 1, 0, 0, R*2, R*2, 0, 360 * 64);
	gdk_draw_arc(GDK_DRAWABLE(pxmp), gc, 1, width - R*2, 0, R*2, R*2, 0, 360
			* 64);
	gdk_draw_arc(GDK_DRAWABLE(pxmp), gc, 1, width - R*2, height - R*2, R*2, R*2, 0,
			360 * 64);
	gdk_draw_arc(GDK_DRAWABLE(pxmp), gc, 1, 0, height - R*2, R*2, R*2, 0, 360
			* 64);
	gdk_draw_rectangle(GDK_DRAWABLE(pxmp), gc, 1, 0, R, width, height - R*2);
	gdk_draw_rectangle(GDK_DRAWABLE(pxmp), gc, 1, R, 0, width - R*2, height);

	g_object_unref(gc);

	gtk_widget_reset_shapes(widget);
	gtk_widget_shape_combine_mask(widget, pxmp, 0, 0);
	gtk_widget_input_shape_combine_mask(widget, pxmp, 0, 0);
	g_object_unref(pxmp);
}
