/*
 * UI_gtk.c -- provide UI via gtk
 *
 *  Created on: 2010-2-4
 *      Author: cai
 */


//#include <math.h>
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

    static void clicked(GtkGestureClick* click,gint n_press, gdouble x,gdouble y, IBusHandwriteEngine *engine)
		{
            printf("a button was clicked");
            GtkWidget* button = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(click));
			ibus_handwrite_engine_commit_text(engine,GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button),"order")));
		}

static void paint_lines(GtkDrawingArea *widget, cairo_t * cr,int width,int height,
        gpointer user_data)
        //IBusHandwriteEngine * engine)
{
    IBusHandwriteEngine* engine = IBUS_HANDWRITE_ENGINE(user_data); 

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

    printf("engine color: %f,%f,%f,%f\n", engine->color->red,engine->color->green,engine->color->blue, engine->color->alpha );

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
        gtk_widget_set_hexpand(bt,TRUE);
        gtk_widget_set_valign(bt,GTK_ALIGN_END);

		gtk_grid_attach(GTK_GRID(widget), bt, i%5, i/5, 1, 1);

		gtk_widget_set_visible(bt,TRUE);


		g_object_set_data(G_OBJECT(bt),"order",GINT_TO_POINTER(i));
        GtkGesture* gesture = gtk_gesture_click_new();
        g_signal_connect(gesture, "pressed", G_CALLBACK(clicked),engine);
        gtk_widget_add_controller(bt,GTK_EVENT_CONTROLLER(gesture));

		gtk_widget_set_visible(bt,TRUE);
	}
}


static void on_drag(GtkGestureDrag* gesture,gdouble start_x,gdouble start_y,IBusHandwriteEngine* engine)
{
    printf("drag\n");

	gint width,height;

	GtkWindow * window;

	width=gtk_widget_get_width(engine->drawpanel);
    height=gtk_widget_get_height(engine->drawpanel);

    GdkEventSequence* sequence = gtk_gesture_get_last_updated_sequence(GTK_GESTURE(gesture));
    GdkEvent* event = gtk_gesture_get_last_event(GTK_GESTURE(gesture),sequence);
    GdkModifierType mods = gdk_event_get_modifier_state(event);


    double event_x;
    double event_y;
    gdk_event_get_position( event, &event_x,&event_y);
	if(event_y < (height - PANELHEIGHT)){gtk_widget_set_cursor_from_name(engine->drawpanel,"pencil");
    }else{
        gtk_widget_set_cursor_from_name(engine->drawpanel,"center_ptr");
    }

	if(mods & GDK_BUTTON2_MASK )
		gtk_widget_set_cursor_from_name(engine->drawpanel,"sw-resize");

    
	if( mods & GDK_BUTTON2_MASK | GDK_BUTTON3_MASK )
	    gtk_widget_set_cursor_from_name(engine->drawpanel,"all-resize");

    GtkRoot *root = gtk_widget_get_root(engine->drawpanel);
	window = GTK_WINDOW(root);


    printf("mods %x", mods);

    if((mods & (GDK_BUTTON2_MASK | GDK_BUTTON3_MASK))){
        GdkDevice*device = gdk_event_get_device(event);
        GdkSurface* surface = gtk_native_get_surface(
                gtk_widget_get_native(GTK_WIDGET(engine->drawpanel)));
        GdkToplevel* toplevel = GDK_TOPLEVEL(surface);
        guint32 time = gdk_event_get_time(event);

        printf("toplevel %lx, device %lx,event_x %f, event_y %f, time %i \n",
                (long)toplevel,(long)device,event_x,event_y,time);
        if(mods == GDK_BUTTON2_MASK)
	    {
            gdk_toplevel_begin_resize(toplevel,GDK_SURFACE_EDGE_SOUTH_EAST,device,2,event_x,event_y,time);

	    }else if( mods == (GDK_BUTTON3_MASK ))
	    {
            gdk_toplevel_begin_move(toplevel, device,3,event_x,event_y,time);
        }

    }else // 鼠标按下状态
	{
        
		engine->currentstroke.points
				= g_renew(graphene_point_t,engine->currentstroke.points,engine->currentstroke.segments +1  );
       

                
		engine->currentstroke.points[engine->currentstroke.segments].x = event_x;
        engine->currentstroke.points[engine->currentstroke.segments].y = event_y;

        
		engine->currentstroke.segments++;
		printf("move, x= %lf, Y=%lf, segments = %d \n",event_x,event_y,engine->currentstroke.segments);
       	gtk_widget_queue_draw(engine->drawarea);


	}
    
	
}
static void on_drag_begin(GtkGestureDrag* gesture,gdouble start_x,gdouble start_y,IBusHandwriteEngine* engine)
{

    GdkEventSequence* sequence = gtk_gesture_get_last_updated_sequence(GTK_GESTURE(gesture));
    GdkEvent* event = gtk_gesture_get_last_event(GTK_GESTURE(gesture),sequence);
    GdkModifierType mods = gdk_event_get_modifier_state(event);

    double event_x, event_y;
    gdk_event_get_position(event,&event_x,&event_y);

		g_print("mouse clicked\n");

		engine->currentstroke.segments = 1;

		engine->currentstroke.points = g_new(graphene_point_t,1);

		engine->currentstroke.points[0].x = event_x;
		engine->currentstroke.points[0].y = event_y;

    }
static void on_drag_end(GtkGestureDrag* gesture,gdouble start_x,gdouble start_y,IBusHandwriteEngine* engine){

		ibus_handwrite_recog_append_stroke(engine->engine,engine->currentstroke);

		engine->currentstroke.segments = 0;
		g_free(engine->currentstroke.points);

		engine->currentstroke.points = NULL;

		ibus_handwrite_recog_domatch(engine->engine,10);

		g_print("mouse released\n");

		gtk_widget_queue_draw(engine->drawarea);
		regen_lookuptable(GTK_GRID(engine->lookuppanel),engine);

}


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

		engine->drawarea = gtk_drawing_area_new();
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(engine->drawarea),paint_lines,engine,NULL);
        gtk_widget_set_vexpand(engine->drawarea,TRUE);


		gtk_box_prepend(GTK_BOX(vbox),engine->drawarea);

		gtk_widget_set_size_request(engine->drawarea, WIDTH, HEIGHT - PANELHEIGHT);

		engine->lookuppanel = gtk_grid_new();

		gtk_box_append(GTK_BOX(vbox),engine->lookuppanel);
		gtk_widget_set_size_request(engine->lookuppanel, WIDTH, PANELHEIGHT);
        gtk_widget_set_vexpand(engine->lookuppanel,FALSE);
        gtk_widget_set_valign(engine->lookuppanel,GTK_ALIGN_END);


        //drag for drawarea
        gesture = gtk_gesture_drag_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),0);
        g_signal_connect(gesture,"drag-begin",G_CALLBACK(on_drag),engine);
		g_signal_connect(gesture,"drag-update",G_CALLBACK(on_drag),engine);
        g_signal_connect(gesture,"drag-end", G_CALLBACK(on_drag_end),engine);
        gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture),GTK_PHASE_BUBBLE);
        gtk_widget_add_controller(engine->drawarea, GTK_EVENT_CONTROLLER(gesture));

        g_print("widget built\n");

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
