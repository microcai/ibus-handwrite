/* vim:set et sts=4: */
#include <config.h>
#include <unistd.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <ibus.h>
#include <locale.h>
#include <gtk/gtkgl.h>

#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#include "engine.h"
#include "global_var.h"


IBusBus *bus = NULL;
static IBusFactory *factory = NULL;

char *tablefile=  TABLEFILE ;
char icondir[4096]= PKGDATADIR"/icons";
char lang[20]	= "zh_CN";

int main(int argc, char* argv[])
{
	IBusComponent *component;
	IBusEngineDesc * desc;

	GError * err = NULL;

	gboolean have_ibus=FALSE;

	const gchar * language="zh";

	const gchar * icon_dir = NULL;

	const gchar * locale_dir = NULL;


	setlocale(LC_ALL, "");
	gtk_set_locale();
	textdomain(GETTEXT_PACKAGE);

	GOptionEntry args[] =
	{
			{"ibus",'\0',0,G_OPTION_ARG_NONE,&have_ibus},
			{"icondir",'\0',0,G_OPTION_ARG_STRING,&icon_dir,_("the icon file"),N_("icon file")},
			{"table",'\0',0,G_OPTION_ARG_STRING,&tablefile,_("set table file path"),N_("tablefile")},
			{"locale",'\0',0,G_OPTION_ARG_STRING,&locale_dir,_("set locale path"),N_("locale")},
#ifdef WITH_ZINNIA
			{"lang",'\0',0,G_OPTION_ARG_STRING,&language,_("set languate, accept zh and jp"),N_("lang")},
#endif
			{0}
	};

	gtk_init_with_args(&argc,&argv,PACKAGE_NAME,args,PACKAGE_NAME,&err);


	if(G_UNLIKELY(!gtk_gl_init_check(&argc, &argv)))
	{
		g_warning("GLX extension not available, use slow soft cairo rendering instead");
	}

#ifdef WITH_ZINNIA
	if(strcmp(language,"zh")==0 ||strcmp(language,"zh_CN") ==0 )
	{

	}else if( strcmp(language,"jp") ==0 || strcmp(language,"ja")==0 )
	{
		g_strlcpy(lang,"ja",20);
	}else
	{
		g_error("pass jp or zh to --lang!");
	}

#endif

	if(locale_dir)
	{
		bindtextdomain(GETTEXT_PACKAGE,locale_dir);
	}

	gchar * engine_name = g_strdup_printf("handwrite-%s",lang);

	gchar * dbus_name = g_strdup_printf("org.freedesktop.IBus.handwrite-%s",lang);

	ibus_init();

	if(icon_dir)
		realpath(icon_dir, icondir);

	bus = ibus_bus_new();

	g_signal_connect (bus, "disconnected", G_CALLBACK (gtk_main_quit), NULL);

	factory = ibus_factory_new(ibus_bus_get_connection(bus));


	ibus_bus_request_name(bus, dbus_name, 0);

//	g_free(dbus_name);

	if (!have_ibus)
	{
		char * exefile ;

		exefile = realpath(argv[0],NULL);

		component = ibus_component_new(dbus_name,
				"handwrite", PACKAGE_VERSION, "GPL", MICROCAI_WITHEMAIL, PACKAGE_BUGREPORT,
				exefile, GETTEXT_PACKAGE);

		gchar * iconfile =  g_strdup_printf("%s/ibus-handwrite.svg",icondir);

		desc = ibus_engine_desc_new(engine_name, "handwrite",
				_("hand write recognizer"), lang, "GPL",
				MICROCAI_WITHEMAIL, iconfile, "us");

		ibus_component_add_engine(component, desc);

		free(exefile);
		g_free(iconfile);

	}else
	{
		component = ibus_component_new(dbus_name,
				"handwrite", PACKAGE_VERSION, "GPL", MICROCAI_WITHEMAIL, PACKAGE_BUGREPORT,
				PKGDATADIR, GETTEXT_PACKAGE);
	}

	g_free(dbus_name);


	ibus_bus_register_component(bus, component);


	ibus_factory_add_engine(factory, engine_name, IBUS_TYPE_HANDWRITE_ENGINE);

	g_free(engine_name);

	g_object_unref(component);

	GdkScreen * screen = gdk_screen_get_default();
	GdkColormap * map = gdk_screen_get_rgba_colormap(screen);
	if(map)	gtk_widget_set_default_colormap(map);

	printf(_("ibus-handwrite Version %s Start Up\n"), PACKAGE_VERSION);

	gtk_main();
	return 0;
}
